/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_BitmapTextRenderStep_DEFINED
#define skgpu_graphite_render_BitmapTextRenderStep_DEFINED

#include "src/gpu/graphite/Renderer.h"

namespace skgpu::graphite {

class BitmapTextRenderStep final : public RenderStep {
public:
    BitmapTextRenderStep(bool isLCD);

    ~BitmapTextRenderStep() override;

    std::string vertexSkSL(const char* ssboIndex) const override;
    std::string texturesAndSamplersSkSL(const ResourceBindingRequirements&,
                                        int* nextBindingIndex) const override;
    std::string fragmentCoverageSkSL(const char* ssboIndex) const override;

    void writeVertices(DrawWriter*, const DrawParams&, uint32_t ssboIndices) const override;
    void writeUniformsAndTextures(const DrawParams&, PipelineDataGatherer*) const override;

private:
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_BitmapTextRenderStep_DEFINED
