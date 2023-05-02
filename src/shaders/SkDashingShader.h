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
    static constexpr int kFoo = 4;

    SkDashingShader(const SkPMColor4f& color,
                    const SkPoint line[2],
                    const float intervals[kFoo],
                    int numIntervals,
                    float phase);

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
    friend void ::SkRegisterDashingShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkDashingShader)

    void flatten(SkWriteBuffer&) const override;

    bool appendStages(const SkStageRec&, const MatrixRec&) const override;

    SkPMColor4f fColor;
    SkPoint fLine[2];
    float fCumulativeIntervals[kFoo];
    int fNumIntervals;
    float fPhase;
};

#endif
