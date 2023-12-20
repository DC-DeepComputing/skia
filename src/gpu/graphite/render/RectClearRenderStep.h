/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_RectClearRenderStep_DEFINED
#define skgpu_graphite_render_RectClearRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class StaticBufferManager;

class RectClearRenderStep final : public RenderStep {
public:
    RectClearRenderStep(StaticBufferManager* bufferManager);

    ~RectClearRenderStep() override;

    std::string vertexSkSL() const override;

    void writeVertices(DrawWriter*, const DrawParams&, skvx::ushort2 ssboIndices) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
};

}  // namespace skgpu::graphite

#endif // skgpu_render_RectClearRenderStep_DEFINED
