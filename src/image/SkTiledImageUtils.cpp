/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTiledImageUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixelRef.h"
#include "include/private/SkAssert.h"
#include "include/private/SkTFitsIn.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Picture.h"

#include <string.h>

namespace SkTiledImageUtils {

void DrawImageRect(SkCanvas* canvas,
                   const SkImage* image,
                   const SkRect& src,
                   const SkRect& dst,
                   const SkSamplingOptions& sampling,
                   const SkPaint* paint,
                   SkCanvas::SrcRectConstraint constraint) {
    if (!image || !canvas) {
        return;
    }

    SkPaint p;
    if (paint) {
        p = *paint;
    }
    if (!SkCanvasPriv::TopDevice(canvas)->drawAsTiledImageRect(
                canvas, image, &src, dst, sampling, p, constraint)) {
        // Either the image didn't require tiling or this is a raster-backed
        // canvas. In either case fall back to a default draw.
        canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
    }
}

void GetImageKeyValues(const SkImage* image, uint32_t keyValues[kNumImageKeyValues]) {
    if (!image || !keyValues) {
        if (keyValues) {
            memset(keyValues, 0, kNumImageKeyValues * sizeof(uint32_t));
        }
        return;
    }

    const SkImage_Base* imageBase = as_IB(image);
    if (const SkBitmap* bm = imageBase->onPeekBitmap()) {
        uint64_t genID = bm->pixelRef()->getGenerationID();
        keyValues[0] = static_cast<uint32_t>(genID & 0xFFFFFFFFULL);
        keyValues[1] = static_cast<uint32_t>(genID >> 32);
        SkIRect subset = image->bounds();
        subset.offset(bm->pixelRefOrigin());

        SkASSERT(SkTFitsIn<uint32_t>(subset.fLeft));
        SkASSERT(SkTFitsIn<uint32_t>(subset.fTop));
        SkASSERT(SkTFitsIn<uint32_t>(subset.fRight));
        SkASSERT(SkTFitsIn<uint32_t>(subset.fBottom));

        keyValues[2] = 0;  // This empty slot is to disambiguate picture IDs
        keyValues[3] = subset.fLeft;
        keyValues[4] = subset.fTop;
        keyValues[5] = subset.fRight;
        keyValues[6] = subset.fBottom;
        return;
    }

    if (imageBase->type() == SkImage_Base::Type::kLazyPicture) {
        const SkImage_Picture* pictureImage = static_cast<const SkImage_Picture*>(imageBase);
        if (pictureImage->getImageKeyValues(keyValues)) {
            return;
        }
    }

    uint64_t uniqueID = image->uniqueID();
    keyValues[0] = static_cast<uint32_t>(uniqueID & 0xFFFFFFFFULL);
    keyValues[1] = static_cast<uint32_t>(uniqueID >> 32);
    memset(&keyValues[2], 0, (kNumImageKeyValues - 2) * sizeof(uint32_t));
}

} // namespace SkTiledImageUtils
