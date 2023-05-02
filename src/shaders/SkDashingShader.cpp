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

SkDashingShader::SkDashingShader(const SkPMColor4f& color,
                                 const SkPoint line[2],
                                 const float intervals[kFoo],
                                 int numIntervals,
                                 float phase)
        : fColor(color)
        , fPhase(phase) {
    SkASSERT(numIntervals == 2 || numIntervals == 4);   // TODO: also, a multiple of 2
    SkASSERT(numIntervals <= kFoo);
    memcpy(fLine, line, sizeof(fLine));

    fCumulativeIntervals[0] = intervals[0];
    for (int i = 1; i < numIntervals; ++i) {
        fCumulativeIntervals[i] = fCumulativeIntervals[i-1] + intervals[i];
    }
    for (int i = numIntervals; i < kFoo; ++i) {
        fCumulativeIntervals[i] = fCumulativeIntervals[numIntervals-1];
    }
}


sk_sp<SkFlattenable> SkDashingShader::CreateProc(SkReadBuffer& buffer) {
    return nullptr;
}

void SkDashingShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(1);
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

    DashingShaderBlock::DashingShaderData dashData(fColor, fLine,
                                                   fCumulativeIntervals, fPhase);

    DashingShaderBlock::BeginBlock(keyContext, builder, gatherer, &dashData);
    builder->endBlock();
}

#endif // SK_GRAPHITE

void SkRegisterDashingShaderFlattenable() { SK_REGISTER_FLATTENABLE(SkDashingShader); }

bool SkDashingShader::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
    return false;
}

sk_sp<SkShader> SkShaders::Dashing(const SkColor4f& color, const SkPoint line[2],
                                   const float intervals[4], int numIntervals, float phase) {
    return sk_ref_sp(new SkDashingShader(color.premul(), line, intervals,
                                         numIntervals, phase));
}
