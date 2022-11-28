/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_PRECOMPILE

#include "src/gpu/graphite/FactoryFunctions.h"

#include "src/gpu/Blend.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
class PrecompileBlendModeBlender : public PrecompileBlender {
public:
    PrecompileBlendModeBlender(SkBlendMode blendMode) : fBlendMode(blendMode) {}

    std::optional<SkBlendMode> asBlendMode() const final { return fBlendMode; }

private:
    int numIntrinsicCombinations() const override {
        return 1;
    }

    int numChildCombinations() const override {
        return 1;
    }

    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        SkASSERT(desiredCombination == 0); // The blend mode blender only ever has one combination

        // The blend mode is used in this BeginBlock! It is used to choose between fixed function
        // and shader-based blending
        BlendModeBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr, fBlendMode);
        builder->endBlock();
    }

    SkBlendMode fBlendMode;
};

sk_sp<PrecompileBlender> PrecompileBlender::Mode(SkBlendMode blendMode) {
    return sk_make_sp<PrecompileBlendModeBlender>(blendMode);
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorShader : public PrecompileShader {
public:
    PrecompileColorShader() {}

private:
    int numIntrinsicCombinations() const override {
        return 1;
    }

    int numChildCombinations() const override {
        return 1;
    }

    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        SkASSERT(desiredCombination == 0); // The color shader only ever has one combination

        constexpr SkPMColor4f kUnusedColor = {1, 0, 0, 1};

        SolidColorShaderBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr,
                                          kUnusedColor); // color isn't used w/o a gatherer
        builder->endBlock();
    }

};

sk_sp<PrecompileShader> PrecompileShaders::Color() {
    return sk_make_sp<PrecompileColorShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileBlendShader : public PrecompileShader {
public:
    PrecompileBlendShader(SkSpan<const sk_sp<PrecompileBlender>> blenders,
                          SkSpan<const sk_sp<PrecompileShader>> dsts,
                          SkSpan<const sk_sp<PrecompileShader>> srcs,
                          bool needsPorterDuffBased,
                          bool needsBlendModeBased)
            : fNeedsPorterDuffBased(needsPorterDuffBased)
            , fNeedsBlendModeBased(needsBlendModeBased)
            , fBlenders(blenders.begin(), blenders.end())
            , fDsts(dsts.begin(), dsts.end())
            , fSrcs(srcs.begin(), srcs.end()) {
#ifdef SK_DEBUG
        // Only RuntimeEffect blenders should be in this list
        for (auto b : fBlenders) {
            SkASSERT(!b->asBlendMode().has_value());
        }
#endif

        fNumBlenderCombos = fBlenders.size();
        if (fNeedsPorterDuffBased) {
            ++fNumBlenderCombos;
        }
        if (fNeedsBlendModeBased) {
            ++fNumBlenderCombos;
        }

        SkASSERT(fNumBlenderCombos >= 1);

        fNumDstCombos = 0;
        for (auto d : fDsts) {
            fNumDstCombos += d->priv().numChildCombinations();
        }

        fNumSrcCombos = 0;
        for (auto s : fSrcs) {
            fNumSrcCombos += s->priv().numChildCombinations();
        }
    }

private:
    int numIntrinsicCombinations() const override {
        int numIntrinsicCombinations = 0;

        // The PorterDuff and BlendMode-based variants will each only create one new combination.
        if (fNeedsPorterDuffBased) {
            ++numIntrinsicCombinations;
        }
        if (fNeedsBlendModeBased) {
            ++numIntrinsicCombinations;
        }
        if (!fBlenders.empty()) {
            ++numIntrinsicCombinations;
        }

        SkASSERT(numIntrinsicCombinations);
        return numIntrinsicCombinations;
    }

    int numChildCombinations() const override {
        return fNumBlenderCombos * fNumDstCombos * fNumSrcCombos;
    }

    void beginBlock(const KeyContext& keyContext,
                    int intrinsicCombination,
                    PaintParamsKeyBuilder* builder) const override {

        int PDIndex = -1;
        int BMIndex = -1;
        int BlenderIndex = -1;

        if (fNeedsPorterDuffBased) {
            PDIndex = 0;
            if (fNeedsBlendModeBased) {
                BMIndex = 1;
                if (!fBlenders.empty()) {
                    BlenderIndex = 2;
                }
            } else if (!fBlenders.empty()) {
                BlenderIndex = 1;
            }
        } else if (fNeedsBlendModeBased) {
            BMIndex = 0;
            if (!fBlenders.empty()) {
                BlenderIndex = 1;
            }
        } else {
            SkASSERT(!fBlenders.empty());
            BlenderIndex = 0;
        }

        if (intrinsicCombination == PDIndex) {
            PorterDuffBlendShaderBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr,
                                                   {}); // Porter/Duff coeffs aren't used
        } else if (intrinsicCombination == BMIndex) {
            BlendShaderBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr,
                                         { SkBlendMode::kSrcOver }); // the blendmode is unused
        } else {
            SkASSERT(intrinsicCombination == BlenderIndex);
            // ??
        }
    }

    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        SkASSERT(desiredCombination < this->priv().numCombinations());

        int intrinsicCombination = desiredCombination / this->numChildCombinations();
        int childCombination = desiredCombination % this->numChildCombinations();

        this->beginBlock(keyContext, intrinsicCombination, builder);


//        as_SB(fSrc)->addToKey(keyContext, builder);
//        as_SB(fDst)->addToKey(keyContext, builder);

        builder->endBlock();
    }

    bool fNeedsPorterDuffBased = false;
    bool fNeedsBlendModeBased = false;
    std::vector<sk_sp<PrecompileBlender>> fBlenders;
    std::vector<sk_sp<PrecompileShader>> fDsts;
    std::vector<sk_sp<PrecompileShader>> fSrcs;

    int fNumBlenderCombos;
    int fNumDstCombos;
    int fNumSrcCombos;
};

sk_sp<PrecompileShader> PrecompileShaders::Blend(
        SkSpan<const sk_sp<PrecompileBlender>> blenders,
        SkSpan<const sk_sp<PrecompileShader>> dsts,
        SkSpan<const sk_sp<PrecompileShader>> srcs) {
    std::vector<const sk_sp<PrecompileBlender>> tmp;
    tmp.reserve(blenders.size());

    bool needsPorterDuffBased = false;
    bool needsBlendModeBased = false;

    for (auto b : blenders) {
        if (b->asBlendMode().has_value()) {
            SkBlendMode bm = b->asBlendMode().value();

            SkSpan<const float> porterDuffConstants = skgpu::GetPorterDuffBlendConstants(bm);
            if (!porterDuffConstants.empty()) {
                needsPorterDuffBased = true;
            } else {
                needsBlendModeBased = true;
            }
        } else {
            tmp.push_back(b);
        }
    }

    if (!needsPorterDuffBased && !needsBlendModeBased && tmp.empty()) {
        needsPorterDuffBased = true; // fallback to kSrcOver
    }

    return sk_make_sp<PrecompileBlendShader>(SkSpan<const sk_sp<PrecompileBlender>>(tmp),
                                             dsts, srcs,
                                             needsPorterDuffBased, needsBlendModeBased);
}

sk_sp<PrecompileShader> PrecompileShaders::Blend(
        SkSpan<SkBlendMode> blendModes,
        SkSpan<const sk_sp<PrecompileShader>> dsts,
        SkSpan<const sk_sp<PrecompileShader>> srcs) {

    bool needsPorterDuffBased = false;
    bool needsBlendModeBased = false;

    for (SkBlendMode bm : blendModes) {
        SkSpan<const float> porterDuffConstants = skgpu::GetPorterDuffBlendConstants(bm);
        if (!porterDuffConstants.empty()) {
            needsPorterDuffBased = true;
        } else {
            needsBlendModeBased = true;
        }
    }

    if (!needsPorterDuffBased && !needsBlendModeBased) {
        needsPorterDuffBased = true; // fallback to kSrcOver
    }

    return sk_make_sp<PrecompileBlendShader>(SkSpan<const sk_sp<PrecompileBlender>>(),
                                             dsts, srcs,
                                             needsPorterDuffBased, needsBlendModeBased);
}

//--------------------------------------------------------------------------------------------------
class PrecompileImageShader : public PrecompileShader {
public:
    PrecompileImageShader() {}

private:
    int numIntrinsicCombinations() const override {
        return 1;
    }

    int numChildCombinations() const override {
        return 1;
    }

    void beginBlock(const KeyContext& keyContext,
                    int intrinsicCombination,
                    PaintParamsKeyBuilder* builder) const override {
        SkASSERT(intrinsicCombination == 0);

        // The ImageData is unused when there is no gatherer
        ImageShaderBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr, {});
    }
};

sk_sp<PrecompileShader> PrecompileShaders::Image() {
    return sk_make_sp<PrecompileImageShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileGradientShader : public PrecompileShader {
public:
    PrecompileGradientShader(SkShaderBase::GradientType type) : fType(type) {}

private:
    /*
     * The gradients currently have two specializations based on the number of stops.
     */
    inline static constexpr int kNumStopVariants = 2;
    inline static constexpr int kStopVariants[kNumStopVariants] = { 4, 8 };

    int numIntrinsicCombinations() const override {
        return kNumStopVariants;
    }

    int numChildCombinations() const override {
        return 1;
    }

    void beginBlock(const KeyContext& keyContext,
                    int intrinsicCombination,
                    PaintParamsKeyBuilder* builder) const override {
        SkASSERT(intrinsicCombination < kNumStopVariants);

        // Only the type and number of stops are accessed when there is no gatherer
        GradientShaderBlocks::GradientData gradData(fType, kStopVariants[intrinsicCombination]);

        // TODO: we may need SkLocalMatrixShader-wrapped versions too
        GradientShaderBlocks::BeginBlock(keyContext, builder, /* gatherer= */ nullptr, gradData);
    }

    SkShaderBase::GradientType fType;
};

sk_sp<PrecompileShader> PrecompileShaders::LinearGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kLinear);
}

sk_sp<PrecompileShader> PrecompileShaders::RadialGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kRadial);
}

sk_sp<PrecompileShader> PrecompileShaders::SweepGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kSweep);
}

sk_sp<PrecompileShader> PrecompileShaders::TwoPointConicalGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kConical);
}

//--------------------------------------------------------------------------------------------------
class PrecompileBlurMaskFilter : public PrecompileMaskFilter {
public:
    PrecompileBlurMaskFilter() {}

private:
    int numIntrinsicCombinations() const override {
        return 1;
    }

    int numChildCombinations() const override {
        return 1;
    }

    void beginBlock(const KeyContext& keyContext,
                    int intrinsicCombination,
                    PaintParamsKeyBuilder* builder) const override {
        SkASSERT(intrinsicCombination == 0);

        // TODO: need to add a BlurMaskFilter Block
    }
};

sk_sp<PrecompileMaskFilter> PrecompileMaskFilters::Blur() {
    return sk_make_sp<PrecompileBlurMaskFilter>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileMatrixColorFilter : public PrecompileColorFilter {
public:
    PrecompileMatrixColorFilter() {}

private:
    int numIntrinsicCombinations() const override {
        return 1;
    }

    int numChildCombinations() const override {
        return 1;
    }

    void beginBlock(const KeyContext& keyContext,
                    int intrinsicCombination,
                    PaintParamsKeyBuilder* builder) const override {
        SkASSERT(intrinsicCombination == 0);

        MatrixColorFilterBlock::BeginBlock(keyContext, builder,
                                           /* gatherer= */ nullptr,
                                           /* matrixCFData= */ nullptr);
    }
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Matrix() {
    return sk_make_sp<PrecompileMatrixColorFilter>();
}

//--------------------------------------------------------------------------------------------------
// TODO: need to figure out how we're going to decompose ImageFilters
sk_sp<PrecompileImageFilter> PrecompileImageFilters::Blur() {
    return nullptr; // sk_make_sp<PrecompileImageFilter>();
}

sk_sp<PrecompileImageFilter> PrecompileImageFilters::Image() {
    return nullptr; // sk_make_sp<PrecompileImageFilter>();
}

//--------------------------------------------------------------------------------------------------
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileShader> s) : fChild(std::move(s)) {}
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileColorFilter> cf)
        : fChild(std::move(cf)) {
}
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileBlender> b) : fChild(std::move(b)) {}

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

PrecompileShader* PrecompileChildPtr::shader() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kShader)
           ? static_cast<PrecompileShader*>(fChild.get())
           : nullptr;
}

PrecompileColorFilter* PrecompileChildPtr::colorFilter() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kColorFilter)
           ? static_cast<PrecompileColorFilter*>(fChild.get())
           : nullptr;
}

PrecompileBlender* PrecompileChildPtr::blender() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kBlender)
           ? static_cast<PrecompileBlender*>(fChild.get())
           : nullptr;
}

//--------------------------------------------------------------------------------------------------
template<typename T>
class PrecompileRTEffect : public T {
public:
    PrecompileRTEffect(sk_sp<SkRuntimeEffect> effect,
                       SkSpan<const PrecompileChildOptions> childOptions)
            : fEffect(std::move(effect)) {
        fChildOptions.reserve(childOptions.size());
        for (PrecompileChildOptions c : childOptions) {
            fChildOptions.push_back({ c.begin(), c.end() });
        }
    }

private:
    int numIntrinsicCombinations() const override {
        // Externally supplied runtime effects cannot alter/specialize their code
        return 1;
    }

    int numChildCombinations() const override {
        int numChildCombinations = 1;

        for (const std::vector<PrecompileChildPtr>& oneChildsOptions : fChildOptions) {
            int oneChildsCnt = 1;

            for (const PrecompileChildPtr& childOption : oneChildsOptions) {
                oneChildsCnt += childOption.base()->priv().numChildCombinations();
            }

            numChildCombinations *= oneChildsCnt;
        }

        return numChildCombinations;
    }

    void beginBlock(const KeyContext& keyContext,
                    int intrinsicCombination,
                    PaintParamsKeyBuilder* builder) const override {
        SkASSERT(intrinsicCombination == 0);

        RuntimeEffectBlock::ShaderData data(fEffect);

        RuntimeEffectBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr,
                                       data);
    }

    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::vector<PrecompileChildPtr>> fChildOptions;
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
    return sk_make_sp<PrecompileRTEffect<PrecompileColorFilter>>(std::move(effect),
                                                                 childOptions);
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

#endif // SK_ENABLE_PRECOMPILE
