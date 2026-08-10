/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCPURecorder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkContext.h"
#include "include/core/SkContextOptions.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSurface.h"

#include "tests/Test.h"

#include <memory>

DEF_TEST(SkContext_Make_Success, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctxWithOptions = SkContext::Make(opts);
    REPORTER_ASSERT(reporter, ctxWithOptions != nullptr);

    std::unique_ptr<SkContext> ctxDefault = SkContext::Make();
    REPORTER_ASSERT(reporter, ctxDefault != nullptr);

    const SkContext* todoCtx = SkContext::TODO();
    REPORTER_ASSERT(reporter, todoCtx != nullptr);
    REPORTER_ASSERT(reporter, SkContext::TODO() == todoCtx);
}

DEF_TEST(CPUSurface_UsesContextAndRecorderToDraw_DrawsPixels, reporter) {
    SkContextOptions opts;
    auto ctx = SkContext::Make(opts);
    REPORTER_ASSERT(reporter, ctx != nullptr);

    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeRecorder();
    REPORTER_ASSERT(reporter, recorder != nullptr);
    REPORTER_ASSERT(reporter, recorder->type() == SkRecorder::Type::kCPU);
    REPORTER_ASSERT(reporter, recorder->ctx() == ctx.get());

    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = recorder->makeBitmapSurface(imageInfo, imageInfo.minRowBytes(), {});
    REPORTER_ASSERT(reporter, surface != nullptr);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);
    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);
    REPORTER_ASSERT(reporter, surface->getCanvas()->baseRecorder() == recorder.get());
}

DEF_TEST(CPUSurface_UsesTODORecorder_DrawsPixels, reporter) {
    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = skcpu::Recorder::TODO()->makeBitmapSurface(imageInfo, imageInfo.minRowBytes(), {});
    REPORTER_ASSERT(reporter, surface != nullptr);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->clear(SK_ColorGREEN);
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);
    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);
    REPORTER_ASSERT(reporter, surface->getCanvas()->baseRecorder() == skcpu::Recorder::TODO());
}

DEF_TEST(ImageMakeColorSpace_UsesCPURecorderToMakeImage_Success, reporter) {
    auto ctx = SkContext::Make();
    REPORTER_ASSERT(reporter, ctx != nullptr);

    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeRecorder();
    REPORTER_ASSERT(reporter, recorder != nullptr);

    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto img = SkImages::RasterFromBitmap(bm);
    SkASSERT(img);
    REPORTER_ASSERT(reporter, img->isValid(recorder.get()));
    auto newImg = img->makeColorSpace(recorder.get(), SkColorSpace::MakeSRGBLinear(), {});
    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, !newImg->isTextureBacked());
}

DEF_TEST(ImageMakeScaled_UsesCPURecorderToMakeImage_Success, reporter) {
    auto ctx = SkContext::Make();
    REPORTER_ASSERT(reporter, ctx != nullptr);

    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeRecorder();
    REPORTER_ASSERT(reporter, recorder != nullptr);

    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto img = SkImages::RasterFromBitmap(bm);
    SkASSERT(img);
    auto newImg =
            img->makeScaled(recorder.get(),
                            SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            {SkCubicResampler::Mitchell()});
    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 70);
    REPORTER_ASSERT(reporter, !newImg->isTextureBacked());
    auto legacyAPI =
            img->makeScaled(SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            {SkCubicResampler::Mitchell()});
    REPORTER_ASSERT(reporter, legacyAPI);
    REPORTER_ASSERT(reporter, legacyAPI->width() == 70);
    REPORTER_ASSERT(reporter, !legacyAPI->isTextureBacked());
}
