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

class ShaderInfo;
class RenderStep;

enum class PipelineCreationFlags : uint8_t {
    kNone             = 0b000,
    // For Dawn, this flag overrides the DawnCaps::fUseAsyncPipelineCreation
    // parameter and forces Synchronous Pipeline creation.
    kForPrecompilation = 0b001,
};

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

    DstReadRequirement dstReadRequirement() const { return fPipelineInfo.fDstReadReq; }

    int  numFragTexturesAndSamplers() const { return fPipelineInfo.fNumFragTexturesAndSamplers; }
    bool hasPaintUniforms()           const { return fPipelineInfo.fHasPaintUniforms;           }
    bool hasStepUniforms()            const { return fPipelineInfo.fHasStepUniforms;            }
    bool hasGradientBuffer()          const { return fPipelineInfo.fHasGradientBuffer;          }

    struct PipelineInfo {
        PipelineInfo() = default;

        // NOTE: Subclasses must manually fill in native shader code in GPU_TEST_UTILS builds.
        PipelineInfo(const ShaderInfo&, SkEnumBitMask<PipelineCreationFlags>
#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
                     , uint32_t hash, uint32_t id
#endif
                     );

        DstReadRequirement fDstReadReq = DstReadRequirement::kNone;
        int  fNumFragTexturesAndSamplers = 0;
        bool fHasPaintUniforms  = false;
        bool fHasStepUniforms   = false;
        bool fHasGradientBuffer = false;

        // In test-enabled builds, we preserve the generated shader code to display in the viewer
        // slide UI. This is not quite enough information to fully recreate the pipeline, as the
        // RenderPassDesc used to make the pipeline is not preserved.
#if defined(GPU_TEST_UTILS)
        std::string fLabel;

        std::string fSkSLVertexShader;
        std::string fSkSLFragmentShader;
        std::string fNativeVertexShader;
        std::string fNativeFragmentShader;
#endif
#if SK_HISTOGRAMS_ENABLED || defined(GPU_TEST_UTILS)
        bool fFromPrecompile1 = false;
#endif
#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
        const uint32_t fHash1 = 0;
        const uint32_t fID1 = 0;
#endif
    };

#if defined(GPU_TEST_UTILS)
    const PipelineInfo& getPipelineInfo() const {
        return fPipelineInfo;
    }
#endif
#if SK_HISTOGRAMS_ENABLED || defined(GPU_TEST_UTILS)
    bool fromPrecompile1() const { return fPipelineInfo.fFromPrecompile1; }
#endif

protected:
    GraphicsPipeline(const SharedContext*, const PipelineInfo&);

private:
    PipelineInfo fPipelineInfo;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_GraphicsPipeline_DEFINED
