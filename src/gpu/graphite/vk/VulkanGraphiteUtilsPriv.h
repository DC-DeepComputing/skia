/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanGraphiteUtilsPriv_DEFINED
#define skgpu_graphite_VulkanGraphiteUtilsPriv_DEFINED

#include "include/core/SkSpan.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/DescriptorTypes.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/vk/VulkanInterface.h"

#include <string>

// Helper macros to call functions on the VulkanInterface
#define VULKAN_CALL(IFACE, X) (IFACE)->fFunctions.f##X

// TODO: This needs to add checks for device lost on calls. See Ganesh version
#define VULKAN_LOG_IF_NOT_SUCCESS(RESULT, X, ...)                                       \
    do {                                                                                \
        if (RESULT != VK_SUCCESS) {                                                     \
            SkDebugf("Failed vulkan call. Error: %d, " X "\n", RESULT, ##__VA_ARGS__);  \
        }                                                                               \
    } while (false)

// TODO: This needs to add checks for device lost on calls. See Ganesh version
#define VULKAN_CALL_RESULT(IFACE, RESULT, X)                               \
    do {                                                                   \
        (RESULT) = VULKAN_CALL(IFACE, X);                                  \
        SkASSERT(VK_SUCCESS == RESULT || VK_ERROR_DEVICE_LOST == RESULT);  \
        if (RESULT != VK_SUCCESS) {                                        \
            SKGPU_LOG_E("Failed vulkan call. Error: %d," #X "\n", RESULT); \
        }                                                                  \
    } while (false)

// same as VULKAN_CALL but checks for success
#define VULKAN_CALL_ERRCHECK(IFACE, X)                   \
    VkResult SK_MACRO_APPEND_LINE(ret);                  \
    VULKAN_CALL_RESULT(IFACE, SK_MACRO_APPEND_LINE(ret), X)

#define VULKAN_CALL_RESULT_NOCHECK(IFACE, RESULT, X) \
    do {                                             \
        (RESULT) = VULKAN_CALL(IFACE, X);            \
    } while (false)
namespace skgpu::graphite {

class VulkanSharedContext;

VkShaderModule createVulkanShaderModule(const VulkanSharedContext*,
                                        const std::string& spirv,
                                        VkShaderStageFlagBits);

VkDescriptorType DsTypeEnumToVkDs(DescriptorType);
void DescriptorDataToVkDescSetLayout(const VulkanSharedContext*,
                                     const SkSpan<DescriptorData>&,
                                     VkDescriptorSetLayout*);

VkShaderStageFlags PipelineStageFlagsToVkShaderStageFlags(SkEnumBitMask<PipelineStageFlags>);

bool vkFormatIsSupported(VkFormat);

inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
inline static constexpr unsigned int kPaintUniformBufferIndex = 2;
inline static constexpr unsigned int kNumUniformBuffers = 3;

// For now, rigidly assign all uniform buffer descriptors to be in set 0 and all texture/samplers
// to be in set 1.
// TODO(b/274762935): Make the bindings and descriptor set organization more flexible.
inline static constexpr unsigned int kUniformBufferDescSetIndex = 0;
inline static constexpr unsigned int kTextureBindDescSetIndex = 1;
// Currently input attachments are only used for loading MSAA from resolve, so we can use the
// descriptor set index normally assigned to texture desc sets.
inline static constexpr unsigned int kInputAttachmentDescSetIndex = kTextureBindDescSetIndex;

inline static constexpr unsigned int kVertexBufferIndex = 0;
inline static constexpr unsigned int kInstanceBufferIndex = 1;
inline static constexpr unsigned int kNumInputBuffers = 2;

inline static const DescriptorData kIntrinsicUniformBufferDescriptor = {
        DescriptorType::kUniformBuffer, /*descCount=*/1,
        kIntrinsicUniformBufferIndex,
        PipelineStageFlags::kVertexShader | PipelineStageFlags::kFragmentShader};

inline static const DescriptorData kRenderStepUniformDescriptor = {
        DescriptorType::kUniformBuffer, /*descCount=*/1,
        kRenderStepUniformBufferIndex,
        PipelineStageFlags::kVertexShader | PipelineStageFlags::kFragmentShader};

inline static const DescriptorData kPaintUniformDescriptor = {
        DescriptorType::kUniformBuffer, /*descCount=*/1,
        kPaintUniformBufferIndex,
        PipelineStageFlags::kFragmentShader};

// Currently we only have one input attachment descriptor by itself within a set, so its binding
// index will always be 0.
inline static constexpr unsigned int kInputAttachmentBindingIndex = 0;
inline static const DescriptorData kInputAttachmentDescriptor = {
        DescriptorType::kInputAttachment, /*descCount=*/1,
        kInputAttachmentBindingIndex,
        PipelineStageFlags::kFragmentShader};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanGraphiteUtilsPriv_DEFINED
