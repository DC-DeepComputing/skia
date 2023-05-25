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
#include "src/gpu/graphite/GraphicsPipeline.h"

namespace skgpu::graphite {

class VulkanSharedContext;
class Attribute;

class VulkanGraphicsPipeline final : public GraphicsPipeline {
public:
    static sk_sp<VulkanGraphicsPipeline> Make(const VulkanSharedContext* sharedContext,
                                              VkShaderModule vertexShader,
                                              SkSpan<const Attribute> vertexAttrs,
                                              SkSpan<const Attribute> instanceAttrs,
                                              VkShaderModule fragShader);

    ~VulkanGraphicsPipeline() override {}

    VkPipeline pipeline() const { return fPipeline; }

    VkPipelineLayout layout() const {
        SkASSERT(fPipelineLayout != VK_NULL_HANDLE);
        return fPipelineLayout;
    }

private:
    VulkanGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext
                           /* TODO: fill out argument list */)
        : GraphicsPipeline(sharedContext) { }

    void freeGpuData() override;

    VkPipeline  fPipeline;
    VkPipelineLayout  fPipelineLayout;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED
