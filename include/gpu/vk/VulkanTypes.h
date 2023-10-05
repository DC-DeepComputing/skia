/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanTypes_DEFINED
#define skgpu_VulkanTypes_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

#include <functional>

#ifndef VK_VERSION_1_1
#error Skia requires the use of Vulkan 1.1 headers
#endif

namespace skgpu {

using VulkanGetProc = std::function<PFN_vkVoidFunction(
        const char*, // function name
        VkInstance,  // instance or VK_NULL_HANDLE
        VkDevice     // device or VK_NULL_HANDLE
        )>;

typedef intptr_t VulkanBackendMemory;

/**
 * Types for interacting with Vulkan resources created externally to Skia.
 */
struct VulkanAlloc {
    // can be VK_NULL_HANDLE iff is an RT and is borrowed
    VkDeviceMemory      fMemory = VK_NULL_HANDLE;
    VkDeviceSize        fOffset = 0;
    VkDeviceSize        fSize = 0;  // this can be indeterminate iff Tex uses borrow semantics
    uint32_t            fFlags = 0;
    // handle to memory allocated via skgpu::VulkanMemoryAllocator.
    VulkanBackendMemory fBackendMemory = 0;

    enum Flag {
        kNoncoherent_Flag     = 0x1,   // memory must be flushed to device after mapping
        kMappable_Flag        = 0x2,   // memory is able to be mapped.
        kLazilyAllocated_Flag = 0x4,   // memory was created with lazy allocation
    };

    bool operator==(const VulkanAlloc& that) const {
        return fMemory == that.fMemory && fOffset == that.fOffset && fSize == that.fSize &&
               fFlags == that.fFlags && fUsesSystemHeap == that.fUsesSystemHeap;
    }

private:
    bool fUsesSystemHeap = false;
};

// Used to pass in the necessary information to create a VkSamplerYcbcrConversion
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
    VkSamplerYcbcrRange fYcbcrRange           = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    VkChromaLocation fXChromaOffset           = VK_CHROMA_LOCATION_COSITED_EVEN;
    VkChromaLocation fYChromaOffset           = VK_CHROMA_LOCATION_COSITED_EVEN;
    VkFilter fChromaFilter                    = VK_FILTER_NEAREST;
    VkBool32 fForceExplicitReconstruction     = false;

    // For external images format features here should be those returned by a call to
    // vkAndroidHardwareBufferFormatPropertiesANDROID
    VkFormatFeatureFlags fFormatFeatures = 0;
};

static inline void setup_sampler_ycbcr_conversion_info(
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

} // namespace skgpu

#endif // skgpu_VulkanTypes_DEFINED
