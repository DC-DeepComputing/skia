/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/SkAttributes.h"
#include "include/private/SkLog.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/SkSLToBackend.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderInfo.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanSpirvTransforms.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/utils/SkShaderUtils.h"

// DO NOT LAND: includes for the SPIR-V 1.3 / NVIDIA tryjob diagnostics ("spirv_debug") below.
#include "include/core/SkString.h"

#include <algorithm>
#include <atomic>
#include <string>
#include <vector>

// Debug builds of Graphite-Vulkan link SPIRV-Tools (see BUILD.gn), so the diagnostics can
// validate and disassemble modules directly in the log. Release builds only get hex dumps.
#if defined(SK_DEBUG) && defined(__has_include)
#if __has_include("spirv-tools/libspirv.hpp")
#include "spirv-tools/libspirv.hpp"
#define SK_SPIRV_DEBUG_HAS_SPVTOOLS 1
#endif
#endif

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
        case VertexAttribType::kUInt2:
            return VK_FORMAT_R32G32_UINT;
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

// A helper to derive the vertex input state either for version 1 or 2 of the Vulkan structs.
template <typename VertexInputBindingDescription, typename VertexInputAttributeDescription>
static void get_vertex_input_state(
        VkVertexInputRate appendVertexRate,
        const SkSpan<const Attribute>& staticAttrs,
        const SkSpan<const Attribute>& appendAttrs,
        skia_private::STArray<2, VertexInputBindingDescription>& bindingDescs,
        skia_private::STArray<16, VertexInputAttributeDescription>& attributeDescs,
        const VertexInputBindingDescription& defaultBindingDesc,
        const VertexInputAttributeDescription& defaultAttributeDesc) {
    int attribIndex = 0;
    size_t staticAttributeOffset = 0;
    for (auto attrib : staticAttrs) {
        VertexInputAttributeDescription vkAttrib = defaultAttributeDesc;
        vkAttrib.location = attribIndex++;
        vkAttrib.binding = VulkanGraphicsPipeline::kStaticDataBufferIndex;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = staticAttributeOffset;
        staticAttributeOffset += attrib.sizeAlign4();
        attributeDescs.push_back(vkAttrib);
    }

    size_t appendAttributeOffset = 0;
    for (auto attrib : appendAttrs) {
        VertexInputAttributeDescription vkAttrib = defaultAttributeDesc;
        vkAttrib.location = attribIndex++;
        vkAttrib.binding = VulkanGraphicsPipeline::kAppendDataBufferIndex;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = appendAttributeOffset;
        appendAttributeOffset += attrib.sizeAlign4();
        attributeDescs.push_back(vkAttrib);
    }

    if (!staticAttrs.empty()) {
        VertexInputBindingDescription vkBinding = defaultBindingDesc;
        vkBinding.binding = VulkanGraphicsPipeline::kStaticDataBufferIndex;
        vkBinding.stride = (uint32_t)staticAttributeOffset;
        vkBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescs.push_back(vkBinding);
    }
    if (!appendAttrs.empty()) {
        VertexInputBindingDescription vkBinding = defaultBindingDesc;
        vkBinding.binding = VulkanGraphicsPipeline::kAppendDataBufferIndex;
        vkBinding.stride = (uint32_t)appendAttributeOffset;
        vkBinding.inputRate = appendVertexRate;
        bindingDescs.push_back(vkBinding);
    }
}

static void setup_vertex_input_state(
        VkVertexInputRate appendVertexRate,
        const SkSpan<const Attribute>& staticAttrs,
        const SkSpan<const Attribute>& appendAttrs,
        VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
        skia_private::STArray<2, VkVertexInputBindingDescription>& bindingDescs,
        skia_private::STArray<16, VkVertexInputAttributeDescription>& attributeDescs) {
    // Setup attribute & binding descriptions
    get_vertex_input_state(appendVertexRate,
                           staticAttrs,
                           appendAttrs,
                           bindingDescs,
                           attributeDescs,
                           /*defaultBindingDesc=*/{},
                           /*defaultAttributeDesc=*/{});

    *vertexInputInfo = {};
    vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo->vertexBindingDescriptionCount = bindingDescs.size();
    vertexInputInfo->pVertexBindingDescriptions = bindingDescs.begin();
    vertexInputInfo->vertexAttributeDescriptionCount = attributeDescs.size();
    vertexInputInfo->pVertexAttributeDescriptions = attributeDescs.begin();
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

static void setup_input_assembly_state(const Caps& caps,
                                       PrimitiveType primitiveType,
                                       VkPipelineInputAssemblyStateCreateInfo* inputAssemblyInfo) {
    *inputAssemblyInfo = {};
    inputAssemblyInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo->primitiveRestartEnable = VK_FALSE;
    if (caps.useBasicDynamicState()) {
        // Note: when topology is dynamic state, the topology _class_ still needs to be specified.
        inputAssemblyInfo->topology = primitiveType == PrimitiveType::kPoints
                                              ? VK_PRIMITIVE_TOPOLOGY_POINT_LIST
                                              : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    } else {
        inputAssemblyInfo->topology = primitive_type_to_vk_topology(primitiveType);
    }
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
    return SK_UNSAFE_TODO(gTable[(int)op]);
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

    return SK_UNSAFE_TODO(gTable[(int)op]);
}

static void setup_stencil_op_state(VkStencilOpState* opState,
                                   const StencilSettings::Face& face,
                                   uint32_t referenceValue,
                                   uint32_t readMask,
                                   uint32_t writeMask) {
    opState->failOp = stencil_op_to_vk_stencil_op(face.fStencilFailOp);
    opState->passOp = stencil_op_to_vk_stencil_op(face.fDepthStencilPassOp);
    opState->depthFailOp = stencil_op_to_vk_stencil_op(face.fDepthFailOp);
    opState->compareOp = compare_op_to_vk_compare_op(face.fCompareOp);
    opState->compareMask = readMask;
    // Note: On some old ARM driver versions, dynamic state for stencil write mask doesn't work
    // correctly in the presence of discard or alpha to coverage, if the static state provided
    // when creating the pipeline has a value of 0.  However, both alphaToCoverageEnable and
    // rasterizerDiscardEnable are always false in Graphite, so no driver bug workaround is
    // necessary at the moment.
    opState->writeMask = writeMask;
    opState->reference = referenceValue;
}

static void setup_depth_stencil_state(const Caps& caps,
                                      const DepthStencilSettings& depthStencilSettings,
                                      VkPipelineDepthStencilStateCreateInfo* stencilInfo) {
    *stencilInfo = {};
    stencilInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    if (!caps.useBasicDynamicState()) {
        const auto& [depthSettings, stencilSettings] = depthStencilSettings;

        stencilInfo->depthTestEnable = depthSettings.enabled();
        stencilInfo->depthWriteEnable = depthSettings.fWriteEnabled;
        stencilInfo->depthCompareOp = compare_op_to_vk_compare_op(depthSettings.fCompareOp);

        stencilInfo->stencilTestEnable = stencilSettings.enabled();
        if (stencilInfo->stencilTestEnable) {
            setup_stencil_op_state(&stencilInfo->front,
                                   stencilSettings.fFrontFace,
                                   stencilSettings.fReferenceValue,
                                   stencilSettings.fReadMask,
                                   stencilSettings.fWriteMask);
            setup_stencil_op_state(&stencilInfo->back,
                                   stencilSettings.fBackFace,
                                   stencilSettings.fReferenceValue,
                                   stencilSettings.fReadMask,
                                   stencilSettings.fWriteMask);
        }

        stencilInfo->depthBoundsTestEnable = VK_FALSE;
        stencilInfo->minDepthBounds = 0.0f;
        stencilInfo->maxDepthBounds = 1.0f;
    }
}

static void setup_viewport_scissor_state(VkPipelineViewportStateCreateInfo* viewportInfo) {
    *viewportInfo = {};
    viewportInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    // The viewport and scissor are set dyanmically with draw pass commands
    viewportInfo->viewportCount = 1;
    viewportInfo->scissorCount = 1;

    SkASSERT(viewportInfo->viewportCount == viewportInfo->scissorCount);
}

static void setup_multisample_state(SampleCount sampleCount,
                                    VkPipelineMultisampleStateCreateInfo* multisampleInfo) {
    *multisampleInfo = {};
    multisampleInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo->rasterizationSamples = SampleCountToVkSampleCount(sampleCount);
    multisampleInfo->sampleShadingEnable = VK_FALSE;
    multisampleInfo->minSampleShading = 0.0f;
    multisampleInfo->pSampleMask = nullptr;
    multisampleInfo->alphaToCoverageEnable = VK_FALSE;
    multisampleInfo->alphaToOneEnable = VK_FALSE;
}

static VkBlendFactor blend_coeff_to_vk_blend(skgpu::BlendCoeff coeff) {
    switch (coeff) {
        case skgpu::BlendCoeff::kZero:
            return VK_BLEND_FACTOR_ZERO;
        case skgpu::BlendCoeff::kOne:
            return VK_BLEND_FACTOR_ONE;
        case skgpu::BlendCoeff::kSC:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case skgpu::BlendCoeff::kISC:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case skgpu::BlendCoeff::kDC:
            return VK_BLEND_FACTOR_DST_COLOR;
        case skgpu::BlendCoeff::kIDC:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case skgpu::BlendCoeff::kSA:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case skgpu::BlendCoeff::kISA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case skgpu::BlendCoeff::kDA:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case skgpu::BlendCoeff::kIDA:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case skgpu::BlendCoeff::kConstC:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case skgpu::BlendCoeff::kIConstC:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case skgpu::BlendCoeff::kS2C:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case skgpu::BlendCoeff::kIS2C:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case skgpu::BlendCoeff::kS2A:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case skgpu::BlendCoeff::kIS2A:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        case skgpu::BlendCoeff::kIllegal:
            return VK_BLEND_FACTOR_ZERO;
    }
    SkUNREACHABLE;
}

static VkBlendOp blend_equation_to_vk_blend_op(skgpu::BlendEquation equation) {
    static const VkBlendOp gTable[] = {
        // Basic blend ops
        VK_BLEND_OP_ADD,
        VK_BLEND_OP_SUBTRACT,
        VK_BLEND_OP_REVERSE_SUBTRACT,

        // Advanced blend ops
        VK_BLEND_OP_SCREEN_EXT,
        VK_BLEND_OP_OVERLAY_EXT,
        VK_BLEND_OP_DARKEN_EXT,
        VK_BLEND_OP_LIGHTEN_EXT,
        VK_BLEND_OP_COLORDODGE_EXT,
        VK_BLEND_OP_COLORBURN_EXT,
        VK_BLEND_OP_HARDLIGHT_EXT,
        VK_BLEND_OP_SOFTLIGHT_EXT,
        VK_BLEND_OP_DIFFERENCE_EXT,
        VK_BLEND_OP_EXCLUSION_EXT,
        VK_BLEND_OP_MULTIPLY_EXT,
        VK_BLEND_OP_HSL_HUE_EXT,
        VK_BLEND_OP_HSL_SATURATION_EXT,
        VK_BLEND_OP_HSL_COLOR_EXT,
        VK_BLEND_OP_HSL_LUMINOSITY_EXT,

        // Illegal.
        VK_BLEND_OP_ADD,
    };
    static_assert(0 == (int)skgpu::BlendEquation::kAdd);
    static_assert(1 == (int)skgpu::BlendEquation::kSubtract);
    static_assert(2 == (int)skgpu::BlendEquation::kReverseSubtract);
    static_assert(3 == (int)skgpu::BlendEquation::kScreen);
    static_assert(4 == (int)skgpu::BlendEquation::kOverlay);
    static_assert(5 == (int)skgpu::BlendEquation::kDarken);
    static_assert(6 == (int)skgpu::BlendEquation::kLighten);
    static_assert(7 == (int)skgpu::BlendEquation::kColorDodge);
    static_assert(8 == (int)skgpu::BlendEquation::kColorBurn);
    static_assert(9 == (int)skgpu::BlendEquation::kHardLight);
    static_assert(10 == (int)skgpu::BlendEquation::kSoftLight);
    static_assert(11 == (int)skgpu::BlendEquation::kDifference);
    static_assert(12 == (int)skgpu::BlendEquation::kExclusion);
    static_assert(13 == (int)skgpu::BlendEquation::kMultiply);
    static_assert(14 == (int)skgpu::BlendEquation::kHSLHue);
    static_assert(15 == (int)skgpu::BlendEquation::kHSLSaturation);
    static_assert(16 == (int)skgpu::BlendEquation::kHSLColor);
    static_assert(17 == (int)skgpu::BlendEquation::kHSLLuminosity);
    static_assert(std::size(gTable) == skgpu::kBlendEquationCnt);

    SkASSERT((unsigned)equation < skgpu::kBlendEquationCnt);
    return SK_UNSAFE_TODO(gTable[(int)equation]);
}

static void setup_color_blend_state(const VulkanCaps& caps,
                                    const skgpu::BlendInfo& blendInfo,
                                    VkPipelineColorBlendStateCreateInfo* colorBlendInfo,
                                    VkPipelineColorBlendAttachmentState* attachmentState) {
    skgpu::BlendEquation equation = blendInfo.fEquation;
    skgpu::BlendCoeff srcCoeff = blendInfo.fSrcBlend;
    skgpu::BlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = skgpu::BlendShouldDisable(equation, srcCoeff, dstCoeff);

    *attachmentState = {};
    attachmentState->blendEnable = !blendOff;
    if (!blendOff) {
        attachmentState->srcColorBlendFactor = blend_coeff_to_vk_blend(srcCoeff);
        attachmentState->dstColorBlendFactor = blend_coeff_to_vk_blend(dstCoeff);
        attachmentState->colorBlendOp = blend_equation_to_vk_blend_op(equation);
        attachmentState->srcAlphaBlendFactor = blend_coeff_to_vk_blend(srcCoeff);
        attachmentState->dstAlphaBlendFactor = blend_coeff_to_vk_blend(dstCoeff);
        attachmentState->alphaBlendOp = blend_equation_to_vk_blend_op(equation);
    }

    if (!blendInfo.fWritesColor) {
        attachmentState->colorWriteMask = 0;
    } else {
        attachmentState->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    *colorBlendInfo = {};
    colorBlendInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo->logicOpEnable = VK_FALSE;
    colorBlendInfo->attachmentCount = 1;
    colorBlendInfo->pAttachments = attachmentState;
    // colorBlendInfo->blendConstants is set dynamically

    if (caps.supportsRasterizationOrderColorAttachmentAccess()) {
        colorBlendInfo->flags =
                VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT;
    }
}

static void setup_raster_state(const Caps& caps,
                               bool isWireframe,
                               VkPipelineRasterizationStateCreateInfo* rasterInfo) {
    *rasterInfo = {};
    rasterInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo->depthClampEnable = VK_FALSE;
    rasterInfo->rasterizerDiscardEnable = VK_FALSE;
    rasterInfo->polygonMode = isWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterInfo->cullMode = VK_CULL_MODE_NONE;
    rasterInfo->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterInfo->depthBiasEnable = VK_FALSE;
    rasterInfo->depthBiasConstantFactor = 0.0f;
    rasterInfo->depthBiasClamp = 0.0f;
    rasterInfo->depthBiasSlopeFactor = 0.0f;
    rasterInfo->lineWidth = 1.0f;
}

static void setup_shader_stage_info(VkShaderStageFlagBits stage,
                                    VkShaderModule shaderModule,
                                    VkPipelineShaderStageCreateInfo* shaderStageInfo) {
    *shaderStageInfo = {};
    shaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo->stage = stage;
    shaderStageInfo->module = shaderModule;
    shaderStageInfo->pName = "main";
}

static VkDescriptorSetLayout descriptor_data_to_layout(
        const VulkanSharedContext* sharedContext, const SkSpan<DescriptorData>& descriptorData) {
    // descriptorData can be empty to indicate that we should create a mock placeholder layout
    // with no descriptors.
    VkDescriptorSetLayout setLayout;
    DescriptorDataToVkDescSetLayout(sharedContext, descriptorData, &setLayout);
    if (setLayout == VK_NULL_HANDLE) {
        SKIA_LOG_E("Failed to create descriptor set layout; pipeline creation will fail.\n");
        return VK_NULL_HANDLE;
    }
    return setLayout;
}

static void destroy_desc_set_layouts(const VulkanSharedContext* sharedContext,
                                     skia_private::TArray<VkDescriptorSetLayout>& setLayouts) {
    for (int i = 0; i < setLayouts.size(); i++) {
        if (setLayouts[i] != VK_NULL_HANDLE) {
            VULKAN_CALL(sharedContext->interface(),
            DestroyDescriptorSetLayout(sharedContext->device(),
                                       setLayouts[i],
                                       nullptr));
        }
    }
}

static bool input_attachment_desc_set_layout(VkDescriptorSetLayout& outLayout,
                                             const VulkanSharedContext* sharedContext,
                                             bool mockOnly) {
    skia_private::STArray<1, DescriptorData> inputAttachmentDesc;

    if (!mockOnly) {
        inputAttachmentDesc.push_back(VulkanGraphicsPipeline::GetInputAttachmentDescriptor());
    }

    // If mockOnly is true (meaning no input attachment descriptor is actually needed), then still
    // request a mock VkDescriptorSetLayout handle by passing in the unpopulated span.
    outLayout = descriptor_data_to_layout(sharedContext, {inputAttachmentDesc});
    return outLayout != VK_NULL_HANDLE;
}

static bool uniform_desc_set_layout(VkDescriptorSetLayout& outLayout,
                                    const VulkanSharedContext* sharedContext,
                                    bool hasCombinedUniforms,
                                    SkEnumBitMask<PipelineStageFlags> storageStageFlags) {
    // Define a container with size reserved for up to kMaxNumUniformBuffers descriptors. Only add
    // DescriptorData for uniforms that actually are used and need to be included in the layout.
    skia_private::STArray<
            VulkanGraphicsPipeline::kMaxNumUniformBuffers, DescriptorData> uniformDescriptors;

    DescriptorType uniformBufferType =
            sharedContext->caps()->storageBufferSupport() ? DescriptorType::kStorageBufferDynamic
                                                          : DescriptorType::kUniformBufferDynamic;
    if (hasCombinedUniforms) {
        uniformDescriptors.push_back({
                uniformBufferType, /*count=*/1,
                VulkanGraphicsPipeline::kCombinedUniformIndex,
                PipelineStageFlags::kVertexShader | PipelineStageFlags::kFragmentShader});
    }

    if (SkToBool(storageStageFlags)) {
        DescriptorType storageType = sharedContext->caps()->storageBufferSupport()
                ? DescriptorType::kStorageBufferDynamic
                : DescriptorType::kTexture;
        uniformDescriptors.push_back({
                storageType,
                /*count=*/1,
                VulkanGraphicsPipeline::kStorageBufferIndex,
                storageStageFlags});
    }

    // If no uniforms are used, still request a mock VkDescriptorSetLayout handle by passing in the
    // unpopulated span of uniformDescriptors to descriptor set layout creation.
    outLayout = descriptor_data_to_layout(sharedContext, {uniformDescriptors});
    return true;
}

static bool texture_sampler_desc_set_layout(VkDescriptorSetLayout& outLayout,
                                            const VulkanSharedContext* sharedContext,
                                            const int numTextureSamplers,
                                            SkSpan<sk_sp<VulkanSampler>> immutableSamplers) {
    SkASSERT(numTextureSamplers >= 0);
    // The immutable sampler span size must be = the total number of texture/samplers such that
    // we can use the index of a sampler as its binding index (or we just have none, which
    // enables us to skip some of this logic entirely).
    SkASSERT(immutableSamplers.empty() ||
             SkTo<int>(immutableSamplers.size()) == numTextureSamplers);

    skia_private::TArray<DescriptorData> textureSamplerDescs(numTextureSamplers);
    for (int i = 0; i < numTextureSamplers; i++) {
        Sampler* immutableSampler = nullptr;
        if (!immutableSamplers.empty() && immutableSamplers[i]) {
            immutableSampler = immutableSamplers[i].get();
        }
        textureSamplerDescs.push_back({DescriptorType::kCombinedTextureSampler,
                                       /*count=*/1,
                                       /*bindingIdx=*/i,
                                       PipelineStageFlags::kFragmentShader,
                                       immutableSampler});
    }

    // If no texture/samplers are used, a mock VkDescriptorSetLayout handle by passing in the
    // unpopulated span of textureSamplerDescs to descriptor set layout creation.
    outLayout = descriptor_data_to_layout(sharedContext, {textureSamplerDescs});
    return outLayout != VK_NULL_HANDLE;
}

static VkPipelineLayout setup_pipeline_layout(const VulkanSharedContext* sharedContext,
                                              uint32_t pushConstantSize,
                                              VkShaderStageFlagBits pushConstantPipelineStageFlags,
                                              bool hasCombinedUniforms,
                                              SkEnumBitMask<PipelineStageFlags> storageStageFlags,
                                              int numTextureSamplers,
                                              bool loadMsaaFromResolve,
                                              SkSpan<sk_sp<VulkanSampler>> immutableSamplers) {
    // Create a container with the max anticipated amount (kMaxNumDescSets) of VkDescriptorSetLayout
    // handles which will be used to create the pipeline layout.
    skia_private::STArray<
            VulkanGraphicsPipeline::kMaxNumDescSets, VkDescriptorSetLayout> setLayouts;
    setLayouts.push_back_n(VulkanGraphicsPipeline::kMaxNumDescSets, VkDescriptorSetLayout());

    // Populate the container with actual descriptor set layout handles. Each index should contain
    // either a valid/real or a mock/placehodler layout handle. Mock VkDescriptorSetLayouts do not
    // actually contain any descriptors, but are needed as placeholders to maintain expected
    // descriptor set binding indices. This is because VK_NULL_HANDLE is a valid
    // VkDescriptorSetLayout value iff the graphicsPipelineLibrary feature is enabled, which is not
    // the case for all targeted devices (see
    // VUID-VkPipelineLayoutCreateInfo-graphicsPipelineLibrary-06753). If any of the helpers
    // encounter an error (i.e., return false), return a null VkPipelineLayout.
    if (!input_attachment_desc_set_layout(
                setLayouts[VulkanGraphicsPipeline::kDstAsInputDescSetIndex],
                sharedContext,
                /*mockOnly=*/false) || // We always add an input attachment descriptor
        !uniform_desc_set_layout(
                setLayouts[VulkanGraphicsPipeline::kUniformBufferDescSetIndex],
                sharedContext,
                hasCombinedUniforms,
                storageStageFlags) ||
        !texture_sampler_desc_set_layout(
                setLayouts[VulkanGraphicsPipeline::kTextureBindDescSetIndex],
                sharedContext,
                numTextureSamplers,
                immutableSamplers) ||
        !input_attachment_desc_set_layout(
                setLayouts[VulkanGraphicsPipeline::kLoadMsaaFromResolveInputDescSetIndex],
                sharedContext,
                /*mockOnly=*/!loadMsaaFromResolve)) { // Actual descriptor needed iff loading MSAA
        destroy_desc_set_layouts(sharedContext, setLayouts);
        return VK_NULL_HANDLE;
    }

    // Generate a pipeline layout using the now-populated descriptor set layout array
    VkPushConstantRange pushConstantRange;
    if (pushConstantSize) {
        pushConstantRange.offset = 0;
        pushConstantRange.size = pushConstantSize;
        pushConstantRange.stageFlags = pushConstantPipelineStageFlags;
    }
    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = setLayouts.size();
    layoutCreateInfo.pSetLayouts = setLayouts.begin();
    layoutCreateInfo.pushConstantRangeCount = pushConstantSize ? 1 : 0;
    layoutCreateInfo.pPushConstantRanges = pushConstantSize ? &pushConstantRange : nullptr;

    VkResult result;
    VkPipelineLayout layout;
    VULKAN_CALL_RESULT(sharedContext,
                       result,
                       CreatePipelineLayout(sharedContext->device(),
                                            &layoutCreateInfo,
                                            /*const VkAllocationCallbacks*=*/nullptr,
                                            &layout));

    // DescriptorSetLayouts can be deleted after the pipeline layout is created.
    destroy_desc_set_layouts(sharedContext, setLayouts);

    return result == VK_SUCCESS ? layout : VK_NULL_HANDLE;
}

static VkPipeline create_graphics_pipeline(VulkanSharedContext* sharedContext,
                                           VkGraphicsPipelineCreateInfo* pipelineCreateInfo) {
    VkPipeline vkPipeline = VK_NULL_HANDLE;
    VkResult result = VK_PIPELINE_COMPILE_REQUIRED;
    {
        TRACE_EVENT0_ALWAYS("skia.shaders", "CreateGraphicsPipeline");

        if (sharedContext->vulkanCaps().supportsPipelineCreationCacheControl()) {
            TRACE_EVENT0_ALWAYS("skia.shaders", "CreateGraphicsPipeline-CacheLookup");

            pipelineCreateInfo->flags |=
                    VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT;
            {
                // DO NOT LAND: serialize driver compile calls (see ScopedDriverCompileLock).
                ScopedDriverCompileLock compileLock("vkCreateGraphicsPipelines (cache lookup)");
                VULKAN_CALL_RESULT_NOCHECK(
                        sharedContext->interface(),
                        result,
                        CreateGraphicsPipelines(sharedContext->device(),
                                                sharedContext->getPipelineCache(),
                                                /*createInfoCount=*/ 1,
                                                pipelineCreateInfo,
                                                /*pAllocator=*/nullptr,
                                                &vkPipeline));
            }
            sharedContext->checkVkResult(result);
        }

        if (result == VK_PIPELINE_COMPILE_REQUIRED) {
            TRACE_EVENT0_ALWAYS(
                "skia.shaders",
                TRACE_STR_STATIC(sharedContext->vulkanCaps().supportsPipelineCreationCacheControl()
                                         ? "CreateGraphicsPipeline-CompileAfterCacheMiss"
                                         : "CreateGraphicsPipeline-CacheLookupOrCompile"));

            pipelineCreateInfo->flags &=
                    ~VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT;
            // DO NOT LAND: NOCHECK so that a driver compile failure doesn't trip
            // VULKAN_CALL_RESULT's SkASSERT in Debug builds before the spirv_debug diagnostics run.
            {
                // DO NOT LAND: serialize driver compile calls (see ScopedDriverCompileLock).
                ScopedDriverCompileLock compileLock("vkCreateGraphicsPipelines");
                VULKAN_CALL_RESULT_NOCHECK(sharedContext->interface(),
                                           result,
                                           CreateGraphicsPipelines(
                                                   sharedContext->device(),
                                                   sharedContext->getPipelineCache(),
                                                   /*createInfoCount=*/1,
                                                   pipelineCreateInfo,
                                                   /*pAllocator=*/nullptr,
                                                   &vkPipeline));
            }
            sharedContext->checkVkResult(result);
            if (result == VK_SUCCESS) {
                sharedContext->pipelineCompileWasRequired();
            }
        }
    }
    if (result != VK_SUCCESS) {
        SKIA_LOG_E("Failed to create pipeline. Error: %d\n", result);
        return VK_NULL_HANDLE;
    }

    return vkPipeline;
}

static bool create_shaders_pipeline(VulkanSharedContext* sharedContext,
                                    VkGraphicsPipelineCreateInfo& completePipelineInfo,
                                    VkPipelineLibraryCreateInfoKHR& shadersLibraryInfo,
                                    VkGraphicsPipelineLibraryCreateInfoEXT& libraryInfo,
                                    VkPipeline* shadersPipeline) {
    // Provide state that only pertains to the shaders subset to create the library.
    VkGraphicsPipelineCreateInfo shadersPipelineCreateInfo = {};
    shadersPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    shadersPipelineCreateInfo.flags = 0;
    shadersPipelineCreateInfo.stageCount = completePipelineInfo.stageCount;
    shadersPipelineCreateInfo.pStages = completePipelineInfo.pStages;
    shadersPipelineCreateInfo.pViewportState = completePipelineInfo.pViewportState;
    shadersPipelineCreateInfo.pRasterizationState = completePipelineInfo.pRasterizationState;
    shadersPipelineCreateInfo.pMultisampleState = completePipelineInfo.pMultisampleState;
    shadersPipelineCreateInfo.pDepthStencilState = completePipelineInfo.pDepthStencilState;
    shadersPipelineCreateInfo.pDynamicState = completePipelineInfo.pDynamicState;
    shadersPipelineCreateInfo.layout = completePipelineInfo.layout;
    shadersPipelineCreateInfo.renderPass = completePipelineInfo.renderPass;
    shadersPipelineCreateInfo.subpass = completePipelineInfo.subpass;

    if (sharedContext->vulkanCaps().supportsPipelineProtectedAccess()) {
        shadersPipelineCreateInfo.flags |= sharedContext->caps()->protectedSupport()
                                            ? VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT
                                            : VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT;
    }

    // Specify that this is the shaders subset of the pipeline.
    libraryInfo.flags = VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                        VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
    shadersPipelineCreateInfo.flags |= VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    skgpu::AddToPNextChain(&shadersPipelineCreateInfo, &libraryInfo);

    *shadersPipeline = create_graphics_pipeline(sharedContext, &shadersPipelineCreateInfo);
    if (*shadersPipeline == VK_NULL_HANDLE) {
        return false;
    }

    // The complete pipeline doesn't need all the state anymore, it inherits them from the
    // shaders library.
    completePipelineInfo.stageCount = 0;
    completePipelineInfo.pStages = nullptr;
    completePipelineInfo.pViewportState = nullptr;
    completePipelineInfo.pRasterizationState = nullptr;
    completePipelineInfo.pDepthStencilState = nullptr;

    // The complete pipeline provides the vertex input and fragment output interface state.
    // It's important that these states are not built into a separate library, because some
    // drivers can generate more efficient blend code if they have information about the
    // fragment shader.
    libraryInfo.flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
                        VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;
    skgpu::AddToPNextChain(&completePipelineInfo, &libraryInfo);

    // Tell the complete pipeline to link with the shaders library:
    shadersLibraryInfo.libraryCount = 1;
    shadersLibraryInfo.pLibraries = shadersPipeline;
    AddToPNextChain(&completePipelineInfo, &shadersLibraryInfo);

    return true;
}

using VkDynamicStateList = std::array<VkDynamicState, 22>;
static void setup_dynamic_state(const Caps& caps,
                                VkPipelineDynamicStateCreateInfo* dynamicInfo,
                                VkDynamicStateList* dynamicStates) {
    uint32_t count = 0;

    // The following state is always dynamic in Graphite
    (*dynamicStates)[count++] = VK_DYNAMIC_STATE_VIEWPORT;
    (*dynamicStates)[count++] = VK_DYNAMIC_STATE_SCISSOR;
    (*dynamicStates)[count++] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;

    if (caps.useBasicDynamicState()) {
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_LINE_WIDTH;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_DEPTH_BIAS;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;

        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_STENCIL_OP;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE;

        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;

        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_CULL_MODE;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_FRONT_FACE;
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE;
    }
    if (caps.useVertexInputDynamicState()) {
        (*dynamicStates)[count++] = VK_DYNAMIC_STATE_VERTEX_INPUT_EXT;
    }

    // VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT and VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT are not
    // used because Graphite only sets one scissor and viewport.
    //
    // VK_DYNAMIC_STATE_LOGIC_OP_EXT and VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT are irrelevant to
    // Graphite.
    //
    // VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE is not used because it will be eventually made
    // obsolete with VK_EXT_vertex_input_dynamic_state and VK_EXT_graphics_pipeline_library.
    //
    // Dynamic state from VK_EXT_extended_dynamic_state3 is not yet used.  They _could_ be useful,
    // but very few bits are universal and lots of the interesting bits are covered by
    // VK_EXT_graphics_pipeline_library already.  Still, state like
    // VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT or VK_DYNAMIC_STATE_POLYGON_MODE_EXT could be used in
    // the future.

    SK_UNSAFE_TODO(memset(dynamicInfo, 0, sizeof(VkPipelineDynamicStateCreateInfo)));
    dynamicInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo->dynamicStateCount = count;
    dynamicInfo->pDynamicStates = dynamicStates->data();
}

VulkanProgramInfo::~VulkanProgramInfo() {
    if (fVS != VK_NULL_HANDLE) {
        VULKAN_CALL(fSharedContext->interface(),
                    DestroyShaderModule(fSharedContext->device(), fVS, nullptr));
        fVS = VK_NULL_HANDLE;
    }
    if (fFS != VK_NULL_HANDLE) {
        VULKAN_CALL(fSharedContext->interface(),
                    DestroyShaderModule(fSharedContext->device(), fFS, nullptr));
        fFS = VK_NULL_HANDLE;
    }
    if (fLayout != VK_NULL_HANDLE) {
        VULKAN_CALL(fSharedContext->interface(),
                    DestroyPipelineLayout(fSharedContext->device(), fLayout, nullptr));
        fLayout = VK_NULL_HANDLE;
    }
}

const DescriptorData& VulkanGraphicsPipeline::GetInputAttachmentDescriptor() {
    static const DescriptorData descriptor = {
            DescriptorType::kInputAttachment,
            /*count=*/1,
            /*bindingIdx=*/0,  // We only expect to encounter one input attachment
            PipelineStageFlags::kFragmentShader};
    return descriptor;
}

// =================================================================================================
// DO NOT LAND: diagnostics for the NVIDIA (GTX1660 / Windows) "unexpected failure compiling SPIR-V
// shader" seen when SkSL emits SPIR-V 1.3 (first hit by gm/vertices.cpp:vertices_batching).
//
// When vkCreateGraphicsPipelines fails, VulkanGraphicsPipeline::Make() below:
//   1. logs the pipeline label and every Input variable of both stages (and whether it's used),
//   2. re-creates the pipeline with modified SPIR-V to bisect what the driver rejects
//      (per-stage version word, no pipeline library, unused inputs removed, ...), and
//   3. dumps the SkSL and the SPIR-V as hex (decode offline; spvtools isn't linked in Release).
// Grep the dm log for "[spirv-debug]".
//
// The SPIR-V helpers only depend on the standard library; they were tested standalone (validated
// with spirv-val and cross-checked against spirv-opt's RemoveUnusedInterfaceVariables pass).
// =================================================================================================
namespace spirv_debug {
namespace {

// Every failure gets a one-line SUMMARY; full experiments + dumps are budgeted per category.
constexpr int kMaxSummarizedFailures = 500;
constexpr int kMaxDetailedNotReproducible = 2;
constexpr int kMaxDetailedCategoryA = 2;          // fixed by removing unused VS inputs
constexpr int kMaxDetailedOldWorkaroundRegressions = 4;
constexpr int kMaxDetailedCategoryB = 6;          // NOT fixed by removing unused VS inputs
constexpr uint32_t kSPIRV_1_0 = 0x00010000;

constexpr size_t kHeaderWordCount = 5;

constexpr uint32_t kOpSource = 3;
constexpr uint32_t kOpSourceExtension = 4;
constexpr uint32_t kOpName = 5;
constexpr uint32_t kOpMemberName = 6;
constexpr uint32_t kOpString = 7;
constexpr uint32_t kOpLine = 8;
constexpr uint32_t kOpExtension = 10;
constexpr uint32_t kOpExtInstImport = 11;
constexpr uint32_t kOpExtInst = 12;
constexpr uint32_t kOpMemoryModel = 14;
constexpr uint32_t kOpEntryPoint = 15;
constexpr uint32_t kOpExecutionMode = 16;
constexpr uint32_t kOpCapability = 17;
constexpr uint32_t kOpTypeFirst = 19;  // OpTypeVoid
constexpr uint32_t kOpTypeLast = 39;   // OpTypeForwardPointer
constexpr uint32_t kOpConstant = 43;
constexpr uint32_t kOpSpecConstant = 50;
constexpr uint32_t kOpVariable = 59;
constexpr uint32_t kOpLoad = 61;
constexpr uint32_t kOpDecorate = 71;
constexpr uint32_t kOpMemberDecorate = 72;
constexpr uint32_t kOpVectorShuffle = 79;
constexpr uint32_t kOpCompositeExtract = 81;
constexpr uint32_t kOpCompositeInsert = 82;

constexpr uint32_t kStorageClassInput = 1;
constexpr uint32_t kDecorationRelaxedPrecision = 0;
constexpr uint32_t kDecorationBuiltIn = 11;
constexpr uint32_t kDecorationLocation = 30;

struct Instruction {
    size_t offset;  // index of the first word (opcode/word-count) in the module
    uint32_t opcode;
    uint32_t wordCount;
};

struct InputVariable {
    uint32_t id = 0;
    std::string name;
    int location = -1;
    bool builtin = false;
    // True if anything other than debug info, annotations, or OpEntryPoint refers to the variable.
    bool referenced = false;
};

// Splits a module into instructions. Returns false if the module is malformed.
bool parse(const std::vector<uint32_t>& words, std::vector<Instruction>* out) {
    if (words.size() < kHeaderWordCount) {
        return false;
    }
    size_t i = kHeaderWordCount;
    while (i < words.size()) {
        const uint32_t wordCount = words[i] >> 16;
        const uint32_t opcode = words[i] & 0xFFFF;
        if (wordCount == 0 || i + wordCount > words.size()) {
            return false;
        }
        out->push_back({i, opcode, wordCount});
        i += wordCount;
    }
    return true;
}

// Decodes a nul-terminated SPIR-V literal string starting at words[start]. If `next` is non-null
// it receives the index of the first word after the string.
std::string decode_string(const std::vector<uint32_t>& words,
                          size_t start,
                          size_t end,
                          size_t* next = nullptr) {
    std::string result;
    size_t i = start;
    bool terminated = false;
    for (; i < end && !terminated; ++i) {
        const uint32_t word = words[i];
        for (int byte = 0; byte < 4; ++byte) {
            const char c = static_cast<char>((word >> (8 * byte)) & 0xFF);
            if (c == '\0') {
                terminated = true;
                break;
            }
            result.push_back(c);
        }
    }
    if (next) {
        *next = i;
    }
    return result;
}

// Conservatively determines whether `inst` refers to `id` as an operand. Literal operands are
// skipped for the instructions where they are common; a literal that happens to equal `id`
// elsewhere would only cause a variable to be reported as referenced (i.e. not stripped).
bool references_id(const std::vector<uint32_t>& words, const Instruction& inst, uint32_t id) {
    const size_t end = inst.offset + inst.wordCount;
    auto anyOf = [&](size_t first, size_t last) {
        for (size_t w = first; w < last && w < end; ++w) {
            if (words[w] == id) {
                return true;
            }
        }
        return false;
    };
    switch (inst.opcode) {
        case kOpSource:
        case kOpSourceExtension:
        case kOpName:
        case kOpMemberName:
        case kOpString:
        case kOpLine:
        case kOpExtension:
        case kOpExtInstImport:
        case kOpMemoryModel:
        case kOpEntryPoint:
        case kOpExecutionMode:
        case kOpCapability:
        case kOpDecorate:
        case kOpMemberDecorate:
        case kOpConstant:
        case kOpSpecConstant:
            return false;
        case kOpVariable:
            // <result type> <result id> <storage class> [<initializer>]
            return anyOf(inst.offset + 4, inst.offset + 5);
        case kOpExtInst:
            // <result type> <result id> <set> <instruction literal> <operands...>
            return anyOf(inst.offset + 5, end);
        case kOpLoad:
            // <result type> <result id> <pointer> [<memory operands>]
            return anyOf(inst.offset + 3, inst.offset + 4);
        case kOpVectorShuffle:
        case kOpCompositeInsert:
            // <result type> <result id> <id> <id> <literals...>
            return anyOf(inst.offset + 3, inst.offset + 5);
        case kOpCompositeExtract:
            // <result type> <result id> <composite> <literals...>
            return anyOf(inst.offset + 3, inst.offset + 4);
        default:
            if (inst.opcode >= kOpTypeFirst && inst.opcode <= kOpTypeLast) {
                return false;
            }
            return anyOf(inst.offset + 1, end);
    }
}

// Returns every OpVariable with the Input storage class along with its name, location, and
// whether the shader body actually refers to it.
std::vector<InputVariable> collect_inputs(const std::vector<uint32_t>& words) {
    std::vector<InputVariable> vars;
    std::vector<Instruction> insts;
    if (!parse(words, &insts)) {
        return vars;
    }
    for (const Instruction& inst : insts) {
        if (inst.opcode == kOpVariable && inst.wordCount >= 4 &&
            words[inst.offset + 3] == kStorageClassInput) {
            InputVariable var;
            var.id = words[inst.offset + 2];
            vars.push_back(var);
        }
    }
    for (InputVariable& var : vars) {
        for (const Instruction& inst : insts) {
            const size_t end = inst.offset + inst.wordCount;
            if (inst.opcode == kOpName && inst.wordCount >= 3 && words[inst.offset + 1] == var.id) {
                var.name = decode_string(words, inst.offset + 2, end);
            } else if (inst.opcode == kOpDecorate && inst.wordCount >= 3 &&
                       words[inst.offset + 1] == var.id) {
                const uint32_t decoration = words[inst.offset + 2];
                if (decoration == kDecorationLocation && inst.wordCount >= 4) {
                    var.location = static_cast<int>(words[inst.offset + 3]);
                } else if (decoration == kDecorationBuiltIn) {
                    var.builtin = true;
                }
            } else if (references_id(words, inst, var.id)) {
                var.referenced = true;
            }
        }
    }
    return vars;
}

// Ids of user-defined (non-builtin) Input variables that the shader never refers to.
std::vector<uint32_t> unreferenced_input_ids(const std::vector<InputVariable>& vars) {
    std::vector<uint32_t> ids;
    for (const InputVariable& var : vars) {
        if (!var.builtin && !var.referenced) {
            ids.push_back(var.id);
        }
    }
    return ids;
}

std::vector<uint32_t> with_version(const std::vector<uint32_t>& words, uint32_t version) {
    std::vector<uint32_t> result = words;
    if (result.size() > 1) {
        result[1] = version;
    }
    return result;
}

// Returns a copy of `words` with `ids` dropped from every OpEntryPoint interface list. If
// `removeDeclarations` is true, the variables' OpVariable, OpName, and OpDecorate instructions are
// dropped too. The id bound in the header is left as-is (it remains a valid upper bound).
std::vector<uint32_t> strip_variables(const std::vector<uint32_t>& words,
                                      const std::vector<uint32_t>& ids,
                                      bool removeDeclarations) {
    std::vector<Instruction> insts;
    if (!parse(words, &insts)) {
        return words;
    }
    auto shouldStrip = [&](uint32_t id) {
        return std::find(ids.begin(), ids.end(), id) != ids.end();
    };
    std::vector<uint32_t> result;
    result.reserve(words.size());
    for (size_t w = 0; w < kHeaderWordCount; ++w) {
        result.push_back(words[w]);
    }
    for (const Instruction& inst : insts) {
        const size_t end = inst.offset + inst.wordCount;
        if (inst.opcode == kOpEntryPoint && inst.wordCount >= 3) {
            // <execution model> <entry point id> <name literal...> <interface ids...>
            size_t interfaceStart = end;
            decode_string(words, inst.offset + 3, end, &interfaceStart);
            const size_t headerIndex = result.size();
            for (size_t w = inst.offset; w < interfaceStart; ++w) {
                result.push_back(words[w]);
            }
            for (size_t w = interfaceStart; w < end; ++w) {
                if (!shouldStrip(words[w])) {
                    result.push_back(words[w]);
                }
            }
            const uint32_t newWordCount = static_cast<uint32_t>(result.size() - headerIndex);
            result[headerIndex] = (newWordCount << 16) | kOpEntryPoint;
            continue;
        }
        if (removeDeclarations) {
            if (inst.opcode == kOpVariable && inst.wordCount >= 3 &&
                shouldStrip(words[inst.offset + 2])) {
                continue;
            }
            if ((inst.opcode == kOpName || inst.opcode == kOpDecorate) && inst.wordCount >= 2 &&
                shouldStrip(words[inst.offset + 1])) {
                continue;
            }
        }
        for (size_t w = inst.offset; w < end; ++w) {
            result.push_back(words[w]);
        }
    }
    return result;
}

// Returns a copy of `words` without the instructions for which `shouldDrop` returns true.
template <typename ShouldDrop>
std::vector<uint32_t> drop_instructions(const std::vector<uint32_t>& words,
                                        ShouldDrop shouldDrop) {
    std::vector<Instruction> insts;
    if (!parse(words, &insts)) {
        return words;
    }
    std::vector<uint32_t> result;
    result.reserve(words.size());
    for (size_t w = 0; w < kHeaderWordCount; ++w) {
        result.push_back(words[w]);
    }
    for (const Instruction& inst : insts) {
        if (shouldDrop(inst)) {
            continue;
        }
        for (size_t w = inst.offset; w < inst.offset + inst.wordCount; ++w) {
            result.push_back(words[w]);
        }
    }
    return result;
}

// Drops every RelaxedPrecision decoration (OpDecorate and OpMemberDecorate).
std::vector<uint32_t> strip_relaxed_precision(const std::vector<uint32_t>& words) {
    return drop_instructions(words, [&](const Instruction& inst) {
        return (inst.opcode == kOpDecorate && inst.wordCount >= 3 &&
                words[inst.offset + 2] == kDecorationRelaxedPrecision) ||
               (inst.opcode == kOpMemberDecorate && inst.wordCount >= 4 &&
                words[inst.offset + 3] == kDecorationRelaxedPrecision);
    });
}

// Drops OpName/OpMemberName debug instructions.
std::vector<uint32_t> strip_debug_names(const std::vector<uint32_t>& words) {
    return drop_instructions(words, [](const Instruction& inst) {
        return inst.opcode == kOpName || inst.opcode == kOpMemberName;
    });
}

// "name@location,name@location" for the given (unreferenced) variable ids.
std::string describe(const std::vector<InputVariable>& vars, const std::vector<uint32_t>& ids) {
    std::string result;
    for (const InputVariable& var : vars) {
        if (std::find(ids.begin(), ids.end(), var.id) == ids.end()) {
            continue;
        }
        if (!result.empty()) {
            result += ",";
        }
        result += var.name + "@" + std::to_string(var.location);
    }
    return result;
}

void log_inputs(int index, const char* stage, const std::vector<InputVariable>& vars) {
    for (const InputVariable& var : vars) {
        SkDebugf("[spirv-debug] #%d %s input %%%u '%s' location=%d builtin=%d referenced=%d\n",
                 index, stage, var.id, var.name.c_str(), var.location,
                 var.builtin ? 1 : 0, var.referenced ? 1 : 0);
    }
}

void log_text(int index, const char* tag, const std::string& text) {
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            end = text.size();
        }
        SkDebugf("[spirv-debug] #%d %s| %s\n", index, tag, text.substr(start, end - start).c_str());
        start = end + 1;
    }
}

// Decode offline with: spirv_debug_tool <log> "[spirv-debug] #<index> <tag>:"
void log_hex(int index, const char* tag, const std::vector<uint32_t>& words) {
    SkDebugf("[spirv-debug] #%d %s (%zu words)\n", index, tag, words.size());
    SkString line;
    for (size_t i = 0; i < words.size(); ++i) {
        line.appendf(" %08x", words[i]);
        if (i % 8 == 7 || i + 1 == words.size()) {
            SkDebugf("[spirv-debug] #%d %s:%s\n", index, tag, line.c_str());
            line.reset();
        }
    }
}

// Debug builds only (SPIRV-Tools is linked there): spirv-val result + disassembly in the log.
void log_spirv_tools(int index, const char* tag, const std::vector<uint32_t>& words) {
#if defined(SK_SPIRV_DEBUG_HAS_SPVTOOLS)
    spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
    std::string messages;
    tools.SetMessageConsumer(
            [&messages](spv_message_level_t, const char*, const spv_position_t&, const char* m) {
                messages += m;
                messages += '\n';
            });
    const bool valid = tools.Validate(words.data(), words.size());
    SkDebugf("[spirv-debug] #%d %s spirv-val (vulkan1.1): %s\n",
             index, tag, valid ? "VALID" : "INVALID");
    if (!messages.empty()) {
        log_text(index, tag, messages);
    }
    std::string text;
    uint32_t options = spvtools::SpirvTools::kDefaultDisassembleOption;
    options |= SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES | SPV_BINARY_TO_TEXT_OPTION_INDENT;
    if (tools.Disassemble(words.data(), words.size(), &text, options)) {
        log_text(index, (std::string(tag) + " asm").c_str(), text);
    }
#else
    (void)index;
    (void)tag;
    (void)words;
#endif
}

}  // anonymous namespace
}  // namespace spirv_debug
// ======================== end of DO NOT LAND spirv_debug helpers ================================

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::Make(
        VulkanSharedContext* sharedContext,
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t compilationID) {
    VulkanThreadSafeResourceProvider* threadSafeResourceProvider =
        sharedContext->threadSafeResourceProvider();

    SkSL::ProgramSettings settings;
    settings.fSharpenTextures = true;
    settings.fForceNoRTFlip = true;

    ShaderErrorHandler* errorHandler = sharedContext->caps()->shaderErrorHandler();

    const RenderStep* step = sharedContext->rendererProvider()->lookup(pipelineDesc.renderStepID());

    skia_private::TArray<SamplerDesc> descContainer {};
    std::unique_ptr<ShaderInfo> shaderInfo =
            ShaderInfo::Make(sharedContext->caps(),
                             sharedContext->shaderCodeDictionary(),
                             runtimeDict,
                             renderPassDesc,
                             step,
                             pipelineDesc.paintParamsID(),
                             &descContainer);

    if (step->staticAttributes().size() + shaderInfo->appendAttributes().size() >
        sharedContext->vulkanCaps().maxVertexAttributes()) {
        SKIA_LOG_W("Requested more than the supported number of vertex attributes");
        return nullptr;
    }

    // Populate an array of sampler ptrs where a sampler's index within the array indicates their
    // binding index within the descriptor set. Initialize all values to nullptr, which represents a
    // "regular", dynamic sampler at that index.
    skia_private::TArray<sk_sp<VulkanSampler>> immutableSamplers;
    immutableSamplers.push_back_n(shaderInfo->numFragmentTexturesAndSamplers());

    // This logic relies upon Vulkan using combined texture/sampler bindings, which is necessary for
    // ycbcr samplers per the Vulkan spec.
    SkASSERT(!sharedContext->caps()->resourceBindingRequirements().fSeparateTextureAndSamplerBinding
             && shaderInfo->numFragmentTexturesAndSamplers() == descContainer.size());
    for (int i = 0; i < descContainer.size(); i++) {
        // If a SamplerDesc is not equivalent to the default-initialized SamplerDesc, that indicates
        // the usage of an immutable sampler. That sampler desc should then be used to obtain an
        // actual immutable sampler from the resource provider and added at the proper index within
        // immutableSamplers for inclusion in the pipeline layout.
        if (descContainer.at(i) != SamplerDesc()) {
            sk_sp<Sampler> immutableSampler =
                    threadSafeResourceProvider->findOrCreateCompatibleSampler(descContainer.at(i));
            sk_sp<VulkanSampler> vulkanSampler =
                    sk_ref_sp<VulkanSampler>(static_cast<VulkanSampler*>(immutableSampler.get()));
            SkASSERT(vulkanSampler);
            immutableSamplers[i] = std::move(vulkanSampler);
        }
    }

    auto program = VulkanProgramInfo::Make(sharedContext);

    const std::string& fsSkSL = shaderInfo->fragmentSkSL();
    const bool hasFragmentSkSL = !fsSkSL.empty();
    SkSL::NativeShader vsSPIRV, fsSPIRV;
    SkSL::Program::Interface vsInterface, fsInterface;
    if (hasFragmentSkSL) {
        if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                                fsSkSL,
                                SkSL::ProgramKind::kGraphiteFragment,
                                settings,
                                &fsSPIRV,
                                &fsInterface,
                                errorHandler)) {
            return nullptr;
        }

        // Apply transformations to the fragment shader SPIR-V if needed.
        //
        // SkSL->SPIR-V compilations are relatively costly. In anticipation of caching those
        // operations, we allow certain transformations to be applied directly on to compiled
        // SPIR-V. This allows the SkSL to be compiled only once, and the SPIR-V from the cache can
        // simply be directly modified afterwards as needed. This is why transform options exist
        // independently from flags used in SkSL->SPIRV compilation.
        SPIRVTransformOptions options;
        options.fMultisampleInputLoad =
                renderPassDesc.fSampleCount > SampleCount::k1 &&
                shaderInfo->dstReadStrategy() == DstReadStrategy::kReadFromInput;
        if (options.fMultisampleInputLoad) {
            fsSPIRV = TransformSPIRV(fsSPIRV, options);
        }

        if(!program->setFragmentShader(CreateVulkanShaderModule(
                sharedContext, fsSPIRV, VK_SHADER_STAGE_FRAGMENT_BIT))) {
            return nullptr;
        }
    }

    const std::string& vsSkSL = shaderInfo->vertexSkSL();
    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                           vsSkSL,
                           SkSL::ProgramKind::kGraphiteVertex,
                           settings,
                           &vsSPIRV,
                           &vsInterface,
                           errorHandler)) {
        return nullptr;
    }
    if (!program->setVertexShader(CreateVulkanShaderModule(
                sharedContext, vsSPIRV, VK_SHADER_STAGE_VERTEX_BIT))) {
        return nullptr;
    }

    // TODO: Query RenderPassDesc for input attachment information. For now, we only use one for
    // loading MSAA from resolve so we can simply pass in 0 when not doing that.
    if (!program->setLayout(setup_pipeline_layout(
                sharedContext,
                VulkanResourceProvider::kIntrinsicConstantSize,
                VulkanResourceProvider::kIntrinsicConstantStageFlags,
                shaderInfo->hasCombinedUniforms(),
                shaderInfo->storageBufferStages(),
                shaderInfo->numFragmentTexturesAndSamplers(),
                /*loadMsaaFromResolve=*/false,
                SkSpan<sk_sp<VulkanSampler>>(immutableSamplers)))) {
        return nullptr;
    }

    // This pipeline factory is for regular pipelines that run in the main subpass. Depending on
    // if there will be a load-MSAA subpass, the index changes. Ideally with
    // VK_dynamic_rendering_local_read, we can reuse pipelines across subpasses for layer elision.
    int subpassIndex = RenderPassDescWillLoadMSAAFromResolve(renderPassDesc) ? 1 : 0;
    VertexInputBindingDescriptions vertexBindingDescriptions;
    VertexInputAttributeDescriptions vertexAttributeDescriptions;
    VkPipeline shadersPipeline = VK_NULL_HANDLE;
    VkPipeline vkPipeline = MakePipeline(
            sharedContext,
            *program,
            subpassIndex,
            step->primitiveType(),
            step->appendsVertices() ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE,
            step->staticAttributes(),
            shaderInfo->appendAttributes(),
            vertexBindingDescriptions,
            vertexAttributeDescriptions,
            shaderInfo->depthStencilSettings(),
            shaderInfo->blendInfo(),
            renderPassDesc,
            &shadersPipeline);

    // DO NOT LAND: SPIR-V 1.3 / NVIDIA diagnostics (see the spirv_debug block above Make()).
    //
    // Every failing pipeline gets a greppable "SUMMARY cat=..." line:
    //   NOT-REPRODUCIBLE : rebuilding the identical pipeline right away succeeds (state/threading)
    //   A                : fixed by removing the unreferenced vertex-shader inputs
    //   A+OLDWA-REGRESS  : as A, but additionally stripping unused FS inputs (what the reverted
    //                      workaround also did) makes it fail again
    //   B                : NOT fixed by removing unreferenced VS inputs, i.e. a different trigger
    // The first few of each category also get the full experiment set and SkSL/SPIR-V dumps.
    static std::atomic<int> sDiagFailures{0};
    static std::atomic<int> sDiagDetailedNotReproducible{0};
    static std::atomic<int> sDiagDetailedA{0};
    static std::atomic<int> sDiagDetailedOldWorkaround{0};
    static std::atomic<int> sDiagDetailedB{0};
    if (!vkPipeline) {
        const int diagIndex = ++sDiagFailures;
        if (diagIndex <= spirv_debug::kMaxSummarizedFailures) {
            using namespace spirv_debug;
            const Caps* caps = sharedContext->caps();
            const std::vector<uint32_t>& vs = vsSPIRV.fBinary;
            const std::vector<uint32_t>* fs = hasFragmentSkSL ? &fsSPIRV.fBinary : nullptr;
            const char* label = shaderInfo->pipelineLabel().c_str();

            // Re-creates the pipeline from scratch with the given modules and reports whether it
            // succeeded. `fsWords` may be null (no fragment shader). allowShadersLibrary=false
            // forces a monolithic pipeline even when VK_EXT_graphics_pipeline_library is in use.
            auto tryPipeline = [&](const std::string& name,
                                   const std::vector<uint32_t>& vsWords,
                                   const std::vector<uint32_t>* fsWords,
                                   bool allowShadersLibrary) -> bool {
                std::unique_ptr<VulkanProgramInfo> expProgram =
                        VulkanProgramInfo::Make(sharedContext);
                bool setupOk = true;
                if (fsWords) {
                    SkSL::NativeShader fsModule;
                    fsModule.fBinary = *fsWords;
                    setupOk = expProgram->setFragmentShader(CreateVulkanShaderModule(
                            sharedContext, fsModule, VK_SHADER_STAGE_FRAGMENT_BIT));
                }
                if (setupOk) {
                    SkSL::NativeShader vsModule;
                    vsModule.fBinary = vsWords;
                    setupOk = expProgram->setVertexShader(CreateVulkanShaderModule(
                            sharedContext, vsModule, VK_SHADER_STAGE_VERTEX_BIT));
                }
                if (setupOk) {
                    setupOk = expProgram->setLayout(setup_pipeline_layout(
                            sharedContext,
                            VulkanResourceProvider::kIntrinsicConstantSize,
                            VulkanResourceProvider::kIntrinsicConstantStageFlags,
                            shaderInfo->hasCombinedUniforms(),
                            shaderInfo->storageBufferStages(),
                            shaderInfo->numFragmentTexturesAndSamplers(),
                            /*loadMsaaFromResolve=*/false,
                            SkSpan<sk_sp<VulkanSampler>>(immutableSamplers)));
                }
                VkPipeline expShadersPipeline = VK_NULL_HANDLE;
                VkPipeline expPipeline = VK_NULL_HANDLE;
                if (setupOk) {
                    VertexInputBindingDescriptions expBindings;
                    VertexInputAttributeDescriptions expAttributes;
                    expPipeline = MakePipeline(
                            sharedContext,
                            *expProgram,
                            subpassIndex,
                            step->primitiveType(),
                            step->appendsVertices() ? VK_VERTEX_INPUT_RATE_VERTEX
                                                    : VK_VERTEX_INPUT_RATE_INSTANCE,
                            step->staticAttributes(),
                            shaderInfo->appendAttributes(),
                            expBindings,
                            expAttributes,
                            shaderInfo->depthStencilSettings(),
                            shaderInfo->blendInfo(),
                            renderPassDesc,
                            allowShadersLibrary ? &expShadersPipeline : nullptr);
                }
                const bool passed = expPipeline != VK_NULL_HANDLE;
                SkDebugf("[spirv-debug] #%d EXPERIMENT %-66s -> %s\n", diagIndex, name.c_str(),
                         !setupOk ? "SETUP FAILED" : (passed ? "PASS" : "FAIL"));
                for (VkPipeline p : {expPipeline, expShadersPipeline}) {
                    if (p != VK_NULL_HANDLE) {
                        VULKAN_CALL(sharedContext->interface(),
                                    DestroyPipeline(sharedContext->device(), p, nullptr));
                    }
                }
                return passed;
            };

            const std::vector<InputVariable> vsInputs = collect_inputs(vs);
            const std::vector<InputVariable> fsInputs =
                    fs ? collect_inputs(*fs) : std::vector<InputVariable>();
            const std::vector<uint32_t> vsUnused = unreferenced_input_ids(vsInputs);
            const std::vector<uint32_t> fsUnused = unreferenced_input_ids(fsInputs);
            const std::vector<uint32_t> vsStripped =
                    strip_variables(vs, vsUnused, /*removeDeclarations=*/true);
            const std::vector<uint32_t> fsStripped =
                    fs ? strip_variables(*fs, fsUnused, /*removeDeclarations=*/true)
                       : std::vector<uint32_t>();
            const std::vector<uint32_t>* fsStrippedPtr = fs ? &fsStripped : nullptr;

            SkDebugf("[spirv-debug] #%d ===== vkCreateGraphicsPipelines failed: %s\n",
                     diagIndex, label);

            // Classification (always).
            const bool baselinePassed =
                    tryPipeline("classify: baseline (identical VS + FS)", vs, fs, true);
            const bool vsFixPassed =
                    !vsUnused.empty() &&
                    tryPipeline("classify: VS unused inputs fully removed", vsStripped, fs, true);
            const bool bothFixPassed =
                    (!vsUnused.empty() || !fsUnused.empty()) &&
                    tryPipeline("classify: VS+FS unused inputs removed (reverted workaround)",
                                vsStripped, fsStrippedPtr, true);

            const char* category;
            bool detailed;
            if (baselinePassed) {
                category = "NOT-REPRODUCIBLE";
                detailed = ++sDiagDetailedNotReproducible <= kMaxDetailedNotReproducible;
            } else if (vsFixPassed && !bothFixPassed) {
                category = "A+OLDWA-REGRESS";
                detailed = ++sDiagDetailedOldWorkaround <= kMaxDetailedOldWorkaroundRegressions;
            } else if (vsFixPassed) {
                category = "A";
                detailed = ++sDiagDetailedA <= kMaxDetailedCategoryA;
            } else {
                category = "B";
                detailed = ++sDiagDetailedB <= kMaxDetailedCategoryB;
            }
            SkDebugf("[spirv-debug] #%d SUMMARY cat=%s unusedVS=[%s] unusedFS=[%s] baseline=%s "
                     "vsUnusedRemoved=%s bothRemoved=%s | %s\n",
                     diagIndex, category,
                     describe(vsInputs, vsUnused).c_str(),
                     describe(fsInputs, fsUnused).c_str(),
                     baselinePassed ? "PASS" : "FAIL",
                     vsUnused.empty() ? "n/a" : (vsFixPassed ? "PASS" : "FAIL"),
                     (vsUnused.empty() && fsUnused.empty()) ? "n/a"
                                                            : (bothFixPassed ? "PASS" : "FAIL"),
                     label);

            if (detailed) {
                SkDebugf("[spirv-debug] #%d VS label: %s\n",
                         diagIndex, shaderInfo->vsLabel().c_str());
                SkDebugf("[spirv-debug] #%d FS label: %s\n",
                         diagIndex, shaderInfo->fsLabel().c_str());
                SkDebugf("[spirv-debug] #%d VS version=0x%08x, FS version=0x%08x, "
                         "usePipelineLibraries=%d, useVertexInputDynamicState=%d, "
                         "useBasicDynamicState=%d, subpass=%d\n",
                         diagIndex, vs.size() > 1 ? vs[1] : 0u,
                         (fs && fs->size() > 1) ? (*fs)[1] : 0u,
                         caps->usePipelineLibraries() ? 1 : 0,
                         caps->useVertexInputDynamicState() ? 1 : 0,
                         caps->useBasicDynamicState() ? 1 : 0, subpassIndex);
                log_inputs(diagIndex, "VS", vsInputs);
                log_inputs(diagIndex, "FS", fsInputs);

                // Which stage's module trips the 1.3 path?
                const std::vector<uint32_t> vs10 = with_version(vs, kSPIRV_1_0);
                const std::vector<uint32_t> fs10 =
                        fs ? with_version(*fs, kSPIRV_1_0) : std::vector<uint32_t>();
                const std::vector<uint32_t>* fs10Ptr = fs ? &fs10 : nullptr;
                tryPipeline("version: VS 1.0 + FS as-is", vs10, fs, true);
                if (fs) {
                    tryPipeline("version: VS as-is + FS 1.0", vs, fs10Ptr, true);
                    tryPipeline("version: VS 1.0 + FS 1.0", vs10, fs10Ptr, true);
                }
                // Specific to the VK_EXT_graphics_pipeline_library path?
                if (caps->usePipelineLibraries()) {
                    tryPipeline("no pipeline library (monolithic)", vs, fs, false);
                }
                // Unused VS inputs: interface listing vs declaration, and which variable(s).
                if (!vsUnused.empty()) {
                    tryPipeline("VS unused inputs: dropped from OpEntryPoint only",
                                strip_variables(vs, vsUnused, /*removeDeclarations=*/false),
                                fs, true);
                    for (const InputVariable& var : vsInputs) {
                        if (var.builtin || var.referenced) {
                            continue;
                        }
                        const std::string what = "'" + var.name + "' (location " +
                                                 std::to_string(var.location) + ")";
                        tryPipeline("VS only " + what + ": dropped from OpEntryPoint only",
                                    strip_variables(vs, {var.id}, /*removeDeclarations=*/false),
                                    fs, true);
                        tryPipeline("VS only " + what + ": fully removed",
                                    strip_variables(vs, {var.id}, /*removeDeclarations=*/true),
                                    fs, true);
                    }
                }
                if (fs && !fsUnused.empty()) {
                    tryPipeline("FS unused inputs: fully removed", vs, fsStrippedPtr, true);
                }
                // Other candidate triggers (mainly for category B).
                const std::vector<uint32_t> vsNoRP = strip_relaxed_precision(vs);
                const std::vector<uint32_t> fsNoRP =
                        fs ? strip_relaxed_precision(*fs) : std::vector<uint32_t>();
                tryPipeline("RelaxedPrecision stripped: VS only", vsNoRP, fs, true);
                if (fs) {
                    tryPipeline("RelaxedPrecision stripped: FS only", vs, &fsNoRP, true);
                    tryPipeline("RelaxedPrecision stripped: VS + FS", vsNoRP, &fsNoRP, true);
                }
                const std::vector<uint32_t> vsNoNames = strip_debug_names(vs);
                const std::vector<uint32_t> fsNoNames =
                        fs ? strip_debug_names(*fs) : std::vector<uint32_t>();
                tryPipeline("debug names stripped: VS + FS",
                            vsNoNames, fs ? &fsNoNames : nullptr, true);

                log_text(diagIndex, "VS sksl", shaderInfo->vertexSkSL());
                if (fs) {
                    log_text(diagIndex, "FS sksl", shaderInfo->fragmentSkSL());
                }
                log_spirv_tools(diagIndex, "VS", vs);
                if (fs) {
                    log_spirv_tools(diagIndex, "FS", *fs);
                }
                log_hex(diagIndex, "VS hex", vs);
                if (fs) {
                    log_hex(diagIndex, "FS hex", *fs);
                }
                SkDebugf("[spirv-debug] #%d ===== end of details =====\n", diagIndex);
            }
        }
    }
    // ======================== end of DO NOT LAND diagnostics ====================================

    sk_sp<VulkanGraphicsPipeline> pipeline;
    if (vkPipeline) {
        PipelineInfo pipelineInfo{ *shaderInfo, pipelineCreationFlags,
                                    pipelineKey.hash(), compilationID };
#if defined(GPU_TEST_UTILS)
        pipelineInfo.fNativeVertexShader = SkShaderUtils::SpirvAsHexStream(vsSPIRV.fBinary);
        pipelineInfo.fNativeFragmentShader = SkShaderUtils::SpirvAsHexStream(fsSPIRV.fBinary);
#endif

        bool hasPaintParamAttributes = !shaderInfo->appendAttributes().empty();
        pipeline = sk_sp<VulkanGraphicsPipeline>(
                new VulkanGraphicsPipeline(sharedContext,
                                           pipelineInfo,
                                           shaderInfo->pipelineLabel(),
                                           program->releaseLayout(),
                                           vkPipeline,
                                           shadersPipeline,
                                           /*ownsPipelineLayout=*/true,
                                           std::move(immutableSamplers),
                                           step->renderStepID(),
                                           step->primitiveType(),
                                           shaderInfo->depthStencilSettings(),
                                           std::move(vertexBindingDescriptions),
                                           std::move(vertexAttributeDescriptions),
                                           hasPaintParamAttributes));
    }

    return pipeline;
}

VkPipeline VulkanGraphicsPipeline::MakePipeline(
        VulkanSharedContext* sharedContext,
        const VulkanProgramInfo& program,
        int subpassIndex,
        PrimitiveType primitiveType,
        VkVertexInputRate appendInputRate,
        SkSpan<const Attribute> staticAttrs,
        SkSpan<const Attribute> appendAttrs,
        VertexInputBindingDescriptions& vertexBindingDescriptions,
        VertexInputAttributeDescriptions& vertexAttributeDescriptions,
        const DepthStencilSettings& depthStencilSettings,
        const BlendInfo& blendInfo,
        const RenderPassDesc& renderPassDesc,
        VkPipeline* shadersPipeline) {
    SkASSERT(program.layout() && program.vs()); // but a fragment shader isn't required

    VulkanThreadSafeResourceProvider* threadSafeResourceProvider =
        sharedContext->threadSafeResourceProvider();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    skia_private::STArray<2, VkVertexInputBindingDescription> bindingDescs;
    skia_private::STArray<16, VkVertexInputAttributeDescription> attributeDescs;
    if (sharedContext->caps()->useVertexInputDynamicState()) {
        VkVertexInputBindingDescription2EXT defaultBindingDesc = {};
        defaultBindingDesc.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        defaultBindingDesc.divisor = 1;

        VkVertexInputAttributeDescription2EXT defaultAttributeDesc = {};
        defaultAttributeDesc.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;

        get_vertex_input_state(appendInputRate,
                               staticAttrs,
                               appendAttrs,
                               vertexBindingDescriptions,
                               vertexAttributeDescriptions,
                               defaultBindingDesc,
                               defaultAttributeDesc);
    } else {
        setup_vertex_input_state(appendInputRate,
                                 staticAttrs,
                                 appendAttrs,
                                 &vertexInputInfo,
                                 bindingDescs,
                                 attributeDescs);
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    setup_input_assembly_state(*sharedContext->caps(), primitiveType, &inputAssemblyInfo);

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    setup_depth_stencil_state(*sharedContext->caps(), depthStencilSettings, &depthStencilInfo);

    VkPipelineViewportStateCreateInfo viewportInfo;
    setup_viewport_scissor_state(&viewportInfo);

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    setup_multisample_state(renderPassDesc.fSampleCount, &multisampleInfo);

    // We will only have one color blend attachment per pipeline.
    VkPipelineColorBlendAttachmentState attachmentStates[1];
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    setup_color_blend_state(
            sharedContext->vulkanCaps(), blendInfo, &colorBlendInfo, attachmentStates);

    VkPipelineRasterizationStateCreateInfo rasterInfo;
    // TODO: Check for wire frame mode once that is an available context option within graphite.
    setup_raster_state(*sharedContext->caps(), /*isWireframe=*/false, &rasterInfo);

    VkPipelineShaderStageCreateInfo pipelineShaderStages[2];
    setup_shader_stage_info(VK_SHADER_STAGE_VERTEX_BIT,
                            program.vs(),
                            &pipelineShaderStages[0]);
    if (program.fs()) {
        setup_shader_stage_info(VK_SHADER_STAGE_FRAGMENT_BIT,
                                program.fs(),
                                &pipelineShaderStages[1]);
    }

    VkDynamicStateList dynamicStates;
    VkPipelineDynamicStateCreateInfo dynamicInfo;
    setup_dynamic_state(*sharedContext->caps(), &dynamicInfo, &dynamicStates);

    sk_sp<VulkanRenderPass> compatibleRenderPass =
            threadSafeResourceProvider->findOrCreateRenderPass(renderPassDesc,
                                                               /*compatibleOnly=*/true);
    if (!compatibleRenderPass) {
        SKIA_LOG_E("Failed to create compatible renderpass for pipeline");
        return VK_NULL_HANDLE;
    }
    SkDEBUGCODE(int subpassCount = RenderPassDescWillLoadMSAAFromResolve(renderPassDesc) ? 2 : 1;)
    SkASSERT(subpassIndex >= 0 && subpassIndex < subpassCount);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = program.fs() ? 2 : 1;
    pipelineCreateInfo.pStages = &pipelineShaderStages[0];
    pipelineCreateInfo.pVertexInputState =
            sharedContext->caps()->useVertexInputDynamicState() ? nullptr : &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineCreateInfo.pTessellationState = nullptr;
    pipelineCreateInfo.pViewportState = &viewportInfo;
    pipelineCreateInfo.pRasterizationState = &rasterInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
    pipelineCreateInfo.pDynamicState = &dynamicInfo;
    pipelineCreateInfo.layout = program.layout();
    pipelineCreateInfo.renderPass = compatibleRenderPass->renderPass();
    pipelineCreateInfo.subpass = subpassIndex;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    if (sharedContext->vulkanCaps().supportsPipelineProtectedAccess()) {
        pipelineCreateInfo.flags |= sharedContext->caps()->protectedSupport()
                                        ? VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT
                                        : VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT;
    }

    VkPipelineLibraryCreateInfoKHR shadersLibraryInfo = {};
    shadersLibraryInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;

    VkGraphicsPipelineLibraryCreateInfoEXT libraryInfo = {};
    libraryInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT;

    if (shadersPipeline && sharedContext->caps()->usePipelineLibraries()) {
        // When using VK_EXT_graphics_pipeline_library, create the "shaders" subset of the pipeline,
        // and then create the full pipeline (with the vertex input and fragment output state)
        // immediately afterwards using the shaders library, which is a cheap operation.
        // This is typically what vendors without dynamic blend state do in their GL drivers.
        //
        // This can be further optimized by the front-end creating fewer shaders pipelines, and only
        // create multiple full pipelines out of them as needed.
        if (!create_shaders_pipeline(sharedContext,
                                     pipelineCreateInfo,
                                     shadersLibraryInfo,
                                     libraryInfo,
                                     shadersPipeline)) {
            return VK_NULL_HANDLE;
        }
    }

    return create_graphics_pipeline(sharedContext, &pipelineCreateInfo);
}

std::unique_ptr<VulkanProgramInfo> VulkanGraphicsPipeline::CreateLoadMSAAProgram(
        const VulkanSharedContext* sharedContext) {
    SkSL::ProgramSettings settings;
    settings.fForceNoRTFlip = true;
    SkSL::NativeShader vsSPIRV, fsSPIRV;
    ShaderErrorHandler* errorHandler = sharedContext->caps()->shaderErrorHandler();

    std::string vertShaderText;
    vertShaderText.append(
            // MSAA Load Program VS
            "void main() {"
                // Derive [0,1]x[0,1] coordinates from vertex ID and then scale to [-1,1] for NDC.
                "float2 position = float2(sk_VertexID >> 1, sk_VertexID & 1);"
                "sk_Position = float4(2 * position - 1, 0, 1);"
            "}");

    std::string fragShaderText;
    fragShaderText.append(
            "layout(vulkan, input_attachment_index=0, set=" +
            std::to_string(VulkanGraphicsPipeline::kLoadMsaaFromResolveInputDescSetIndex) +
            ", binding=0) subpassInput uInput;"

            // MSAA Load Program FS
            "void main() {"
                "sk_FragColor = subpassLoad(uInput);"
            "}");

    auto program = VulkanProgramInfo::Make(sharedContext);

    SkSL::Program::Interface vsInterface, fsInterface;
    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                            vertShaderText,
                            SkSL::ProgramKind::kGraphiteVertex,
                            settings,
                            &vsSPIRV,
                            &vsInterface,
                            errorHandler)) {
        return nullptr;
    }
    if (!program->setVertexShader(CreateVulkanShaderModule(
                sharedContext, vsSPIRV, VK_SHADER_STAGE_VERTEX_BIT))) {
        return nullptr;
    }

    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                            fragShaderText,
                            SkSL::ProgramKind::kGraphiteFragment,
                            settings,
                            &fsSPIRV,
                            &fsInterface,
                            errorHandler)) {
        return nullptr;
    }
    if (!program->setFragmentShader(CreateVulkanShaderModule(
                sharedContext, fsSPIRV, VK_SHADER_STAGE_FRAGMENT_BIT))) {
        return nullptr;
    }

    // The load msaa pipeline takes no step or paint uniforms and no instance attributes. It only
    // references one input attachment texture (which does not require a sampler) and one vertex
    // attribute (NDC position)
    skia_private::TArray<DescriptorData> inputAttachmentDescriptors(1);
    inputAttachmentDescriptors.push_back(VulkanGraphicsPipeline::GetInputAttachmentDescriptor());
    // This pipeline is used to read from the resolve attachment to a color attachment. We should
    // never require an immutable sampler for this, since that would imply that we are rendering to
    // a surface with an external format.
    if (!program->setLayout(setup_pipeline_layout(
                sharedContext,
                /*pushConstantSize=*/0,
                (VkShaderStageFlagBits)VK_SHADER_STAGE_VERTEX_BIT,
                /*hasCombinedUniforms=*/false,
                /*storageStageFlags=*/{},
                /*numTextureSamplers=*/0,
                /*loadMsaaFromResolve=*/true,
                /*immutableSamplers=*/{}))) {
        return nullptr;
    }

    return program;
}

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::MakeLoadMSAAPipeline(
        VulkanSharedContext* sharedContext,
        const VulkanProgramInfo& loadMSAAProgram,
        const RenderPassDesc& renderPassDesc) {
    // The load MSAA pipeline does not have any vertex or instance attributes, does not use the
    // depth or stencil attachments. Unlike the general Make factory, we do not destroy any of the
    // shader modules or layout. The pipeline layout will not be owned by the created
    // VulkanGraphicsPipeline either, as it's owned by the resource provider.
    SkASSERT(RenderPassDescWillLoadMSAAFromResolve(renderPassDesc));
    VertexInputBindingDescriptions vertexBindingDescriptions;
    VertexInputAttributeDescriptions vertexAttributeDescriptions;
    VkPipeline vkPipeline = MakePipeline(sharedContext,
                                         loadMSAAProgram,
                                         /*subpassIndex=*/0,  // loading to MSAA is always first
                                         PrimitiveType::kTriangleStrip,
                                         /*appendInputRate=*/{},
                                         /*staticAttrs=*/{},
                                         /*appendAttrs=*/{},
                                         vertexBindingDescriptions,
                                         vertexAttributeDescriptions,
                                         /*depthStencilSettings=*/{},
                                         /*blendInfo=*/{},
                                         renderPassDesc,
                                         /*shadersPipeline=*/nullptr);
    if (!vkPipeline) {
        return nullptr;
    }

    SkASSERT(vertexBindingDescriptions.empty());
    SkASSERT(vertexAttributeDescriptions.empty());

    std::string pipelineLabel = "LoadMSAAFromResolve + ";
    pipelineLabel += renderPassDesc.toString().c_str();
    return sk_sp<VulkanGraphicsPipeline>(
            new VulkanGraphicsPipeline(sharedContext,
                                       /*pipelineInfo=*/{},  // leave empty for an internal pipeline
                                       pipelineLabel,
                                       loadMSAAProgram.layout(),
                                       vkPipeline,
                                       /*shadersPipeline=*/VK_NULL_HANDLE,
                                       /*ownsPipelineLayout=*/false,
                                       /*immutableSamplers=*/{},
                                       RenderStep::RenderStepID::kInvalid,
                                       PrimitiveType::kTriangleStrip,
                                       /*depthStencilSettings=*/{},
                                       /*vertexBindingDescriptions=*/{},
                                       /*vertexAttributeDescriptions=*/{},
                                       /*hasPaintParamAttributes=*/false));
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(
        const VulkanSharedContext* sharedContext,
        const PipelineInfo& pipelineInfo,
        std::string_view pipelineLabel,
        VkPipelineLayout pipelineLayout,
        VkPipeline pipeline,
        VkPipeline shadersPipeline,
        bool ownsPipelineLayout,
        skia_private::TArray<sk_sp<VulkanSampler>>&& immutableSamplers,
        RenderStep::RenderStepID renderStepID,
        PrimitiveType primitiveType,
        const DepthStencilSettings& depthStencilSettings,
        VertexInputBindingDescriptions&& vertexBindingDescriptions,
        VertexInputAttributeDescriptions&& vertexAttributeDescriptions,
        bool hasPaintParamAttributes)
    : GraphicsPipeline(sharedContext, pipelineInfo, pipelineLabel)
    , fPipelineLayout(pipelineLayout)
    , fPipeline(pipeline)
    , fShadersPipeline(shadersPipeline)
    , fOwnsPipelineLayout(ownsPipelineLayout)
    , fImmutableSamplers(std::move(immutableSamplers))
    , fPrimitiveType(primitiveType)
    , fDepthStencilSettings(depthStencilSettings)
    , fRenderStepID(renderStepID)
    , fHasPaintParamAttributes(hasPaintParamAttributes)
    , fVertexBindingDescriptions(std::move(vertexBindingDescriptions))
    , fVertexAttributeDescriptions(std::move(vertexAttributeDescriptions)) {
    // Update the newly-created underlying GPU object's label to match the Resource's
    this->synchronizeBackendLabel();
}

void VulkanGraphicsPipeline::freeGpuData() {
    auto sharedCtxt = static_cast<const VulkanSharedContext*>(this->sharedContext());
    if (fShadersPipeline != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedCtxt->interface(),
                    DestroyPipeline(sharedCtxt->device(), fShadersPipeline, nullptr));
    }
    if (fPipeline != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedCtxt->interface(),
            DestroyPipeline(sharedCtxt->device(), fPipeline, nullptr));
    }
    if (fOwnsPipelineLayout && fPipelineLayout != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedCtxt->interface(),
                    DestroyPipelineLayout(sharedCtxt->device(), fPipelineLayout, nullptr));
    }
}

void VulkanGraphicsPipeline::updateDynamicState(const VulkanSharedContext* sharedContext,
                                                VkCommandBuffer commandBuffer,
                                                const VulkanGraphicsPipeline* previous) const {
    // The front-end currently is unaware of dynamic state, so we have no choice but to calculate
    // the diff between pipelines and update the state as needed.  This is not quite so efficient,
    // but will need awareness from the front-end to optimize.
    if (sharedContext->caps()->useBasicDynamicState()) {
        // Primitive type
        const bool primitiveTypeDirty =
                previous == nullptr || previous->fPrimitiveType != fPrimitiveType;
        if (primitiveTypeDirty) {
            VULKAN_CALL(sharedContext->interface(),
                        CmdSetPrimitiveTopology(commandBuffer,
                                                primitive_type_to_vk_topology(fPrimitiveType)));
        }

        // Depth-stencil settings
        const auto& [depth, stencil] = fDepthStencilSettings;
        const auto& [pastDepth, pastStencil] =
                previous ? previous->fDepthStencilSettings
                         : std::make_pair(DepthSettings(), StencilSettings()); // ignored

        const bool depthEnabledDirty = !previous || pastDepth.enabled() != depth.enabled();
        if (depthEnabledDirty) {
            VULKAN_CALL(sharedContext->interface(),
                        CmdSetDepthTestEnable(commandBuffer, depth.enabled()));
        }
        const bool depthCompareOpDirty = !previous || pastDepth.fCompareOp != depth.fCompareOp;
        if (depthCompareOpDirty) {
            VULKAN_CALL(sharedContext->interface(),
                        CmdSetDepthCompareOp(commandBuffer,
                                             compare_op_to_vk_compare_op(depth.fCompareOp)));
        }
        const bool depthWriteDirty = !previous || pastDepth.fWriteEnabled != depth.fWriteEnabled;
        if (depthWriteDirty) {
            VULKAN_CALL(sharedContext->interface(),
                        CmdSetDepthWriteEnable(commandBuffer, depth.fWriteEnabled));
        }

        const bool stencilEnabledDirty = !previous || pastStencil.enabled() != stencil.enabled();
        if (stencilEnabledDirty) {
            VULKAN_CALL(sharedContext->interface(),
                        CmdSetStencilTestEnable(commandBuffer, stencil.enabled()));
        }

        // The rest of the stencil state only needs to be checked if we have non-default state or
        // if the test state was dirty (in which case it needs to be set back to the default).
        if (stencilEnabledDirty || stencil.enabled()) {
            // FRONT_AND_BACK
            const bool readMaskDirty = !previous || pastStencil.fReadMask != stencil.fReadMask;
            if (readMaskDirty) {
                VULKAN_CALL(sharedContext->interface(),
                            CmdSetStencilCompareMask(commandBuffer,
                                                     VK_STENCIL_FACE_FRONT_AND_BACK,
                                                     stencil.fReadMask));
            }
            const bool writeMaskDirty = !previous || pastStencil.fWriteMask != stencil.fWriteMask;
            if (writeMaskDirty) {
                VULKAN_CALL(sharedContext->interface(),
                            CmdSetStencilWriteMask(commandBuffer,
                                                   VK_STENCIL_FACE_FRONT_AND_BACK,
                                                   stencil.fWriteMask));
            }
            const bool referenceValueDirty =
                    !previous || pastStencil.fReferenceValue != stencil.fReferenceValue;
            if (referenceValueDirty) {
                VULKAN_CALL(sharedContext->interface(),
                            CmdSetStencilReference(commandBuffer,
                                                   VK_STENCIL_FACE_FRONT_AND_BACK,
                                                   stencil.fReferenceValue));
            }
\
            // FRONT-only
            const StencilSettings::Face& front = stencil.fFrontFace;
            const bool frontStencilOpsDirty = !previous || pastStencil.fFrontFace != front;
            if (frontStencilOpsDirty) {
                VULKAN_CALL(
                        sharedContext->interface(),
                        CmdSetStencilOp(commandBuffer,
                                        VK_STENCIL_FACE_FRONT_BIT,
                                        /*failOp=*/
                                        stencil_op_to_vk_stencil_op(front.fStencilFailOp),
                                        /*passOp=*/
                                        stencil_op_to_vk_stencil_op(front.fDepthStencilPassOp),
                                        /*depthFailOp=*/
                                        stencil_op_to_vk_stencil_op(front.fDepthFailOp),
                                        /*compareOp=*/
                                        compare_op_to_vk_compare_op(front.fCompareOp)));
            }

            // BACK-only
            const StencilSettings::Face& back = stencil.fBackFace;
            const bool backStencilOpsDirty = !previous || pastStencil.fBackFace != back;
            if (backStencilOpsDirty) {
                VULKAN_CALL(
                        sharedContext->interface(),
                        CmdSetStencilOp(commandBuffer,
                                        VK_STENCIL_FACE_BACK_BIT,
                                        /*failOp=*/
                                        stencil_op_to_vk_stencil_op(back.fStencilFailOp),
                                        /*passOp=*/
                                        stencil_op_to_vk_stencil_op(back.fDepthStencilPassOp),
                                        /*depthFailOp=*/
                                        stencil_op_to_vk_stencil_op(back.fDepthFailOp),
                                        /*compareOp=*/
                                        compare_op_to_vk_compare_op(back.fCompareOp)));
            }
        }
    }

    if (sharedContext->caps()->useVertexInputDynamicState()) {
        const bool vertexInputDirty =
                previous == nullptr ||
                previous->fRenderStepID != fRenderStepID ||
                fHasPaintParamAttributes || previous->fHasPaintParamAttributes;

        if (vertexInputDirty) {
            VULKAN_CALL(sharedContext->interface(),
                        CmdSetVertexInput(commandBuffer,
                                          fVertexBindingDescriptions.size(),
                                          fVertexBindingDescriptions.begin(),
                                          fVertexAttributeDescriptions.size(),
                                          fVertexAttributeDescriptions.begin()));
        }
    }
}

} // namespace skgpu::graphite
