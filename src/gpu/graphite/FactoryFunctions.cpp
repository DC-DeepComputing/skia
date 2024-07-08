/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/FactoryFunctions.h"

#include "include/gpu/graphite/precompile/PrecompileBase.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"
#include "src/gpu/graphite/precompile/PrecompileBasePriv.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShaderPriv.h"

namespace skgpu::graphite {

#if 0
namespace {

#ifdef SK_DEBUG

bool precompilebase_is_valid_as_child(const PrecompileBase *child) {
    if (!child) {
        return true;
    }

    switch (child->type()) {
        case PrecompileBase::Type::kShader:
        case PrecompileBase::Type::kColorFilter:
        case PrecompileBase::Type::kBlender:
            return true;
        default:
            return false;
    }
}

#endif // SK_DEBUG

} // anonymous namespace
#endif

//--------------------------------------------------------------------------------------------------
class PrecompileYUVImageShader : public PrecompileShader {
public:
    PrecompileYUVImageShader() {}

private:
    // non-cubic and cubic sampling
    inline static constexpr int kNumIntrinsicCombinations = 2;

    int numIntrinsicCombinations() const override { return kNumIntrinsicCombinations; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < kNumIntrinsicCombinations);

        static constexpr SkSamplingOptions kDefaultCubicSampling(SkCubicResampler::Mitchell());
        static constexpr SkSamplingOptions kDefaultSampling;

        YUVImageShaderBlock::ImageData imgData(desiredCombination == 1 ? kDefaultCubicSampling
                                                                       : kDefaultSampling,
                                               SkTileMode::kClamp, SkTileMode::kClamp,
                                               SkISize::MakeEmpty(), SkRect::MakeEmpty());

        YUVImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
    }
};

sk_sp<PrecompileShader> PrecompileShaders::YUVImage() {
    return sk_make_sp<PrecompileYUVImageShader>();
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
#if 0
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileShader> s) : fChild(std::move(s)) {}
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileColorFilter> cf)
        : fChild(std::move(cf)) {
}
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileBlender> b) : fChild(std::move(b)) {}

PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileBase> child)
        : fChild(std::move(child)) {
    SkASSERT(precompilebase_is_valid_as_child(fChild.get()));
}

std::optional<SkRuntimeEffect::ChildType> PrecompileChildPtr::type() const {
    if (fChild) {
        switch (fChild->type()) {
            case PrecompileBase::Type::kShader:
                return SkRuntimeEffect::ChildType::kShader;
            case PrecompileBase::Type::kColorFilter:
                return SkRuntimeEffect::ChildType::kColorFilter;
            case PrecompileBase::Type::kBlender:
                return SkRuntimeEffect::ChildType::kBlender;
            default:
                break;
        }
    }
    return std::nullopt;
}

PrecompileShader* PrecompileChildPtr::shader1() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kShader)
           ? static_cast<PrecompileShader*>(fChild.get())
           : nullptr;
}

PrecompileColorFilter* PrecompileChildPtr::colorFilter1() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kColorFilter)
           ? static_cast<PrecompileColorFilter*>(fChild.get())
           : nullptr;
}

PrecompileBlender* PrecompileChildPtr::blender1() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kBlender)
           ? static_cast<PrecompileBlender*>(fChild.get())
           : nullptr;
}
#endif

//--------------------------------------------------------------------------------------------------
namespace {

int num_options_in_set2(const SkSpan<const sk_sp<PrecompileBase>>& optionSet) {
    int numOptions = 0;
    for (const sk_sp<PrecompileBase>& childOption : optionSet) {
        // A missing child will fall back to a passthrough object
        if (childOption) {
            numOptions += childOption->priv().numCombinations();
        } else {
            ++numOptions;
        }
    }

    return numOptions;
}

} // anonymous namespace

template<typename T>
class PrecompileRTEffect : public T {
public:
    PrecompileRTEffect(sk_sp<SkRuntimeEffect> effect,
                       SkSpan<const PrecompileChildOptions> childOptions)
            : fEffect(std::move(effect)) {
        fChildOptions1.reserve(childOptions.size());
        for (PrecompileChildOptions c : childOptions) {
            fChildOptions1.push_back({ c.begin(), c.end() });
        }

        fNumSlotCombinations.reserve(childOptions.size());
        fNumChildCombinations = 1;
        for (const std::vector<sk_sp<PrecompileBase>>& optionSet : fChildOptions1) {
            fNumSlotCombinations.push_back(num_options_in_set2(optionSet));
            fNumChildCombinations *= fNumSlotCombinations.back();
        }

        SkASSERT(fChildOptions1.size() == fEffect->children().size());
    }

private:
    int numChildCombinations() const override { return fNumChildCombinations; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {

        SkASSERT(desiredCombination < this->numCombinations());

        SkSpan<const SkRuntimeEffect::Child> childInfo = fEffect->children();

        RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer, { fEffect });

        KeyContextWithScope childContext(keyContext, KeyContext::Scope::kRuntimeEffect);

        int remainingCombinations = desiredCombination;

        for (size_t rowIndex = 0; rowIndex < fChildOptions1.size(); ++rowIndex) {
            const std::vector<sk_sp<PrecompileBase>>& slotOptions = fChildOptions1[rowIndex];
            int numSlotCombinations = fNumSlotCombinations[rowIndex];

            const int slotOption = remainingCombinations % numSlotCombinations;
            remainingCombinations /= numSlotCombinations;

            auto [option, childOptions] = PrecompileBase::SelectOption(
                    SkSpan<const sk_sp<PrecompileBase>>(slotOptions),
                    slotOption);
            if (option) {
                option->priv().addToKey(keyContext, builder, gatherer, childOptions);
            } else {
                SkASSERT(childOptions == 0);

                // We don't have a child effect. Substitute in a no-op effect.
                switch (childInfo[rowIndex].type) {
                    case SkRuntimeEffect::ChildType::kShader:
                        // A missing shader returns transparent black
                        SolidColorShaderBlock::AddBlock(childContext, builder, gatherer,
                                                        SK_PMColor4fTRANSPARENT);
                        break;

                    case SkRuntimeEffect::ChildType::kColorFilter:
                        // A "passthrough" shader returns the input color as-is.
                        builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
                        break;

                    case SkRuntimeEffect::ChildType::kBlender:
                        // A "passthrough" blender performs `blend_src_over(src, dest)`.
                        AddKnownModeBlend(childContext, builder, gatherer, SkBlendMode::kSrcOver);
                        break;
                }
            }
        }

#if 0
        for (size_t index = 0; index < optionSet.size(); ++index) {
            const PrecompileChildPtr& childOption = optionSet[index];

            const int numChildCombos = childOption.base() ? childOption.base()->priv().numCombinations()
                                                          : 1;
            const int curCombo = remainingCombinations % numChildCombos;
            remainingCombinations /= numChildCombos;

            std::optional<ChildType> type = childOption.type();
            if (type == ChildType::kShader) {
                childOption.shader()->priv().addToKey(childContext, builder, gatherer, curCombo);
            } else if (type == ChildType::kColorFilter) {
                childOption.colorFilter()->priv().addToKey(childContext, builder, gatherer, curCombo);
            } else if (type == ChildType::kBlender) {
                childOption.blender()->priv().addToKey(childContext, builder, gatherer, curCombo);
            } else {
                SkASSERT(curCombo == 0);

                // We don't have a child effect. Substitute in a no-op effect.
                switch (childInfo[index].type) {
                    case SkRuntimeEffect::ChildType::kShader:
                        // A missing shader returns transparent black
                        SolidColorShaderBlock::AddBlock(childContext, builder, gatherer,
                                                        SK_PMColor4fTRANSPARENT);
                        break;

                    case SkRuntimeEffect::ChildType::kColorFilter:
                        // A "passthrough" shader returns the input color as-is.
                        builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
                        break;

                    case SkRuntimeEffect::ChildType::kBlender:
                        // A "passthrough" blender performs `blend_src_over(src, dest)`.
                        AddKnownModeBlend(childContext, builder, gatherer, SkBlendMode::kSrcOver);
                        break;
                }
            }
        }
#endif

        builder->endBlock();
    }

    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::vector<sk_sp<PrecompileBase>>> fChildOptions1;
    skia_private::TArray<int> fNumSlotCombinations;
    int fNumChildCombinations;
};

sk_sp<PrecompileShader> MakePrecompileShader(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowShader_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<PrecompileRTEffect<PrecompileShader>>(std::move(effect), childOptions);
}

sk_sp<PrecompileColorFilter> MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowColorFilter_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<PrecompileRTEffect<PrecompileColorFilter>>(std::move(effect), childOptions);
}

sk_sp<PrecompileBlender> MakePrecompileBlender(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowBlender_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<PrecompileRTEffect<PrecompileBlender>>(std::move(effect), childOptions);
}

} // namespace skgpu::graphite

//--------------------------------------------------------------------------------------------------
