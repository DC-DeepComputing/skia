/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkModeColorFilter.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

SkModeColorFilter::SkModeColorFilter(const SkColor4f& color,
                                     sk_sp<SkColorSpace> colorSpace,
                                     SkBlendMode mode)
        : fColorSpace(std::move(colorSpace))
        , fColor(color)
        , fMode(mode) {}

bool SkModeColorFilter::onAsAColorMode(SkColor* color, SkBlendMode* mode) const {
    if (color) {
        *color = this->mapColorToDst(sk_srgb_singleton()).toSkColor();
    }
    if (mode) {
        *mode = fMode;
    }
    return true;
}

bool SkModeColorFilter::onIsAlphaUnchanged() const {
    switch (fMode) {
        case SkBlendMode::kDst:      //!< [Da, Dc]
        case SkBlendMode::kSrcATop:  //!< [Da, Sc * Da + (1 - Sa) * Dc]
            return true;
        default:
            break;
    }
    return false;
}

void SkModeColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor4f(fColor);
    buffer.writeUInt((int) fMode);

    sk_sp<SkData> colorSpaceData = fColorSpace ? fColorSpace->serialize() : nullptr;
    if (colorSpaceData) {
        buffer.writeBool(true);
        buffer.writeDataAsByteArray(colorSpaceData.get());
    } else {
        buffer.writeBool(false);
    }
}

sk_sp<SkFlattenable> SkModeColorFilter::CreateProc(SkReadBuffer& buffer) {
    if (buffer.isVersionLT(SkPicturePriv::kBlend4fColorFilter)) {
        // No color space, color is an SkColor
        SkColor color = buffer.readColor();
        SkBlendMode mode = (SkBlendMode)buffer.readUInt();
        return SkColorFilters::Blend(color, mode);
    } else {
        SkColor4f color;
        buffer.readColor4f(&color);
        SkBlendMode mode = (SkBlendMode)buffer.readUInt();

        sk_sp<SkColorSpace> colorSpace;
        if (buffer.readBool()) {
            sk_sp<SkData> data = buffer.readByteArrayAsData();
            colorSpace = data ? SkColorSpace::Deserialize(data->data(), data->size()) : nullptr;
        }
        return SkColorFilters::Blend(color, std::move(colorSpace), mode);
    }
}

SkColor4f SkModeColorFilter::mapColorToDst(const SkColorSpace* dst) const {
    SkColor4f color = fColor;
    SkColorSpaceXformSteps(fColorSpace.get(), kUnpremul_SkAlphaType,
                           dst,               kUnpremul_SkAlphaType).apply(color.vec());
    return color;
}

bool SkModeColorFilter::onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    rec.fPipeline->append(SkRasterPipeline::move_src_dst);
    SkColor4f color = this->mapColorToDst(rec.fDstCS);
    rec.fPipeline->append_constant_color(rec.fAlloc, color.premul().vec());
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

skvm::Color SkModeColorFilter::onProgram(skvm::Builder* p, skvm::Color c,
                                         const SkColorInfo& dstInfo,
                                         skvm::Uniforms* uniforms, SkArenaAlloc*) const {
    SkColor4f color = this->mapColorToDst(dstInfo.colorSpace());
    skvm::Color dst = c,
                src = p->uniformColor(color, uniforms);
    return p->blend(fMode, src,dst);
}

///////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "src/gpu/Blend.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"

GrFPResult SkModeColorFilter::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                  GrRecordingContext*,
                                                  const GrColorInfo& dstColorInfo,
                                                  const SkSurfaceProps& props) const {
    if (fMode == SkBlendMode::kDst) {
        // If the blend mode is "dest," the blend color won't factor into it at all.
        // We can return the input FP as-is.
        return GrFPSuccess(std::move(inputFP));
    }

    SkDEBUGCODE(const bool fpHasConstIO = !inputFP || inputFP->hasConstantOutputForConstantInput();)

    SkColor4f color = this->mapColorToDst(dstColorInfo.colorSpace());

    auto colorFP = GrFragmentProcessor::MakeColor(color.premul());
    auto xferFP = GrBlendFragmentProcessor::Make(std::move(colorFP), std::move(inputFP), fMode);

    if (xferFP == nullptr) {
        // This is only expected to happen if the blend mode is "dest" and the input FP is null.
        // Since we already did an early-out in the "dest" blend mode case, we shouldn't get here.
        SkDEBUGFAIL("GrBlendFragmentProcessor::Make returned null unexpectedly");
        return GrFPFailure(nullptr);
    }

    // With a solid color input this should always be able to compute the blended color
    // (at least for coeff modes).
    // Occasionally, we even do better than we started; specifically, in "src" blend mode, we end up
    // ditching the input FP entirely, which turns a non-constant operation into a constant one.
    SkASSERT(fMode > SkBlendMode::kLastCoeffMode ||
             xferFP->hasConstantOutputForConstantInput() >= fpHasConstIO);

    return GrFPSuccess(std::move(xferFP));
}

#endif

#ifdef SK_ENABLE_SKSL

#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPaintParamsKey.h"

void SkModeColorFilter::addToKey(const SkKeyContext& keyContext,
                                 SkPaintParamsKeyBuilder* builder,
                                 SkPipelineDataGatherer* gatherer) const {
    // TODO: Take into account the render target color space once graphite has color management.
    SkColor4f color = this->mapColorToDst(nullptr);
    BlendColorFilterBlock::BlendColorFilterData data(fMode, color.premul());

    BlendColorFilterBlock::BeginBlock(keyContext, builder, gatherer, data);
    builder->endBlock();
}

#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilters::Blend(const SkColor4f& color,
                                           sk_sp<SkColorSpace> colorSpace,
                                           SkBlendMode mode) {
    if (!SkIsValidMode(mode)) {
        return nullptr;
    }

    // first collapse some modes if possible
    float alpha = color.fA;
    if (SkBlendMode::kClear == mode) {
        return SkModeColorFilter::Make(SkColors::kTransparent, nullptr, SkBlendMode::kSrc);
    } else if (SkBlendMode::kSrcOver == mode) {
        if (0.f == alpha) {
            mode = SkBlendMode::kDst;
        } else if (1.f == alpha) {
            mode = SkBlendMode::kSrc;
        }
        // else just stay srcover
    }

    // weed out combinations that are noops, and just return null
    if (SkBlendMode::kDst == mode ||
        (0.f == alpha && (SkBlendMode::kSrcOver == mode ||
                          SkBlendMode::kDstOver == mode ||
                          SkBlendMode::kDstOut == mode ||
                          SkBlendMode::kSrcATop == mode ||
                          SkBlendMode::kXor == mode ||
                          SkBlendMode::kDarken == mode)) ||
            (1.f == alpha && SkBlendMode::kDstIn == mode)) {
        return nullptr;
    }

    return SkModeColorFilter::Make(color, std::move(colorSpace), mode);
}

sk_sp<SkColorFilter> SkColorFilters::Blend(SkColor color, SkBlendMode mode) {
    return Blend(SkColor4f::FromColor(color), /*sRGB*/nullptr, mode);
}
