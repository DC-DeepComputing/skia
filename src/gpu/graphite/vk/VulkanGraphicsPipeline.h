/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanGraphicsPipeline_DEFINED
#define skgpu_graphite_VulkanGraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

namespace SkSL {
    class Compiler;
}

namespace skgpu::graphite {

class Attribute;
class GraphicsPipelineDesc;
class RuntimeEffectDictionary;
class VulkanSharedContext;
struct RenderPassDesc;
class VulkanRenderPass;

class VulkanGraphicsPipeline final : public GraphicsPipeline {
public:
    static sk_sp<VulkanGraphicsPipeline> Make(const VulkanSharedContext*,
                                              SkSL::Compiler* compiler,
                                              const RuntimeEffectDictionary*,
                                              const GraphicsPipelineDesc&,
                                              const RenderPassDesc&,
                                              const sk_sp<VulkanRenderPass>& compatibleRenderPass,
                                              VkPipelineCache);

    ~VulkanGraphicsPipeline() override {}

    VkPipelineLayout layout() const {
        SkASSERT(fPipelineLayout != VK_NULL_HANDLE);
        return fPipelineLayout;
    }

    VkPipeline pipeline() const {
        SkASSERT(fPipeline != VK_NULL_HANDLE);
        return fPipeline;
    }

    bool hasFragmentUniforms() const { return fHasFragmentUniforms; }
    bool hasStepUniforms() const { return fHasStepUniforms; }
    int numTextureSamplers() const { return fNumTextureSamplers; }

private:
    VulkanGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                           PipelineInfo* pipelineInfo,
                           VkPipelineLayout,
                           VkPipeline,
                           bool hasFragmentUniforms,
                           bool hasStepUniforms,
                           int numTextureSamplers);

    void freeGpuData() override;

    VkPipelineLayout fPipelineLayout = VK_NULL_HANDLE;
    VkPipeline fPipeline = VK_NULL_HANDLE;
    bool fHasFragmentUniforms = false;
    bool fHasStepUniforms = false;
    int fNumTextureSamplers = 0;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED
