/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/TiledImageUtils.h"

#include "src/gpu/TiledTextureUtils.h"

namespace skgpu::graphite {

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
    // The canvas should not be Ganesh backed (but could be raster or Graphite-backed)
    SkASSERT(!canvas->recordingContext());

    if (canvas->recorder()) {
        if (skgpu::TiledTextureUtils::DrawAsTiledImageRect(canvas, image, src, dst,
                                                           SkCanvas::kAll_QuadAAFlags, sampling,
                                                           paint, constraint)) {
            return;
        }
    }
    // Fallback for raster (or if Graphite tiling failed).
    canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
}

}  // namespace skgpu::graphite
