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
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkBlendModeBlender.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkRuntimeBlender.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/effects/SkShaderMaskFilterImpl.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/shaders/SkShaderBase.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace GrFragmentProcessors {
std::unique_ptr<GrFragmentProcessor> Make(const SkMaskFilter* maskfilter,
                                          const GrFPArgs& args,
                                          const SkMatrix& ctm) {
    if (!maskfilter) {
        return nullptr;
    }
    auto mfb = as_MFB(maskfilter);
    if (mfb->type() != SkMaskFilterBase::Type::kShader) {
        return nullptr;
    }
    auto shaderMF = static_cast<const SkShaderMaskFilterImpl*>(maskfilter);
    auto fp = as_SB(shaderMF->shader())->asFragmentProcessor(args, SkShaderBase::MatrixRec(ctm));
    return GrFragmentProcessor::MulInputByChildAlpha(std::move(fp));
}

using ChildType = SkRuntimeEffect::ChildType;

static GrFPResult make_effect_fp(sk_sp<SkRuntimeEffect> effect,
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

std::unique_ptr<GrFragmentProcessor> Make(const SkBlenderBase* bm,
        std::unique_ptr<GrFragmentProcessor> srcFP,
        std::unique_ptr<GrFragmentProcessor> dstFP,
        const GrFPArgs& fpArgs) {
    if (!bm) {
        return nullptr;
    }
    if (bm->type() == SkBlenderBase::Type::kBlendMode) {
        auto mode = static_cast<const SkBlendModeBlender*>(bm)->mode();
        return GrBlendFragmentProcessor::Make(std::move(srcFP), std::move(dstFP), mode);
    }
    SkASSERT(bm->type() == SkBlenderBase::Type::kRuntime);
    auto rtb = static_cast<const SkRuntimeBlender*>(bm);
    if (!SkRuntimeEffectPriv::CanDraw(fpArgs.fContext->priv().caps(), rtb->fEffect.get())) {
        return nullptr;
    }

    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
            rtb->fEffect->uniforms(),
            rtb->fUniforms,
            fpArgs.fDstColorInfo->colorSpace());
    SkASSERT(uniforms);
    auto [success, fp] = make_effect_fp(rtb->fEffect,
                                        "runtime_blender",
                                        std::move(uniforms),
                                        std::move(srcFP),
                                        std::move(dstFP),
                                        SkSpan(rtb->fChildren),
                                        fpArgs);

    return success ? std::move(fp) : nullptr;
    return nullptr;
}

bool IsSupported(const SkMaskFilter* maskfilter) {
    if (!maskfilter) {
        return false;
    }
    auto mfb = as_MFB(maskfilter);
    if (mfb->type() != SkMaskFilterBase::Type::kShader) {
        return false;
    }
    return true;
}
}  // namespace GrFragmentProcessors
