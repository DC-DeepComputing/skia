/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/TiledTextureUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkDevice.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkSamplingPriv.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/image/SkImage_Base.h"

#if GR_TEST_UTILS
extern int gOverrideMaxTextureSize;
extern std::atomic<int>  gNumTilesDrawn;
#endif

//////////////////////////////////////////////////////////////////////////////
//  Helper functions for tiling a large SkBitmap

namespace {

// This method outsets 'iRect' by 'outset' all around and then clamps its extents to
// 'clamp'. 'offset' is adjusted to remain positioned over the top-left corner
// of 'iRect' for all possible outsets/clamps.
void clamped_outset_with_offset(SkIRect* iRect, int outset, SkPoint* offset,
                                const SkIRect& clamp) {
    iRect->outset(outset, outset);

    int leftClampDelta = clamp.fLeft - iRect->fLeft;
    if (leftClampDelta > 0) {
        offset->fX -= outset - leftClampDelta;
        iRect->fLeft = clamp.fLeft;
    } else {
        offset->fX -= outset;
    }

    int topClampDelta = clamp.fTop - iRect->fTop;
    if (topClampDelta > 0) {
        offset->fY -= outset - topClampDelta;
        iRect->fTop = clamp.fTop;
    } else {
        offset->fY -= outset;
    }

    if (iRect->fRight > clamp.fRight) {
        iRect->fRight = clamp.fRight;
    }
    if (iRect->fBottom > clamp.fBottom) {
        iRect->fBottom = clamp.fBottom;
    }
}

} // anonymous namespace

namespace skgpu {

void TiledTextureUtils::DrawTiledBitmap_Graphite(SkCanvas* canvas,
                                                 const SkBitmap& bitmap,
                                                 int tileSize,
                                                 const SkMatrix& srcToDst,
                                                 const SkRect& srcRect,
                                                 const SkIRect& clippedSrcIRect,
                                                 const SkPaint* paint,
                                                 SkCanvas::QuadAAFlags origAAFlags,
                                                 SkCanvas::SrcRectConstraint constraint,
                                                 SkSamplingOptions sampling) {
    if (sampling.isAniso()) {
        sampling = SkSamplingPriv::AnisoFallback(/* imageIsMipped= */ false);
    }
    SkRect clippedSrcRect = SkRect::Make(clippedSrcIRect);

    int nx = bitmap.width() / tileSize;
    int ny = bitmap.height() / tileSize;

#if GR_TEST_UTILS
    gNumTilesDrawn.store(0, std::memory_order_relaxed);
#endif

    skia_private::TArray<SkCanvas::ImageSetEntry> imgSet(nx * ny);

    for (int x = 0; x <= nx; x++) {
        for (int y = 0; y <= ny; y++) {
            SkRect tileR;
            tileR.setLTRB(SkIntToScalar(x * tileSize),       SkIntToScalar(y * tileSize),
                          SkIntToScalar((x + 1) * tileSize), SkIntToScalar((y + 1) * tileSize));

            if (!SkRect::Intersects(tileR, clippedSrcRect)) {
                continue;
            }

            if (!tileR.intersect(srcRect)) {
                continue;
            }

            SkIRect iTileR;
            tileR.roundOut(&iTileR);
            SkVector offset = SkPoint::Make(SkIntToScalar(iTileR.fLeft),
                                            SkIntToScalar(iTileR.fTop));
            SkRect rectToDraw = tileR;
            if (!srcToDst.mapRect(&rectToDraw)) {
                continue;
            }

            if (sampling.filter != SkFilterMode::kNearest || sampling.useCubic) {
                SkIRect iClampRect;

                if (SkCanvas::kFast_SrcRectConstraint == constraint) {
                    // In bleed mode we want to always expand the tile on all edges
                    // but stay within the bitmap bounds
                    iClampRect = SkIRect::MakeWH(bitmap.width(), bitmap.height());
                } else {
                    // In texture-domain/clamp mode we only want to expand the
                    // tile on edges interior to "srcRect" (i.e., we want to
                    // not bleed across the original clamped edges)
                    srcRect.roundOut(&iClampRect);
                }
                int outset = sampling.useCubic ? kBicubicFilterTexelPad : 1;
                clamped_outset_with_offset(&iTileR, outset, &offset, iClampRect);
            }

            // We must subset as a bitmap and then turn it into an SkImage if we want caching to
            // work. Image subsets always make a copy of the pixels and lose the association with
            // the original's SkPixelRef.
            if (SkBitmap subsetBmp; bitmap.extractSubset(&subsetBmp, iTileR)) {
                sk_sp<SkImage> image = SkMakeImageFromRasterBitmap(subsetBmp,
                                                                   kNever_SkCopyPixelsMode);
                if (!image) {
                    continue;
                }

                unsigned aaFlags = SkCanvas::kNone_QuadAAFlags;
                // Preserve the original edge AA flags for the exterior tile edges.
                if (tileR.fLeft <= srcRect.fLeft && (origAAFlags & SkCanvas::kLeft_QuadAAFlag)) {
                    aaFlags |= SkCanvas::kLeft_QuadAAFlag;
                }
                if (tileR.fRight >= srcRect.fRight && (origAAFlags & SkCanvas::kRight_QuadAAFlag)) {
                    aaFlags |= SkCanvas::kRight_QuadAAFlag;
                }
                if (tileR.fTop <= srcRect.fTop && (origAAFlags & SkCanvas::kTop_QuadAAFlag)) {
                    aaFlags |= SkCanvas::kTop_QuadAAFlag;
                }
                if (tileR.fBottom >= srcRect.fBottom &&
                    (origAAFlags & SkCanvas::kBottom_QuadAAFlag)) {
                    aaFlags |= SkCanvas::kBottom_QuadAAFlag;
                }

                // now offset it to make it "local" to our tmp bitmap
                tileR.offset(-offset.fX, -offset.fY);
//                SkMatrix offsetSrcToDst = srcToDst;
//                offsetSrcToDst.preTranslate(offset.fX, offset.fY);

                imgSet.push_back(SkCanvas::ImageSetEntry(std::move(image),
                                                         tileR,
                                                         rectToDraw,
                                                         /* matrixIndex= */ -1,
                                                         /* alpha= */ 1.0f,
                                                         aaFlags,
                                                         /* hasClip= */ false));

#if 0
                device->drawEdgeAAImage(image.get(),
                                        tileR,
                                        rectToDraw,
                                        /* dstClip= */ nullptr,
                                        static_cast<SkCanvas::QuadAAFlags>(aaFlags),
                                        localToDevice,
                                        sampling,
                                        paint,
                                        constraint,
                                        offsetSrcToDst,
                                        tileMode);
#endif

#if GR_TEST_UTILS
                (void)gNumTilesDrawn.fetch_add(+1, std::memory_order_relaxed);
#endif
            }
        }
    }

    canvas->experimental_DrawEdgeAAImageSet(imgSet.data(),
                                            imgSet.size(),
                                            /* dstClips= */ nullptr,
                                            /* preViewMatrices= */ nullptr,
                                            sampling,
                                            paint,
                                            constraint);
}

void TiledTextureUtils::DrawImageRect_Graphite(SkCanvas* canvas,
                                               const SkImage* image,
                                               const SkRect& srcRect,
                                               const SkRect& dstRect,
                                               const SkSamplingOptions& origSampling,
                                               const SkPaint* paint,
                                               SkCanvas::SrcRectConstraint constraint) {
    if (!image->isTextureBacked()) {
        SkRect src;
        SkRect dst;
        SkMatrix srcToDst;
        ImageDrawMode mode = OptimizeSampleArea(SkISize::Make(image->width(), image->height()),
                                                srcRect, dstRect, /* dstClip= */ nullptr,
                                                &src, &dst, &srcToDst);
        if (mode == ImageDrawMode::kSkip) {
            return;
        }

        SkASSERT(mode != ImageDrawMode::kDecal); // only happens if there is a 'dstClip;

        if (src.contains(image->bounds())) {
            constraint = SkCanvas::kFast_SrcRectConstraint;
        }

        auto aaFlags = (paint && paint->isAntiAlias()) ? SkCanvas::kAll_QuadAAFlags
                                                       : SkCanvas::kNone_QuadAAFlags;

        SkMatrix ctm = canvas->getTotalMatrix();

        SkSamplingOptions sampling = origSampling;
        if (sampling.mipmap != SkMipmapMode::kNone && CanDisableMipmap(ctm, srcToDst)) {
            sampling = SkSamplingOptions(sampling.filter);
        }
//        const GrClip* clip = this->clip();

        int tileFilterPad;
        if (sampling.useCubic) {
            tileFilterPad = kBicubicFilterTexelPad;
        } else if (sampling.filter == SkFilterMode::kLinear || sampling.isAniso()) {
            // Aniso will fallback to linear filtering in the tiling case.
            tileFilterPad = 1;
        } else {
            tileFilterPad = 0;
        }

        auto caps = canvas->recorder()->priv().caps();
        int maxTileSize = caps->maxTextureSize() - 2*tileFilterPad;
#if GR_TEST_UTILS
        if (gOverrideMaxTextureSize) {
            maxTileSize = gOverrideMaxTextureSize - 2 * tileFilterPad;
        }
#endif
        size_t cacheSize = 0;
//        if (auto dContext = fContext->asDirectContext(); dContext) {
            // NOTE: if the context is not a direct context, it doesn't have access to the resource
            // cache, and theoretically, the resource cache's limits could be being changed on
            // another thread, so even having access to just the limit wouldn't be a reliable
            // test during recording here.
//            cacheSize = dContext->getResourceCacheLimit();
//        }
        int tileSize;
        SkIRect clippedSubset;
        if (skgpu::TiledTextureUtils::ShouldTileImage(
//                clip ? clip->getConservativeBounds() :
                     SkIRect::MakeSize(canvas->imageInfo().dimensions()),
                image->dimensions(),
                ctm,
                srcToDst,
                &src,
                maxTileSize,
                cacheSize,
                &tileSize,
                &clippedSubset)) {
            // Extract pixels on the CPU, since we have to split into separate textures before
            // sending to the GPU if tiling.
            if (SkBitmap bm; as_IB(image)->getROPixels(nullptr, &bm)) {
                // This is the funnel for all paths that draw tiled bitmaps/images.
                skgpu::TiledTextureUtils::DrawTiledBitmap_Graphite(canvas,
                                                                   bm,
                                                                   tileSize,
                                                                   srcToDst,
                                                                   src,
                                                                   clippedSubset,
                                                                   paint,
                                                                   aaFlags,
                                                                   constraint,
                                                                   sampling);
                return;
            }
        }
    }

    canvas->drawImageRect(image, srcRect, dstRect, origSampling, paint, constraint);
}

} // namespace skgpu
