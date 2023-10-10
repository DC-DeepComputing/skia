/*
 * Copyright 2023 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanAHardwareBufferUtils_DEFINED
#define skgpu_VulkanAHardwareBufferUtils_DEFINED

#ifdef SK_BUILD_FOR_ANDROID // TODO: Reorder this?

#include <android/hardware_buffer.h>
#include "include/gpu/vk/VulkanTypes.h" // Assert building for Vulkan?

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
} // namespace skgpu

#endif // SK_BUILD_FOR_ANDROID
#endif // skgpu_VulkanAHardwareBufferUtils_DEFINED
