/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTiledImageUtils_DEFINED
#define SkTiledImageUtils_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"

namespace SkTiledImageUtils {

SK_API void DrawImageRect(SkCanvas* canvas,
                          const SkImage* image,
                          const SkRect& src,
                          const SkRect& dst,
                          const SkSamplingOptions& sampling = {},
                          const SkPaint* paint = nullptr,
                          SkCanvas::SrcRectConstraint constraint =
                                  SkCanvas::kFast_SrcRectConstraint);

SK_API void DrawImageRect(SkCanvas* canvas,
                          const sk_sp<SkImage>& image,
                          const SkRect& src,
                          const SkRect& dst,
                          const SkSamplingOptions& sampling = {},
                          const SkPaint* paint = nullptr,
                          SkCanvas::SrcRectConstraint constraint =
                                  SkCanvas::kFast_SrcRectConstraint) {
    DrawImageRect(canvas, image.get(), src, dst, sampling, paint, constraint);
}

SK_API void DrawImageRect(SkCanvas* canvas,
                          const SkImage* image,
                          const SkRect& dst,
                          const SkSamplingOptions& sampling = {},
                          const SkPaint* paint = nullptr,
                          SkCanvas::SrcRectConstraint constraint =
                                  SkCanvas::kFast_SrcRectConstraint) {
    if (!image) {
        return;
    }

    SkRect src = SkRect::MakeIWH(image->width(), image->height());

    DrawImageRect(canvas, image, src, dst, sampling, paint, constraint);
}

SK_API void DrawImageRect(SkCanvas* canvas,
                          const sk_sp<SkImage>& image,
                          const SkRect& dst,
                          const SkSamplingOptions& sampling = {},
                          const SkPaint* paint = nullptr,
                          SkCanvas::SrcRectConstraint constraint =
                                  SkCanvas::kFast_SrcRectConstraint) {
    DrawImageRect(canvas, image.get(), dst, sampling, paint, constraint);
}

SK_API void DrawImage(SkCanvas* canvas,
                      const SkImage* image,
                      SkScalar x, SkScalar y,
                      const SkSamplingOptions& sampling = {},
                      const SkPaint* paint = nullptr,
                      SkCanvas::SrcRectConstraint constraint =
                              SkCanvas::kFast_SrcRectConstraint) {
    if (!image) {
        return;
    }

    SkRect src = SkRect::MakeIWH(image->width(), image->height());
    SkRect dst = SkRect::MakeXYWH(x, y, image->width(), image->height());

    DrawImageRect(canvas, image, src, dst, sampling, paint, constraint);
}

SK_API void DrawImage(SkCanvas* canvas,
                      const sk_sp<SkImage>& image,
                      SkScalar x, SkScalar y,
                      const SkSamplingOptions& sampling = {},
                      const SkPaint* paint = nullptr,
                      SkCanvas::SrcRectConstraint constraint =
                              SkCanvas::kFast_SrcRectConstraint) {
    DrawImage(canvas, image.get(), x, y, sampling, paint, constraint);
}

}  // namespace SkTiledImageUtils

#endif // SkTiledImageUtils_DEFINED
