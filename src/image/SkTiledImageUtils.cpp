/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTiledImageUtils.h"

#include "src/gpu/TiledTextureUtils.h"

#if 0
static void print_rect1(const char* label, SkRect r) {
    SkDebugf("%s: %.2f %.2f %.2f %.2f\n", label, r.fLeft, r.fTop, r.fRight, r.fBottom);
}

static void print_rect2(const char* label, SkIRect r) {
    SkDebugf("%s: %d %d %d %d\n", label, r.fLeft, r.fTop, r.fRight, r.fBottom);
}
#endif

namespace SkTiledImageUtils {

void DrawImageRect(SkCanvas* canvas,
                   const SkImage* image,
                   const SkRect& src,
                   const SkRect& dst,
                   const SkSamplingOptions& sampling,
                   const SkPaint* paint,
                   SkCanvas::SrcRectConstraint constraint) {
    if (!image) {
        return;
    }

#if 0
    SkRect r1;
    canvas->getLocalClipBounds(&r1);
    print_rect1("localClipBounds", r1);
    SkIRect r2;
    canvas->getDeviceClipBounds(&r2);
    print_rect2("deviceClipBounds", r2);
#endif

    if (canvas->recorder()) {
        skgpu::TiledTextureUtils::DrawImageRect_Graphite(canvas, image, src, dst, sampling, paint,
                                                         constraint);
        return;
    }

    canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
}

} // namespace SkTiledImageUtils
