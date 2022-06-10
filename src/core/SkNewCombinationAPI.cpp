/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkNewCombinationAPI.h"

namespace {

std::vector<sk_sp<SkPrecompileBlender>> to_blenders(const std::vector<SkBlendMode>& blendModes) {
    std::vector<sk_sp<SkPrecompileBlender>> blenders;
    blenders.reserve(blendModes.size());

    for (SkBlendMode blendMode: blendModes) {
        blenders.push_back(SkPrecompileBlender::Mode(blendMode));
    }
    return blenders;
}

} // anonymous namespace

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
void PaintOptions::setBlendModes(std::vector<SkBlendMode> blendModes) {
    this->setBlenders(to_blenders(blendModes));
}

//--------------------------------------------------------------------------------------------------
class SkBlendPrecompileShader : public SkPrecompileShader {
public:
    SkBlendPrecompileShader(std::vector<sk_sp<SkPrecompileBlender>> blenders,
                            std::vector<sk_sp<SkPrecompileShader>> dsts,
                            std::vector<sk_sp<SkPrecompileShader>> srcs)
            : fBlenders(blenders)
            , fDsts(dsts)
            , fSrcs(srcs) {
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

sk_sp<SkPrecompileShader> SkPrecompileShaders::Blend(std::vector<sk_sp<SkPrecompileBlender>> blenders,
                                                     std::vector<sk_sp<SkPrecompileShader>> dsts,
                                                     std::vector<sk_sp<SkPrecompileShader>> srcs) {
    return sk_make_sp<SkBlendPrecompileShader>(std::move(blenders),
                                               std::move(dsts), std::move(srcs));
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::Blend(std::vector<SkBlendMode> blendModes,
                                                     std::vector<sk_sp<SkPrecompileShader>> dsts,
                                                     std::vector<sk_sp<SkPrecompileShader>> srcs) {
    return Blend(to_blenders(blendModes), std::move(dsts), std::move(srcs));
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
