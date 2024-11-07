/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"

#include <thread>

using namespace::skgpu::graphite;

namespace {

static constexpr int kMaxNumStops = 9;
static constexpr SkColor gColors[kMaxNumStops] = {
        SK_ColorRED,
        SK_ColorGREEN,
        SK_ColorBLUE,
        SK_ColorCYAN,
        SK_ColorMAGENTA,
        SK_ColorYELLOW,
        SK_ColorBLACK,
        SK_ColorDKGRAY,
        SK_ColorLTGRAY,
};
static constexpr SkPoint gPts[kMaxNumStops] = {
        { -100.0f, -100.0f },
        { -50.0f, -50.0f },
        { -25.0f, -25.0f },
        { -12.5f, -12.5f },
        { 0.0f, 0.0f },
        { 12.5f, 12.5f },
        { 25.0f, 25.0f },
        { 50.0f, 50.0f },
        { 100.0f, 100.0f }
};
static constexpr float gOffsets[kMaxNumStops] =
            { 0.0f, 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f };
static constexpr int kNumStops = 2; // should be 7 or 9 too

std::pair<SkPaint, PaintOptions> linear() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(gPts,
                                                 gColors, gOffsets, kNumStops,
                                                 SkTileMode::kClamp));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

std::pair<SkPaint, PaintOptions> radial() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::RadialGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    SkPaint paint;
    paint.setShader(SkGradientShader::MakeRadial(/* center= */ {0, 0}, /* radius= */ 100,
                                                 gColors, gOffsets, kNumStops,
                                                 SkTileMode::kClamp));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

std::pair<SkPaint, PaintOptions> sweep() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::SweepGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    SkPaint paint;
//    paint.setShader(SkGradientShader::MakeSweep(/* cx= */ 0, /* cy= */ 0,
//                                                gColors, gOffsets, kNumStops,
//                                                SkTileMode::kClamp,
//                                                /* startAngle= */ 0, /* endAngle= */ 359));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

std::pair<SkPaint, PaintOptions> conical() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::TwoPointConicalGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    SkPaint paint;
    paint.setShader(SkGradientShader::MakeTwoPointConical(/* start= */ {100, 100},
                                                          /* startRadius= */ 100,
                                                          /* end= */ {-100, -100},
                                                          /* endRadius= */ 100,
                                                          gColors, gOffsets, kNumStops,
                                                          SkTileMode::kClamp));
    paint.setBlendMode(SkBlendMode::kSrcOver);

    return { paint, paintOptions };
}

void precompile_gradients(Context* context,
                          skiatest::Reporter* reporter,
                          int threadID) {
    constexpr RenderPassProperties kProps = { DepthStencilFlags::kDepth,
                                              kBGRA_8888_SkColorType,
                                              /* requiresMSAA= */ false };

    for (auto createOptionsMtd : { linear, radial, sweep, conical }) {
        auto [_, paintOptions] = createOptionsMtd();
        Precompile(context,
                   paintOptions,
                   DrawTypeFlags::kBitmapText_Mask,
                   { &kProps, 1 });
    }
}

void compile_gradients(Context* context,
                       skiatest::Reporter* reporter,
                       int threadID) {
    SkImageInfo ii = SkImageInfo::Make(16, 16,
                                       kBGRA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(recorder.get(), ii,
                                                     skgpu::Mipmapped::kNo,
                                                     /* props= */ nullptr);
    SkCanvas* canvas = surf->getCanvas();

    for (auto createOptionsMtd : { linear, radial, sweep, conical }) {
        auto [paint, _] = createOptionsMtd();

        canvas->drawRect({0, 0, 100, 100}, paint);
    }
}

} // anonymous namespace

// This test precompiles all four flavors of gradient sequentially but on multiple
// threads with the goal of creating cache races.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedPrecompileTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    constexpr int kNumThreads = 4;

    std::thread threads[kNumThreads];
    for (int i = 0; i < kNumThreads; ++i) {
        threads[i] = std::thread([context, reporter, i]() {
            precompile_gradients(context, reporter, i);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();

    // Four types of gradient times three combinations (i.e., 4,8,N) for each one.
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == 12);
    REPORTER_ASSERT(reporter, stats.fGraphicsRaces > 0);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheMisses =
                              stats.fGraphicsCacheAdditions + stats.fGraphicsRaces);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedPrecompileTest2,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    constexpr int kNumThreads = 4;

    std::thread threads[kNumThreads];
    for (int i = 0; i < kNumThreads/2; ++i) {
        threads[i] = std::thread([context, reporter, i]() {
            precompile_gradients(context, reporter, i);
        });
    }
    for (int i = kNumThreads/2; i < kNumThreads; ++i) {
        threads[i] = std::thread([context, reporter, i]() {
            compile_gradients(context, reporter, i);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();

    // Four types of gradient times three combinations (i.e., 4,8,N) for each one.
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == 12);
    REPORTER_ASSERT(reporter, stats.fGraphicsRaces > 0);
    REPORTER_ASSERT(reporter, stats.fGraphicsCacheMisses =
                                      stats.fGraphicsCacheAdditions + stats.fGraphicsRaces);
}

#endif // SK_GRAPHITE
