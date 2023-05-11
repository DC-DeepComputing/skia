/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
// #include "src/gpu/graphite/Descriptors.h"

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
    int actualDescSetCount = 1;

    const int numUniforms = 2; // TODO: Determine from intrinsic, render step, and paint uniforms.
    const int numTexturesAndSamplers = 1; // TODO: Determine from frag shader.

    // Obtain a descriptor set layout for the determined number of uniforms.
    //SkSpan<DescTypeAndCount> descriptors;
    DescTypeAndCount descTypeCount = DescTypeAndCount(DescriptorType::kUniformBuffer, numUniforms);
    // Get desc set layouts from VulkanResourceProvider(descriptor type, count) [ as opposed to]
    dsLayout[0] =
            VulkanResourceProvider::DescTypeAndCountToVkDescSetLayout(sharedContext, SkSpan<DescTypeAndCount> {&descTypeCount, 1});
    if (numTexturesAndSamplers > 0) {
        // // Adjust and reuse the original descriptors buffer to request a set layout for the determined
        // // number of texture samplers.
        // descCounts[static_cast<int>(DescriptorType::kUniformBuffer)] = 0;
        // descCounts[static_cast<int>(DescriptorType::kCombinedTextureSampler)] =
        //         numTexturesAndSamplers;
        // dsLayout[1] = VulkanResourceProvider::DescTypeAndCountToVkDescSetLayout(sharedContext,
        //                                                                       descCounts);
        // actualDescSetCount++;
    }
    // Using these set layouts, subsequently generate the pipeline layout.


    return sk_sp<VulkanGraphicsPipeline>(new VulkanGraphicsPipeline(sharedContext));
}

void VulkanGraphicsPipeline::freeGpuData() {

}

} // namespace skgpu::graphite
