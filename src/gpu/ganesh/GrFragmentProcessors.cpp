/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrFragmentProcessors.h"

#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkBlendModeBlender.h"
#include "src/effects/SkShaderMaskFilterImpl.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"
#include "src/shaders/SkShaderBase.h"

#include <memory>
#include <utility>

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

//    if (!SkRuntimeEffectPriv::CanDraw(args.fContext->priv().caps(), fEffect.get())) {
//        return nullptr;
//    }
//
//    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
//            fEffect->uniforms(),
//            fUniforms,
//            args.fDstColorInfo->colorSpace());
//    SkASSERT(uniforms);
//    auto [success, fp] = make_effect_fp(fEffect,
//                                        "runtime_blender",
//                                        std::move(uniforms),
//                                        std::move(srcFP),
//                                        std::move(dstFP),
//                                        SkSpan(fChildren),
//                                        args);
//
//    return success ? std::move(fp) : nullptr;
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
