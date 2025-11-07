/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RasterPathAtlas.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu::graphite {

static constexpr uint32_t kDefaultAtlasDim = 2048;

static constexpr uint32_t kSmallPathPlotWidth = 512;
static constexpr uint32_t kSmallPathPlotHeight = 256;

static constexpr uint32_t kUncachedAtlasDim = 2048;

// When using the default path renderer strategy AND opting into the small path atlas AND there is
// support for MSAA, then the RasterPathAtlas will operate in a reduced mode that only has a single
// texture page for non-volatile small paths, and will not fall back to the ProxyCache (as it
// assumes Device will fallback to MSAA tessellation instead)
static bool is_small_path_atlas_only(const Caps* caps) {
    return caps->requestedPathRendererStrategy() == PathRendererStrategy::kDefault &&
           caps->minPathSizeForMSAA() > 0.f &&
           caps->defaultMSAASamplesCount() > 1;
}

RasterPathAtlas::AtlasConfig RasterPathAtlas::GetDefaultAtlasConfig(uint32_t size, const Caps* caps) {
    if (is_small_path_atlas_only(caps)) {
        // MSAA will be the preferred way to render paths, barring small paths. These will go to
        // the small path atlas config, so configure this to be an empty atlas manager.
        return {/*width=*/0, /*height=*/0, /*plotWidth=*/0, /*plotHeight=*/0,
                DrawAtlas::AllowMultitexturing::kNo};
    } else {
        // To preserve legacy behavior, we configure the main caching atlas the same regardless of
        // whether or not we expect MSAA to be supported.
        // TODO(michaelludwig): After the small path atlas experiment is completed, update the
        // cache config to always create an empty atlas when MSAA is supported since only the
        // small path atlas should be used in that scenario.
        return {/*width=*/size, /*height=*/size, /*plotWidth=*/size / 2, /*plotHeight=*/size / 2,
                caps->allowMultipleAtlasTextures() ? DrawAtlas::AllowMultitexturing::kYes
                                                   : DrawAtlas::AllowMultitexturing::kNo};
    }
}

RasterPathAtlas::AtlasConfig RasterPathAtlas::GetSmallPathAtlasConfig(const Caps* caps) {
    if (is_small_path_atlas_only(caps)) {
        // The intent of the small path atlas when MSAA is available is to avoid triggering MSAA for
        // render passes when there are only a few small paths (e.g. icons) that occupy a relatively
        // little part of the render pass area. As such, we configure the atlas to hold 16 maximally
        // sized "small" paths per plot, and to have 16 total plots. For a size of 32, this works
        // out to a 512x512 texture.
        //
        // We also explicitly disable multitexturing in this case because the fallback is just to
        // render with MSAA instead of flushing the recorder.
        const uint32_t plotDim = 4 * SkNextPow2(sk_float_ceil2int(caps->minPathSizeForMSAA()));
        const uint32_t atlasDim = 4 * plotDim;
        return {/*width=*/atlasDim, /*height=*/atlasDim,
                /*plotWidth=*/plotDim, /*plotHeight=*/plotDim,
                DrawAtlas::AllowMultitexturing::kNo};
    } else {
        // To preserve legacy behavior, we configure the main caching atlas the same regardless of
        // whether or not we expect MSAA to be supported.
        // TODO(michaelludwig): After the small path atlas experiment is completed, update the
        // cache config to always create an empty atlas when MSAA is supported since only the
        // small path atlas should be used in that scenario.
        return {/*width=*/kDefaultAtlasDim, /*height=*/kDefaultAtlasDim,
                /*plotWidth=*/kSmallPathPlotWidth, /*plotHeight=*/kSmallPathPlotHeight,
                caps->allowMultipleAtlasTextures() ? DrawAtlas::AllowMultitexturing::kYes
                                                   : DrawAtlas::AllowMultitexturing::kNo};
    }
}

RasterPathAtlas::RasterPathAtlas(Recorder* recorder)
        : PathAtlas(recorder, kDefaultAtlasDim, kDefaultAtlasDim)
        , fCachedAtlasMgr(GetDefaultAtlasConfig(kDefaultAtlasDim, recorder->priv().caps()),
                          fWidth, fHeight, /*label=*/"RasterPathAtlas[cached]")
        , fSmallPathAtlasMgr(GetSmallPathAtlasConfig(recorder->priv().caps()),
                             fWidth, fHeight, /*label=*/"RasterPathAtlas[small]")
        , fUncachedAtlasMgr(GetDefaultAtlasConfig(kUncachedAtlasDim, recorder->priv().caps()),
                            fWidth, fHeight, /*label=*/"RasterPathAtlas[uncached]") {
    if (is_small_path_atlas_only(recorder->priv().caps())) {
        fMaxSmallPathSize = recorder->priv().caps()->minPathSizeForMSAA() + 2.f; // Add padding
        fUseProxyCacheFallback = false;
    } else {
        fMaxSmallPathSize = 162.f; // Historic value
        fUseProxyCacheFallback = true;
    }
}

static bool draw_path_to_pixmap(const Shape& shape,
                                const Transform& localToDevice,
                                const SkStrokeRec& strokeRec,
                                SkIRect shapeBounds,
                                SkIVector transformedMaskOffset,
                                SkISize pixmapSize,
                                SkAutoPixmapStorage* dst) {
    RasterMaskHelper helper(dst);
    if (!helper.init(pixmapSize, transformedMaskOffset)) {
        return false;
    }
    helper.drawShape(shape, localToDevice, strokeRec, shapeBounds);

    return true;
}

sk_sp<TextureProxy> RasterPathAtlas::onAddShape(const Shape& shape,
                                                const Transform& localToDevice,
                                                const SkStrokeRec& strokeRec,
                                                skvx::half2 maskOrigin,
                                                skvx::half2 maskSize,
                                                SkIVector transformedMaskOffset,
                                                skvx::half2* outPos) {
    sk_sp<TextureProxy> proxy;

    if (!fUseProxyCacheFallback) {
        // AtlasLocator loc;
        // if (maskSize.x() > fMaxSmallPathSize || maskSize.y() > fMaxSmallPathSize) {
        //     SkDebugf("why are we trying this? %d x %d vs %f\n",
        //         maskSize.x(),  maskSize.y(), fMaxSmallPathSize);
        // }
        return fSmallPathAtlasMgr.findOrCreateEntry(fRecorder, shape, localToDevice, strokeRec, maskOrigin, maskSize, transformedMaskOffset, outPos);
    }

    if (!shape.isVolatilePath()) {
        // Try to locate or add to cached DrawAtlas
        if (maskSize.x() <= fMaxSmallPathSize && maskSize.y() <= fMaxSmallPathSize) {
            proxy = fSmallPathAtlasMgr.findOrCreateEntry(fRecorder,
                                                         shape,
                                                         localToDevice,
                                                         strokeRec,
                                                         maskOrigin,
                                                         maskSize,
                                                         transformedMaskOffset,
                                                         outPos);
        } else {
            // SkDebugf("rejecting path for small atlas... %d x %d vs %f\n",
            //         maskSize.x(), maskSize.y(), fMaxSmallPathSize);
        }
        if (!proxy) {
            proxy = fCachedAtlasMgr.findOrCreateEntry(fRecorder,
                                                      shape,
                                                      localToDevice,
                                                      strokeRec,
                                                      maskOrigin,
                                                      maskSize,
                                                      transformedMaskOffset,
                                                      outPos);
        }
    }

    // Try to add to uncached DrawAtlas
    if (!proxy) {
        AtlasLocator loc;
        proxy = fUncachedAtlasMgr.addToAtlas(fRecorder,
                                             shape,
                                             localToDevice,
                                             strokeRec,
                                             maskSize,
                                             transformedMaskOffset,
                                             outPos,
                                             &loc);
    }
    if (proxy || !fUseProxyCacheFallback) {
        return proxy;
    }

    // Failed to add to atlases, try to add to ProxyCache
    // TODO(michaelludwig): Move this fallback to Device::drawGeometry() so it can control switching
    // to an MSAA fallback (e.g. if just the small path atlas is available), and so that compute
    // atlases can share this SW fallback if they fail to create an e
    skgpu::UniqueKey maskKey = GeneratePathMaskKey(shape, localToDevice, strokeRec,
                                                   maskOrigin, maskSize);
    struct PathDrawContext {
        const Shape& fShape;
        const Transform& fLocalToDevice;
        const SkStrokeRec& fStrokeRec;
        SkIRect fShapeBounds;
        SkIVector fTransformedMaskOffset;
    } context = { shape, localToDevice, strokeRec,
                  SkIRect::MakeSize({maskSize.x(), maskSize.y()}).makeOffset(kEntryPadding,
                                                                             kEntryPadding),
                  transformedMaskOffset };
    sk_sp<TextureProxy> cachedProxy = fRecorder->priv().proxyCache()->findOrCreateCachedProxy(
            fRecorder, maskKey, &context,
            [](const void* ctx) {
                const PathDrawContext* pdc = static_cast<const PathDrawContext*>(ctx);
                SkAutoPixmapStorage dst;
                SkISize pixmapSize = pdc->fShapeBounds.size();
                pixmapSize.fWidth += 2*kEntryPadding;
                pixmapSize.fHeight += 2*kEntryPadding;
                draw_path_to_pixmap(pdc->fShape, pdc->fLocalToDevice, pdc->fStrokeRec,
                                    pdc->fShapeBounds, pdc->fTransformedMaskOffset,
                                    pixmapSize, &dst);
                SkBitmap bm;
                // The bitmap needs to take ownership of the pixels, so we detach them from the
                // SkAutoPixmapStorage and pass them to SkBitmap::installPixels().
                SkImageInfo ii = dst.info();
                size_t rowBytes = dst.rowBytes();
                SkAssertResult(bm.installPixels(ii, dst.detachPixels(), rowBytes,
                                                [](void* addr, void* context) { sk_free(addr); },
                                                nullptr));
                bm.setImmutable();
                return bm;
            });

    *outPos = { kEntryPadding, kEntryPadding };
    return cachedProxy;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool RasterPathAtlas::RasterAtlasMgr::onAddToAtlas(const Shape& shape,
                                                   const Transform& localToDevice,
                                                   const SkStrokeRec& strokeRec,
                                                   SkIRect shapeBounds,
                                                   SkIVector transformedMaskOffset,
                                                   const AtlasLocator& locator) {
    // Rasterize path to backing pixmap.
    // This pixmap will be the size of the Plot that contains the given rect, not the entire atlas,
    // and hence the position we render at will be relative to that Plot.
    // The value of outPos is relative to the entire texture, to be used for texture coords.
    SkAutoPixmapStorage dst;
    SkIPoint renderPos = fDrawAtlas->prepForRender(locator, &dst);

    // Offset to plot location and draw
    shapeBounds.offset(renderPos.x()+kEntryPadding, renderPos.y()+kEntryPadding);
    return draw_path_to_pixmap(shape, localToDevice, strokeRec, shapeBounds,
                               transformedMaskOffset, fDrawAtlas->plotSize(), &dst);
}

}  // namespace skgpu::graphite
