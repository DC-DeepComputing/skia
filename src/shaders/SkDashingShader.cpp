/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkDashingShader.h"

#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#if defined(SK_GANESH)
#include "src/gpu/ganesh/GrFPArgs.h"
#endif

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

SkDashingShader::SkDashingShader(int foo)
        : fFoo(foo) {
}


sk_sp<SkFlattenable> SkDashingShader::CreateProc(SkReadBuffer& buffer) {
    return nullptr;
}

void SkDashingShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(fFoo);
}

bool SkDashingShader::isOpaque() const {
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(SK_GANESH)

std::unique_ptr<GrFragmentProcessor>
SkDashingShader::asFragmentProcessor(const GrFPArgs& args, const MatrixRec& mRec) const {
    return nullptr;
}

#endif // SK_GANESH

#if defined(SK_GRAPHITE)

void SkDashingShader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                               skgpu::graphite::PaintParamsKeyBuilder* builder,
                               skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    DashingShaderBlock::DashingShaderData dashData(fFoo);

    DashingShaderBlock::BeginBlock(keyContext, builder, gatherer, &dashData);
    builder->endBlock();
}

#endif // SK_GRAPHITE

void SkShaderBase::RegisterFlattenables() { SK_REGISTER_FLATTENABLE(SkDashingShader); }

bool SkDashingShader::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
    return false;
}

sk_sp<SkShader> SkShaders::Dashing(int foo) {
    return sk_ref_sp(new SkDashingShader(foo));
}
