/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PaintOptions.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/FactoryFunctionsPriv.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/precompile/PaintOption.h"
#include "src/gpu/graphite/precompile/PaintOptionsPriv.h"

namespace skgpu::graphite {

PaintOptions::PaintOptions() = default;
PaintOptions::PaintOptions(const PaintOptions&) = default;
PaintOptions::~PaintOptions() = default;
PaintOptions& PaintOptions::operator=(const PaintOptions&) = default;

void PaintOptions::setShaders(SkSpan<const sk_sp<PrecompileShader>> shaders) {
    fShaderOptions.clear();
    fShaderOptions.push_back_n(shaders.size(), shaders.data());
}

void PaintOptions::setImageFilters(SkSpan<const sk_sp<PrecompileImageFilter>> imageFilters) {
    fImageFilterOptions.clear();
    fImageFilterOptions.push_back_n(imageFilters.size(), imageFilters.data());
}

void PaintOptions::setMaskFilters(SkSpan<const sk_sp<PrecompileMaskFilter>> maskFilters) {
    fMaskFilterOptions.clear();
    fMaskFilterOptions.push_back_n(maskFilters.size(), maskFilters.data());
}

void PaintOptions::setColorFilters(SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters) {
    fColorFilterOptions.clear();
    fColorFilterOptions.push_back_n(colorFilters.size(), colorFilters.data());
}

void PaintOptions::addColorFilter(sk_sp<PrecompileColorFilter> cf) {
    fColorFilterOptions.push_back(std::move(cf));
}

void PaintOptions::setBlenders(SkSpan<const sk_sp<PrecompileBlender>> blenders) {
    for (const sk_sp<PrecompileBlender>& b: blenders) {
        if (b->asBlendMode().has_value()) {
            fBlendModeOptions.push_back(b->asBlendMode().value());
        } else {
            fBlenderOptions.push_back(b);
        }
    }
}

void PaintOptions::setClipShaders(SkSpan<const sk_sp<PrecompileShader>> clipShaders) {
    // In the normal API this modification happens in SkDevice::clipShader()
    fClipShaderOptions.reserve(2 * clipShaders.size());
    for (const sk_sp<PrecompileShader>& cs : clipShaders) {
        // All clipShaders get wrapped in a CTMShader ...
        sk_sp<PrecompileShader> withCTM = cs ? PrecompileShadersPriv::CTM({ cs }) : nullptr;
        // and, if it is a SkClipOp::kDifference clip, an additional ColorFilterShader
        sk_sp<PrecompileShader> inverted =
                withCTM ? withCTM->makeWithColorFilter(PrecompileColorFilters::Blend())
                        : nullptr;

        fClipShaderOptions.emplace_back(std::move(withCTM));
        fClipShaderOptions.emplace_back(std::move(inverted));
    }
}

int PaintOptions::numShaderCombinations() const {
    int numShaderCombinations = 0;
    for (const sk_sp<PrecompileShader>& s : fShaderOptions) {
        numShaderCombinations += s->numCombinations();
    }

    // If no shader option is specified we will add a solid color shader option
    return numShaderCombinations ? numShaderCombinations : 1;
}

int PaintOptions::numColorFilterCombinations() const {
    int numColorFilterCombinations = 0;
    for (const sk_sp<PrecompileColorFilter>& cf : fColorFilterOptions) {
        if (!cf) {
            ++numColorFilterCombinations;
        } else {
            numColorFilterCombinations += cf->numCombinations();
        }
    }

    // If no color filter options are specified we will use the unmodified result color
    return numColorFilterCombinations ? numColorFilterCombinations : 1;
}

int PaintOptions::numBlendCombinations() const {
    int numBlendCombos = fBlendModeOptions.size();
    for (const sk_sp<PrecompileBlender>& b: fBlenderOptions) {
        SkASSERT(!b->asBlendMode().has_value());
        numBlendCombos += b->numChildCombinations();
    }

    if (!numBlendCombos) {
        // If the user didn't specify a blender we will fall back to kSrcOver blending
        numBlendCombos = 1;
    }

    return numBlendCombos;
}

int PaintOptions::numClipShaderCombinations() const {
    int numClipShaderCombos = 0;
    for (const sk_sp<PrecompileShader>& cs: fClipShaderOptions) {
        if (cs) {
            numClipShaderCombos += cs->numChildCombinations();
        } else {
            ++numClipShaderCombos;
        }
    }

    // If no clipShader options are specified we will just have the unclipped options
    return numClipShaderCombos ? numClipShaderCombos : 1;
}

int PaintOptions::numCombinations() const {
    return this->numShaderCombinations() *
           this->numColorFilterCombinations() *
           this->numBlendCombinations() *
           this->numClipShaderCombinations();
}

namespace {

DstReadRequirement get_dst_read_req(const Caps* caps,
                                    Coverage coverage,
                                    PrecompileBlender* blender) {
    if (blender) {
        return GetDstReadRequirement(caps, blender->asBlendMode(), coverage);
    }
    return GetDstReadRequirement(caps, SkBlendMode::kSrcOver, coverage);
}

} // anonymous namespace

void PaintOptions::createKey(const KeyContext& keyContext,
                             PaintParamsKeyBuilder* keyBuilder,
                             PipelineDataGatherer* gatherer,
                             int desiredCombination,
                             bool addPrimitiveBlender,
                             Coverage coverage) const {
    SkDEBUGCODE(keyBuilder->checkReset();)
    SkASSERT(desiredCombination < this->numCombinations());

    const int numClipShaderCombos = this->numClipShaderCombinations();
    const int numBlendModeCombos = this->numBlendCombinations();
    const int numColorFilterCombinations = this->numColorFilterCombinations();

    const int desiredClipShaderCombination = desiredCombination % numClipShaderCombos;
    int remainingCombinations = desiredCombination / numClipShaderCombos;

    const int desiredBlendCombination = remainingCombinations % numBlendModeCombos;
    remainingCombinations /= numBlendModeCombos;

    const int desiredColorFilterCombination = remainingCombinations % numColorFilterCombinations;
    remainingCombinations /= numColorFilterCombinations;

    const int desiredShaderCombination = remainingCombinations;
    SkASSERT(desiredShaderCombination < this->numShaderCombinations());

    // TODO: this probably needs to be passed in just like addPrimitiveBlender
    const bool kOpaquePaintColor = true;

    auto clipShader = PrecompileBase::SelectOption(SkSpan(fClipShaderOptions),
                                                   desiredClipShaderCombination);

    std::pair<sk_sp<PrecompileBlender>, int> finalBlender;
    if (desiredBlendCombination < fBlendModeOptions.size()) {
        finalBlender = { PrecompileBlender::Mode(fBlendModeOptions[desiredBlendCombination]), 0 };
    } else {
        finalBlender = PrecompileBase::SelectOption(
                            SkSpan(fBlenderOptions),
                            desiredBlendCombination - fBlendModeOptions.size());
    }
    if (!finalBlender.first) {
        finalBlender = { PrecompileBlender::Mode(SkBlendMode::kSrcOver), 0 };
    }
    DstReadRequirement dstReadReq = get_dst_read_req(keyContext.caps(), coverage,
                                                     finalBlender.first.get());

    PaintOption option(kOpaquePaintColor,
                       finalBlender,
                       PrecompileBase::SelectOption(SkSpan(fShaderOptions),
                                                    desiredShaderCombination),
                       PrecompileBase::SelectOption(SkSpan(fColorFilterOptions),
                                                    desiredColorFilterCombination),
                       addPrimitiveBlender,
                       clipShader,
                       dstReadReq,
                       fDither);

    option.toKey(keyContext, keyBuilder, gatherer);
}

} // namespace skgpu::graphite
