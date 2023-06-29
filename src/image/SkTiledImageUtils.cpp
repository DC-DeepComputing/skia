/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTiledImageUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPixelRef.h"
#include "src/image/SkImage_Base.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/TiledTextureUtils.h"
#endif

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

#if defined(SK_GRAPHITE)
    if (canvas->recorder()) {
        skgpu::TiledTextureUtils::DrawImageRect_Graphite(canvas, image, src, dst, sampling, paint,
                                                         constraint);
        return;
    }
#endif

    canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
}

void foo(const SkImage* image, uint32_t bar[kFoo]) {
    SkIRect subset = image->bounds();

    if (const SkBitmap* bm = as_IB(image)->onPeekBitmap()) {
        bar[0] = bm->pixelRef()->getGenerationID();
        subset.offset(bm->pixelRefOrigin());
    } else {
        bar[0] = image->uniqueID();
    }

    SkASSERT(SkTFitsIn<uint32_t>(subset.fLeft));
    SkASSERT(SkTFitsIn<uint32_t>(subset.fTop));
    SkASSERT(SkTFitsIn<uint32_t>(subset.fRight));
    SkASSERT(SkTFitsIn<uint32_t>(subset.fBottom));

    bar[1] = subset.fLeft;
    bar[2] = subset.fTop;
    bar[3] = subset.fRight;
    bar[4] = subset.fBottom;
}

} // namespace SkTiledImageUtils
