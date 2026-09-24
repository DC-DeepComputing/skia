/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#if defined(SK_GANESH)
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/SkGaneshRecorder.h"
#include "tests/Test.h"

#include <memory>

// TODO(alexisdavidc) Re-enable once the new SkContext / CPU Context & Recorder API is implemented.
#if 0
DEF_GANESH_TEST_FOR_ALL_CONTEXTS(CPUSurface_UsesGaneshContextAndRasterRecorderToDraw_DrawsPixels,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_202604) {
    std::unique_ptr<skcpu::Recorder> recorder = ctxInfo.directContext()->makeCPURecorder();

    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = recorder->makeBitmapSurface(imageInfo, imageInfo.minRowBytes(), {});

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);

    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);
}
#endif  // 0

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(ImageMakeColorSpace_GaneshImageWithContext_Success,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_202604) {
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto img = SkImages::RasterFromBitmap(bm);
    SkASSERT(img);
    auto grImage = SkImages::TextureFromImage(ctxInfo.directContext(), img);
    SkASSERT(grImage);

    auto newImg = grImage->makeColorSpace(
            ctxInfo.directContext()->asRecorder(), SkColorSpace::MakeSRGBLinear(), {});

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, newImg->colorSpace() == SkColorSpace::MakeSRGBLinear().get());
    REPORTER_ASSERT(reporter, newImg->isTextureBacked());
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(ImageMakeColorSpace_GaneshImageWithRecorder_Success,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_202604) {
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto rImg = SkImages::RasterFromBitmap(bm);
    SkASSERT(rImg);

    auto img = SkImages::TextureFromImage(ctxInfo.directContext(), rImg);
    SkASSERT(img);
    SkASSERT(img->isTextureBacked());

    auto newImg = img->makeColorSpace(
            ctxInfo.directContext()->asRecorder(), SkColorSpace::MakeSRGBLinear(), {});

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, newImg->colorSpace() == SkColorSpace::MakeSRGBLinear().get());
    REPORTER_ASSERT(reporter, newImg->isTextureBacked());
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(ImageMakeScaled_GaneshImageWithRecorder_Success,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_202604) {
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto rImg = SkImages::RasterFromBitmap(bm);
    SkASSERT(rImg);

    auto img = SkImages::TextureFromImage(ctxInfo.directContext(), rImg);
    SkASSERT(img);
    SkASSERT(img->isTextureBacked());

    REPORTER_ASSERT(reporter, img->isValid(ctxInfo.directContext()->asRecorder()));

    auto newImg =
            img->makeScaled(ctxInfo.directContext()->asRecorder(),
                            SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            {SkCubicResampler::Mitchell()});

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 70);
    REPORTER_ASSERT(reporter, newImg->isTextureBacked());

    auto legacyAPI =
            img->makeScaled(SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            {SkCubicResampler::Mitchell()});

    REPORTER_ASSERT(reporter, legacyAPI);
    REPORTER_ASSERT(reporter, legacyAPI->width() == 70);
    REPORTER_ASSERT(reporter, legacyAPI->isTextureBacked());
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(CanvasBaseRecorder_GaneshBasedCanvas_IsLinkedToDirectContext,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_202604) {
    auto ctx = ctxInfo.directContext();

    auto ii = SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kYes, ii, 0, nullptr);
    SkASSERT(surface);

    auto canvas = surface->getCanvas();
    SkASSERT(canvas);

    auto baseRecorder = canvas->baseRecorder();

    REPORTER_ASSERT(reporter, baseRecorder);
    REPORTER_ASSERT(reporter, baseRecorder->type() == SkRecorder::Type::kGanesh);

    auto ganeshRecorder = AsGaneshRecorder(baseRecorder);
    REPORTER_ASSERT(reporter, ganeshRecorder->directContext() == ctx);
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(GaneshRecorder_CpuRecorder_ProvidesValidCPURecorder,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_202604) {
    auto ctx = ctxInfo.directContext();
    SkRecorder* ganeshRecorder = ctx->asRecorder();
    REPORTER_ASSERT(reporter, ganeshRecorder != nullptr);
    REPORTER_ASSERT(reporter, ganeshRecorder->type() == SkRecorder::Type::kGanesh);

    // Strict downcast to skcpu::Recorder returns nullptr for a Ganesh recorder,
    // whereas cpuRecorder() always provides a valid skcpu::Recorder*.
    REPORTER_ASSERT(reporter, skcpu::AsRecorder(ganeshRecorder) == nullptr);
    skcpu::Recorder* cpuRec = ganeshRecorder->cpuRecorder();
    REPORTER_ASSERT(reporter, cpuRec != nullptr);
    REPORTER_ASSERT(reporter, cpuRec->type() == SkRecorder::Type::kCPU);
    REPORTER_ASSERT(reporter, cpuRec->cpuRecorder() == cpuRec);

    // Client use case 1: Create a synchronous CPU bitmap surface from a GPU recorder's cpuRecorder().
    auto ii = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> cpuSurface = cpuRec->makeBitmapSurface(ii);
    REPORTER_ASSERT(reporter, cpuSurface != nullptr);
    REPORTER_ASSERT(reporter, cpuSurface->getCanvas()->baseRecorder() == cpuRec);
    cpuSurface->getCanvas()->clear(SK_ColorRED);
    SkPixmap pmap;
    REPORTER_ASSERT(reporter, cpuSurface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(10, 10) == SK_ColorRED);

    // Client use case 2: Raster SkImages use recorder->cpuRecorder() to validate and allocate
    // surfaces (e.g., in makeScaled) even when passed a Ganesh SkRecorder*.
    sk_sp<SkImage> rasterImg = cpuSurface->makeImageSnapshot();
    REPORTER_ASSERT(reporter, rasterImg != nullptr);
    REPORTER_ASSERT(reporter, !rasterImg->isTextureBacked());
    REPORTER_ASSERT(reporter, rasterImg->isValid(ganeshRecorder));

    sk_sp<SkImage> scaledRasterImg = rasterImg->makeScaled(
            ganeshRecorder, ii.makeWH(32, 32), SkSamplingOptions(SkFilterMode::kLinear));
    REPORTER_ASSERT(reporter, scaledRasterImg != nullptr);
    REPORTER_ASSERT(reporter, scaledRasterImg->width() == 32);
    REPORTER_ASSERT(reporter, !scaledRasterImg->isTextureBacked());
}

#endif  // defined(SK_GANESH)
