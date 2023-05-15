/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include <vector>

namespace skgpu::graphite {

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::Make(const VulkanSharedContext* sharedContext
                                                           /* TODO: fill in argument list*/) {
    // TODO: Implement. For now, hard-code the number of uniforms and texture samplers requested to
    // demonstrate integration with descriptor sets.

    // For now, mimic Dawn's behavior and only have up to two different descriptor sets (one for
    // uniforms and one for texture/samplers).
    static int constexpr kMaxDescSetCount = 2;
    VkDescriptorSetLayout dsLayout[kMaxDescSetCount];
    // There will always be at least one uniform descriptor for intrinsic uniform support,
    // and optionally more to support texture samplers.
    // int actualDescSetCount = 1;

    const int numUniforms = 2; // TODO: Determine from intrinsic, render step, and paint uniforms.
    const int numTexturesAndSamplers = 1; // TODO: Determine from frag shader.

    // Obtain a descriptor set layout for the determined number of uniforms.
    DescTypeAndCount uniformDescriptor = DescTypeAndCount(DescriptorType::kUniformBuffer, 1);
    std::vector<DescTypeAndCount> descriptors;
    for (int i = 0; i < numUniforms; i++) {
        descriptors.push_back(uniformDescriptor);
    }
    SkSpan<DescTypeAndCount> uniformDescSpan {&descriptors.front(), numUniforms};

    dsLayout[0] = VulkanResourceProvider::DescTypeAndCountToVkDescSetLayout(sharedContext,
                                                                            uniformDescSpan);

    if (numTexturesAndSamplers > 0) {
        descriptors.clear();
        DescTypeAndCount textureSamplerDesc =
                DescTypeAndCount(DescriptorType::kCombinedTextureSampler, 1);

        for (int i = 0; i < numTexturesAndSamplers; i++) {
            descriptors.push_back(textureSamplerDesc);
        }

        SkSpan<DescTypeAndCount> textureSamplerDescSpan {&descriptors.front(), numUniforms};
        dsLayout[1] = VulkanResourceProvider::DescTypeAndCountToVkDescSetLayout(sharedContext,
                                                                                uniformDescSpan);
    }

    // Using these descriptor set layouts, subsequently generate the pipeline layout.
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateFlags));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.setLayoutCount = numTexturesAndSamplers > 0 ? 2 : 1;
    layoutCreateInfo.pSetLayouts = dsLayout;
    // TODO: Add support for push constants.
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

    VkPipelineLayout pipelineLayout;
    VkResult result;
    VULKAN_CALL_RESULT(sharedContext->interface(),
                       result,
                       CreatePipelineLayout(sharedContext->device(),
                                            &layoutCreateInfo,
                                            /*const VkAllocationCallbacks*=*/ nullptr,
                                            &pipelineLayout));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanGraphicsPipeline>(new VulkanGraphicsPipeline(sharedContext,
                                                                    pipelineLayout));
}

void VulkanGraphicsPipeline::freeGpuData() {

}

} // namespace skgpu::graphite
