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
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "src/capture/SkCapture.h"
#include "src/core/SkResourceCache.h"
#include "src/image/SkImageGeneratorPriv.h"
#include "tests/Test.h"

#include <memory>

DEF_TEST(SkContext_CaptureDisabledByDefault, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    REPORTER_ASSERT(reporter, ctx != nullptr);

    ctx->startCapture();
    sk_sp<SkCapture> capture = ctx->endCapture();
    REPORTER_ASSERT(reporter, capture == nullptr);
}

DEF_TEST(CpuRecorder_TODO, reporter) {
    SkRecorder* todoRecorder = SkRecorder::TODO();
    REPORTER_ASSERT(reporter, todoRecorder != nullptr);
    REPORTER_ASSERT(reporter, todoRecorder->type() == SkRecorder::Type::kCPU);
}

DEF_TEST(SkContext_MakeRecorder_RasterBackendSelection, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    REPORTER_ASSERT(reporter, ctx != nullptr);

    // Explicit kAutomatic
    std::unique_ptr<SkRecorder> autoRecorder =
            ctx->makeRecorder(SkContext::BackendSelection::kAutomatic);
    REPORTER_ASSERT(reporter, autoRecorder != nullptr);
    REPORTER_ASSERT(reporter, autoRecorder->type() == SkRecorder::Type::kCPU);

    // Explicit kCPU
    std::unique_ptr<SkRecorder> cpuRecorder = ctx->makeRecorder(SkContext::BackendSelection::kCPU);
    REPORTER_ASSERT(reporter, cpuRecorder != nullptr);
    REPORTER_ASSERT(reporter, cpuRecorder->type() == SkRecorder::Type::kCPU);

    // kGPU must return nullptr when no GPU backend is attached.
    std::unique_ptr<SkRecorder> gpuRecorder = ctx->makeRecorder(SkContext::BackendSelection::kGPU);
    REPORTER_ASSERT(reporter, gpuRecorder == nullptr);
}

DEF_TEST(CPURecorder_RasterImageOperations_UseCpuRecorder, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    std::unique_ptr<SkRecorder> recorder = ctx->makeRecorder(SkContext::BackendSelection::kCPU);

    SkImageInfo info = SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bm;
    bm.setInfo(info);
    bm.allocPixels();
    bm.eraseColor(SK_ColorGREEN);

    REPORTER_ASSERT(reporter, SkImages::MakeFromPixmap(nullptr, bm.pixmap()) == nullptr);
    sk_sp<SkImage> img = SkImages::MakeFromPixmap(recorder.get(), bm.pixmap());
    REPORTER_ASSERT(reporter, img != nullptr);
    REPORTER_ASSERT(reporter, !img->isTextureBacked());
    REPORTER_ASSERT(reporter, img->isValid(recorder.get()));

    sk_sp<SkData> rawData = SkData::MakeWithCopy(bm.getPixels(), bm.computeByteSize());
    sk_sp<SkImage> rawDataImg =
            SkImages::MakeFromData(recorder.get(), info, std::move(rawData), bm.rowBytes());
    REPORTER_ASSERT(reporter, rawDataImg != nullptr);
    REPORTER_ASSERT(reporter, !rawDataImg->isTextureBacked());
    REPORTER_ASSERT(reporter, rawDataImg->isValid(recorder.get()));

    sk_sp<SkData> pngData = SkPngEncoder::Encode(bm.pixmap(), {});
    REPORTER_ASSERT(reporter, pngData != nullptr);
    sk_sp<SkImage> encodedImg = SkImages::MakeFromData(recorder.get(), pngData);
    REPORTER_ASSERT(reporter, encodedImg != nullptr);
    REPORTER_ASSERT(reporter, !encodedImg->isTextureBacked());
    REPORTER_ASSERT(reporter, encodedImg->isValid(recorder.get()));

    std::unique_ptr<SkImageGenerator> gen = SkImageGenerators::MakeFromEncoded(std::move(pngData));
    REPORTER_ASSERT(reporter, gen != nullptr);
    sk_sp<SkImage> genImg = SkImages::MakeFromGenerator(recorder.get(), std::move(gen));
    REPORTER_ASSERT(reporter, genImg != nullptr);
    REPORTER_ASSERT(reporter, !genImg->isTextureBacked());
    REPORTER_ASSERT(reporter, genImg->isValid(recorder.get()));

    sk_sp<SkImage> scaled =
            img->makeScaled(recorder.get(),
                            SkImageInfo::Make(50, 50, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            SkSamplingOptions(SkFilterMode::kLinear));
    REPORTER_ASSERT(reporter, scaled != nullptr);
    REPORTER_ASSERT(reporter, scaled->width() == 50);
    REPORTER_ASSERT(reporter, scaled->height() == 50);
    REPORTER_ASSERT(reporter, !scaled->isTextureBacked());
}

// TODO(alexisdavidc) Re-enable once the new SkContext / CPU Context & Recorder API is implemented.
#if 0
DEF_TEST(CPURecorder_FromCanvasBaseRecorder_MakesOffscreenSurface, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    std::unique_ptr<SkRecorder> recorder = ctx->makeRecorder();
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
    std::unique_ptr<SkRecorder> recorder = ctx->makeRecorder();
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
    auto surface = SkRecorder::TODO()->makeSurface(imageInfo);
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->clear(SK_ColorGREEN);
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);
    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);
    REPORTER_ASSERT(reporter, surface->getCanvas()->baseRecorder() == SkRecorder::TODO());
}

DEF_TEST(ImageMakeColorSpace_UsesCPURecorderToMakeImage_Success, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    std::unique_ptr<SkRecorder> recorder = ctx->makeRecorder();
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
    std::unique_ptr<SkRecorder> recorder = ctx->makeRecorder();
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
