/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanUtilsPriv_DEFINED
#define skgpu_VulkanUtilsPriv_DEFINED

#include "include/gpu/vk/VulkanTypes.h"

#include "include/core/SkColor.h"

namespace skgpu {

static constexpr uint32_t VkFormatChannels(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:           return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R8_UNORM:                 return kRed_SkColorChannelFlag;
        case VK_FORMAT_B8G8R8A8_UNORM:           return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:      return kRGB_SkColorChannelFlags;
        case VK_FORMAT_R16G16B16A16_SFLOAT:      return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16_SFLOAT:               return kRed_SkColorChannelFlag;
        case VK_FORMAT_R8G8B8_UNORM:             return kRGB_SkColorChannelFlags;
        case VK_FORMAT_R8G8_UNORM:               return kRG_SkColorChannelFlags;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:    return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:    return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R8G8B8A8_SRGB:            return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:  return kRGB_SkColorChannelFlags;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:      return kRGB_SkColorChannelFlags;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16_UNORM:                return kRed_SkColorChannelFlag;
        case VK_FORMAT_R16G16_UNORM:             return kRG_SkColorChannelFlags;
        case VK_FORMAT_R16G16B16A16_UNORM:       return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16G16_SFLOAT:            return kRG_SkColorChannelFlags;
        case VK_FORMAT_S8_UINT:                  return 0;
        case VK_FORMAT_D24_UNORM_S8_UINT:        return 0;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:       return 0;
        default:                                 return 0;
    }
}

static constexpr size_t VkFormatBytesPerBlock(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:            return 4;
        case VK_FORMAT_R8_UNORM:                  return 1;
        case VK_FORMAT_B8G8R8A8_UNORM:            return 4;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:       return 2;
        case VK_FORMAT_R16G16B16A16_SFLOAT:       return 8;
        case VK_FORMAT_R16_SFLOAT:                return 2;
        case VK_FORMAT_R8G8B8_UNORM:              return 3;
        case VK_FORMAT_R8G8_UNORM:                return 2;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:  return 4;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:  return 4;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:     return 2;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:     return 2;
        case VK_FORMAT_R8G8B8A8_SRGB:             return 4;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:   return 8;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:       return 8;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:      return 8;
        case VK_FORMAT_R16_UNORM:                 return 2;
        case VK_FORMAT_R16G16_UNORM:              return 4;
        case VK_FORMAT_R16G16B16A16_UNORM:        return 8;
        case VK_FORMAT_R16G16_SFLOAT:             return 4;
        // Currently we are just over estimating this value to be used in gpu size calculations even
        // though the actually size is probably less. We should instead treat planar formats similar
        // to compressed textures that go through their own special query for calculating size.
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: return 3;
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:  return 3;
        case VK_FORMAT_S8_UINT:                   return 1;
        case VK_FORMAT_D24_UNORM_S8_UINT:         return 4;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:        return 8;

        default:                                 return 0;
    }
}

static constexpr int VkFormatIsStencil(VkFormat format) {
    switch (format) {
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

static constexpr int VkFormatIsDepth(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

static constexpr int VkFormatStencilBits(VkFormat format) {
    switch (format) {
        case VK_FORMAT_S8_UINT:
            return 8;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return 8;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return 8;
        default:
            return 0;
    }
}

static constexpr bool VkFormatNeedsYcbcrSampler(VkFormat format)  {
    return format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM ||
           format == VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
}

static constexpr bool SampleCountToVkSampleCount(uint32_t samples,
                                                 VkSampleCountFlagBits* vkSamples) {
    SkASSERT(samples >= 1);
    switch (samples) {
        case 1:
            *vkSamples = VK_SAMPLE_COUNT_1_BIT;
            return true;
        case 2:
            *vkSamples = VK_SAMPLE_COUNT_2_BIT;
            return true;
        case 4:
            *vkSamples = VK_SAMPLE_COUNT_4_BIT;
            return true;
        case 8:
            *vkSamples = VK_SAMPLE_COUNT_8_BIT;
            return true;
        case 16:
            *vkSamples = VK_SAMPLE_COUNT_16_BIT;
            return true;
        default:
            return false;
    }
}

/**
 * Returns true if the format is compressed.
 */
static constexpr bool VkFormatIsCompressed(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            return true;
        default:
            return false;
    }
    SkUNREACHABLE;
}


static constexpr void SetupSamplerYcbcrConversionInfo(
        VkSamplerYcbcrConversionCreateInfo* outInfo,
        const VulkanYcbcrConversionInfo& conversionInfo) {
#ifdef SK_DEBUG
    const VkFormatFeatureFlags& featureFlags = conversionInfo.fFormatFeatures;
    if (conversionInfo.fXChromaOffset == VK_CHROMA_LOCATION_MIDPOINT ||
        conversionInfo.fYChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT);
    }
    if (conversionInfo.fXChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN ||
        conversionInfo.fYChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT);
    }
    if (conversionInfo.fChromaFilter == VK_FILTER_LINEAR) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT);
    }
    if (conversionInfo.fForceExplicitReconstruction) {
        SkASSERT(featureFlags &
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT);
    }
#endif

    outInfo->sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    outInfo->pNext = nullptr;
    outInfo->format = conversionInfo.fFormat;
    outInfo->ycbcrModel = conversionInfo.fYcbcrModel;
    outInfo->ycbcrRange = conversionInfo.fYcbcrRange;

    // Components is ignored for external format conversions. For all other formats identity swizzle
    // is used. It can be added to VulkanYcbcrConversionInfo if necessary.
    outInfo->components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                           VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    outInfo->xChromaOffset = conversionInfo.fXChromaOffset;
    outInfo->yChromaOffset = conversionInfo.fYChromaOffset;
    outInfo->chromaFilter = conversionInfo.fChromaFilter;
    outInfo->forceExplicitReconstruction = conversionInfo.fForceExplicitReconstruction;

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    if (conversionInfo.fExternalFormat) {
        // Format must not be specified for external images.
        SkASSERT(conversionInfo.fFormat == VK_FORMAT_UNDEFINED);
        externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.pNext = nullptr;
        externalFormat.externalFormat = conversionInfo.fExternalFormat;
        outInfo->pNext = &externalFormat;
    }
#else
    // External images are supported only on Android.
    SkASSERT(!conversionInfo.fExternalFormat);
#endif

    if (!conversionInfo.fExternalFormat) {
        SkASSERT(conversionInfo.fFormat != VK_FORMAT_UNDEFINED);
    }
}

#if defined(SK_DEBUG) || defined(GR_TEST_UTILS)
static constexpr const char* VkFormatToStr(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:           return "R8G8B8A8_UNORM";
        case VK_FORMAT_R8_UNORM:                 return "R8_UNORM";
        case VK_FORMAT_B8G8R8A8_UNORM:           return "B8G8R8A8_UNORM";
        case VK_FORMAT_R5G6B5_UNORM_PACK16:      return "R5G6B5_UNORM_PACK16";
        case VK_FORMAT_R16G16B16A16_SFLOAT:      return "R16G16B16A16_SFLOAT";
        case VK_FORMAT_R16_SFLOAT:               return "R16_SFLOAT";
        case VK_FORMAT_R8G8B8_UNORM:             return "R8G8B8_UNORM";
        case VK_FORMAT_R8G8_UNORM:               return "R8G8_UNORM";
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "A2B10G10R10_UNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return "A2R10G10B10_UNORM_PACK32";
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:    return "B4G4R4A4_UNORM_PACK16";
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:    return "R4G4B4A4_UNORM_PACK16";
        case VK_FORMAT_R32G32B32A32_SFLOAT:      return "R32G32B32A32_SFLOAT";
        case VK_FORMAT_R8G8B8A8_SRGB:            return "R8G8B8A8_SRGB";
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:  return "ETC2_R8G8B8_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:      return "BC1_RGB_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return "BC1_RGBA_UNORM_BLOCK";
        case VK_FORMAT_R16_UNORM:                return "R16_UNORM";
        case VK_FORMAT_R16G16_UNORM:             return "R16G16_UNORM";
        case VK_FORMAT_R16G16B16A16_UNORM:       return "R16G16B16A16_UNORM";
        case VK_FORMAT_R16G16_SFLOAT:            return "R16G16_SFLOAT";
        case VK_FORMAT_S8_UINT:                  return "S8_UINT";
        case VK_FORMAT_D24_UNORM_S8_UINT:        return "D24_UNORM_S8_UINT";
        case VK_FORMAT_D32_SFLOAT_S8_UINT:       return "D32_SFLOAT_S8_UINT";

        default:                                 return "Unknown";
    }
}
#endif // defined(SK_DEBUG) || defined(GR_TEST_UTILS)

}  // namespace skgpu

#endif // skgpu_VulkanUtilsPriv_DEFINED
