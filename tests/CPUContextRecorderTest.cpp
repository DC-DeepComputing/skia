/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/RasterContext.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkContext.h"
#include "include/core/SkContextOptions.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRecorder.h"
#include "include/core/SkSurface.h"
#include "src/core/SkResourceCache.h"
#include "src/cpu/Recorder.h"
#include "tests/Test.h"

#include <memory>

class SkContext;
struct SkContextOptions;

namespace skcpu {
class Recorder;
}

DEF_TEST(CpuRecorder_TODO, reporter) {
    skcpu::Recorder* todoRecorder = skcpu::Recorder::TODO();
    REPORTER_ASSERT(reporter, todoRecorder != nullptr);
    REPORTER_ASSERT(reporter, todoRecorder->type() == SkRecorder::Type::kCPU);
    REPORTER_ASSERT(reporter, skcpu::AsRecorder(todoRecorder) == todoRecorder);
}

// TODO(alexisdavidc) Re-enable once the new SkContext / CPU Context & Recorder API is implemented.
#if 0
DEF_TEST(CPURecorder_AsRecorder_ReturnsSelf, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeCPURecorder();
    SkRecorder* baseRecorder = recorder.get();

    REPORTER_ASSERT(reporter, recorder->type() == SkRecorder::Type::kCPU);
    REPORTER_ASSERT(reporter, baseRecorder->type() == SkRecorder::Type::kCPU);
    REPORTER_ASSERT(reporter, skcpu::AsRecorder(baseRecorder) == recorder.get());
}

DEF_TEST(CPURecorder_RasterImageOperations_UseCpuRecorder, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeCPURecorder();
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    bm.eraseColor(SK_ColorGREEN);
    sk_sp<SkImage> img = SkImages::FromBitmap(recorder.get(), bm);
    REPORTER_ASSERT(reporter, img != nullptr);

    REPORTER_ASSERT(reporter, img->isValid(recorder.get()));

    sk_sp<SkImage> scaled =
            img->makeScaled(recorder.get(),
                            SkImageInfo::Make(50, 50, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            SkSamplingOptions(SkFilterMode::kLinear));
    REPORTER_ASSERT(reporter, scaled != nullptr);
    REPORTER_ASSERT(reporter, scaled->width() == 50);
    REPORTER_ASSERT(reporter, scaled->height() == 50);
    REPORTER_ASSERT(reporter, !scaled->isTextureBacked());
}

DEF_TEST(CPURecorder_FromCanvasBaseRecorder_MakesOffscreenSurface, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeCPURecorder();
    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> mainSurface = recorder->makeSurface(imageInfo);
    REPORTER_ASSERT(reporter, mainSurface != nullptr);

    SkRecorder* baseRecorder = mainSurface->getCanvas()->baseRecorder();
    REPORTER_ASSERT(reporter, baseRecorder == recorder.get());
    REPORTER_ASSERT(reporter, baseRecorder->type() == SkRecorder::Type::kCPU);

    // Client use case: use an arbitrary canvas's baseRecorder() to allocate an offscreen
    // surface without needing a backend-specific surface creation function.
    sk_sp<SkSurface> offscreen = baseRecorder->makeSurface(imageInfo.makeWH(50, 50));
    REPORTER_ASSERT(reporter, offscreen != nullptr);
    REPORTER_ASSERT(reporter, offscreen->getCanvas()->baseRecorder() == recorder.get());

    offscreen->getCanvas()->clear(SK_ColorBLUE);
    SkPixmap pmap;
    REPORTER_ASSERT(reporter, offscreen->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorBLUE);
}

DEF_TEST(CPUSurface_UsesCPUContextAndRecorderToDraw_DrawsPixels, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeCPURecorder();
    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = recorder->makeSurface(imageInfo);
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
    auto surface = skcpu::Recorder::TODO()->makeSurface(imageInfo);
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
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeCPURecorder();
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto img = SkImages::FromBitmap(recorder.get(), bm);
    SkASSERT(img);
    REPORTER_ASSERT(reporter, img->isValid(recorder.get()));
    auto newImg = img->makeColorSpace(recorder.get(), SkColorSpace::MakeSRGBLinear(), {});
    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, !newImg->isTextureBacked());
}

DEF_TEST(ImageMakeScaled_UsesCPURecorderToMakeImage_Success, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeCPURecorder();
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto img = SkImages::FromBitmap(recorder.get(), bm);
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

#endif  // 0
