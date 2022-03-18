/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_render_TessellateWedgesRenderStep_DEFINED
#define skgpu_render_TessellateWedgesRenderStep_DEFINED

#include "experimental/graphite/src/Renderer.h"
#include "include/core/SkString.h"

namespace skgpu {

class TessellateWedgesRenderStep final : public RenderStep {
public:
    TessellateWedgesRenderStep(const char* variantName,
                               DepthStencilSettings depthStencilSettings);

    ~TessellateWedgesRenderStep() override;

    const char* name() const override { return fName.c_str(); }

    const char* vertexSkSL() const override;
    void writeVertices(DrawWriter*,
                       const SkIRect&,
                       const Transform&,
                       const Shape&) const override;
    sk_sp<SkUniformData> writeUniforms(Layout,
                                       const SkIRect&,
                                       const Transform&,
                                       const Shape&) const override;

private:
    const SkString fName;
};

}  // namespace skgpu

#endif // skgpu_render_TessellateWedgesRenderStep_DEFINED
