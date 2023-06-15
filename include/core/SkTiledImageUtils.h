/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTiledImageUtils_DEFINED
#define SkTiledImageUtils_DEFINED

#include "include/core/SkCanvas.h"

namespace SkTiledImageUtils {

SK_API void DrawTiledImage(SkCanvas* canvas,
                           const SkImage* image,
                           SkScalar x, SkScalar y,
                           const SkSamplingOptions& sampling = {},
                           const SkPaint* paint = nullptr,
                           SkCanvas::SrcRectConstraint = SkCanvas::kFast_SrcRectConstraint);

SK_API void DrawTiledImageRect(SkCanvas* canvas,
                               const SkImage* image,
                               const SkRect& dst,
                               const SkSamplingOptions& sampling = {},
                               const SkPaint* paint = nullptr,
                               SkCanvas::SrcRectConstraint = SkCanvas::kFast_SrcRectConstraint);

SK_API void DrawTiledImageRect(SkCanvas* canvas,
                               const SkImage* image,
                               const SkRect& src,
                               const SkRect& dst,
                               const SkSamplingOptions& sampling = {},
                               const SkPaint* paint = nullptr,
                               SkCanvas::SrcRectConstraint = SkCanvas::kFast_SrcRectConstraint);

}  // namespace SkTiledImageUtils

#endif // SkTiledImageUtils_DEFINED
