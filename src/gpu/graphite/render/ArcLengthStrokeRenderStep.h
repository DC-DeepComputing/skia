/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_ArcLengthStrokeRenderStep_DEFINED
#define skgpu_graphite_render_ArcLengthStrokeRenderStep_DEFINED

#include "src/core/SkVx.h"
#include "src/gpu/graphite/Renderer.h"

#include <string>

namespace skgpu::graphite {

class DrawParams;
class DrawWriter;
class PipelineDataGatherer;

class ArcLengthStrokeRenderStep final : public RenderStep {
public:
    // TODO: If this takes DepthStencilSettings directly and a way to adjust the flags to specify
    // that it performs shading, this RenderStep definition could be used to handle inverse-filled
    // stroke draws.
    explicit ArcLengthStrokeRenderStep(Layout, bool computeSupport, bool infinitySupport,
                                       bool vertexStorageSupport);

    ~ArcLengthStrokeRenderStep() override;

    std::string vertexSkSL() const override;
    const char* fragmentColorSkSL() const override;
    void writeVertices(DrawWriter*, const DrawParams&, uint32_t ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;
    bool hasVertexStorageBuffer() const override { return fVertexStorageSupport; }
    size_t vertexStorageStride() const override;
    std::string vertexStorageSkSL(const ResourceBindingRequirements&, int binding) const override;

private:
    bool fComputeSupport;
    bool fInfinitySupport;
    bool fVertexStorageSupport;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_ArcLengthStrokeRenderStep_DEFINED
