/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PaintParams.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkShader.h"
#include "src/core/SkBlendModeBlender.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/gpu/DitherUtils.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Uniform.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {

namespace {

// This should be kept in sync w/ SkPaintPriv::ShouldDither
bool should_dither(const PaintParams& p, SkColorType dstCT) {
    // The paint dither flag can veto.
    if (!p.dither()) {
        return false;
    }

    if (dstCT == kUnknown_SkColorType) {
        return false;
    }

    // We always dither 565 or 4444 when requested.
    if (dstCT == kRGB_565_SkColorType || dstCT == kARGB_4444_SkColorType) {
        return true;
    }

    // Otherwise, dither is only needed for non-const paints.
    return p.shader() && !as_SB(p.shader())->isConstant();
}

} // anonymous namespace

PaintParams::PaintParams(const SkColor4f& color,
                         sk_sp<SkBlender> finalBlender,
                         sk_sp<SkShader> shader,
                         sk_sp<SkColorFilter> colorFilter,
                         sk_sp<SkBlender> primitiveBlender,
                         DstReadRequirement dstReadReq,
                         bool skipColorXform,
                         bool dither)
        : fColor(color)
        , fFinalBlender(std::move(finalBlender))
        , fShader(std::move(shader))
        , fColorFilter(std::move(colorFilter))
        , fPrimitiveBlender(std::move(primitiveBlender))
        , fDstReadReq(dstReadReq)
        , fSkipColorXform(skipColorXform)
        , fDither(dither) {}

PaintParams::PaintParams(const SkPaint& paint,
                         sk_sp<SkBlender> primitiveBlender,
                         DstReadRequirement dstReadReq,
                         bool skipColorXform)
        : fColor(paint.getColor4f())
        , fFinalBlender(paint.refBlender())
        , fShader(paint.refShader())
        , fColorFilter(paint.refColorFilter())
        , fPrimitiveBlender(std::move(primitiveBlender))
        , fDstReadReq(dstReadReq)
        , fSkipColorXform(skipColorXform)
        , fDither(paint.isDither()) {}

PaintParams::PaintParams(const PaintParams& other) = default;
PaintParams::~PaintParams() = default;
PaintParams& PaintParams::operator=(const PaintParams& other) = default;

std::optional<SkBlendMode> PaintParams::asFinalBlendMode() const {
    return fFinalBlender ? as_BB(fFinalBlender)->asBlendMode()
                         : SkBlendMode::kSrcOver;
}

sk_sp<SkBlender> PaintParams::refFinalBlender() const { return fFinalBlender; }

sk_sp<SkShader> PaintParams::refShader() const { return fShader; }

sk_sp<SkColorFilter> PaintParams::refColorFilter() const { return fColorFilter; }

sk_sp<SkBlender> PaintParams::refPrimitiveBlender() const { return fPrimitiveBlender; }

SkColor4f PaintParams::Color4fPrepForDst(SkColor4f srcColor, const SkColorInfo& dstColorInfo) {
    // xform from sRGB to the destination colorspace
    SkColorSpaceXformSteps steps(sk_srgb_singleton(),       kUnpremul_SkAlphaType,
                                 dstColorInfo.colorSpace(), kUnpremul_SkAlphaType);

    SkColor4f result = srcColor;
    steps.apply(result.vec());
    return result;
}

void PaintParams::addShaderToKey(const KeyContext& keyContext,
                                 PaintParamsKeyBuilder* keyBuilder,
                                 PipelineDataGatherer* gatherer) const {
    if (fShader) {
        AddToKeyS(keyContext, keyBuilder, gatherer, fShader.get());
    } else {
        SolidColorShaderBlock::BeginBlock(keyContext, keyBuilder, gatherer,
                                          keyContext.paintColor());
        keyBuilder->endBlock();
    }
}

void PaintParams::handlePrimitiveColor(const KeyContext& keyContext,
                                       PaintParamsKeyBuilder* keyBuilder,
                                       PipelineDataGatherer* gatherer) const {

    auto emitSrc = [this](const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* keyBuilder,
                                  PipelineDataGatherer* gatherer) -> void {
        // src - whatever
        this->addShaderToKey(keyContext, keyBuilder, gatherer);
    };

    if (fPrimitiveBlender) {
        BlendShaderBlock::BeginBlock(keyContext, keyBuilder, gatherer);

            auto emitDst = [](const KeyContext& keyContext,
                              PaintParamsKeyBuilder* keyBuilder,
                              PipelineDataGatherer* gatherer) -> void {
                // dst -- primitive color
                PrimitiveColorBlock::BeginBlock(keyContext, keyBuilder, gatherer);
                keyBuilder->endBlock();
            };

            // blender -- shader based blending
            AddToKeyB(keyContext, keyBuilder, gatherer, fPrimitiveBlender.get(),
                      emitSrc, emitDst);

        keyBuilder->endBlock();  // BlendShaderBlock
    } else {
        emitSrc(keyContext, keyBuilder, gatherer);
    }
}

void PaintParams::applyPaintAlpha(const KeyContext& keyContext,
                                  PaintParamsKeyBuilder* keyBuilder,
                                  PipelineDataGatherer* gatherer) const {
    // Apply the paint's alpha value.
    if (fColor.fA != 1.0f) {
        BlendShaderBlock::BeginBlock(keyContext, keyBuilder, gatherer);

            // blender -- shader based blending
            BlendModeBlenderBlock::BeginBlock(keyContext, keyBuilder, gatherer, SkBlendMode::kDstIn);

                // src -- solid color
                SolidColorShaderBlock::BeginBlock(keyContext, keyBuilder, gatherer, {0, 0, 0, fColor.fA});
                keyBuilder->endBlock();

                // dst
                this->handlePrimitiveColor(keyContext, keyBuilder, gatherer);

            keyBuilder->endBlock();

        keyBuilder->endBlock();  // BlendShaderBlock
    } else {
        this->handlePrimitiveColor(keyContext, keyBuilder, gatherer);
    }
}

//$$
void PaintParams::toKey(const KeyContext& keyContext,
                        PaintParamsKeyBuilder* builder,
                        PipelineDataGatherer* gatherer) const {
    bool needsDstSample = fDstReadReq == DstReadRequirement::kTextureCopy ||
                          fDstReadReq == DstReadRequirement::kTextureSample;
    SkASSERT(needsDstSample == SkToBool(keyContext.dstTexture()));
    if (needsDstSample) {
        DstReadSampleBlock::BeginBlock(
                keyContext, builder, gatherer, keyContext.dstTexture(), keyContext.dstOffset());
        builder->endBlock();

    } else if (fDstReadReq == DstReadRequirement::kFramebufferFetch) {
        DstReadFetchBlock::BeginBlock(keyContext, builder, gatherer);
        builder->endBlock();
    }

    this->applyPaintAlpha(keyContext, builder, gatherer);

    AddToKeyCF(keyContext, builder, gatherer, fColorFilter.get());

#ifndef SK_IGNORE_GPU_DITHER
    SkColorType ct = keyContext.dstColorInfo().colorType();
    if (should_dither(*this, ct)) {
        DitherShaderBlock::DitherData data(skgpu::DitherRangeForConfig(ct));

        DitherShaderBlock::BeginBlock(keyContext, builder, gatherer, &data);
        builder->endBlock();
    }
#endif

    auto emitSrc = [](const KeyContext& keyContext,
                      PaintParamsKeyBuilder* keyBuilder,
                      PipelineDataGatherer* gatherer) -> void {

    };

    std::optional<SkBlendMode> finalBlendMode = this->asFinalBlendMode();
    // If this draw needs a dst read, we are either doing custom blending or we cannot handle the
    // combination of blend mode and coverage using fixed function blend hardware.
    if (fDstReadReq != DstReadRequirement::kNone) {
        // Add a block to implement the blending in the shader.
        static const SkBlendModeBlender kDefaultBlender(SkBlendMode::kSrcOver);
        AddDstBlendBlock(keyContext,
                         builder,
                         gatherer,
                         fFinalBlender ? fFinalBlender.get() : &kDefaultBlender,
                         emitSrc);
        finalBlendMode = SkBlendMode::kSrc;
    }

    // Set the hardware blend mode.
    SkASSERT(finalBlendMode);
    BuiltInCodeSnippetID fixedFuncBlendModeID = static_cast<BuiltInCodeSnippetID>(
            kFixedFunctionBlendModeIDOffset + static_cast<int>(*finalBlendMode));
    builder->beginBlock(fixedFuncBlendModeID);
    builder->endBlock();
}

} // namespace skgpu::graphite
