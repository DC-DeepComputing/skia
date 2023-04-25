/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDashingShader_DEFINED
#define SkDashingShader_DEFINED

#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {
class KeyContext;
}

class SkDashingShader : public SkShaderBase {
public:
    static sk_sp<SkShader> Make(int foo);

    SkDashingShader(int foo);

    bool isOpaque() const override;

#if defined(SK_GANESH)
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif
#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

private:
    friend class SkShaderBase;

    SK_FLATTENABLE_HOOKS(SkDashingShader)

    void flatten(SkWriteBuffer&) const override;

    bool appendStages(const SkStageRec&, const MatrixRec&) const override;

    int fFoo;
};

#endif
