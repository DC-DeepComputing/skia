/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/TiledTextureUtils.h"

#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/gpu/GrDirectContext.h"
#include "src/base/SkSafeMath.h"

#include "src/gpu/ganesh/GrClip.h"

//////////////////////////////////////////////////////////////////////////////
//  Helper functions for tiling a large SkBitmap

namespace {

static const int kBmpSmallTileSize = 1 << 10;

inline size_t get_tile_count(const SkIRect& srcRect, int tileSize)  {
    int tilesX = (srcRect.fRight / tileSize) - (srcRect.fLeft / tileSize) + 1;
    int tilesY = (srcRect.fBottom / tileSize) - (srcRect.fTop / tileSize) + 1;
    // We calculate expected tile count before we read the bitmap's pixels, so hypothetically we can
    // have lazy images with excessive dimensions that would cause (tilesX*tilesY) to overflow int.
    // In these situations we also later fail to allocate a bitmap to store the lazy image, so there
    // isn't really a performance concern around one image turning into millions of tiles.
    return SkSafeMath::Mul(tilesX, tilesY);
}

int determine_tile_size(const SkIRect& src, int maxTileSize) {
    if (maxTileSize <= kBmpSmallTileSize) {
        return maxTileSize;
    }

    size_t maxTileTotalTileSize = get_tile_count(src, maxTileSize);
    size_t smallTotalTileSize = get_tile_count(src, kBmpSmallTileSize);

    maxTileTotalTileSize *= maxTileSize * maxTileSize;
    smallTotalTileSize *= kBmpSmallTileSize * kBmpSmallTileSize;

    if (maxTileTotalTileSize > 2 * smallTotalTileSize) {
        return kBmpSmallTileSize;
    } else {
        return maxTileSize;
    }
}

// Given a bitmap, an optional src rect, and a context with a clip and matrix determine what
// pixels from the bitmap are necessary.
SkIRect determine_clipped_src_rect(int width, int height,
                                   const GrClip* clip,
                                   const SkMatrix& viewMatrix,
                                   const SkMatrix& srcToDstRect,
                                   const SkISize& imageDimensions,
                                   const SkRect* srcRectPtr) {
    SkIRect clippedSrcIRect = clip ? clip->getConservativeBounds()
                                   : SkIRect::MakeWH(width, height);
    SkMatrix inv = SkMatrix::Concat(viewMatrix, srcToDstRect);
    if (!inv.invert(&inv)) {
        return SkIRect::MakeEmpty();
    }
    SkRect clippedSrcRect = SkRect::Make(clippedSrcIRect);
    inv.mapRect(&clippedSrcRect);
    if (srcRectPtr) {
        if (!clippedSrcRect.intersect(*srcRectPtr)) {
            return SkIRect::MakeEmpty();
        }
    }
    clippedSrcRect.roundOut(&clippedSrcIRect);
    SkIRect bmpBounds = SkIRect::MakeSize(imageDimensions);
    if (!clippedSrcIRect.intersect(bmpBounds)) {
        return SkIRect::MakeEmpty();
    }

    return clippedSrcIRect;
}

} // anonymous namespace

namespace skgpu {

// tileSize and clippedSubset are valid if true is returned
bool ShouldTileImage(GrRecordingContext* context,
                     SkISize rtSize,
                     const GrClip* clip,
                     uint32_t /* imageID */,
                     const SkISize& imageSize,
                     const SkMatrix& ctm,
                     const SkMatrix& srcToDst,
                     const SkRect* src,
                     int maxTileSize,
                     int* tileSize,
                     SkIRect* clippedSubset) {
    // if it's larger than the max tile size, then we have no choice but tiling.
    if (imageSize.width() > maxTileSize || imageSize.height() > maxTileSize) {
        *clippedSubset = determine_clipped_src_rect(rtSize.width(), rtSize.height(), clip, ctm,
                                                    srcToDst, imageSize, src);
        *tileSize = determine_tile_size(*clippedSubset, maxTileSize);
        return true;
    }

    // If the image would only produce 4 tiles of the smaller size, don't bother tiling it.
    const size_t area = imageSize.width() * imageSize.height();
    if (area < 4 * kBmpSmallTileSize * kBmpSmallTileSize) {
        return false;
    }

    // At this point we know we could do the draw by uploading the entire bitmap as a texture.
    // However, if the texture would be large compared to the cache size and we don't require most
    // of it for this draw then tile to reduce the amount of upload and cache spill.
    // NOTE: if the context is not a direct context, it doesn't have access to the resource cache,
    // and theoretically, the resource cache's limits could be being changed on another thread, so
    // even having access to just the limit wouldn't be a reliable test during recording here.
    // Instead, we will just upload the entire image to be on the safe side and not tile.
    auto direct = context->asDirectContext();
    if (!direct) {
        return false;
    }

    // assumption here is that sw bitmap size is a good proxy for its size as
    // a texture
    size_t bmpSize = area * sizeof(SkPMColor);  // assume 32bit pixels
    size_t cacheSize = direct->getResourceCacheLimit();
    if (bmpSize < cacheSize / 2) {
        return false;
    }

    // Figure out how much of the src we will need based on the src rect and clipping. Reject if
    // tiling memory savings would be < 50%.
    *clippedSubset = determine_clipped_src_rect(rtSize.width(), rtSize.height(), clip, ctm,
                                                srcToDst, imageSize, src);
    *tileSize = kBmpSmallTileSize; // already know whole bitmap fits in one max sized tile.
    size_t usedTileBytes = get_tile_count(*clippedSubset, kBmpSmallTileSize) *
                           kBmpSmallTileSize * kBmpSmallTileSize *
                           sizeof(SkPMColor);  // assume 32bit pixels;

    return usedTileBytes * 2 < bmpSize;
}

} // namespace skgpu
