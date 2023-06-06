/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSweepGradientShader_DEFINED
#define SkSweepGradientShader_DEFINED

#include "include/private/base/SkFloatingPoint.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkLocalMatrixShader.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

#include "src/shaders/gradients/SkGradientBaseShader.h"

class SkSweepGradient final : public SkGradientBaseShader {
public:
    SkSweepGradient(const SkPoint& center, SkScalar t0, SkScalar t1, const Descriptor&);

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override;

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

    SkScalar tBias() const { return fTBias;}
    SkScalar tScale() const { return fTScale; }

protected:
    void flatten(SkWriteBuffer& buffer) const override;

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const override;
#if defined(SK_ENABLE_SKVM)
    skvm::F32 transformT(skvm::Builder*, skvm::Uniforms*,
                         skvm::Coord coord, skvm::I32* mask) const final;
#endif

private:
    friend void ::SkRegisterSweepGradientShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkSweepGradient)

    const SkPoint  fCenter;
    const SkScalar fTBias;
    const SkScalar fTScale;
};

#endif