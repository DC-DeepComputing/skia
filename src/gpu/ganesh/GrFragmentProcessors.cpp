/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrFragmentProcessors.h"

#include "include/core/SkColorSpace.h"  // IWYU pragma: keep
#include "include/core/SkData.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkBlendModeBlender.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkRuntimeBlender.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/effects/SkShaderMaskFilterImpl.h"
#include "src/gpu/Blend.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/shaders/SkShaderBase.h"


#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace GrFragmentProcessors {
static std::unique_ptr<GrFragmentProcessor>
        make_fp_from_shader_mask_filter(const SkMaskFilterBase* maskfilter,
                                        const GrFPArgs& args,
                                        const SkMatrix& ctm) {
    SkASSERT(maskfilter);
    auto shaderMF = static_cast<const SkShaderMaskFilterImpl*>(maskfilter);
    auto fp = as_SB(shaderMF->shader())->asFragmentProcessor(args, SkShaderBase::MatrixRec(ctm));
    return GrFragmentProcessor::MulInputByChildAlpha(std::move(fp));
}

std::unique_ptr<GrFragmentProcessor> Make(const SkMaskFilter* maskfilter,
                                          const GrFPArgs& args,
                                          const SkMatrix& ctm) {
    if (!maskfilter) {
        return nullptr;
    }
    auto mfb = as_MFB(maskfilter);
    switch (mfb->type()) {
        case SkMaskFilterBase::Type::kShader:
            return make_fp_from_shader_mask_filter(mfb, args, ctm);
        case SkMaskFilterBase::Type::kBlur:
        case SkMaskFilterBase::Type::kEmboss:
        case SkMaskFilterBase::Type::kSDF:
        case SkMaskFilterBase::Type::kTable:
            return nullptr;
    }
    SkUNREACHABLE;
}

using ChildType = SkRuntimeEffect::ChildType;

GrFPResult make_effect_fp(sk_sp<SkRuntimeEffect> effect,
                          const char* name,
                          sk_sp<const SkData> uniforms,
                          std::unique_ptr<GrFragmentProcessor> inputFP,
                          std::unique_ptr<GrFragmentProcessor> destColorFP,
                          SkSpan<const SkRuntimeEffect::ChildPtr> children,
                          const GrFPArgs& childArgs) {
    skia_private::STArray<8, std::unique_ptr<GrFragmentProcessor>> childFPs;
    for (const auto& child : children) {
        std::optional<ChildType> type = child.type();
        if (type == ChildType::kShader) {
            // Convert a SkShader into a child FP.
            SkShaderBase::MatrixRec mRec(SkMatrix::I());
            mRec.markTotalMatrixInvalid();
            auto childFP = as_SB(child.shader())->asFragmentProcessor(childArgs, mRec);
            if (!childFP) {
                return GrFPFailure(std::move(inputFP));
            }
            childFPs.push_back(std::move(childFP));
        } else if (type == ChildType::kColorFilter) {
            // Convert a SkColorFilter into a child FP.
            auto [success, childFP] = as_CFB(child.colorFilter())
                                              ->asFragmentProcessor(/*inputFP=*/nullptr,
                                                                    childArgs.fContext,
                                                                    *childArgs.fDstColorInfo,
                                                                    childArgs.fSurfaceProps);
            if (!success) {
                return GrFPFailure(std::move(inputFP));
            }
            childFPs.push_back(std::move(childFP));
        } else if (type == ChildType::kBlender) {
            // Convert a SkBlender into a child FP.
            auto childFP = GrFragmentProcessors::Make(as_BB(child.blender()),
                                                      /*srcFP=*/nullptr,
                                                      GrFragmentProcessor::DestColor(),
                                                      childArgs);
            if (!childFP) {
                return GrFPFailure(std::move(inputFP));
            }
            childFPs.push_back(std::move(childFP));
        } else {
            // We have a null child effect.
            childFPs.push_back(nullptr);
        }
    }
    auto fp = GrSkSLFP::MakeWithData(std::move(effect),
                                     name,
                                     childArgs.fDstColorInfo->refColorSpace(),
                                     std::move(inputFP),
                                     std::move(destColorFP),
                                     std::move(uniforms),
                                     SkSpan(childFPs));
    SkASSERT(fp);
    return GrFPSuccess(std::move(fp));
}

static std::unique_ptr<GrFragmentProcessor> make_blender_fp(
        const SkRuntimeBlender* rtb,
        std::unique_ptr<GrFragmentProcessor> srcFP,
        std::unique_ptr<GrFragmentProcessor> dstFP,
        const GrFPArgs& fpArgs) {
    SkASSERT(rtb);
    if (!SkRuntimeEffectPriv::CanDraw(fpArgs.fContext->priv().caps(), rtb->effect().get())) {
        return nullptr;
    }

    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
            rtb->effect()->uniforms(),
            rtb->uniforms(),
            fpArgs.fDstColorInfo->colorSpace());
    SkASSERT(uniforms);
    auto children = rtb->children();
    auto [success, fp] = make_effect_fp(rtb->effect(),
                                        "runtime_blender",
                                        std::move(uniforms),
                                        std::move(srcFP),
                                        std::move(dstFP),
                                        SkSpan(children),
                                        fpArgs);

    return success ? std::move(fp) : nullptr;
}

static std::unique_ptr<GrFragmentProcessor> make_blender_fp(
        const SkBlendModeBlender* blender,
        std::unique_ptr<GrFragmentProcessor> srcFP,
        std::unique_ptr<GrFragmentProcessor> dstFP,
        const GrFPArgs& fpArgs) {
    SkASSERT(blender);
    return GrBlendFragmentProcessor::Make(std::move(srcFP), std::move(dstFP), blender->mode());
}

std::unique_ptr<GrFragmentProcessor> Make(const SkBlenderBase* blender,
                                          std::unique_ptr<GrFragmentProcessor> srcFP,
                                          std::unique_ptr<GrFragmentProcessor> dstFP,
                                          const GrFPArgs& fpArgs) {
    if (!blender) {
        return nullptr;
    }
    switch (blender->type()) {
#define M(type)                                                                \
    case SkBlenderBase::BlenderType::k##type:                                  \
        return make_blender_fp(static_cast<const Sk##type##Blender*>(blender), \
                               std::move(srcFP),                               \
                               std::move(dstFP),                               \
                               fpArgs);
        SK_ALL_BLENDERS(M)
#undef M
    }
    SkUNREACHABLE;
}

//static GrFPResult make_compose_fp(std::unique_ptr<GrFragmentProcessor> inputFP,
//                                  GrRecordingContext* context,
//                                  const GrColorInfo& dstColorInfo,
//                                  const SkSurfaceProps& props) {
//    // Unfortunately, we need to clone the input before we know we need it. This lets us return
//    // the original FP if either internal color filter fails.
//    auto inputClone = inputFP ? inputFP->clone() : nullptr;
//
//    auto [innerSuccess, innerFP] =
//            fInner->asFragmentProcessor(std::move(inputFP), context, dstColorInfo, props);
//    if (!innerSuccess) {
//        return GrFPFailure(std::move(inputClone));
//    }
//
//    auto [outerSuccess, outerFP] =
//            fOuter->asFragmentProcessor(std::move(innerFP), context, dstColorInfo, props);
//    if (!outerSuccess) {
//        return GrFPFailure(std::move(inputClone));
//    }
//
//    return GrFPSuccess(std::move(outerFP));
//}

// colorspacexform
//GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
//                               GrRecordingContext* context,
//                               const GrColorInfo& dstColorInfo,
//                               const SkSurfaceProps& props) const override {
//    // wish our caller would let us know if our input was opaque...
//    constexpr SkAlphaType alphaType = kPremul_SkAlphaType;
//    return GrFPSuccess(GrColorSpaceXformEffect::Make(
//            std::move(inputFP), fSrc.get(), alphaType, fDst.get(), alphaType));
//    SkUNREACHABLE;
//}

// Working format
//#if defined(SK_GANESH)
//    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
//                                   GrRecordingContext* context,
//                                   const GrColorInfo& dstColorInfo,
//                                   const SkSurfaceProps& props) const override {
//        sk_sp<SkColorSpace> dstCS = dstColorInfo.refColorSpace();
//        if (!dstCS) { dstCS = SkColorSpace::MakeSRGB(); }
//
//        SkAlphaType workingAT;
//        sk_sp<SkColorSpace> workingCS = this->workingFormat(dstCS, &workingAT);
//
//        GrColorInfo dst = {dstColorInfo.colorType(), dstColorInfo.alphaType(), dstCS},
//                working = {dstColorInfo.colorType(), workingAT, workingCS};
//
//        auto [ok, fp] = as_CFB(fChild)->asFragmentProcessor(
//                GrColorSpaceXformEffect::Make(std::move(inputFP), dst,working), context, working,
//                                              props);
//
//        return ok ? GrFPSuccess(GrColorSpaceXformEffect::Make(std::move(fp), working,dst))
//                  : GrFPFailure(std::move(fp));
//    }
//#endif

// Gaussian
//GrFPResult SkGaussianColorFilter::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
//                                                      GrRecordingContext*,
//                                                      const GrColorInfo&,
//                                                      const SkSurfaceProps&) const {
//    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
//            "half4 main(half4 inColor) {"
//                "half factor = 1 - inColor.a;"
//                "factor = exp(-factor * factor * 4) - 0.018;"
//                "return half4(factor);"
//            "}"
//        );
//    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
//    return GrFPSuccess(GrSkSLFP::Make(effect, "gaussian_fp", std::move(inputFP),
//                                      GrSkSLFP::OptFlags::kNone));
//}

// Matrix
//static std::unique_ptr<GrFragmentProcessor> rgb_to_hsl(std::unique_ptr<GrFragmentProcessor> child) {
//    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
//        "half4 main(half4 color) {"
//            "return $rgb_to_hsl(color.rgb, color.a);"
//        "}"
//    );
//    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
//    return GrSkSLFP::Make(effect, "RgbToHsl", std::move(child),
//                          GrSkSLFP::OptFlags::kPreservesOpaqueInput);
//}
//
//static std::unique_ptr<GrFragmentProcessor> hsl_to_rgb(std::unique_ptr<GrFragmentProcessor> child) {
//    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
//        "half4 main(half4 color) {"
//            "return $hsl_to_rgb(color.rgb, color.a);"
//        "}"
//    );
//    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
//    return GrSkSLFP::Make(effect, "HslToRgb", std::move(child),
//                          GrSkSLFP::OptFlags::kPreservesOpaqueInput);
//}
//GrFPResult SkMatrixColorFilter::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> fp,
//                                                     GrRecordingContext*,
//                                                     const GrColorInfo&,
//                                                     const SkSurfaceProps&) const {
//    switch (fDomain) {
//        case Domain::kRGBA:
//            fp = GrFragmentProcessor::ColorMatrix(std::move(fp), fMatrix,
//                                                  /* unpremulInput = */  true,
//                                                  /* clampRGBOutput = */ true,
//                                                  /* premulOutput = */   true);
//            break;
//
//        case Domain::kHSLA:
//            fp = rgb_to_hsl(std::move(fp));
//            fp = GrFragmentProcessor::ColorMatrix(std::move(fp), fMatrix,
//                                                  /* unpremulInput = */  false,
//                                                  /* clampRGBOutput = */ false,
//                                                  /* premulOutput = */   false);
//            fp = hsl_to_rgb(std::move(fp));
//            break;
//    }
//
//    return GrFPSuccess(std::move(fp));
//}

// mode
//static SkRGBA4f<kDstAT> map_color(const SkColor4f& c, SkColorSpace* src, SkColorSpace* dst) {
//    SkRGBA4f<kDstAT> color = {c.fR, c.fG, c.fB, c.fA};
//    SkColorSpaceXformSteps(src, kUnpremul_SkAlphaType,
//                           dst, kDstAT).apply(color.vec());
//    return color;
//}
//GrFPResult SkModeColorFilter::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
//                                                  GrRecordingContext*,
//                                                  const GrColorInfo& dstColorInfo,
//                                                  const SkSurfaceProps& props) const {
//    if (fMode == SkBlendMode::kDst) {
//        // If the blend mode is "dest," the blend color won't factor into it at all.
//        // We can return the input FP as-is.
//        return GrFPSuccess(std::move(inputFP));
//    }
//
//    SkDEBUGCODE(const bool fpHasConstIO = !inputFP || inputFP->hasConstantOutputForConstantInput();)
//
//    SkPMColor4f color = map_color(fColor, sk_srgb_singleton(), dstColorInfo.colorSpace());
//
//    auto colorFP = GrFragmentProcessor::MakeColor(color);
//    auto xferFP = GrBlendFragmentProcessor::Make(std::move(colorFP), std::move(inputFP), fMode);
//
//    if (xferFP == nullptr) {
//        // This is only expected to happen if the blend mode is "dest" and the input FP is null.
//        // Since we already did an early-out in the "dest" blend mode case, we shouldn't get here.
//        SkDEBUGFAIL("GrBlendFragmentProcessor::Make returned null unexpectedly");
//        return GrFPFailure(nullptr);
//    }
//
//    // With a solid color input this should always be able to compute the blended color
//    // (at least for coeff modes).
//    // Occasionally, we even do better than we started; specifically, in "src" blend mode, we end up
//    // ditching the input FP entirely, which turns a non-constant operation into a constant one.
//    SkASSERT(fMode > SkBlendMode::kLastCoeffMode ||
//             xferFP->hasConstantOutputForConstantInput() >= fpHasConstIO);
//
//    return GrFPSuccess(std::move(xferFP));
//}

//// colorfilter
//GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
//                               GrRecordingContext* context,
//                               const GrColorInfo& colorInfo,
//                               const SkSurfaceProps& props) const override {
//    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
//            fEffect->uniforms(),
//            fUniforms,
//            colorInfo.colorSpace());
//    SkASSERT(uniforms);
//
//    GrFPArgs childArgs(context, &colorInfo, props);
//    return GrFragmentProcessors::make_effect_fp(fEffect,
//                                                "runtime_color_filter",
//                                                std::move(uniforms),
//                                                std::move(inputFP),
//                                                /*destColorFP=*/nullptr,
//                                                SkSpan(fChildren),
//                                                childArgs);
//}

bool IsSupported(const SkMaskFilter* maskfilter) {
    if (!maskfilter) {
        return false;
    }
    auto mfb = as_MFB(maskfilter);
    switch (mfb->type()) {
        case SkMaskFilterBase::Type::kShader:
            return true;
        case SkMaskFilterBase::Type::kBlur:
        case SkMaskFilterBase::Type::kEmboss:
        case SkMaskFilterBase::Type::kSDF:
        case SkMaskFilterBase::Type::kTable:
            return false;
    }
    SkUNREACHABLE;
}
}  // namespace GrFragmentProcessors
