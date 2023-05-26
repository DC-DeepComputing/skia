/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkComposeColorFilter.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkTDArray.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkNoDestructor.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

SkComposeColorFilter::SkComposeColorFilter(sk_sp<SkColorFilter> outer,
                                           sk_sp<SkColorFilter> inner)
    : fOuter(as_CFB_sp(std::move(outer)))
    , fInner(as_CFB_sp(std::move(inner)))
{}

bool SkComposeColorFilter::onIsAlphaUnchanged() const  {
    // Can only claim alphaunchanged support if both our proxys do.
    return fOuter->isAlphaUnchanged() && fInner->isAlphaUnchanged();
}

bool SkComposeColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const  {
    bool innerIsOpaque = shaderIsOpaque;
    if (!fInner->isAlphaUnchanged()) {
        innerIsOpaque = false;
    }
    return fInner->appendStages(rec, shaderIsOpaque) &&
           fOuter->appendStages(rec, innerIsOpaque);
}

#if defined(SK_ENABLE_SKVM)
skvm::Color SkComposeColorFilter::onProgram(skvm::Builder* p, skvm::Color c,
                      const SkColorInfo& dst,
                      skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const  {
           c = fInner->program(p, c, dst, uniforms, alloc);
    return c ? fOuter->program(p, c, dst, uniforms, alloc) : skvm::Color{};
}
#endif

#if defined(SK_GRAPHITE)
void SkComposeColorFilter::addToKey(const skgpu::graphite::KeyContext& keyContext,
              skgpu::graphite::PaintParamsKeyBuilder* builder,
              skgpu::graphite::PipelineDataGatherer* gatherer) const  {
    using namespace skgpu::graphite;

    ComposeColorFilterBlock::BeginBlock(keyContext, builder, gatherer);

    as_CFB(fInner)->addToKey(keyContext, builder, gatherer);
    as_CFB(fOuter)->addToKey(keyContext, builder, gatherer);

    builder->endBlock();
}
#endif // SK_GRAPHITE

void SkComposeColorFilter::flatten(SkWriteBuffer& buffer) const  {
    buffer.writeFlattenable(fOuter.get());
    buffer.writeFlattenable(fInner.get());
}

sk_sp<SkFlattenable> SkComposeColorFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorFilter> outer(buffer.readColorFilter());
    sk_sp<SkColorFilter> inner(buffer.readColorFilter());
    return outer ? outer->makeComposed(std::move(inner)) : inner;
}

sk_sp<SkColorFilter> SkColorFilter::makeComposed(sk_sp<SkColorFilter> inner) const {
    if (!inner) {
        return sk_ref_sp(this);
    }

    return sk_sp<SkColorFilter>(new SkComposeColorFilter(sk_ref_sp(this), std::move(inner)));
}

void SkRegisterComposeColorFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkComposeColorFilter);
}
