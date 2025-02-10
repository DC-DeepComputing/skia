/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/precompile/PrecompileEffectFactories.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/core/SkRuntimeEffectPriv.h"

namespace skiatest::graphite {

using namespace skgpu::graphite;

namespace PrecompileFactories {

//--------------------------------------------------------------------------------------------------
const char* GetAnnulusShaderCode() {
    static const char* kAnnulusShaderString =
            // draw a circle centered at "center" w/ inner and outer radii in "radii"
            "uniform float2 center;"
            "uniform float2 radii;"
            "half4 main(float2 xy) {"
                "float len = length(xy - center);"
                "half value = len < radii.x ? 0.0 : (len > radii.y ? 0.0 : 1.0);"
                "return half4(value);"
            "}";
    return kAnnulusShaderString;
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> CreateAnnulusRuntimeShader() {
    static SkRuntimeEffect* sEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                                          GetAnnulusShaderCode());

    static const float kUniforms[4] = { 50.0f, 50.0f, 40.0f, 50.0f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));

    sk_sp<SkShader> s = sEffect->makeShader(std::move(uniforms), /* children= */ {});
    sk_sp<PrecompileShader> o = PrecompileRuntimeEffects::MakePrecompileShader(sk_ref_sp(sEffect));
    return { std::move(s), std::move(o) };
}

//--------------------------------------------------------------------------------------------------
const char* GetSrcBlenderCode() {
    static const char* kSrcBlenderString =
        "half4 main(half4 src, half4 dst) {"
            "return src;"
        "}";
    return kSrcBlenderString;
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> CreateSrcRuntimeBlender() {
    static SkRuntimeEffect* sSrcEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForBlender,
                                                             GetSrcBlenderCode());

    sk_sp<SkBlender> b = sSrcEffect->makeBlender(/* uniforms= */ nullptr);
    sk_sp<PrecompileBlender> o =
            PrecompileRuntimeEffects::MakePrecompileBlender(sk_ref_sp(sSrcEffect));
    return { std::move(b) , std::move(o) };
}

const char* GetDstBlenderCode() {
    static const char* kDstBlenderString =
        "half4 main(half4 src, half4 dst) {"
            "return dst;"
        "}";
    return kDstBlenderString;
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> CreateDstRuntimeBlender() {
    static SkRuntimeEffect* sDestEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForBlender,
                                                              GetDstBlenderCode());

    sk_sp<SkBlender> b = sDestEffect->makeBlender(/* uniforms= */ nullptr);
    sk_sp<PrecompileBlender> o =
            PrecompileRuntimeEffects::MakePrecompileBlender(sk_ref_sp(sDestEffect));
    return { std::move(b) , std::move(o) };
}

const char* GetComboBlenderCode() {
    static const char* kComboBlenderString =
        "uniform float blendFrac;"
        "uniform blender a;"
        "uniform blender b;"
        "half4 main(half4 src, half4 dst) {"
            "return (blendFrac * a.eval(src, dst)) + ((1 - blendFrac) * b.eval(src, dst));"
        "}";
    return kComboBlenderString;
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> CreateComboRuntimeBlender() {
    static SkRuntimeEffect* sComboEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForBlender,
                                                               GetComboBlenderCode());

    auto [src, srcO] = CreateSrcRuntimeBlender();
    auto [dst, dstO] = CreateDstRuntimeBlender();

    SkRuntimeEffect::ChildPtr children[] = { src, dst };

    const float kUniforms[] = { 1.0f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));
    sk_sp<SkBlender> b = sComboEffect->makeBlender(std::move(uniforms), children);
    sk_sp<PrecompileBlender> o = PrecompileRuntimeEffects::MakePrecompileBlender(
            sk_ref_sp(sComboEffect),
            { { srcO }, { dstO } });
    return { std::move(b) , std::move(o) };
}

//--------------------------------------------------------------------------------------------------
const char* GetDoubleColorFilterCode() {
    static const char* kDoubleColorFilterString =
        "half4 main(half4 c) {"
            "return 2*c;"
        "}";
    return kDoubleColorFilterString;
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> CreateDoubleRuntimeColorFilter() {
    static SkRuntimeEffect* sSrcEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                                             GetDoubleColorFilterCode());

    return { sSrcEffect->makeColorFilter(/* uniforms= */ nullptr),
             PrecompileRuntimeEffects::MakePrecompileColorFilter(sk_ref_sp(sSrcEffect)) };
}

const char* GetHalfColorFilterCode() {
    static const char* kHalfColorFilterString =
        "half4 main(half4 c) {"
            "return 0.5*c;"
        "}";
    return kHalfColorFilterString;
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> CreateHalfRuntimeColorFilter() {
    static SkRuntimeEffect* sDestEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                                              GetHalfColorFilterCode());

    return { sDestEffect->makeColorFilter(/* uniforms= */ nullptr),
             PrecompileRuntimeEffects::MakePrecompileColorFilter(sk_ref_sp(sDestEffect)) };
}

const char* GetComboColorFilterCode() {
    static const char* kComboColorFilterString =
        "uniform float blendFrac;"
        "uniform colorFilter a;"
        "uniform colorFilter b;"
        "half4 main(half4 c) {"
            "return (blendFrac * a.eval(c)) + ((1 - blendFrac) * b.eval(c));"
        "}";
    return kComboColorFilterString;
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> CreateComboRuntimeColorFilter() {
    static SkRuntimeEffect* sComboEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                                               GetComboColorFilterCode());

    auto [src, srcO] = CreateDoubleRuntimeColorFilter();
    auto [dst, dstO] = CreateHalfRuntimeColorFilter();

    SkRuntimeEffect::ChildPtr children[] = { src, dst };

    const float kUniforms[] = { 0.5f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));
    sk_sp<SkColorFilter> cf = sComboEffect->makeColorFilter(std::move(uniforms), children);
    sk_sp<PrecompileColorFilter> o =
            PrecompileRuntimeEffects::MakePrecompileColorFilter(sk_ref_sp(sComboEffect),
                                                                { { srcO }, { dstO } });
    return { std::move(cf) , std::move(o) };
}

} // namespace PrecompileFactories

} // namespace skiatest::graphite
