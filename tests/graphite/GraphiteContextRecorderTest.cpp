/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkContext.h"
#include "include/core/SkContextOptions.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRecorder.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkContextPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "tests/Test.h"
#include "tools/graphite/GraphiteTestContext.h"
#include "tools/graphite/TestOptions.h"

#include <memory>

// TODO(alexisdavidc) Re-enable once the new SkContext / CPU Context & Recorder API is implemented.
#if 0
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(
        CPUSurface_UsesGraphiteContextAndRasterRecorderToDraw_DrawsPixels,
        reporter,
        context,
        CtsEnforcement::kApiLevel_202604) {
    std::unique_ptr<skcpu::Recorder> recorder = context->makeCPURecorder();

    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = recorder->makeBitmapSurface(imageInfo, imageInfo.minRowBytes(), {});

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);

    // If this were a graphite surface, we'd have to do async read-back, but we make a raster
    // surface which is synchronous.
    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);
}
#endif  // 0

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ImageMakeColorSpace_GraphiteImageWithRecorder_Success,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202604) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();

    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto rImg = SkImages::RasterFromBitmap(bm);
    SkASSERT(rImg);

    auto img = SkImages::TextureFromImage(recorder.get(), rImg);
    SkASSERT(img);
    SkASSERT(img->isTextureBacked());

    auto newImg = img->makeColorSpace(recorder.get(), SkColorSpace::MakeSRGBLinear(), {});

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, newImg->colorSpace() == SkColorSpace::MakeSRGBLinear().get());
    REPORTER_ASSERT(reporter, newImg->isTextureBacked());
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ImageMakeScaled_GraphiteImageWithRecorder_Success,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202604) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();

    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto rImg = SkImages::RasterFromBitmap(bm);
    SkASSERT(rImg);

    auto img = SkImages::TextureFromImage(recorder.get(), rImg);
    SkASSERT(img);
    SkASSERT(img->isTextureBacked());

    REPORTER_ASSERT(reporter, img->isValid(recorder.get()));

    auto newImg =
            img->makeScaled(recorder.get(),
                            SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            {SkCubicResampler::Mitchell()});

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 70);
    REPORTER_ASSERT(reporter, newImg->isTextureBacked());

    // Unlike Ganesh and Graphite, makeScaled doesn't work for Graphite images w/o a recorder
    auto legacyAPI =
            img->makeScaled(SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            {SkCubicResampler::Mitchell()});

    REPORTER_ASSERT(reporter, !legacyAPI);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(CanvasBaseRecorder_GraphiteBasedCanvas_IsOriginalRecorder,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202604) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();

    auto ii = SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkASSERT(surface);

    auto canvas = surface->getCanvas();
    SkASSERT(canvas);

    auto baseRecorder = canvas->baseRecorder();

    REPORTER_ASSERT(reporter, baseRecorder);
    REPORTER_ASSERT(reporter, baseRecorder->type() == SkRecorder::Type::kGraphite);
    REPORTER_ASSERT(reporter, baseRecorder == recorder.get());
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(
        SkContext_MakeRecorder_GraphiteBackendSelectionAndOptions,
        reporter,
        context,
        testContext,
        true,
        CtsEnforcement::kApiLevel_202604) {
    if (!testContext) {
        return;
    }
    std::unique_ptr<skgpu::graphite::Context> graphiteContext =
            testContext->makeContext(skiatest::graphite::TestOptions{});
    if (!graphiteContext) {
        return;
    }

    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContextCtorAccessor::MakeContext(
            opts, skgpu::graphite::ContextPriv::AsGPUContext(std::move(graphiteContext)));
    REPORTER_ASSERT(reporter, ctx != nullptr);

    // Default is kAutomatic, which prefers attached GPU backend (Graphite).
    std::unique_ptr<SkRecorder> defaultRecorder = ctx->makeRecorder();
    REPORTER_ASSERT(reporter, defaultRecorder != nullptr);
    REPORTER_ASSERT(reporter, defaultRecorder->type() == SkRecorder::Type::kGraphite);

    // Passing nullptr explicitly returns default Graphite recorder.
    std::unique_ptr<SkRecorder> nullRecorder = ctx->makeRecorder(nullptr);
    REPORTER_ASSERT(reporter, nullRecorder != nullptr);
    REPORTER_ASSERT(reporter, nullRecorder->type() == SkRecorder::Type::kGraphite);

    // Passing custom Graphite options
    skgpu::graphite::RecorderOptions recorderOptions;
    recorderOptions.fGpuBudgetInBytes = 64 * (1 << 20);
    std::unique_ptr<SkRecorder> optionsRecorder = ctx->makeRecorder(&recorderOptions);
    REPORTER_ASSERT(reporter, optionsRecorder != nullptr);
    REPORTER_ASSERT(reporter, optionsRecorder->type() == SkRecorder::Type::kGraphite);

    // Explicit kAutomatic with options
    std::unique_ptr<SkRecorder> autoRecorder =
            ctx->makeRecorder(SkBackendSelection::kAutomatic, &recorderOptions);
    REPORTER_ASSERT(reporter, autoRecorder != nullptr);
    REPORTER_ASSERT(reporter, autoRecorder->type() == SkRecorder::Type::kGraphite);

    // Explicit kGPU with options
    std::unique_ptr<SkRecorder> gpuRecorder =
            ctx->makeRecorder(SkBackendSelection::kGPU, &recorderOptions);
    REPORTER_ASSERT(reporter, gpuRecorder != nullptr);
    REPORTER_ASSERT(reporter, gpuRecorder->type() == SkRecorder::Type::kGraphite);

    // Explicit kCPU with options falls back to CPU recorder
    std::unique_ptr<SkRecorder> cpuRecorder =
            ctx->makeRecorder(SkBackendSelection::kCPU, &recorderOptions);
    REPORTER_ASSERT(reporter, cpuRecorder != nullptr);
    REPORTER_ASSERT(reporter, cpuRecorder->type() == SkRecorder::Type::kCPU);
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(
        GraphiteContextOptions_ImageProvider,
        reporter,
        context,
        testContext,
        true,
        CtsEnforcement::kApiLevel_202604) {
    if (!testContext) {
        return;
    }

    class TestImageProvider : public skgpu::graphite::ImageProvider {
    public:
        sk_sp<SkImage> findOrCreate(skgpu::graphite::Recorder*,
                                    const SkImage*,
                                    SkImage::RequiredProperties) override {
            return nullptr;
        }
    };

    sk_sp<TestImageProvider> contextProvider = sk_make_sp<TestImageProvider>();
    sk_sp<TestImageProvider> overrideProvider = sk_make_sp<TestImageProvider>();

    skiatest::graphite::TestOptions testOpts;
    testOpts.fContextOptions.fImageProvider = contextProvider;

    {
        // 1. Direct Graphite Context: recorders must be destroyed before graphiteContext.
        std::unique_ptr<skgpu::graphite::Context> graphiteContext =
                testContext->makeContext(testOpts);
        if (!graphiteContext) {
            return;
        }

        REPORTER_ASSERT(reporter, graphiteContext->clientImageProvider() == contextProvider.get());

        // Direct Context::makeRecorder inherits ContextOptions::fImageProvider
        std::unique_ptr<skgpu::graphite::Recorder> directRecorder = graphiteContext->makeRecorder();
        REPORTER_ASSERT(reporter, directRecorder != nullptr);
        REPORTER_ASSERT(reporter, directRecorder->clientImageProvider() == contextProvider.get());

        // Direct Context::makeRecorder with custom RecorderOptions overriding ImageProvider
        skgpu::graphite::RecorderOptions overrideOptions;
        overrideOptions.fImageProvider = overrideProvider;
        std::unique_ptr<skgpu::graphite::Recorder> overriddenRecorder =
                graphiteContext->makeRecorder(overrideOptions);
        REPORTER_ASSERT(reporter, overriddenRecorder != nullptr);
        REPORTER_ASSERT(reporter, overriddenRecorder->clientImageProvider() == overrideProvider.get());

        // Direct Context::makeRecorder with RecorderOptions without ImageProvider inherits
        skgpu::graphite::RecorderOptions defaultOptions;
        std::unique_ptr<skgpu::graphite::Recorder> defaultRecorder =
                graphiteContext->makeRecorder(defaultOptions);
        REPORTER_ASSERT(reporter, defaultRecorder != nullptr);
        REPORTER_ASSERT(reporter, defaultRecorder->clientImageProvider() == contextProvider.get());
    }

    {
        // 2. Wrap in SkContext: recorders created from ctx must be destroyed before ctx.
        std::unique_ptr<skgpu::graphite::Context> graphiteContext =
                testContext->makeContext(testOpts);
        if (!graphiteContext) {
            return;
        }

        SkContextOptions opts;
        std::unique_ptr<SkContext> ctx = SkContextCtorAccessor::MakeContext(
                opts, skgpu::graphite::ContextPriv::AsGPUContext(std::move(graphiteContext)));
        REPORTER_ASSERT(reporter, ctx != nullptr);

        // SkContext::makeRecorder() automatically inherits ContextOptions::fImageProvider
        std::unique_ptr<SkRecorder> skRecorder = ctx->makeRecorder();
        REPORTER_ASSERT(reporter, skRecorder != nullptr);
        REPORTER_ASSERT(reporter, skRecorder->type() == SkRecorder::Type::kGraphite);
        auto graphiteRecorder = static_cast<skgpu::graphite::Recorder*>(skRecorder.get());
        REPORTER_ASSERT(reporter, graphiteRecorder->clientImageProvider() == contextProvider.get());

        // SkContext::makeRecorder(kAutomatic) also inherits ContextOptions::fImageProvider
        std::unique_ptr<SkRecorder> autoRecorder = ctx->makeRecorder(SkBackendSelection::kAutomatic);
        REPORTER_ASSERT(reporter, autoRecorder != nullptr);
        REPORTER_ASSERT(reporter, autoRecorder->type() == SkRecorder::Type::kGraphite);
        REPORTER_ASSERT(reporter, static_cast<skgpu::graphite::Recorder*>(autoRecorder.get())->clientImageProvider() ==
                                  contextProvider.get());

        // SkContext::makeRecorder(kGPU) also inherits ContextOptions::fImageProvider
        std::unique_ptr<SkRecorder> gpuRecorder = ctx->makeRecorder(SkBackendSelection::kGPU);
        REPORTER_ASSERT(reporter, gpuRecorder != nullptr);
        REPORTER_ASSERT(reporter, gpuRecorder->type() == SkRecorder::Type::kGraphite);
        REPORTER_ASSERT(reporter, static_cast<skgpu::graphite::Recorder*>(gpuRecorder.get())->clientImageProvider() ==
                                  contextProvider.get());

        // SkContext::makeRecorder(&overrideOptions) overrides with the RecorderOptions' ImageProvider
        skgpu::graphite::RecorderOptions overrideOptions;
        overrideOptions.fImageProvider = overrideProvider;
        std::unique_ptr<SkRecorder> customRecorder = ctx->makeRecorder(&overrideOptions);
        REPORTER_ASSERT(reporter, customRecorder != nullptr);
        REPORTER_ASSERT(reporter, customRecorder->type() == SkRecorder::Type::kGraphite);
        REPORTER_ASSERT(reporter, static_cast<skgpu::graphite::Recorder*>(customRecorder.get())->clientImageProvider() ==
                                  overrideProvider.get());

        // SkContext::makeRecorder(&defaultOptions) inherits ContextOptions::fImageProvider
        skgpu::graphite::RecorderOptions defaultOptions;
        std::unique_ptr<SkRecorder> emptyOptionsRecorder = ctx->makeRecorder(&defaultOptions);
        REPORTER_ASSERT(reporter, emptyOptionsRecorder != nullptr);
        REPORTER_ASSERT(reporter, emptyOptionsRecorder->type() == SkRecorder::Type::kGraphite);
        REPORTER_ASSERT(reporter, static_cast<skgpu::graphite::Recorder*>(emptyOptionsRecorder.get())->clientImageProvider() ==
                                  contextProvider.get());

        // SkContext::makeRecorder(kCPU) safely returns CPU recorder
        std::unique_ptr<SkRecorder> cpuRecorder = ctx->makeRecorder(SkBackendSelection::kCPU);
        REPORTER_ASSERT(reporter, cpuRecorder != nullptr);
        REPORTER_ASSERT(reporter, cpuRecorder->type() == SkRecorder::Type::kCPU);
    }
}
