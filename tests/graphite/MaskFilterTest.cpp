/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #include "tests/Test.h"

 #include "include/core/SkBitmap.h"
 #include "include/core/SkCanvas.h"
 #include "include/core/SkPixmap.h"
 #include "include/core/SkVertices.h"
 #include "include/effects/SkTableMaskFilter.h"
 #include "include/gpu/graphite/Context.h"
 #include "include/gpu/graphite/Recorder.h"
 #include "include/gpu/graphite/Surface.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "src/core/SkBlurMask.h"
#include "src/effects/SkEmbossMaskFilter.h"
#include "tools/ToolUtils.h"

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(MaskFilterTestTable, reporter, context,
                                  CtsEnforcement::kApiLevel_202404) {

    // Set up graphite surface.
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(128, 128),
                                      SkColorType::kN32_SkColorType,
                                      SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> graphiteSurface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkCanvas* graphiteCanvas = graphiteSurface->getCanvas();

    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);

    // Create mask filter where entire coverage mask is 128. This will be implemented as
    // an image filter in the graphite device.
    uint8_t table[256];
    for (int i = 0; i < 256; ++i) {
        table[i] = 128;
    }
    paint.setMaskFilter(sk_sp<SkMaskFilter>(SkTableMaskFilter::Create(table)));

    // Draw 5 black rectangles with different alpha 0-1.
    paint.setColor4f({0,0,0,0});
    graphiteCanvas->drawRect(SkRect::MakeXYWH(0, 0, 10, 50), paint);

    paint.setColor4f({0,0,0,.25});
    graphiteCanvas->drawRect(SkRect::MakeXYWH(20, 0, 10, 50), paint);

    paint.setColor4f({0,0,0,.5});
    graphiteCanvas->drawRect(SkRect::MakeXYWH(40, 0, 10, 50), paint);

    paint.setColor4f({0,0,0,.75});
    graphiteCanvas->drawRect(SkRect::MakeXYWH(60, 0, 10, 50), paint);

    paint.setColor4f({0,0,0,1});
    graphiteCanvas->drawRect(SkRect::MakeXYWH(80, 0, 10, 50), paint);


    // Create raster surface and draw same rectanglges.
    sk_sp<SkSurface> rasterSurface =
    SkSurfaces::Raster(SkImageInfo::MakeN32Premul(128, 128));
    SkCanvas* rasterCanvas = rasterSurface->getCanvas();

    paint.setColor4f({0,0,0,0});
    rasterCanvas->drawRect(SkRect::MakeXYWH(0, 0, 10, 50), paint);

    paint.setColor4f({0,0,0,.25});
    rasterCanvas->drawRect(SkRect::MakeXYWH(20, 0, 10, 50), paint);

    paint.setColor4f({0,0,0,.5});
    rasterCanvas->drawRect(SkRect::MakeXYWH(40, 0, 10, 50), paint);

    paint.setColor4f({0,0,0,.75});
    rasterCanvas->drawRect(SkRect::MakeXYWH(60, 0, 10, 50), paint);

    paint.setColor4f({0,0,0,1});
    rasterCanvas->drawRect(SkRect::MakeXYWH(80, 0, 10, 50), paint);

    // Read pixels from both surfaces.
    SkBitmap graphiteBM;
    SkPixmap graphitePM;
    graphiteBM.allocPixels(ii);
    SkAssertResult(graphiteBM.peekPixels(&graphitePM));
    if (!graphiteSurface->readPixels(graphitePM, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }
    SkBitmap rasterBM;
    SkPixmap rasterPM;
    rasterBM.allocPixels(ii);
    rasterBM.peekPixels(&rasterPM);
    rasterSurface->readPixels(rasterPM, 0, 0);

    // Both surfaces should have the same bitmap.
    bool success = ToolUtils::equal_pixels(graphiteBM, rasterBM);
    REPORTER_ASSERT(reporter, success);
}
}  // namespace skgpu::graphite
