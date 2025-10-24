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

RasterPathAtlas::RasterPathAtlas(Recorder* recorder)
        : PathAtlas(recorder, kDefaultAtlasDim, kDefaultAtlasDim)
        , fCachedAtlasMgr(fWidth, fHeight,
                          /*plotWidth=*/fWidth/2, /*plotHeight=*/fHeight/2,
                          recorder->priv().caps())
        , fSmallPathAtlasMgr(std::max(fWidth, kSmallPathPlotWidth),
                             std::max(fHeight, kSmallPathPlotHeight),
                             kSmallPathPlotWidth, kSmallPathPlotHeight,
                             recorder->priv().caps())
        , fUncachedAtlasMgr(kUncachedAtlasDim, kUncachedAtlasDim,
                            /*plotWidth=*/kUncachedAtlasDim/2, /*plotHeight=*/kUncachedAtlasDim/2,
                            recorder->priv().caps()) {
    SkASSERT(recorder);
}

void RasterPathAtlas::recordUploads(DrawContext* dc) {
    fCachedAtlasMgr.recordUploads(dc, fRecorder);
    fSmallPathAtlasMgr.recordUploads(dc, fRecorder);
    fUncachedAtlasMgr.recordUploads(dc, fRecorder);
}

sk_sp<TextureProxy> RasterPathAtlas::onAddShape(const Shape& shape,
                                                const Transform& localToDevice,
                                                const SkStrokeRec& strokeRec,
                                                skvx::half2 maskOrigin,
                                                skvx::half2 maskSize,
                                                SkIVector transformedMaskOffset,
                                                skvx::half2* outPos) {
    sk_sp<TextureProxy> proxy;

    if (!shape.isVolatilePath()) {
        constexpr int kMaxSmallPathSize = 162;
        // Try to locate or add to cached DrawAtlas
        if (maskSize.x() <= kMaxSmallPathSize && maskSize.y() <= kMaxSmallPathSize) {
            proxy = fSmallPathAtlasMgr.findOrCreateEntry(fRecorder,
                                                         shape,
                                                         localToDevice,
                                                         strokeRec,
                                                         maskOrigin,
                                                         maskSize,
                                                         transformedMaskOffset,
                                                         outPos);
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
    if (proxy) {
        return proxy;
    }

    // Failed to add to atlases, try to add to ProxyCache
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
                auto [pm, helper] = RasterMaskHelper::Allocate(pdc->fShapeBounds.size(),
                                                               kEntryPadding,
                                                               pdc->fTransformedMaskOffset);
                helper.drawShape(pdc->fShape, pdc->fLocalToDevice, pdc->fStrokeRec);
                SkBitmap bm;
                // The bitmap needs to take ownership of the pixels, so we detach them from the
                // SkAutoPixmapStorage and pass them to SkBitmap::installPixels().
                SkImageInfo ii = pm.info();
                size_t rowBytes = pm.rowBytes();
                SkAssertResult(bm.installPixels(
                        ii,
                        pm.detachPixels(),
                        rowBytes,
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
    // Rasterize path to pixmap backing the locator, inset by our padding.
    SkPixmap locatorPM = fDrawAtlas->prepForRender(locator);
    SkIRect insetBounds =
            SkIRect::MakeSize(locatorPM.dimensions()).makeInset(kEntryPadding, kEntryPadding);

    SkPixmap insetPixmap;
    SkAssertResult(locatorPM.extractSubset(&insetPixmap, insetBounds));

    RasterMaskHelper helper(insetPixmap, transformedMaskOffset);
    helper.drawShape(shape, localToDevice, strokeRec);

    return true;
}

}  // namespace skgpu::graphite
