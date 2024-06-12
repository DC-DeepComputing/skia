/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PrecompileShader.h"

#include "include/core/SkColorSpace.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"
#include "src/gpu/graphite/FactoryFunctionsPriv.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PrecompileInternal.h"
#include "src/gpu/graphite/ReadSwizzle.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"
#include "src/gpu/graphite/precompile/PrecompileBlenderPriv.h"
#include "src/gpu/graphite/precompile/PrecompileShaderPriv.h"


//#include "src/gpu/graphite/FactoryFunctions.h"

namespace skgpu::graphite {

PrecompileShader::~PrecompileShader() = default;

sk_sp<PrecompileShader> PrecompileShader::makeWithLocalMatrix() {
    if (this->priv().isALocalMatrixShader()) {
        // SkShader::makeWithLocalMatrix collapses chains of localMatrix shaders so we need to
        // follow suit here
        return sk_ref_sp(this);
    }

    return PrecompileShaders::LocalMatrix({ sk_ref_sp(this) });
}

sk_sp<PrecompileShader> PrecompileShader::makeWithColorFilter(sk_sp<PrecompileColorFilter> cf) {
    if (!cf) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::ColorFilter({ sk_ref_sp(this) }, { std::move(cf) });
}

sk_sp<PrecompileShader> PrecompileShader::makeWithWorkingColorSpace(sk_sp<SkColorSpace> cs) {
    if (!cs) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::WorkingColorSpace({ sk_ref_sp(this) }, { std::move(cs) });
}

//--------------------------------------------------------------------------------------------------
class PrecompileEmptyShader final : public PrecompileShader {
private:
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {

        SkASSERT(desiredCombination == 0); // The empty shader only ever has one combination

        builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
    }
};

sk_sp<PrecompileShader> PrecompileShaders::Empty() {
    return sk_make_sp<PrecompileEmptyShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorShader final : public PrecompileShader {
private:
    bool isConstant(int desiredCombination) const override {
        SkASSERT(desiredCombination == 0); // The color shader only ever has one combination
        return true;
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {

        SkASSERT(desiredCombination == 0); // The color shader only ever has one combination

        // The white PMColor is just a placeholder for the actual paint params color
        SolidColorShaderBlock::AddBlock(keyContext, builder, gatherer, SK_PMColor4fWHITE);
    }
};

sk_sp<PrecompileShader> PrecompileShaders::Color() {
    return sk_make_sp<PrecompileColorShader>();
}

// The colorSpace is safe to ignore - it is just applied to the color and doesn't modify the
// generated program.
sk_sp<PrecompileShader> PrecompileShaders::Color(sk_sp<SkColorSpace>) {
    return sk_make_sp<PrecompileColorShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileBlendShader : public PrecompileShader {
public:
    PrecompileBlendShader(SkSpan<const sk_sp<PrecompileBlender>> runtimeBlendEffects,
                          SkSpan<const sk_sp<PrecompileShader>> dsts,
                          SkSpan<const sk_sp<PrecompileShader>> srcs,
                          bool needsPorterDuffBased,
                          bool needsSeparableMode)
            : fRuntimeBlendEffects(runtimeBlendEffects.begin(), runtimeBlendEffects.end())
            , fDstOptions(dsts.begin(), dsts.end())
            , fSrcOptions(srcs.begin(), srcs.end()) {

        fNumBlenderCombos = 0;
        for (const auto& rt : fRuntimeBlendEffects) {
            fNumBlenderCombos += rt->priv().numCombinations();
        }
        if (needsPorterDuffBased) {
            ++fNumBlenderCombos;
        }
        if (needsSeparableMode) {
            ++fNumBlenderCombos;
        }

        SkASSERT(fNumBlenderCombos >= 1);

        fNumDstCombos = 0;
        for (const auto& d : fDstOptions) {
            fNumDstCombos += d->priv().numCombinations();
        }

        fNumSrcCombos = 0;
        for (const auto& s : fSrcOptions) {
            fNumSrcCombos += s->priv().numCombinations();
        }

        if (needsPorterDuffBased) {
            fPorterDuffIndex = 0;
            if (needsSeparableMode) {
                fSeparableModeIndex = 1;
                if (!fRuntimeBlendEffects.empty()) {
                    fBlenderIndex = 2;
                }
            } else if (!fRuntimeBlendEffects.empty()) {
                fBlenderIndex = 1;
            }
        } else if (needsSeparableMode) {
            fSeparableModeIndex = 0;
            if (!fRuntimeBlendEffects.empty()) {
                fBlenderIndex = 1;
            }
        } else {
            SkASSERT(!fRuntimeBlendEffects.empty());
            fBlenderIndex = 0;
        }
    }

private:
    int numChildCombinations() const override {
        return fNumBlenderCombos * fNumDstCombos * fNumSrcCombos;
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        const int desiredDstCombination = desiredCombination % fNumDstCombos;
        int remainingCombinations = desiredCombination / fNumDstCombos;

        const int desiredSrcCombination = remainingCombinations % fNumSrcCombos;
        remainingCombinations /= fNumSrcCombos;

        int desiredBlendCombination = remainingCombinations;
        SkASSERT(desiredBlendCombination < fNumBlenderCombos);

        if (desiredBlendCombination == fPorterDuffIndex ||
            desiredBlendCombination == fSeparableModeIndex) {
            BlendShaderBlock::BeginBlock(keyContext, builder, gatherer);

        } else {
            const SkRuntimeEffect* blendEffect =
                    GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kBlend);

            RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                           { sk_ref_sp(blendEffect) });
            SkASSERT(desiredBlendCombination >= fBlenderIndex);
            desiredBlendCombination -= fBlenderIndex;
        }

        AddToKey<PrecompileShader>(keyContext, builder, gatherer, fSrcOptions,
                                   desiredSrcCombination);
        AddToKey<PrecompileShader>(keyContext, builder, gatherer, fDstOptions,
                                   desiredDstCombination);

        if (desiredBlendCombination == fPorterDuffIndex) {
            CoeffBlenderBlock::AddBlock(keyContext, builder, gatherer,
                                        { 0.0f, 0.0f, 0.0f, 0.0f }); // coeffs aren't used
        } else if (desiredBlendCombination == fSeparableModeIndex) {
            BlendModeBlenderBlock::AddBlock(keyContext, builder, gatherer,
                                            SkBlendMode::kOverlay); // the blendmode is unused
        } else {
            AddToKey<PrecompileBlender>(keyContext, builder, gatherer, fRuntimeBlendEffects,
                                        desiredBlendCombination);
        }

        builder->endBlock();  // BlendShaderBlock or RuntimeEffectBlock
    }

    std::vector<sk_sp<PrecompileBlender>> fRuntimeBlendEffects;
    std::vector<sk_sp<PrecompileShader>> fDstOptions;
    std::vector<sk_sp<PrecompileShader>> fSrcOptions;

    int fNumBlenderCombos;
    int fNumDstCombos;
    int fNumSrcCombos;

    int fPorterDuffIndex = -1;
    int fSeparableModeIndex = -1;
    int fBlenderIndex = -1;
};

sk_sp<PrecompileShader> PrecompileShaders::Blend(
        SkSpan<const sk_sp<PrecompileBlender>> blenders,
        SkSpan<const sk_sp<PrecompileShader>> dsts,
        SkSpan<const sk_sp<PrecompileShader>> srcs) {
    std::vector<sk_sp<PrecompileBlender>> tmp;
    tmp.reserve(blenders.size());

    bool needsPorterDuffBased = false;
    bool needsBlendModeBased = false;

    for (const auto& b : blenders) {
        if (!b) {
            needsPorterDuffBased = true; // fall back to kSrcOver
        } else if (b->priv().asBlendMode().has_value()) {
            SkBlendMode bm = b->priv().asBlendMode().value();

            SkSpan<const float> coeffs = skgpu::GetPorterDuffBlendConstants(bm);
            if (!coeffs.empty()) {
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
class PrecompileCoordClampShader : public PrecompileShader {
public:
    PrecompileCoordClampShader(SkSpan<const sk_sp<PrecompileShader>> shaders)
            : fShaders(shaders.begin(), shaders.end()) {
        fNumShaderCombos = 0;
        for (const auto& s : fShaders) {
            fNumShaderCombos += s->priv().numCombinations();
        }
    }

private:
    int numChildCombinations() const override {
        return fNumShaderCombos;
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < fNumShaderCombos);

        constexpr SkRect kIgnored { 0, 0, 256, 256 }; // ignored bc we're precompiling

        // TODO: update CoordClampShaderBlock so this is optional
        CoordClampShaderBlock::CoordClampData data(kIgnored);

        CoordClampShaderBlock::BeginBlock(keyContext, builder, gatherer, data);
            AddToKey<PrecompileShader>(keyContext, builder, gatherer, fShaders, desiredCombination);
        builder->endBlock();
    }

    std::vector<sk_sp<PrecompileShader>> fShaders;
    int fNumShaderCombos;
};

sk_sp<PrecompileShader> PrecompileShaders::CoordClamp(SkSpan<const sk_sp<PrecompileShader>> input) {
    return sk_make_sp<PrecompileCoordClampShader>(input);
}

//--------------------------------------------------------------------------------------------------
// TODO: Investigate the YUV-image use case
class PrecompileImageShader : public PrecompileShader {
public:
    PrecompileImageShader(SkEnumBitMask<PrecompileImageShaderFlags> flags) : fFlags(flags) {}

private:
    // The ImageShader has 3 potential sampling/tiling variants: hardware-tiled, shader-tiled and
    // cubic sampling (which always uses shader-tiling)
    inline static constexpr int kNumSamplingTilingCombos = 3;
    inline static constexpr int kCubicSampled = 2;
    inline static constexpr int kHWTiled      = 1;
    inline static constexpr int kShaderTiled  = 0;

    // There are also 2 potential alpha combinations: alpha-only and not-alpha-only
    inline static constexpr int kNumAlphaCombinations = 2;
    inline static constexpr int kAlphaOnly    = 1;
    inline static constexpr int kNonAlphaOnly = 0;

    int numIntrinsicCombinations() const override {
        int numSamplingTilingCombos =
                (fFlags & PrecompileImageShaderFlags::kExcludeCubic) ? 2 : kNumSamplingTilingCombos;

        if (fFlags & PrecompileImageShaderFlags::kExcludeAlpha) {
            // RawImageShaders don't blend alpha-only images w/ the paint color
            return numSamplingTilingCombos;
        }
        return numSamplingTilingCombos * kNumAlphaCombinations;
    }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {
        SkASSERT(desiredCombination < this->numIntrinsicCombinations());

        int desiredAlphaCombo, desiredSamplingTilingCombo;

        if (fFlags & PrecompileImageShaderFlags::kExcludeAlpha) {
            desiredAlphaCombo = kNonAlphaOnly;
            desiredSamplingTilingCombo = desiredCombination;
        } else {
            desiredAlphaCombo = desiredCombination % kNumAlphaCombinations;
            desiredSamplingTilingCombo = desiredCombination / kNumAlphaCombinations;
        }
        SkDEBUGCODE(int numSamplingTilingCombos =
            (fFlags & PrecompileImageShaderFlags::kExcludeCubic) ? 2 : kNumSamplingTilingCombos;)
        SkASSERT(desiredSamplingTilingCombo < numSamplingTilingCombos);

        static constexpr SkSamplingOptions kDefaultCubicSampling(SkCubicResampler::Mitchell());
        static constexpr SkSamplingOptions kDefaultSampling;
        constexpr ReadSwizzle kIgnoredSwizzle = ReadSwizzle::kRGBA;

        // ImageShaderBlock will use hardware tiling when the subset covers the entire image, so we
        // create subset + image size combinations where subset == imgSize (for a shader that uses
        // hardware tiling) and subset < imgSize (for a shader that does shader-based tiling).
        static constexpr SkRect kSubset = SkRect::MakeWH(1.0f, 1.0f);
        static constexpr SkISize kHWTileableSize = SkISize::Make(1, 1);
        static constexpr SkISize kShaderTileableSize = SkISize::Make(2, 2);

        ImageShaderBlock::ImageData imgData(
                desiredSamplingTilingCombo == kCubicSampled ? kDefaultCubicSampling
                                                            : kDefaultSampling,
                SkTileMode::kClamp, SkTileMode::kClamp,
                desiredSamplingTilingCombo == kHWTiled ? kHWTileableSize : kShaderTileableSize,
                kSubset, kIgnoredSwizzle);

        if (desiredAlphaCombo == kAlphaOnly) {
            SkASSERT(!(fFlags & PrecompileImageShaderFlags::kExcludeAlpha));

            Blend(keyContext, builder, gatherer,
                  /* addBlendToKey= */ [&] () -> void {
                      AddKnownModeBlend(keyContext, builder, gatherer, SkBlendMode::kDstIn);
                  },
                  /* addSrcToKey= */ [&] () -> void {
                      ImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
                  },
                  /* addDstToKey= */ [&]() -> void {
                      RGBPaintColorBlock::AddBlock(keyContext, builder, gatherer);
                  });
        } else {
            ImageShaderBlock::AddBlock(keyContext, builder, gatherer, imgData);
        }
    }

    SkEnumBitMask<PrecompileImageShaderFlags> fFlags;
};

sk_sp<PrecompileShader> PrecompileShaders::Image() {
    return PrecompileShaders::LocalMatrix(
            { sk_make_sp<PrecompileImageShader>(PrecompileImageShaderFlags::kNone) });
}

sk_sp<PrecompileShader> PrecompileShaders::RawImage() {
    // Raw images do not perform color space conversion, but in Graphite, this is represented as
    // an identity color space xform, not as a distinct shader
    return PrecompileShaders::LocalMatrix(
            { sk_make_sp<PrecompileImageShader>(PrecompileImageShaderFlags::kExcludeAlpha) });
}

sk_sp<PrecompileShader> PrecompileShadersPriv::Image(
        SkEnumBitMask<PrecompileImageShaderFlags> flags) {
    return PrecompileShaders::LocalMatrix({ sk_make_sp<PrecompileImageShader>(flags) });
}

sk_sp<PrecompileShader> PrecompileShadersPriv::RawImage(
        SkEnumBitMask<PrecompileImageShaderFlags> flags) {
    return PrecompileShaders::LocalMatrix(
            { sk_make_sp<PrecompileImageShader>(flags |
                                                PrecompileImageShaderFlags::kExcludeAlpha) });
}

//--------------------------------------------------------------------------------------------------


} // namespace skgpu::graphite
