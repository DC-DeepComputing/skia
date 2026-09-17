/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanComputePipeline.h"

#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <vector>

namespace skgpu::graphite {

sk_sp<VulkanComputePipeline> VulkanComputePipeline::Make(
        const VulkanSharedContext* sharedContext,
        VkPipeline pipeline,
        VkPipelineLayout pipelineLayout,
        VkDescriptorSetLayout descriptorSetLayout,
        skia_private::TArray<DescriptorData> descriptorData,
        std::vector<VkDescriptorType> bindingTypes) {
    if (pipeline == VK_NULL_HANDLE || pipelineLayout == VK_NULL_HANDLE) {
        return nullptr;
    }
    return sk_sp<VulkanComputePipeline>(new VulkanComputePipeline(
            sharedContext, pipeline, pipelineLayout, descriptorSetLayout,
            std::move(descriptorData), std::move(bindingTypes)));
}

sk_sp<VulkanComputePipeline> VulkanComputePipeline::Make(const VulkanSharedContext* sharedContext,
                                                         const ComputePipelineDesc& desc) {
    const ComputeStep* step = desc.computeStep();
    std::string sksl = BuildComputeSkSL(sharedContext->caps(), step, BackendApi::kVulkan);
    SkSL::ProgramSettings settings;
    SkSL::NativeShader spirv;
    SkSL::ProgramInterface outInterface;
    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                            sksl,
                            SkSL::ProgramKind::kCompute,
                            settings,
                            &spirv,
                            &outInterface,
                            sharedContext->caps()->shaderErrorHandler())) {
        return nullptr;
    }

    VkShaderModule shaderModule = CreateVulkanShaderModule(
            sharedContext, spirv, VK_SHADER_STAGE_COMPUTE_BIT);
    if (shaderModule == VK_NULL_HANDLE) {
        return nullptr;
    }

    skia_private::TArray<DescriptorData> descriptorData;
    descriptorData.reserve_exact(step->resources().size());
    std::vector<VkDescriptorType> bindingTypes;
    bindingTypes.reserve(step->resources().size());

    int bindingIndex = 0;
    for (const ComputeStep::ResourceDesc& r : step->resources()) {
        DescriptorType descType;
        switch (r.fType) {
            case ComputeStep::ResourceType::kUniformBuffer:
                descType = DescriptorType::kUniformBuffer;
                break;
            case ComputeStep::ResourceType::kStorageBuffer:
            case ComputeStep::ResourceType::kReadOnlyStorageBuffer:
            case ComputeStep::ResourceType::kIndirectBuffer:
                descType = DescriptorType::kStorageBuffer;
                break;
            case ComputeStep::ResourceType::kWriteOnlyStorageTexture:
                descType = DescriptorType::kStorageTexture;
                break;
            case ComputeStep::ResourceType::kReadOnlyTexture:
                descType = DescriptorType::kTexture;
                break;
            case ComputeStep::ResourceType::kSampledTexture:
                descType = DescriptorType::kCombinedTextureSampler;
                break;
        }
        // TODO (thomsmit): If ComputeStep::ResourceDesc ever expands to support arrayed resources,
        // this count will need to dynamically map to the resource's requested size.
        descriptorData.push_back({descType,
                                  /*count=*/1,
                                  bindingIndex++,
                                  PipelineStageFlags::kCompute});
        bindingTypes.push_back(DsTypeEnumToVkDs(descType, PipelineStageFlags::kCompute));
    }

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    DescriptorDataToVkDescSetLayout(sharedContext, descriptorData, &descriptorSetLayout);
    if (descriptorSetLayout == VK_NULL_HANDLE) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyShaderModule(sharedContext->device(), shaderModule, nullptr));
        return nullptr;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkResult result;
    VULKAN_CALL_RESULT(sharedContext,
                       result,
                       CreatePipelineLayout(sharedContext->device(),
                                            &pipelineLayoutInfo,
                                            nullptr,
                                            &pipelineLayout));
    if (result != VK_SUCCESS) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyDescriptorSetLayout(sharedContext->device(), descriptorSetLayout, nullptr));
        VULKAN_CALL(sharedContext->interface(),
                    DestroyShaderModule(sharedContext->device(), shaderModule, nullptr));
        return nullptr;
    }

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.stage.module = shaderModule;
    pipelineInfo.stage.pName = "main";
    pipelineInfo.layout = pipelineLayout;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VULKAN_CALL_RESULT(sharedContext,
                       result,
                       CreateComputePipelines(sharedContext->device(),
                                              sharedContext->getPipelineCache(),
                                              1,
                                              &pipelineInfo,
                                              nullptr,
                                              &pipeline));

    VULKAN_CALL(sharedContext->interface(),
                DestroyShaderModule(sharedContext->device(), shaderModule, nullptr));

    if (result != VK_SUCCESS) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyPipelineLayout(sharedContext->device(), pipelineLayout, nullptr));
        VULKAN_CALL(sharedContext->interface(),
                    DestroyDescriptorSetLayout(sharedContext->device(), descriptorSetLayout, nullptr));
        return nullptr;
    }

    const_cast<VulkanSharedContext*>(sharedContext)->pipelineCompileWasRequired();

    return Make(sharedContext, pipeline, pipelineLayout, descriptorSetLayout,
                std::move(descriptorData), std::move(bindingTypes));
}

VulkanComputePipeline::VulkanComputePipeline(const SharedContext* sharedContext,
                                             VkPipeline pipeline,
                                             VkPipelineLayout pipelineLayout,
                                             VkDescriptorSetLayout descriptorSetLayout,
                                             skia_private::TArray<DescriptorData> descriptorData,
                                             std::vector<VkDescriptorType> bindingTypes)
        : ComputePipeline(sharedContext)
        , fPipeline(pipeline)
        , fPipelineLayout(pipelineLayout)
        , fDescriptorSetLayout(descriptorSetLayout)
        , fDescriptorData(std::move(descriptorData))
        , fBindingTypes(std::move(bindingTypes)) {}

void VulkanComputePipeline::freeGpuData() {
    auto sharedCtxt = static_cast<const VulkanSharedContext*>(this->sharedContext());
    if (fPipeline != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedCtxt->interface(),
                    DestroyPipeline(sharedCtxt->device(), fPipeline, nullptr));
        fPipeline = VK_NULL_HANDLE;
    }
    if (fPipelineLayout != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedCtxt->interface(),
                    DestroyPipelineLayout(sharedCtxt->device(), fPipelineLayout, nullptr));
        fPipelineLayout = VK_NULL_HANDLE;
    }
    if (fDescriptorSetLayout != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedCtxt->interface(),
                    DestroyDescriptorSetLayout(sharedCtxt->device(), fDescriptorSetLayout, nullptr));
        fDescriptorSetLayout = VK_NULL_HANDLE;
    }
}

}  // namespace skgpu::graphite
