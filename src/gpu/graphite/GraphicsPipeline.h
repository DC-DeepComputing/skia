/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GraphicsPipeline_DEFINED
#define skgpu_graphite_GraphicsPipeline_DEFINED

#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"

namespace skgpu::graphite {

/**
 * GraphicsPipeline corresponds to a backend specific pipeline used for rendering (vs. compute),
 * e.g. MTLRenderPipelineState (Metal),
 *      CreateRenderPipeline (Dawn),
 *      CreateGraphicsPipelineState (D3D12),
 *   or VkGraphicsPipelineCreateInfo (Vulkan).
 *
 * A GraphicsPipeline is created from the combination of a GraphicsPipelineDesc (representing draw
 * specific configuration) and a RenderPassDesc (representing the target of the draw).
 */
class GraphicsPipeline : public Resource {
public:
    ~GraphicsPipeline() override;

#if defined(GRAPHITE_TEST_UTILS)
<<<<<<< PATCH SET (0e6ef0 Remove SkTime from the public API)
    struct Shaders {
=======
    // This is not quite enough information to fully recreate the pipeline, as the RenderPassDesc
    // used to make the pipeline is not preserved.
    struct PipelineInfo {
        uint32_t fRenderStepID;
        UniquePaintParamsID fPaintID;
>>>>>>> BASE      (f3f6c7 Revert "Roll Dawn from beaf20f90f1b to b9afa50913de (668 rev)
        std::string fSkSLVertexShader;
        std::string fSkSLFragmentShader;
        std::string fNativeVertexShader;
        std::string fNativeFragmentShader;
    };

    const PipelineInfo& getPipelineInfo() const {
        return fPipelineInfo;
    }
#else
    struct PipelineInfo;
#endif

protected:
    GraphicsPipeline(const SharedContext*, PipelineInfo*);

private:
#if defined(GRAPHITE_TEST_UTILS)
<<<<<<< PATCH SET (0e6ef0 Remove SkTime from the public API)
    Shaders fPipelineShaders;
=======
    PipelineInfo fPipelineInfo;
>>>>>>> BASE      (f3f6c7 Revert "Roll Dawn from beaf20f90f1b to b9afa50913de (668 rev)
#endif
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_GraphicsPipeline_DEFINED
