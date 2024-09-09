/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GraphicsPipeline_DEFINED
#define skgpu_graphite_GraphicsPipeline_DEFINED

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"

namespace skgpu::graphite {

struct FragSkSLInfo;
class RenderStep;

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

    const char* getResourceType() const override { return "Graphics Pipeline"; }

    DstReadRequirement dstReadRequirement() const { return fMetadata.fDstReadReq; }

    int  numFragTexturesAndSamplers() const { return fMetadata.fNumFragTexturesAndSamplers; }
    bool hasPaintUniforms()           const { return fMetadata.fHasPaintUniforms;           }
    bool hasStepUniforms()            const { return fMetadata.fHasStepUniforms;            }
    bool hasGradientBuffer()          const { return fMetadata.fHasGradientBuffer;          }

#if defined(GPU_TEST_UTILS)
    // This is not quite enough information to fully recreate the pipeline, as the RenderPassDesc
    // used to make the pipeline is not preserved.
    struct PipelineInfo {
        uint32_t fRenderStepID;
        UniquePaintParamsID fPaintID;
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
    struct Metadata {
        Metadata(const FragSkSLInfo&, const RenderStep*);
        Metadata() = default;

        DstReadRequirement fDstReadReq = DstReadRequirement::kNone;
        int  fNumFragTexturesAndSamplers = 0;
        bool fHasPaintUniforms  = false;
        bool fHasStepUniforms   = false;
        bool fHasGradientBuffer = false;
    };

    GraphicsPipeline(const SharedContext*, const Metadata&, PipelineInfo*);

private:
    const Metadata fMetadata;

#if defined(GPU_TEST_UTILS)
    PipelineInfo fPipelineInfo;
#endif
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_GraphicsPipeline_DEFINED
