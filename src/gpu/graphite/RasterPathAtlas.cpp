/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RasterPathAtlas.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {
namespace {

// TODO: select atlas size dynamically? Take ContextOptions::fMaxTextureAtlasSize into account?

// TODO: for now
constexpr uint16_t kAtlasDim = 4096;

}  // namespace

RasterPathAtlas::RasterPathAtlas()
    : PathAtlas(kAtlasDim, kAtlasDim)
    , fPageArray(kMaxPages) {}

void RasterPathAtlas::recordUploads(DrawContext* dc, Recorder* recorder) {
    // build an upload for the dirty rect and record it
    if (!fDirtyRect.isEmpty()) {
        std::vector<MipLevel> levels;
        levels.push_back({fPixels.addr(), fPixels.rowBytes()});

        SkColorInfo colorInfo(kAlpha_8_SkColorType, kUnknown_SkAlphaType, nullptr);

 //       SkASSERT(this->texture());
        if (!dc->recordUpload(recorder, sk_ref_sp(this->texture()), colorInfo, colorInfo, levels,
                              fDirtyRect, nullptr)) {
            SKGPU_LOG_W("Coverage mask upload failed!");
            return;
        }

        // TODO: Keep using this texture until full and cache the results, then get a new one.
    }
}

/*
bool RasterPathAtlas::initializeTextureIfNeeded(Recorder* recorder) {
    if (!fTexture) {
        AtlasProvider* atlasProvider = recorder->priv().atlasProvider();
        fTexture = atlasProvider->getAtlasTexture(recorder,
                                                  this->width(),
                                                  this->height(),
                                                  kAlpha_8_SkColorType,
                                                  /*requiresStorageUsage=*/false);
    }
    return fTexture != nullptr;
}
*/
const TextureProxy* RasterPathAtlas::addRect(Recorder* recorder,
                                             skvx::float2 atlasSize,
                                             SkIPoint16* pos) {
    Page* mru = fPageList.head();
    if (!mru) {
        fPageList.addToHead(&fPageArray.push_back());
    }
    if (!mru->fTexture) {
        const MaskFormat maskFormat = this->coverageMaskFormat(recorder->priv().caps());
        mru->fTexture =
                recorder->priv().atlasProvider()->getAtlasTexture(recorder,
                                                                  this->width(),
                                                                  this->height(),
                                                                  maskFormat.fColorType,
                                                                  maskFormat.requiresStorageUsage);
    }
    if (!mru->fTexture) {
        SKGPU_LOG_E("Failed to instantiate an atlas texture");
        return nullptr;
    }

    if (!mru->fRectanizer.addRect(atlasSize.x(), atlasSize.y(), pos)) {
        return nullptr;
    }

    return mru->fTexture.get();
}

void RasterPathAtlas::onAddShape(const Shape& shape,
                                 const Transform& transform,
                                 const Rect& atlasBounds,
                                 skvx::int2 deviceOffset,
                                 const SkStrokeRec& strokeRec) {
    // TODO: look up shape and use cached texture
    // Need to push this up into addShape() somehow

    // allocate pixmap if needed
    if (!fPixels.addr()) {
        const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(kAtlasDim, kAtlasDim);
        if (!fPixels.tryAlloc(bmImageInfo)) {
            return;
        }
        fPixels.erase(0);
    }

    // Rasterize path to backing pixmap
    // TODO: render in a separate thread?
    SkDrawBase draw;
    draw.fBlitterChooser = SkA8Blitter_Choose;
    draw.fDst      = fPixels;
    SkRasterClip rasterClip;
    SkIRect iAtlasBounds = atlasBounds.asSkIRect();
    rasterClip.setRect(iAtlasBounds);
    draw.fRC       = &rasterClip;

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);  // "Replace" mode
    paint.setAntiAlias(true);
    // SkPaint's color is unpremul so this will produce alpha in every channel.
    paint.setColor(SK_ColorWHITE);
    strokeRec.applyToPaint(&paint);

    SkMatrix translatedMatrix = SkMatrix(transform);
    // The atlas transform of the shape is the linear-components (scale, rotation, skew) of
    // `localToDevice` translated by the top-left offset of `atlasBounds`, accounting for the 1
    // pixel-wide border we added earlier, so that the shape is correctly centered.
    translatedMatrix.postTranslate(atlasBounds.x() + 1 - deviceOffset.x(),
                                   atlasBounds.y() + 1 - deviceOffset.y());
    draw.fCTM = &translatedMatrix;
    SkPath path = shape.asPath();
    if (path.isInverseFillType()) {
        // The shader will handle the inverse fill in this case
        path.toggleInverseFillType();
    }
    draw.drawPathCoverage(path, paint);

    // Add atlasBounds to dirtyRect for later upload
    fDirtyRect.join(iAtlasBounds);

    // TODO: cache shape data and texture used
/*
 bool useCache = fAllowCaching && !inverseFilled && args.fViewMatrix->preservesAxisAlignment() &&
                 args.fShape->hasUnstyledKey() && (GrAAType::kCoverage == args.fAAType);

 if (useCache) {
     // Use the cache only if >50% of the path is visible.
     int unclippedWidth = unclippedDevShapeBounds.width();
     int unclippedHeight = unclippedDevShapeBounds.height();
     int64_t unclippedArea = sk_64_mul(unclippedWidth, unclippedHeight);
     int64_t clippedArea = sk_64_mul(clippedDevShapeBounds.width(),
                                     clippedDevShapeBounds.height());
     int maxTextureSize = args.fSurfaceDrawContext->caps()->maxTextureSize();
     if (unclippedArea > 2 * clippedArea || unclippedWidth > maxTextureSize ||
         unclippedHeight > maxTextureSize) {
         useCache = false;
     } else {
         boundsForMask = &unclippedDevShapeBounds;
     }
 }

 skgpu::UniqueKey maskKey;
 if (useCache) {
    // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
     SkScalar sx = args.fViewMatrix->get(SkMatrix::kMScaleX);
     SkScalar sy = args.fViewMatrix->get(SkMatrix::kMScaleY);
     SkScalar kx = args.fViewMatrix->get(SkMatrix::kMSkewX);
     SkScalar ky = args.fViewMatrix->get(SkMatrix::kMSkewY);
     static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
     skgpu::UniqueKey::Builder builder(&maskKey, kDomain, 7 + args.fShape->unstyledKeySize(),
                                  "SW Path Mask");
     builder[0] = boundsForMask->width();
     builder[1] = boundsForMask->height();
 #ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
     // Fractional translate does not affect caching on Android. This is done for better cache
     // hit ratio and speed, but it is matching HWUI behavior, which doesn't consider the matrix
     // at all when caching paths.
     SkFixed fracX = 0;
     SkFixed fracY = 0;
 #else
     SkScalar tx = args.fViewMatrix->get(SkMatrix::kMTransX);
     SkScalar ty = args.fViewMatrix->get(SkMatrix::kMTransY);
     // Allow 8 bits each in x and y of subpixel positioning.
     SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
     SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;
 #endif
     builder[2] = SkFloat2Bits(sx);
     builder[3] = SkFloat2Bits(sy);
     builder[4] = SkFloat2Bits(kx);
     builder[5] = SkFloat2Bits(ky);
     // Distinguish between hairline and filled paths. For hairlines, we also need to include
     // the cap. (SW grows hairlines by 0.5 pixel with round and square caps). Note that
     // stroke-and-fill of hairlines is turned into pure fill by SkStrokeRec, so this covers
     // all cases we might see.
     uint32_t styleBits = args.fShape->style().isSimpleHairline() ?
                          ((args.fShape->style().strokeRec().getCap() << 1) | 1) : 0;
     builder[6] = fracX | (fracY >> 8) | (styleBits << 16);
     args.fShape->writeUnstyledKey(&builder[7]);
 }
*/
}

void RasterPathAtlas::reset() {
    Page* mru = fPageList.head();
    if (mru) {
        mru->fRectanizer.reset();
    }

    // clear backing data for next pass
    fDirtyRect.setEmpty();
    fPixels.erase(0);
}

}  // namespace skgpu::graphite
