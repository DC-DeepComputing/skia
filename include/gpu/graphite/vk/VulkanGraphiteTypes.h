/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanGraphiteTypes_DEFINED
#define skgpu_graphite_VulkanGraphiteTypes_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/vk/VulkanTypes.h"

namespace skgpu::graphite {

// This struct is the exact same as GrVkYcbcrConversionInfo. TODO: Move this to shared location to
// avoid duplication. Used to pass in the necessary information to create a VkSamplerYcbcrConversion
// object for an VkExternalFormatANDROID.
struct VulkanYcbcrConversionInfo {
    bool operator==(const VulkanYcbcrConversionInfo& that) const {
        // Invalid objects are not required to have all other fields initialized or matching.
        if (!this->isValid() && !that.isValid()) {
            return true;
        }
        return this->fFormat == that.fFormat &&
               this->fExternalFormat == that.fExternalFormat &&
               this->fYcbcrModel == that.fYcbcrModel &&
               this->fYcbcrRange == that.fYcbcrRange &&
               this->fXChromaOffset == that.fXChromaOffset &&
               this->fYChromaOffset == that.fYChromaOffset &&
               this->fChromaFilter == that.fChromaFilter &&
               this->fForceExplicitReconstruction == that.fForceExplicitReconstruction;
    }
    bool operator!=(const VulkanYcbcrConversionInfo& that) const { return !(*this == that); }

    bool isValid() const {
        return fYcbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY ||
               fExternalFormat != 0;
    }

    // Format of the source image. Must be set to VK_FORMAT_UNDEFINED for external images or
    // a valid image format otherwise.
    VkFormat fFormat = VK_FORMAT_UNDEFINED;

    // The external format. Must be non-zero for external images, zero otherwise.
    // Should be compatible to be used in a VkExternalFormatANDROID struct.
    uint64_t fExternalFormat = 0;

    VkSamplerYcbcrModelConversion fYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    VkSamplerYcbcrRange fYcbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    VkChromaLocation fXChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    VkChromaLocation fYChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    VkFilter fChromaFilter = VK_FILTER_NEAREST;
    VkBool32 fForceExplicitReconstruction = false;

    // For external images format features here should be those returned by a call to
    // vkAndroidHardwareBufferFormatPropertiesANDROID
    VkFormatFeatureFlags fFormatFeatures = 0;
};

struct VulkanTextureInfo {
    uint32_t fSampleCount = 1;
    Mipmapped fMipmapped = Mipmapped::kNo;

    // VkImageCreateInfo properties
    // Currently the only supported flag is VK_IMAGE_CREATE_PROTECTED_BIT. Any other flag will not
    // be accepted
    VkImageCreateFlags       fFlags = 0;
    VkFormat                 fFormat = VK_FORMAT_UNDEFINED;
    VkImageTiling            fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags        fImageUsageFlags = 0;
    VkSharingMode            fSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Properties related to the image view and sampling. These are less inherent properties of the
    // VkImage but describe how the VkImage should be used within Skia.

    // What aspect to use for the VkImageView. The normal, default is VK_IMAGE_ASPECT_COLOR_BIT.
    // However, if the VkImage is a Ycbcr format, the client can pass a specific plan here to have
    // Skia directly sample a plane. In that case the client should also pass in a VkFormat that is
    // compatible with the plane as described by the Vulkan spec.
    VkImageAspectFlags         fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VulkanYcbcrConversionInfo  fYcbcrConversionInfo;

    VulkanTextureInfo() = default;
    VulkanTextureInfo(uint32_t sampleCount,
                      Mipmapped mipmapped,
                      VkImageCreateFlags flags,
                      VkFormat format,
                      VkImageTiling imageTiling,
                      VkImageUsageFlags imageUsageFlags,
                      VkSharingMode sharingMode,
                      VkImageAspectFlags aspectMask,
                      VulkanYcbcrConversionInfo ycbcrConversionInfo)
            : fSampleCount(sampleCount)
            , fMipmapped(mipmapped)
            , fFlags(flags)
            , fFormat(format)
            , fImageTiling(imageTiling)
            , fImageUsageFlags(imageUsageFlags)
            , fSharingMode(sharingMode)
            , fAspectMask(aspectMask)
            , fYcbcrConversionInfo(ycbcrConversionInfo) {}
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanGraphiteTypes_DEFINED


