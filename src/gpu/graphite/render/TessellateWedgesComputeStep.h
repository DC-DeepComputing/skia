/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_TessellateWedgesComputeStep_DEFINED
#define skgpu_graphite_render_TessellateWedgesComputeStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

// TODO: TessellateWedgesVertexStep should yield one or more ComputeSteps
class TessellateWedgesComputeStep final : public ComputeStep {
public:
    TessellateWedgesComputeStep();
    ~TessellateWedgesComputeStep() override = default;

    // TODO: pipeline data layout
    // TODO: uniform declarations
    // TODO: uniform data preprocessing

    const char* computeSkSL() const override;
    void writeInputGeometry(const DrawParams&, int ssboIndex, void* inputBuffer) const override;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_TessellateWedgesComputeStep_DEFINED
