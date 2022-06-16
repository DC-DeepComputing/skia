/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCombinationBuilder.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkFactoryFunctions.h"
#include "src/core/SkPrecompile.h"
#include "tests/graphite/CombinationBuilderTestAccess.h"

#include <array>

using namespace::skgpu::graphite;

namespace {

// For an entirely empty combination builder, both solid color shader and kSrcOver options
// will be added
void empty_test(Context* context, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(context);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

// It is expected that the builder will supply a default solid color shader if no other shader
// option is provided
void no_shader_option_test(Context* context, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(context);

    builder.addOption(SkBlendMode::kSrcOver);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

// It is expected that the builder will supply a default kSrcOver blend mode if no other
// options are added
void no_blend_mode_option_test(Context* context, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(context);

    builder.addOption(SkShaderType::kSolidColor);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 1);
}

void big_test_new(Context* context, skiatest::Reporter* reporter) {

    // paintOptions
    //  |- sweepGrad_0 | blendShader_0
    //  |                     0: linearGrad_0 | solid_0
    //  |                     1: linearGrad_1 | blendShader_1
    //  |                                            0: radGrad_0 | solid_1
    //  |                                            1: imageShader_0
    //  |
    //  |- 4-built-in-blend-modes

    SkPaintOptions paintOptions;

    // first, shaders. First top-level option (sweepGrad_0)
    sk_sp<SkPrecompileShader> sweepGrad_0 = SkPrecompileShaders::SweepGradient();

    std::array<SkBlendMode, 1> blendModes{ SkBlendMode::kSrc };

    std::vector<SkBlendMode> moreBlendModes{ SkBlendMode::kDst };

    // Second top-level option (blendShader_0)
    auto blendShader_0 = SkPrecompileShaders::Blend(
                                SkSpan<SkBlendMode>(blendModes),                // std::array
                                {                                               // initializer_list
                                    SkPrecompileShaders::LinearGradient(),
                                    SkPrecompileShaders::Color()
                                },
                                {
                                    SkPrecompileShaders::LinearGradient(),
                                    SkPrecompileShaders::Blend(
                                            SkSpan<SkBlendMode>(moreBlendModes),// std::vector
                                            {
                                                SkPrecompileShaders::RadialGradient(),
                                                 SkPrecompileShaders::Color()
                                            },
                                            {
                                                  SkPrecompileShaders::Image()
                                            })
                                });

    paintOptions.setShaders({ sweepGrad_0, blendShader_0 });

    SkBlendMode evenMoreBlendModes[] = {
        SkBlendMode::kSrcOver,
        SkBlendMode::kSrc,
        SkBlendMode::kDstOver,
        SkBlendMode::kDst
    };

    // now, blend modes
    paintOptions.setBlendModes(evenMoreBlendModes);                             // c array

//    context->precompile({paintOptions});
}

template <typename T, typename Factory>
static std::vector<sk_sp<T>> create_runtime_combinations(skiatest::Reporter* reporter,
                                                         const char* redCode,
                                                         const char* greenCode,
                                                         const char* combineCode) {
    auto [redEffect, error] = SkRuntimeEffect::MakeForShader(SkString(redCode));
    auto [greenEffect, error2] = SkRuntimeEffect::MakeForShader(SkString(greenCode));

    sk_sp<T> red = Factory(redEffect);
    sk_sp<T> green = Factory(greenEffect);

    auto [combineEffect, error3] = SkRuntimeEffect::MakeForShader(SkString(combineCode));
    REPORTER_ASSERT(reporter, combineEffect);
    sk_sp<T> combineRG = Factory(combineEffect, { red, green });
    sk_sp<T> combineGR = Factory(combineEffect, { green, red });

    return { combineRG, combineGR };
}

void runtime_effects(Context* context, skiatest::Reporter* reporter) {
    // paintOptions
    //  |- combineS
    //  |       0: redS   | greenS
    //  |       1: greenS | redS
    //  |
    //  |- combineCF
    //  |       0: redCF   | greenCF
    //  |       1: greenCF | redCF
    //  |
    //  |- combineB
    //  |       0: redB   | greenB
    //  |       1: greenB | redB

    SkPaintOptions paintOptions;

    // shaders
    {
        static const char* kRedS = R"(half4 main(vec2 fragcoord) { return half4(1, 0, 0, 1); })";
        static const char* kGreenS = R"(half4 main(vec2 fragcoord) { return half4(0, 1, 0, 1); })";

        static const char* kCombineS = R"(
            uniform shader first;
            uniform shader second;
            half4 main(vec2 fragcoords) {
                return first.eval(fragcoords) + second.eval(fragcoords);
            }
        )";

#if 1
        paintOptions.setShaders(
                create_runtime_combinations<SkPrecompileShader, MakePrecompileShader>(reporter,
                                                                                      kRedS,
                                                                                      kGreenS,
                                                                                      kCombineS));
#endif

        auto [redEffect, error1] = SkRuntimeEffect::MakeForShader(SkString(kRedS));
        REPORTER_ASSERT(reporter, redEffect, "%s", error1.c_str());
        auto [greenEffect, error2] = SkRuntimeEffect::MakeForShader(SkString(kGreenS));
        REPORTER_ASSERT(reporter, greenEffect, "%s", error2.c_str());
        auto [combineEffect, error3] = SkRuntimeEffect::MakeForShader(SkString(kCombineS));
        REPORTER_ASSERT(reporter, combineEffect, "%s", error3.c_str());

        sk_sp<SkPrecompileShader> redS = MakePrecompileShader(redEffect);
        sk_sp<SkPrecompileShader> greenS = MakePrecompileShader(greenEffect);

        sk_sp<SkPrecompileShader> combineS_RG = MakePrecompileShader(combineEffect,
                                                                     { redS, greenS });
        sk_sp<SkPrecompileShader> combineS_GR = MakePrecompileShader(combineEffect,
                                                                     { greenS, redS });

        paintOptions.setShaders({ combineS_RG, combineS_GR });
    }

    // color filters
    {
        static const char* kRedCF = R"(half4 main(half4 color) { return half4(1, 0, 0, 1); })";
        static const char* kGreenCF = R"(half4 main(half4 color) { return half4(0, 1, 0, 1); })";

        static const char* kCombineCF = R"(
            uniform colorFilter first;
            uniform colorFilter second;
            half4 main(half4 color) { return first.eval(color) + second.eval(color); }
        )";

        auto [redEffect, error1] = SkRuntimeEffect::MakeForColorFilter(SkString(kRedCF));
        REPORTER_ASSERT(reporter, redEffect, "%s", error1.c_str());
        auto [greenEffect, error2] = SkRuntimeEffect::MakeForColorFilter(SkString(kGreenCF));
        REPORTER_ASSERT(reporter, greenEffect, "%s", error2.c_str());
        auto [combineEffect, error3] = SkRuntimeEffect::MakeForColorFilter(SkString(kCombineCF));
        REPORTER_ASSERT(reporter, combineEffect, "%s", error3.c_str());

        sk_sp<SkPrecompileColorFilter> redCF = MakePrecompileColorFilter(redEffect);
        sk_sp<SkPrecompileColorFilter> greenCF = MakePrecompileColorFilter(greenEffect);

        sk_sp<SkPrecompileColorFilter> combineRG = MakePrecompileColorFilter(combineEffect,
                                                                             { redCF, greenCF });
        sk_sp<SkPrecompileColorFilter> combineGR = MakePrecompileColorFilter(combineEffect,
                                                                             { greenCF, redCF });

        paintOptions.setColorFilters({ combineRG, combineGR });
    }

    // blenders
    {
        static const char* kRedB = R"(
            half4 main(half4 src, half4 dst) { return half4(1, 0, 0, 1); }
        )";
        static const char* kGreenB = R"(
            half4 main(half4 src, half4 dst) { return half4(0, 1, 0, 1); }
        )";

        static const char* kCombineB = R"(
            uniform blender first;
            uniform blender second;
            half4 main(half4 src, half4 dst) { return first.eval(src, dst) + second.eval(src, dst); }
        )";

        auto [redEffect, error1] = SkRuntimeEffect::MakeForBlender(SkString(kRedB));
        REPORTER_ASSERT(reporter, redEffect, "%s", error1.c_str());
        auto [greenEffect, error2] = SkRuntimeEffect::MakeForBlender(SkString(kGreenB));
        REPORTER_ASSERT(reporter, greenEffect, "%s", error2.c_str());
        auto [combineEffect, error3] = SkRuntimeEffect::MakeForBlender(SkString(kCombineB));
        REPORTER_ASSERT(reporter, combineEffect, "%s", error3.c_str());

        sk_sp<SkPrecompileBlender> redB = MakePrecompileBlender(redEffect);
        sk_sp<SkPrecompileBlender> greenB = MakePrecompileBlender(greenEffect);

        sk_sp<SkPrecompileBlender> combineRG = MakePrecompileBlender(combineEffect,
                                                                     { redB, greenB });
        sk_sp<SkPrecompileBlender> combineGR = MakePrecompileBlender(combineEffect,
                                                                     { greenB, redB });

        paintOptions.setBlenders({ combineRG, combineGR });
    }

}

void big_test(Context* context, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(context);

    static constexpr int kMinNumStops = 4;
    static constexpr int kMaxNumStops = 8;

    // The resulting number of combinations are in braces
    //
    // builder {428}
    //  |- {107} sweepGrad_0 {5} | blendShader_0 {102}
    //  |                            0  {6}: linearGrad_0 {5} | solid_0 {1}
    //  |                            1 {17}: linearGrad_1 {5} | blendShader_1 {12}
    //  |                                                         0 {6}: radGrad_0 {5} | solid_1 {1}
    //  |                                                         1 {2}: imageShader_0 {2}
    //  |
    //  |- {4} 4-built-in-blend-modes {4}

    // first, shaders. First top-level option (sweepGrad_0)
    [[maybe_unused]] auto sweepGrad_0 = builder.addOption(SkShaderType::kSweepGradient,
                                                          kMinNumStops, kMaxNumStops);

    // Second top-level option (blendShader_0)
    auto blendShader_0 = builder.addOption(SkShaderType::kBlendShader);

    // first child slot of blendShader_0
    {
        // first option (linearGrad_0)
        [[maybe_unused]] auto linearGrad_0 = blendShader_0.addChildOption(
                0, SkShaderType::kLinearGradient,
                kMinNumStops, kMaxNumStops);

        // second option (solid_0)
        [[maybe_unused]] auto solid_0 = blendShader_0.addChildOption(0,
                                                                     SkShaderType::kSolidColor);
    }

    // second child slot of blendShader_0
    {
        // first option (linearGrad_1)
        {
            [[maybe_unused]] auto linearGrad_1 = blendShader_0.addChildOption(
                    1, SkShaderType::kLinearGradient,
                    kMinNumStops, kMaxNumStops);
        }

        // second option (blendShader_1)
        {
            auto blendShader_1 = blendShader_0.addChildOption(1, SkShaderType::kBlendShader);

            // nested: first child slot of blendShader_1
            {
                // first option (radialGrad_0)
                [[maybe_unused]] auto radialGrad_0 = blendShader_1.addChildOption(
                        0, SkShaderType::kRadialGradient,
                        kMinNumStops, kMaxNumStops);

                // second option (solid_1)
                [[maybe_unused]] auto solid_1 = blendShader_1.addChildOption(
                        0, SkShaderType::kSolidColor);
            }

            // nested: second child slot of blendShader_1
            {
                SkTileModePair tilingOptions[] = {
                        { SkTileMode::kRepeat, SkTileMode::kRepeat },
                        { SkTileMode::kClamp,  SkTileMode::kClamp }
                };

                // only option (imageShader_0)
                [[maybe_unused]] auto imageShader_0 = blendShader_1.addChildOption(
                        1, SkShaderType::kImage, tilingOptions);
            }
        }
    }

    // now, blend modes
    builder.addOption(SkBlendMode::kSrcOver);
    builder.addOption(SkBlendMode::kSrc);
    builder.addOption(SkBlendMode::kDstOver);
    builder.addOption(SkBlendMode::kDst);

    REPORTER_ASSERT(reporter, CombinationBuilderTestAccess::NumCombinations(&builder) == 428);
}

#ifdef SK_DEBUG
void epoch_test(Context* context, skiatest::Reporter* reporter) {
    SkCombinationBuilder builder(context);

    // Check that epochs are updated upon builder reset
    {
        SkCombinationOption solid_0 = builder.addOption(SkShaderType::kSolidColor);

        int optionEpoch = CombinationBuilderTestAccess::Epoch(solid_0);
        REPORTER_ASSERT(reporter, optionEpoch == CombinationBuilderTestAccess::Epoch(builder));

        builder.reset();

        REPORTER_ASSERT(reporter, optionEpoch != CombinationBuilderTestAccess::Epoch(builder));
    }
}
#endif

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(CombinationBuilderTest, reporter, context) {
    //big_test_new(context, reporter);
    runtime_effects(context, reporter);
    return;

    empty_test(context, reporter);
    no_shader_option_test(context, reporter);
    no_blend_mode_option_test(context, reporter);
    big_test(context, reporter);
    SkDEBUGCODE(epoch_test(context, reporter));
}
