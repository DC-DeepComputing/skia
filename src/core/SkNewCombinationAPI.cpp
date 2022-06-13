/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkNewCombinationAPI.h"

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
    SkBlendPrecompileShader(SkSpan<sk_sp<SkPrecompileBlender>> blenders,
                            SkSpan<sk_sp<SkPrecompileShader>> dsts,
                            SkSpan<sk_sp<SkPrecompileShader>> srcs)
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
        SkSpan<sk_sp<SkPrecompileBlender>> blenders,
        SkSpan<sk_sp<SkPrecompileShader>> dsts,
        SkSpan<sk_sp<SkPrecompileShader>> srcs) {
    return sk_make_sp<SkBlendPrecompileShader>(std::move(blenders),
                                               std::move(dsts), std::move(srcs));
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::Blend(
        SkSpan<SkBlendMode> blendModes,
        SkSpan<sk_sp<SkPrecompileShader>> dsts,
        SkSpan<sk_sp<SkPrecompileShader>> srcs) {
    std::vector<sk_sp<SkPrecompileBlender>> tmp;
    tmp.reserve(blendModes.size());
    for (SkBlendMode bm : blendModes) {
        tmp.emplace_back(SkPrecompileBlender::Mode(bm));
    }

    return sk_make_sp<SkBlendPrecompileShader>(SkMakeSpan(tmp),
                                               std::move(dsts), std::move(srcs));
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::Image() {
    return sk_make_sp<SkPrecompileShader>();
}

//--------------------------------------------------------------------------------------------------
sk_sp<SkPrecompileShader> SkPrecompileGradientShader::MakeLinear() {
    return sk_make_sp<SkPrecompileShader>();
}

sk_sp<SkPrecompileShader> SkPrecompileGradientShader::MakeRadial() {
    return sk_make_sp<SkPrecompileShader>();
}

sk_sp<SkPrecompileShader> SkPrecompileGradientShader::MakeTwoPointConical() {
    return sk_make_sp<SkPrecompileShader>();
}

sk_sp<SkPrecompileShader> SkPrecompileGradientShader::MakeSweep() {
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
