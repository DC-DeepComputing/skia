/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkVM.h"

class PassthroughColorFilter final : public SkColorFilterBase {
public:

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext*,
                                   const GrColorInfo&) const override;
#endif

#ifdef SK_ENABLE_SKSL
    void addToKey(const SkKeyContext& keyContext,
                  SkPaintParamsKeyBuilder* builder,
                  SkPipelineDataGatherer* gatherer) const override;
#endif

protected:
    skvm::Color onProgram(skvm::Builder*, skvm::Color c,
                          const SkColorInfo&, skvm::Uniforms*, SkArenaAlloc*) const override {
        return c;
    }

    bool onAppendStages(const SkStageRec&, bool) const override { return true; }

    // Should never be flattened / unflattened.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return "PassthroughColorFilter"; }
};


#if SK_SUPPORT_GPU

#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"

GrFPResult PassthroughColorFilter::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                       GrRecordingContext*,
                                                       const GrColorInfo&) const {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
            R"(
                half4 main(half4 inColor) {
                    return inColor;
                }
            )");
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrFPSuccess(GrSkSLFP::Make(effect, "passthrough_fp", std::move(inputFP),
                                      GrSkSLFP::OptFlags::kNone));
}
#endif

#ifdef SK_ENABLE_SKSL

#include "src/core/SkKeyHelpers.h"
#include "src/core/SkPaintParamsKey.h"

void PassthroughColorFilter::addToKey(const SkKeyContext& keyContext,
                                      SkPaintParamsKeyBuilder* builder,
                                      SkPipelineDataGatherer* gatherer) const {
    PassthroughColorFilterBlock::BeginBlock(keyContext, builder, gatherer);
    builder->endBlock();
}
#endif

sk_sp<SkColorFilter> SkColorFilterPriv::MakePassthrough() {
    return sk_sp<SkColorFilter>(new PassthroughColorFilter);
}
