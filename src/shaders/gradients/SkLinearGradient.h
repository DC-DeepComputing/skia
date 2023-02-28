/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearGradient_DEFINED
#define SkLinearGradient_DEFINED

#include "src/shaders/gradients/SkGradientShaderBase.h"

class SkLinearGradient final : public SkGradientShaderBase {
public:
    SkLinearGradient(const SkPoint pts[2], const Descriptor&);

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override;
<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif
#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

protected:
    SkLinearGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const final;

    skvm::F32 transformT(skvm::Builder*, skvm::Uniforms*,
                         skvm::Coord coord, skvm::I32* mask) const final;

private:
    friend void ::SkRegisterLinearGradientShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkLinearGradient)

    class LinearGradient4fContext;

    friend class SkGradientShader;
    using INHERITED = SkGradientShaderBase;
    const SkPoint fStart;
    const SkPoint fEnd;
};

#endif
