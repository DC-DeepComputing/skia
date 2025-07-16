/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/precompile/AndroidRuntimeEffectManager.h"

#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"

namespace {

// Note: passing in a name to 'makeEffect' is a difference from Android's factory functions.
sk_sp<SkRuntimeEffect> makeEffect(const SkString& sksl, const char* name) {
    SkRuntimeEffect::Options options;
    options.fName = name;

    auto [effect, error] = SkRuntimeEffect::MakeForShader(sksl, options);
    if (!effect) {
        SkDebugf("%s\n", error.c_str());
    }
    return effect;
}

} // anonymous namespace

RuntimeEffectManager::RuntimeEffectManager() {
    static const SkString kMixCode(R"(
        uniform shader img1;
        uniform shader img2;

        half4 main(float2 xy) {
            return half4(mix(img1.eval(xy), img2.eval(xy), 0.5)).rgb1;
        }
    )");

    fMixEffect = makeEffect(kMixCode, "RE_BlurFilter_MixEffect");

    // The following code block is just a stub for the Android code. For Skia's testing
    // purposes it only needs to have the same name and number of children as the real code.
    // When the following PaintOptions are used in Android the real SkSL must be supplied.
    static const SkString kEdgeExtensionCode(R"(
        uniform shader img;

        vec4 main(vec2 xy) {
            float3 sample = img.eval(0.115 * xy).rgb;
            return float4(sample, 1.0);
        }
    )");

    fEdgeExtensionEffect = makeEffect(kEdgeExtensionCode, "RE_EdgeExtensionEffect");

    static const SkString kHighSampleBlurCode(R"(
        uniform shader img;

        half4 main(float2 xy) {
            half3 c = img.eval(0.6 * xy).rgb;
            return half4(c * 0.5, 1.0);
        }
    )");

    fKawaseHighSampleEffect = makeEffect(kHighSampleBlurCode,
                                         "RE_KawaseBlurDualFilter_HighSampleBlurEffect");

    static const SkString kLowSampleBlurCode(R"(
        uniform shader img;

        half4 main(float2 xy) {
            half3 c = img.eval(0.55 * xy).rgb;
            return half4(c, 1.0);
        }
    )");

    fKawaseLowSampleEffect = makeEffect(kLowSampleBlurCode,
                                        "RE_KawaseBlurDualFilter_LowSampleBlurEffect");

    static const SkString kBlurCode(R"(
        uniform shader img;
        vec4 main(vec2 xy) {
            return float4(img.eval(0.4 * xy).rgb, 0.0);
        }
    )");

    fBlurEffect = makeEffect(kBlurCode, "RE_MouriMap_BlurEffect");

    // The following code blocks are just stubs for the Android code. For Skia's testing
    // purposes they only need to have the same name and number of children as the real code.
    // When the following PaintOptions are used in Android the real SkSL must be supplied.
    static const SkString kCrosstalkAndChunk16x16Code(R"(
        uniform shader img;
        vec4 main(vec2 xy) {
            float3 linear = img.eval(0.25 * xy).rgb;
            return float4(linear, 1.0);
        }
    )");

    fCrosstalkAndChunk16x16Effect = makeEffect(kCrosstalkAndChunk16x16Code,
                                               "RE_MouriMap_CrossTalkAndChunk16x16Effect");

    static const SkString kChunk8x8Code(R"(
        uniform shader img;
        vec4 main(vec2 xy) {
            return float4(img.eval(0.33 * xy).rgb, 1.0);
        }
    )");

    fChunk8x8Effect = makeEffect(kChunk8x8Code, "RE_MouriMap_Chunk8x8Effect");

    static const SkString kTonemapCode(R"(
        uniform shader image;
        uniform shader lux;
        vec4 main(vec2 xy) {
            float localMax = lux.eval(xy * 0.4).r;
            float4 rgba = image.eval(0.5 * xy);
            float3 linear = rgba.rgb * 0.7;

            return float4(linear, rgba.a);
        }
    )");

    fToneMapEffect = makeEffect(kTonemapCode, "RE_MouriMap_TonemapEffect");
}

sk_sp<SkRuntimeEffect> RuntimeEffectManager::getKnownRuntimeEffect(KnownId id) {

    switch (id) {
        case KnownId::kBlurFilter_MixEffect:
            return fMixEffect;
        case KnownId::kEdgeExtensionEffect:
            return fEdgeExtensionEffect;
        case KnownId::kKawaseBlurDualFilter_HighSampleBlurEffect:
            return fKawaseHighSampleEffect;
        case KnownId::kKawaseBlurDualFilter_LowSampleBlurEffect:
            return fKawaseLowSampleEffect;
        case KnownId::kMouriMap_BlurEffect:
            return fBlurEffect;
        case KnownId::kMouriMap_CrossTalkAndChunk16x16Effect:
            return fCrosstalkAndChunk16x16Effect;
        case KnownId::kMouriMap_Chunk8x8Effect:
            return fChunk8x8Effect;
        case KnownId::kMouriMap_TonemapEffect:
            return fToneMapEffect;
    }

    return nullptr;
}

namespace {

const char* to_str(ui::Dataspace ds) {
    switch (ds) {
        case ui::Dataspace::UNKNOWN:       return "UNKNOWN";
        case ui::Dataspace::SRGB:          return "SRGB";
        case ui::Dataspace::BT2020:        return "BT2020";
        case ui::Dataspace::BT2020_ITU_PQ: return "BT2020_ITU_PQ";
        case ui::Dataspace::DISPLAY_P3:    return "DISPLAY_P3";
        case ui::Dataspace::V0_SRGB:       return "V0_SRGB";
    }
}

const char* to_str(shaders::LinearEffect::SkSLType type) {
    switch (type) {
        case shaders::LinearEffect::SkSLType::Shader: return "Shader";
    }
}

} // anonymous namespace

sk_sp<SkRuntimeEffect> RuntimeEffectManager::getOrCreateLinearRuntimeEffect(foo1 bar) {

    SkString name = SkStringPrintf("RE_LinearEffect_%s__%s__$s__%s__%s",
                                   to_str(bar.inputDataspace),
                                   to_str(bar.outputDataspace),
                                   bar.undoPremultipliedAlpha ? "true" : "false",
                                   to_str(bar.fakeOutputDataspace),
                                   to_str(bar.type));

    auto result = fLinearEffects.find(name.c_str());
    if (result != fLinearEffects.end()) {
        return result->second;
    }

    // Each code snippet must be unique, otherwise Skia will internally find a match
    // and uniquify things. To avoid this we just add an arbitrary alpha constant
    // to the code.
    static float arbitraryAlpha = 0.051f;
    SkString linearEffectCode = SkStringPrintf(
            "uniform shader child;"
            "vec4 main(vec2 xy) {"
            "float3 linear = toLinearSrgb(child.eval(xy).rgb);"
            "return float4(fromLinearSrgb(linear), %f);"
            "}",
            arbitraryAlpha);
    arbitraryAlpha += 0.05f;

    sk_sp<SkRuntimeEffect> linearEffect = makeEffect(linearEffectCode, name.c_str());

    fLinearEffects.insert({ name.c_str(), linearEffect });
    return linearEffect;
}
