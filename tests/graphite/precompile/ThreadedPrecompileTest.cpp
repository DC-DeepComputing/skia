/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

#include <thread>

using namespace::skgpu::graphite;

PaintOptions option1() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions option2() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::RadialGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions option3() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::SweepGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions option4() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::TwoPointConicalGradient() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

RenderPassProperties bgra_1_depth() {
    return { DepthStencilFlags::kDepth, kBGRA_8888_SkColorType, /* requiresMSAA= */ false };
}


void run_test(Context* context, skiatest::Reporter* reporter) {
    constexpr RenderPassProperties kProps = { DepthStencilFlags::kDepth,
                                              kBGRA_8888_SkColorType,
                                              /* requiresMSAA= */ false };


    for (auto foo : { option1, option2, option3, option4 }) {
        PaintOptions paintOptions = foo();
        Precompile(context,
                   paintOptions,
                   DrawTypeFlags::kBitmapText_Mask,
                   { &kProps, 1 });
    }
}


DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ThreadedPrecompileTest,
                                               reporter,
                                               context,
                                               testContext,
                                               true,
                                               CtsEnforcement::kNever) {
    std::thread threads[16];
    for (std::thread& thread : threads) {
        thread = std::thread([context, reporter]() {
            run_test(context, reporter);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}
