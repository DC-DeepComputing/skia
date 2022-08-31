/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_AnalyticRectRenderStep_DEFINED
#define skgpu_graphite_render_CAnalyticRectRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class AnalyticRectRenderStep final : public RenderStep {
public:
    AnalyticRectRenderStep();

    ~AnalyticRectRenderStep() override;

    const char* vertexSkSL() const override;
    void writeVertices(DrawWriter*, const DrawParams&, int ssboIndex) const override;
    void writeUniformsAndTextures(const DrawParams&, SkPipelineDataGatherer*) const override;

private:
};

}  // namespace skgpu::graphite

#endif // skgpu_render_AnalyticRectRenderStep_DEFINED
