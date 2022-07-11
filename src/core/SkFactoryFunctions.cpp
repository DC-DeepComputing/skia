/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_PRECOMPILE

#include "src/core/SkFactoryFunctions.h"

#include "include/private/SkColorData.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPrecompile.h"
#include "src/core/SkPrecompileBasePriv.h"

// TODO: wrap this in an  SK_PRECOMPILE check

//--------------------------------------------------------------------------------------------------
class SkPrecompileBlendModeBlender : public SkPrecompileBlender {
public:
    SkPrecompileBlendModeBlender(SkBlendMode blendMode) : fBlendMode(blendMode) {}

    std::optional<SkBlendMode> asBlendMode() const final { return fBlendMode; }

private:
    int numIntrinsicCombinations(SkBackend) const override {
        return 1;
    }

    int numChildCombinations(SkBackend) const override {
        return 1;
    }

    void beginBlock(const SkKeyContext& keyContext,
                    int intrinsicCombination,
                    SkPaintParamsKeyBuilder* builder) const override {
        //$$
    }

    SkBlendMode fBlendMode;
};

sk_sp<SkPrecompileBlender> SkPrecompileBlender::Mode(SkBlendMode blendMode) {
    return sk_make_sp<SkPrecompileBlendModeBlender>(blendMode);
}

template<class T>
int num_combinations(const std::vector<sk_sp<T>>& v, SkBackend backend) {
    int numCombinations = 0;

    for (const sk_sp<T>& e : v) {
        numCombinations += e->priv().numCombinations1(backend);
    }

    return numCombinations;
}

//--------------------------------------------------------------------------------------------------
static void create_graphite_solid_color_shader_block(const SkKeyContext& keyContext,
                                                     int intrinsicCombination,
                                                     SkPaintParamsKeyBuilder* builder) {
    constexpr SkPMColor4f kUnusedColor = {1, 0, 0, 1};

    SkASSERT(intrinsicCombination == 0);
    SolidColorShaderBlock::BeginBlock(keyContext, builder, /*gatherer=*/nullptr,
                                      kUnusedColor);
}

SkPrecompileShader::foo::foo()
        : fNumIntrinsicCombinations(1)
        , fBeginBlock(create_graphite_solid_color_shader_block) {
}

sk_sp<SkPrecompileShader> SkPrecompileShaders::Color() {
    return sk_make_sp<SkPrecompileShader>(1, create_graphite_solid_color_shader_block);
}

//--------------------------------------------------------------------------------------------------
static void create_graphite_blend_shader_block(const SkKeyContext& keyContext,
                                               int intrinsicCombination,
                                               SkPaintParamsKeyBuilder* builder) {
    SkASSERT(intrinsicCombination == 0);

    BlendShaderBlock::BeginBlock(keyContext, builder, /*gatherer=*/nullptr,
                                 { SkBlendMode::kSrc });  // the blendmode is unused
}

class SkPrecompileBlendShader : public SkPrecompileShader {
public:
    SkPrecompileBlendShader(SkSpan<const sk_sp<SkPrecompileBlender>> blenders,
                            SkSpan<const sk_sp<SkPrecompileShader>> dsts,
                            SkSpan<const sk_sp<SkPrecompileShader>> srcs)
            : SkPrecompileShader(1, create_graphite_blend_shader_block)
            , fBlenders(blenders.begin(), blenders.end())
            , fDsts(dsts.begin(), dsts.end())
            , fSrcs(srcs.begin(), srcs.end()) {
    }

private:
    int numChildCombinations(SkBackend backend) const override {
        return num_combinations<SkPrecompileShader>(fDsts, backend) *
               num_combinations<SkPrecompileShader>(fSrcs, backend);
    }

    std::vector<sk_sp<SkPrecompileBlender>> fBlenders;
    std::vector<sk_sp<SkPrecompileShader>> fDsts;
    std::vector<sk_sp<SkPrecompileShader>> fSrcs;
};

sk_sp<SkPrecompileShader> SkPrecompileShaders::Blend(
        SkSpan<const sk_sp<SkPrecompileBlender>> blenders,
        SkSpan<const sk_sp<SkPrecompileShader>> dsts,
        SkSpan<const sk_sp<SkPrecompileShader>> srcs) {
    return sk_make_sp<SkPrecompileBlendShader>(std::move(blenders),
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

    return sk_make_sp<SkPrecompileBlendShader>(tmp, std::move(dsts), std::move(srcs));
}

//--------------------------------------------------------------------------------------------------
sk_sp<SkPrecompileShader> SkPrecompileShaders::Image() {
    return sk_make_sp<SkPrecompileShader>();
}

//--------------------------------------------------------------------------------------------------
sk_sp<SkPrecompileShader> SkPrecompileShaders::LinearGradient() {
    return sk_make_sp<SkPrecompileShader>();
}

//--------------------------------------------------------------------------------------------------
sk_sp<SkPrecompileShader> SkPrecompileShaders::RadialGradient() {
    return sk_make_sp<SkPrecompileShader>();
}

//--------------------------------------------------------------------------------------------------
sk_sp<SkPrecompileShader> SkPrecompileShaders::TwoPointConicalGradient() {
    return sk_make_sp<SkPrecompileShader>();
}

//--------------------------------------------------------------------------------------------------
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
SkPrecompileChildPtr::SkPrecompileChildPtr(sk_sp<SkPrecompileShader> s) : fChild(std::move(s)) {}
SkPrecompileChildPtr::SkPrecompileChildPtr(sk_sp<SkPrecompileColorFilter> cf)
        : fChild(std::move(cf)) {
}
SkPrecompileChildPtr::SkPrecompileChildPtr(sk_sp<SkPrecompileBlender> b) : fChild(std::move(b)) {}

namespace {

#ifdef SK_DEBUG

bool precompilebase_is_valid_as_child(const SkPrecompileBase *child) {
    if (!child) {
        return true;
    }

    switch (child->type()) {
        case SkPrecompileBase::Type::kShader:
        case SkPrecompileBase::Type::kColorFilter:
        case SkPrecompileBase::Type::kBlender:
            return true;
        default:
            return false;
    }
}

#endif // SK_DEBUG

}

SkPrecompileChildPtr::SkPrecompileChildPtr(sk_sp<SkPrecompileBase> child)
        : fChild(std::move(child)) {
    SkASSERT(precompilebase_is_valid_as_child(fChild.get()));
}

std::optional<SkRuntimeEffect::ChildType> SkPrecompileChildPtr::type() const {
    if (fChild) {
        switch (fChild->type()) {
            case SkPrecompileBase::Type::kShader:
                return SkRuntimeEffect::ChildType::kShader;
            case SkPrecompileBase::Type::kColorFilter:
                return SkRuntimeEffect::ChildType::kColorFilter;
            case SkPrecompileBase::Type::kBlender:
                return SkRuntimeEffect::ChildType::kBlender;
            default:
                break;
        }
    }
    return std::nullopt;
}

SkPrecompileShader* SkPrecompileChildPtr::shader() const {
    return (fChild && fChild->type() == SkPrecompileBase::Type::kShader)
           ? static_cast<SkPrecompileShader*>(fChild.get())
           : nullptr;
}

SkPrecompileColorFilter* SkPrecompileChildPtr::colorFilter() const {
    return (fChild && fChild->type() == SkPrecompileBase::Type::kColorFilter)
           ? static_cast<SkPrecompileColorFilter*>(fChild.get())
           : nullptr;
}

SkPrecompileBlender* SkPrecompileChildPtr::blender() const {
    return (fChild && fChild->type() == SkPrecompileBase::Type::kBlender)
           ? static_cast<SkPrecompileBlender*>(fChild.get())
           : nullptr;
}

//--------------------------------------------------------------------------------------------------
template<typename T>
class SkPrecompileRTEffect : public T {
public:
    SkPrecompileRTEffect(sk_sp<SkRuntimeEffect> effect,
                         SkSpan<const SkPrecompileChildOptions> childOptions)
            : fEffect(std::move(effect)) {
        fChildOptions.reserve(childOptions.size());
        for (SkPrecompileChildOptions c : childOptions) {
            fChildOptions.push_back({ c.begin(), c.end() });
        }
    }

private:
    int numIntrinsicCombinations(SkBackend backend) const override {
        // Externally supplied runtime effects cannot alter/specialize their code
        return 1;
    }

    int numChildCombinations(SkBackend backend) const override {
        int numChildCombinations = 1;

        for (const std::vector<SkPrecompileChildPtr>& oneChildsOptions : fChildOptions) {
            int oneChildsCnt = 1;

            for (const SkPrecompileChildPtr& childOption : oneChildsOptions) {
                oneChildsCnt += childOption.base()->priv().numCombinations1(backend);
            }

            numChildCombinations *= oneChildsCnt;
        }

        return numChildCombinations;
    }

    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::vector<SkPrecompileChildPtr>> fChildOptions;
};

sk_sp<SkPrecompileShader> MakePrecompileShader(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const SkPrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowShader_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<SkPrecompileRTEffect<SkPrecompileShader>>(std::move(effect), childOptions);
}

sk_sp<SkPrecompileColorFilter> MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const SkPrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowColorFilter_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<SkPrecompileRTEffect<SkPrecompileColorFilter>>(std::move(effect),
                                                                     childOptions);
}

sk_sp<SkPrecompileBlender> MakePrecompileBlender(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const SkPrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowBlender_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<SkPrecompileRTEffect<SkPrecompileBlender>>(std::move(effect), childOptions);
}

//--------------------------------------------------------------------------------------------------

#endif // SK_ENABLE_PRECOMPILE
