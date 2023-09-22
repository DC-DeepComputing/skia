/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanSamplerYcbcrConversion.h"

#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

VulkanSamplerYcbcrConversion* VulkanSamplerYcbcrConversion::Make(
        const VulkanSharedContext* context,
        const VulkanYcbcrConversionInfo& conversionInfo) {
    if (!context->vulkanCaps().supportsYcbcrConversion()) {
        return nullptr;
    }
#ifdef SK_DEBUG
// TODO: This part is the exact same.
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

    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;
    ycbcrCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    ycbcrCreateInfo.pNext = nullptr;
    ycbcrCreateInfo.format = conversionInfo.fFormat;
    ycbcrCreateInfo.ycbcrModel = conversionInfo.fYcbcrModel;
    ycbcrCreateInfo.ycbcrRange = conversionInfo.fYcbcrRange;

    // Components is ignored for external format conversions. For all other formats identity swizzle
    // is used. It can be added to GrVkYcbcrConversionInfo if necessary.
    ycbcrCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                  VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcrCreateInfo.xChromaOffset = conversionInfo.fXChromaOffset;
    ycbcrCreateInfo.yChromaOffset = conversionInfo.fYChromaOffset;
    ycbcrCreateInfo.chromaFilter = conversionInfo.fChromaFilter;
    ycbcrCreateInfo.forceExplicitReconstruction = conversionInfo.fForceExplicitReconstruction;

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    if (conversionInfo.fExternalFormat) {
        // Format must not be specified for external images.
        SkASSERT(conversionInfo.fFormat == VK_FORMAT_UNDEFINED);
        externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.pNext = nullptr;
        externalFormat.externalFormat = conversionInfo.fExternalFormat;
        ycbcrCreateInfo.pNext = &externalFormat;
    }
#else
    // External images are supported only on Android;
    SkASSERT(!conversionInfo.fExternalFormat);
#endif

    if (!conversionInfo.fExternalFormat) {
        SkASSERT(conversionInfo.fFormat != VK_FORMAT_UNDEFINED);
    }

    VkSamplerYcbcrConversion conversion;
    VkResult result;
    VULKAN_CALL_RESULT(context->interface(), result,
                       CreateSamplerYcbcrConversion(context->device(),
                                                    &ycbcrCreateInfo,
                                                    nullptr,
                                                    &conversion));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return new VulkanSamplerYcbcrConversion(context, conversion);
}

VulkanSamplerYcbcrConversion::VulkanSamplerYcbcrConversion(
        const VulkanSharedContext* context, VkSamplerYcbcrConversion ycbcrConversion)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kNo, /*gpuMemorySize=*/0)
        , fYcbcrConversion (ycbcrConversion) {}

void VulkanSamplerYcbcrConversion::freeGpuData() {
    auto sharedContext = static_cast<const VulkanSharedContext*>(this->sharedContext());
    SkASSERT(fYcbcrConversion != VK_NULL_HANDLE);
    VULKAN_CALL(sharedContext->interface(),
                DestroySamplerYcbcrConversion(sharedContext->device(), fYcbcrConversion, nullptr));
}

} // namespace skgpu::graphite

