/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

static inline VkFormat attrib_type_to_vkformat(VertexAttribType type) {
    switch (type) {
        case VertexAttribType::kFloat:
            return VK_FORMAT_R32_SFLOAT;
        case VertexAttribType::kFloat2:
            return VK_FORMAT_R32G32_SFLOAT;
        case VertexAttribType::kFloat3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexAttribType::kFloat4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VertexAttribType::kHalf:
            return VK_FORMAT_R16_SFLOAT;
        case VertexAttribType::kHalf2:
            return VK_FORMAT_R16G16_SFLOAT;
        case VertexAttribType::kHalf4:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case VertexAttribType::kInt2:
            return VK_FORMAT_R32G32_SINT;
        case VertexAttribType::kInt3:
            return VK_FORMAT_R32G32B32_SINT;
        case VertexAttribType::kInt4:
            return VK_FORMAT_R32G32B32A32_SINT;
        case VertexAttribType::kByte:
            return VK_FORMAT_R8_SINT;
        case VertexAttribType::kByte2:
            return VK_FORMAT_R8G8_SINT;
        case VertexAttribType::kByte4:
            return VK_FORMAT_R8G8B8A8_SINT;
        case VertexAttribType::kUByte:
            return VK_FORMAT_R8_UINT;
        case VertexAttribType::kUByte2:
            return VK_FORMAT_R8G8_UINT;
        case VertexAttribType::kUByte4:
            return VK_FORMAT_R8G8B8A8_UINT;
        case VertexAttribType::kUByte_norm:
            return VK_FORMAT_R8_UNORM;
        case VertexAttribType::kUByte4_norm:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case VertexAttribType::kShort2:
            return VK_FORMAT_R16G16_SINT;
        case VertexAttribType::kShort4:
            return VK_FORMAT_R16G16B16A16_SINT;
        case VertexAttribType::kUShort2:
            return VK_FORMAT_R16G16_UINT;
        case VertexAttribType::kUShort2_norm:
            return VK_FORMAT_R16G16_UNORM;
        case VertexAttribType::kInt:
            return VK_FORMAT_R32_SINT;
        case VertexAttribType::kUInt:
            return VK_FORMAT_R32_UINT;
        case VertexAttribType::kUShort_norm:
            return VK_FORMAT_R16_UNORM;
        case VertexAttribType::kUShort4_norm:
            return VK_FORMAT_R16G16B16A16_UNORM;
    }
    SK_ABORT("Unknown vertex attrib type");
}

static void setup_vertex_input_state(
        const SkSpan<const Attribute>& vertexAttrs,
        const SkSpan<const Attribute>& instanceAttrs,
        VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
        skia_private::STArray<2, VkVertexInputBindingDescription, true>* bindingDescs,
        VkVertexInputAttributeDescription* attributeDesc) {
    int vaCount = vertexAttrs.size();
    int iaCount = instanceAttrs.size();

    uint32_t vertexBinding = 0, instanceBinding = 0;

    int nextBinding = bindingDescs->size();
    if (vaCount) {
        vertexBinding = nextBinding++;
    }

    if (iaCount) {
        instanceBinding = nextBinding;
    }

    // setup attribute descriptions
    int attribIndex = 0;
    size_t vertexAttributeOffset = 0;
    for (auto attrib : vertexAttrs) {
        VkVertexInputAttributeDescription& vkAttrib = attributeDesc[attribIndex];
        vkAttrib.location = attribIndex++;  // for now assume location = attribIndex
        vkAttrib.binding = vertexBinding;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = vertexAttributeOffset;
        vertexAttributeOffset += attrib.sizeAlign4();
        attribIndex++;
    }

    size_t instanceAttributeOffset = 0;
    for (auto attrib : instanceAttrs) {
        VkVertexInputAttributeDescription& vkAttrib = attributeDesc[attribIndex];
        vkAttrib.location = attribIndex++;  // for now assume location = attribIndex
        vkAttrib.binding = instanceBinding;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = instanceAttributeOffset;
        instanceAttributeOffset += attrib.sizeAlign4();
        attribIndex++;
    }

    if (vaCount) {
        bindingDescs->push_back() = {
                vertexBinding,
                (uint32_t) vertexAttributeOffset,
                VK_VERTEX_INPUT_RATE_VERTEX
        };
    }
    if (iaCount) {
        bindingDescs->push_back() = {
                instanceBinding,
                (uint32_t) instanceAttributeOffset,
                VK_VERTEX_INPUT_RATE_INSTANCE
        };
    }

    memset(vertexInputInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo->pNext = nullptr;
    vertexInputInfo->flags = 0;
    vertexInputInfo->vertexBindingDescriptionCount = bindingDescs->size();
    vertexInputInfo->pVertexBindingDescriptions = bindingDescs->begin();
    vertexInputInfo->vertexAttributeDescriptionCount = vaCount + iaCount;
    vertexInputInfo->pVertexAttributeDescriptions = attributeDesc;
}

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::Make(
        const VulkanSharedContext* sharedContext,
        VkShaderModule vertexShader,
        SkSpan<const Attribute> vertexAttrs,
        SkSpan<const Attribute> instanceAttrs,
        VkShaderModule fragShader) {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    skia_private::STArray<2, VkVertexInputBindingDescription, true> bindingDescs;
    skia_private::STArray<16, VkVertexInputAttributeDescription> attributeDesc;
    uint32_t totalAttributeCnt = vertexAttrs.size() + instanceAttrs.size();

    SkASSERT(totalAttributeCnt <= sharedContext->vulkanCaps().maxVertexAttributes());
    VkVertexInputAttributeDescription* pAttribs =
            attributeDesc.push_back_n(static_cast<int>(totalAttributeCnt));
    setup_vertex_input_state(vertexAttrs, instanceAttrs, &vertexInputInfo, &bindingDescs, pAttribs);

    return sk_sp<VulkanGraphicsPipeline>(new VulkanGraphicsPipeline(sharedContext));
}

void VulkanGraphicsPipeline::freeGpuData() {
}

} // namespace skgpu::graphite
