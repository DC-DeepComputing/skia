/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkFactoryFunctions.h"

#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkPrecompile.h"

//--------------------------------------------------------------------------------------------------
class SkPrecompileBlendModeBlender : public SkPrecompileBlender {
public:
    SkPrecompileBlendModeBlender(SkBlendMode blendMode) : fBlendMode(blendMode) {}

    std::optional<SkBlendMode> asBlendMode() const final { return fBlendMode; }

private:
    SkBlendMode fBlendMode;
};

sk_sp<SkPrecompileBlender> SkPrecompileBlender::Mode(SkBlendMode blendMode) {
    return sk_make_sp<SkPrecompileBlendModeBlender>(blendMode);
}

//--------------------------------------------------------------------------------------------------
class SkBlendPrecompileShader : public SkPrecompileShader {
public:
    SkBlendPrecompileShader(SkSpan<const sk_sp<SkPrecompileBlender>> blenders,
                            SkSpan<const sk_sp<SkPrecompileShader>> dsts,
                            SkSpan<const sk_sp<SkPrecompileShader>> srcs)
            : fBlenders(blenders.begin(), blenders.end())
            , fDsts(dsts.begin(), dsts.end())
            , fSrcs(srcs.begin(), srcs.end()) {
    }

private:
    std::vector<sk_sp<SkPrecompileBlender>> fBlenders;
    std::vector<sk_sp<SkPrecompileShader>> fDsts;
    std::vector<sk_sp<SkPrecompileShader>> fSrcs;
};

//--------------------------------------------------------------------------------------------------
sk_sp<SkPrecompileShader> SkPrecompileShaders::Color() {
    return sk_make_sp<SkPrecompileShader>();
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::Blend(
        SkSpan<const sk_sp<SkPrecompileBlender>> blenders,
        SkSpan<const sk_sp<SkPrecompileShader>> dsts,
        SkSpan<const sk_sp<SkPrecompileShader>> srcs) {
    return sk_make_sp<SkBlendPrecompileShader>(std::move(blenders),
                                               std::move(dsts), std::move(srcs));
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::Blend(
        SkSpan<SkBlendMode> blendModes,
        SkSpan<const sk_sp<SkPrecompileShader>> dsts,
        SkSpan<const sk_sp<SkPrecompileShader>> srcs) {
    std::vector<sk_sp<SkPrecompileBlender>> tmp;
    tmp.reserve(blendModes.size());
    for (SkBlendMode bm : blendModes) {
        tmp.emplace_back(SkPrecompileBlender::Mode(bm));
    }

    return sk_make_sp<SkBlendPrecompileShader>(tmp, std::move(dsts), std::move(srcs));
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::Image() {
    return sk_make_sp<SkPrecompileShader>();
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::LinearGradient() {
    return sk_make_sp<SkPrecompileShader>();
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::RadialGradient() {
    return sk_make_sp<SkPrecompileShader>();
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::TwoPointConicalGradient() {
    return sk_make_sp<SkPrecompileShader>();
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::SweepGradient() {
    return sk_make_sp<SkPrecompileShader>();
}

//--------------------------------------------------------------------------------------------------
sk_sp<SkPrecompileMaskFilter> SkPrecompileMaskFilters::Blur() {
    return sk_make_sp<SkPrecompileMaskFilter>();
}

//--------------------------------------------------------------------------------------------------
sk_sp<SkPrecompileColorFilter> SkPrecompileColorFilters::Matrix() {
    return sk_make_sp<SkPrecompileColorFilter>();
}

//--------------------------------------------------------------------------------------------------
sk_sp<SkPrecompileImageFilter> SkPrecompileImageFilters::Blur() {
    return sk_make_sp<SkPrecompileImageFilter>();
}

sk_sp<SkPrecompileImageFilter> SkPrecompileImageFilters::Image() {
    return sk_make_sp<SkPrecompileImageFilter>();
}

//--------------------------------------------------------------------------------------------------
template<typename T>
class SkPrecompileRTEffect : public T {
public:
    SkPrecompileRTEffect(sk_sp<SkRuntimeEffect> effect,
                         SkSpan<sk_sp<T>> children)
            : fEffect(std::move(effect))
            , fChildren(children.begin(), children.end()) {
    }

private:
    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<sk_sp<T>> fChildren;
};

sk_sp<SkPrecompileShader> MakeShader(sk_sp<SkRuntimeEffect> effect,
                                     SkSpan<sk_sp<SkPrecompileShader>> children) {
    return sk_make_sp<SkPrecompileRTEffect<SkPrecompileShader>>(std::move(effect), children);
}

sk_sp<SkPrecompileColorFilter> MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<sk_sp<SkPrecompileColorFilter>> children) {
    return sk_make_sp<SkPrecompileRTEffect<SkPrecompileColorFilter>>(std::move(effect), children);
}

sk_sp<SkPrecompileBlender> MakePrecompileBlender(sk_sp<SkRuntimeEffect> effect,
                                                 SkSpan<sk_sp<SkPrecompileBlender>> children) {
    return sk_make_sp<SkPrecompileRTEffect<SkPrecompileBlender>>(std::move(effect), children);
}

//--------------------------------------------------------------------------------------------------
