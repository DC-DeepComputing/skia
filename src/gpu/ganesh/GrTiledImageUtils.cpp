/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/GrTiledImageUtils.h"

namespace skgpu::ganesh {

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
    // The canvas should not be Graphite backed (but could be raster or Ganesh-backed)
    SkASSERT(!canvas->recorder());
    // Ganesh does tiling via ganesh/Device::drawAsTiledImageRect
    canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
}

}  // namespace skgpu::ganesh
