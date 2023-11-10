/*
 * Copyright 2023 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanAHardwareBufferUtils_DEFINED
#define skgpu_VulkanAHardwareBufferUtils_DEFINED

#ifdef SK_BUILD_FOR_ANDROID

#include <android/hardware_buffer.h>
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/vk/VulkanInterface.h"

/**
 * Define a macro that both ganesh and graphite can use to make simple calls into Vulkan so we can
 * share more code between them.
*/
#define SHARED_GR_VULKAN_CALL(IFACE, X) (IFACE)->fFunctions.f##X

/**
 * Shared Vulkan AHardwareBuffer utility functions between graphite and ganesh
*/
namespace skgpu {

static VkFormat get_vk_format_from_ahardwarebuffer_format(uint32_t bufferFormat) {
    switch (bufferFormat) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM: {
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT: {
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        }
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM: {
            return VK_FORMAT_R5G6B5_UNORM_PACK16;
        }
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM: {
            return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        }
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM: {
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM: {
            return VK_FORMAT_R8G8B8_UNORM;
        }
#if __ANDROID_API__ >= 33
        case AHARDWAREBUFFER_FORMAT_R8_UNORM: {
            return VK_FORMAT_R8_UNORM;
        }
#endif
        default: {
            return VK_FORMAT_UNDEFINED;
        }
    }
}


static void get_ycbcr_conversion_info_from_format_props(
        const VkAndroidHardwareBufferFormatPropertiesANDROID& formatProps,
        VulkanYcbcrConversionInfo* outConversionInfo) {
    outConversionInfo->fYcbcrModel = formatProps.suggestedYcbcrModel;
    outConversionInfo->fYcbcrRange = formatProps.suggestedYcbcrRange;
    outConversionInfo->fXChromaOffset = formatProps.suggestedXChromaOffset;
    outConversionInfo->fYChromaOffset = formatProps.suggestedYChromaOffset;
    outConversionInfo->fForceExplicitReconstruction = VK_FALSE;
    outConversionInfo->fExternalFormat = formatProps.externalFormat;
    outConversionInfo->fFormatFeatures = formatProps.formatFeatures;
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT &
        formatProps.formatFeatures) {
        outConversionInfo->fChromaFilter = VK_FILTER_LINEAR;
    } else {
        outConversionInfo->fChromaFilter = VK_FILTER_NEAREST;
    }
}

static bool get_android_hardware_buffer_properties(
        VkAndroidHardwareBufferFormatPropertiesANDROID* outHwbFormatProps,
        VkAndroidHardwareBufferPropertiesANDROID* outHwbProps,
        const skgpu::VulkanInterface* interface,
        const AHardwareBuffer* hwBuffer,
        VkDevice device) {
    outHwbFormatProps->sType =
            VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    outHwbFormatProps->pNext = nullptr;

    outHwbProps->sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    outHwbProps->pNext = outHwbFormatProps;

    VkResult result =
            SHARED_GR_VULKAN_CALL(interface,
                                  GetAndroidHardwareBufferProperties(device,
                                                                     hwBuffer,
                                                                     outHwbProps));
    if (result != VK_SUCCESS) {
        SkDebugf("Failed to get AndroidHardwareBufferProperties\n");
        return false;
    }
    return true;
}

static bool allocate_and_bind_image_memory(skgpu::VulkanAlloc* outVulkanAlloc,
                                           VkImage image,
                                           const VkPhysicalDeviceMemoryProperties2& phyDevMemProps,
                                           const VkAndroidHardwareBufferPropertiesANDROID& hwbProps,
                                           AHardwareBuffer* hardwareBuffer,
                                           const skgpu::VulkanInterface* interface,
                                           VkDevice device) { // TODO reorder args
    VkResult result;

    uint32_t typeIndex = 0;
    bool foundHeap = false;
    uint32_t memTypeCnt = phyDevMemProps.memoryProperties.memoryTypeCount;
    for (uint32_t i = 0; i < memTypeCnt && !foundHeap; ++i) {
        if (hwbProps.memoryTypeBits & (1 << i)) {
            const VkPhysicalDeviceMemoryProperties& pdmp = phyDevMemProps.memoryProperties;
            uint32_t supportedFlags = pdmp.memoryTypes[i].propertyFlags &
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            if (supportedFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                typeIndex = i;
                foundHeap = true;
            }
        }
    }

    // Fallback to use any available memory type for AHB
    //
    // For external memory import, compatible memory types are decided by the Vulkan driver since
    // the memory has been allocated externally. There are usually special requirements against
    // external memory. e.g. AHB allocated with CPU R/W often usage bits is only importable for
    // non-device-local heap on some AMD systems.
    if (!foundHeap && hwbProps.memoryTypeBits) {
        typeIndex = ffs(hwbProps.memoryTypeBits) - 1;
        foundHeap = true;
    }

    if (!foundHeap) {
        SHARED_GR_VULKAN_CALL(interface, DestroyImage(device, image, nullptr));
        return false;
    }

    VkImportAndroidHardwareBufferInfoANDROID hwbImportInfo;
    hwbImportInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    hwbImportInfo.pNext = nullptr;
    hwbImportInfo.buffer = hardwareBuffer;

    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo;
    dedicatedAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.pNext = &hwbImportInfo;
    dedicatedAllocInfo.image = image;
    dedicatedAllocInfo.buffer = VK_NULL_HANDLE;

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
        &dedicatedAllocInfo,                         // pNext
        hwbProps.allocationSize,                     // allocationSize
        typeIndex,                                   // memoryTypeIndex
    };

    VkDeviceMemory memory;
    result = SHARED_GR_VULKAN_CALL(interface,
                                   AllocateMemory(device, &allocInfo, nullptr, &memory));
    if (result != VK_SUCCESS) {
        SkDebugf("Failed to allocate memory\n");
        SHARED_GR_VULKAN_CALL(interface, DestroyImage(device, image, nullptr));
        return false;
    }

    VkBindImageMemoryInfo bindImageInfo;
    bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindImageInfo.pNext = nullptr;
    bindImageInfo.image = image;
    bindImageInfo.memory = memory;
    bindImageInfo.memoryOffset = 0;

    result = SHARED_GR_VULKAN_CALL(interface, BindImageMemory2(device, 1, &bindImageInfo));
    if (result != VK_SUCCESS) {
        SkDebugf("Failed to bind image memory\n");
        SHARED_GR_VULKAN_CALL(interface, DestroyImage(device, image, nullptr));
        SHARED_GR_VULKAN_CALL(interface, FreeMemory(device, memory, nullptr));
        return false;
    }

    outVulkanAlloc->fMemory = memory;
    outVulkanAlloc->fOffset = 0;
    outVulkanAlloc->fSize = hwbProps.allocationSize;
    outVulkanAlloc->fFlags = 0;
    return true;
}

} // namespace skgpu

#endif // SK_BUILD_FOR_ANDROID
#endif // skgpu_VulkanAHardwareBufferUtils_DEFINED
