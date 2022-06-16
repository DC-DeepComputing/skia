/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_VerticesRenderStep_DEFINED
#define skgpu_graphite_render_VerticesRenderStep_DEFINED

#include "include/core/SkVertices.h"
#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class VerticesRenderStep final : public RenderStep {
public:
    explicit VerticesRenderStep(const SkVertices*, PrimitiveType);

    ~VerticesRenderStep() override;

    const char* vertexSkSL() const override;
    void writeVertices(DrawWriter*, const DrawParams&) const override;
    void writeUniforms(const DrawParams&, SkPipelineDataGatherer*) const override;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_VerticesRenderStep_DEFINED
