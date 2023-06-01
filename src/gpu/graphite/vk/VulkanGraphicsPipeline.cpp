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

static VkPrimitiveTopology primitive_type_to_vk_topology(PrimitiveType primitiveType) {
    switch (primitiveType) {
        case PrimitiveType::kTriangles:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveType::kTriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveType::kPoints:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }
    SkUNREACHABLE;
}

static void setup_input_assembly_state(PrimitiveType primitiveType,
                                       VkPipelineInputAssemblyStateCreateInfo* inputAssemblyInfo) {
    memset(inputAssemblyInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    inputAssemblyInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo->pNext = nullptr;
    inputAssemblyInfo->flags = 0;
    inputAssemblyInfo->primitiveRestartEnable = false;
    inputAssemblyInfo->topology = primitive_type_to_vk_topology(primitiveType);
}

static VkStencilOp stencil_op_to_vk_stencil_op(StencilOp op) {
    static const VkStencilOp gTable[] = {
        VK_STENCIL_OP_KEEP,                 // kKeep
        VK_STENCIL_OP_ZERO,                 // kZero
        VK_STENCIL_OP_REPLACE,              // kReplace
        VK_STENCIL_OP_INVERT,               // kInvert
        VK_STENCIL_OP_INCREMENT_AND_WRAP,   // kIncWrap
        VK_STENCIL_OP_DECREMENT_AND_WRAP,   // kDecWrap
        VK_STENCIL_OP_INCREMENT_AND_CLAMP,  // kIncClamp
        VK_STENCIL_OP_DECREMENT_AND_CLAMP,  // kDecClamp
    };
    static_assert(std::size(gTable) == kStencilOpCount);
    static_assert(0 == (int)StencilOp::kKeep);
    static_assert(1 == (int)StencilOp::kZero);
    static_assert(2 == (int)StencilOp::kReplace);
    static_assert(3 == (int)StencilOp::kInvert);
    static_assert(4 == (int)StencilOp::kIncWrap);
    static_assert(5 == (int)StencilOp::kDecWrap);
    static_assert(6 == (int)StencilOp::kIncClamp);
    static_assert(7 == (int)StencilOp::kDecClamp);
    SkASSERT(op < (StencilOp)kStencilOpCount);
    return gTable[(int)op];
}

static VkCompareOp compare_op_to_vk_compare_op(CompareOp op) {
    static const VkCompareOp gTable[] = {
        VK_COMPARE_OP_ALWAYS,              // kAlways
        VK_COMPARE_OP_NEVER,               // kNever
        VK_COMPARE_OP_GREATER,             // kGreater
        VK_COMPARE_OP_GREATER_OR_EQUAL,    // kGEqual
        VK_COMPARE_OP_LESS,                // kLess
        VK_COMPARE_OP_LESS_OR_EQUAL,       // kLEqual
        VK_COMPARE_OP_EQUAL,               // kEqual
        VK_COMPARE_OP_NOT_EQUAL,           // kNotEqual
    };
    static_assert(std::size(gTable) == kCompareOpCount);
    static_assert(0 == (int)CompareOp::kAlways);
    static_assert(1 == (int)CompareOp::kNever);
    static_assert(2 == (int)CompareOp::kGreater);
    static_assert(3 == (int)CompareOp::kGEqual);
    static_assert(4 == (int)CompareOp::kLess);
    static_assert(5 == (int)CompareOp::kLEqual);
    static_assert(6 == (int)CompareOp::kEqual);
    static_assert(7 == (int)CompareOp::kNotEqual);
    SkASSERT(op < (CompareOp)kCompareOpCount);

    return gTable[(int)op];
}

static void setup_stencil_op_state(VkStencilOpState* opState,
                                   const DepthStencilSettings::Face& face,
                                   uint32_t referenceValue) {
    opState->failOp = stencil_op_to_vk_stencil_op(face.fStencilFailOp);
    opState->passOp = stencil_op_to_vk_stencil_op(face.fDepthStencilPassOp);
    opState->depthFailOp = stencil_op_to_vk_stencil_op(face.fDepthFailOp);
    opState->compareOp = compare_op_to_vk_compare_op(face.fCompareOp);
    opState->compareMask = face.fReadMask; // TODO - check this.
    opState->writeMask = face.fWriteMask;
    opState->reference = referenceValue;
}

static void setup_depth_stencil_state(const DepthStencilSettings& stencilSettings,
                                      VkPipelineDepthStencilStateCreateInfo* stencilInfo) {
    SkASSERT(stencilSettings.fDepthTestEnabled ||
             stencilSettings.fDepthCompareOp == CompareOp::kAlways);

    memset(stencilInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    stencilInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    stencilInfo->pNext = nullptr;
    stencilInfo->flags = 0;
    stencilInfo->depthTestEnable = stencilSettings.fDepthTestEnabled;
    stencilInfo->depthWriteEnable = stencilSettings.fDepthWriteEnabled;
    stencilInfo->depthCompareOp = compare_op_to_vk_compare_op(stencilSettings.fDepthCompareOp);
    stencilInfo->depthBoundsTestEnable = VK_FALSE; // Default value TODO - Confirm
    stencilInfo->stencilTestEnable = stencilSettings.fStencilTestEnabled;
    if (stencilSettings.fStencilTestEnabled) {
        setup_stencil_op_state(&stencilInfo->front,
                               stencilSettings.fFrontStencil,
                               stencilSettings.fStencilReferenceValue);
        setup_stencil_op_state(&stencilInfo->back,
                               stencilSettings.fBackStencil,
                               stencilSettings.fStencilReferenceValue);
    }
    stencilInfo->minDepthBounds = 0.0f;
    stencilInfo->maxDepthBounds = 1.0f;
}

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::Make(
        const VulkanSharedContext* sharedContext,
        VkShaderModule vertexShader,
        SkSpan<const Attribute> vertexAttrs,
        SkSpan<const Attribute> instanceAttrs,
        VkShaderModule fragShader,
        DepthStencilSettings stencilSettings,
        PrimitiveType primitiveType) {

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    skia_private::STArray<2, VkVertexInputBindingDescription, true> bindingDescs;
    skia_private::STArray<16, VkVertexInputAttributeDescription> attributeDesc;
    uint32_t totalAttributeCnt = vertexAttrs.size() + instanceAttrs.size();

    SkASSERT(totalAttributeCnt <= sharedContext->vulkanCaps().maxVertexAttributes());
    VkVertexInputAttributeDescription* pAttribs =
            attributeDesc.push_back_n(static_cast<int>(totalAttributeCnt));
    setup_vertex_input_state(vertexAttrs, instanceAttrs, &vertexInputInfo, &bindingDescs, pAttribs);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    setup_input_assembly_state(primitiveType, &inputAssemblyInfo);

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    setup_depth_stencil_state(stencilSettings, &depthStencilInfo);

    // VkPipelineViewportStateCreateInfo viewportInfo;
    // setup_viewport_scissor_state(&viewportInfo);

    // TODO: Set up volor blend attachment & state creation
    //     VkPipelineMultisampleStateCreateInfo multisampleInfo;
    // setup_multisample_state(numSamples, gpu->caps(), &multisampleInfo);

    // // We will only have one color attachment per pipeline.
    // VkPipelineColorBlendAttachmentState attachmentStates[1];
    // VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    // setup_color_blend_state(blendInfo, &colorBlendInfo, attachmentStates);

    // VkPipelineRasterizationStateCreateInfo rasterInfo;
    // setup_raster_state(isWireframe, gpu->caps(), &rasterInfo);


     // TODO: Make into helper function. Create shader stage structs for vertex & frag shader.
    // VkPipelineShaderStageCreateInfo shaderStageInfo;
    // memset(&shaderStageInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    // shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // shaderStageInfo.pNext = nullptr;
    // shaderStageInfo.flags = 0;
    // shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; //do for both stages
    // shaderStageInfo.module = VK_NULL_HANDLE; // pass in module(s)
    // shaderStageInfo.pName = "main";
    // shaderStageInfo.pSpecializationInfo = nullptr;

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
