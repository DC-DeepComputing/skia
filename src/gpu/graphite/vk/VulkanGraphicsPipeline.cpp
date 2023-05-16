/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::Make(const VulkanSharedContext* sharedContext
                                                           /* TODO: fill in argument list*/) {
        // VKShaderModule
        // depth stencil state
        // blend state
        // const VulkanSharedContext* sharedContext,
        // SkSL::Compiler* compiler,
        // const RuntimeEffectDictionary* runtimeDict,
        // const GraphicsPipelineDesc& pipelineDesc,
        // const RenderPassDesc& renderPassDesc) {
    // const auto& device = sharedContext->device();

    // SkSL::Program::Interface vsInterface, fsInterface;
    // SkSL::ProgramSettings settings;

    // //ShaderErrorHandler* errorHandler = sharedContext->caps()->shaderErrorHandler();

    // const RenderStep* step =
    //     sharedContext->rendererProvider()->lookup(pipelineDesc.renderStepID());

    // setup_vertex_input_state
    // VkPipelineVertexInputStateCreateInfo vertexInputInfo;

    // TODO: Make into helper function. Create shader stage structs for vertex & frag shader.
    VkPipelineShaderStageCreateInfo shaderStageInfo;
    memset(&shaderStageInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.pNext = nullptr;
    shaderStageInfo.flags = 0;
    shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; //do for both stages
    shaderStageInfo.module = VK_NULL_HANDLE; // pass in module(s)
    shaderStageInfo.pName = "main";
    shaderStageInfo.pSpecializationInfo = nullptr;

    // VkPipelineLayoutCreateInfo layoutCreateInfo;
    // memset(&layoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateFlags));
    // layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // layoutCreateInfo.pNext = nullptr;
    // layoutCreateInfo.flags = 0;
    // // TODO: Generate descriptor sets.
    // layoutCreateInfo.setLayoutCount = 0;
    // layoutCreateInfo.pSetLayouts = VK_NULL_HANDLE;
    // // TODO: Add support for push constants.
    // layoutCreateInfo.pushConstantRangeCount = 0;
    // layoutCreateInfo.pPushConstantRanges = nullptr;

    // VkPipelineLayout pipelineLayout;
    // VkResult result;
    // VULKAN_CALL_RESULT(sharedContext->interface(),
    //                    result,
    //                    CreatePipelineLayout(sharedContext->device(),
    //                                         &layoutCreateInfo,
    //                                         /*const VkAllocationCallbacks*=*/nullptr,
    //                                         &pipelineLayout));
    // if (result != VK_SUCCESS) {
    //     return nullptr;
    // }

    // VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    // memset(&pipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    // pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // pipelineCreateInfo.pNext = nullptr;
    // pipelineCreateInfo.flags = 0;
    // pipelineCreateInfo.stageCount = shaderStageCount;
    // pipelineCreateInfo.pStages = shaderStageInfo;
    // pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    // pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
    // pipelineCreateInfo.pTessellationState = nullptr;
    // pipelineCreateInfo.pViewportState = &viewportInfo;
    // pipelineCreateInfo.pRasterizationState = &rasterInfo;
    // pipelineCreateInfo.pMultisampleState = &multisampleInfo;
    // pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
    // pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
    // pipelineCreateInfo.pDynamicState = &dynamicInfo;
    // pipelineCreateInfo.layout = layout;
    // pipelineCreateInfo.renderPass = compatibleRenderPass;
    // pipelineCreateInfo.subpass = subpass;
    // pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    // pipelineCreateInfo.basePipelineIndex = -1;

    return sk_sp<VulkanGraphicsPipeline>(new VulkanGraphicsPipeline(sharedContext));
}

void VulkanGraphicsPipeline::freeGpuData() {
}

} // namespace skgpu::graphite
