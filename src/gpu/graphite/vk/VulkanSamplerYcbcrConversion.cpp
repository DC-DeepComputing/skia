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

sk_sp<VulkanSamplerYcbcrConversion> VulkanSamplerYcbcrConversion::Make(
        const VulkanSharedContext* context,
        const VulkanYcbcrConversionInfo& conversionInfo) {
    if (!context->vulkanCaps().supportsYcbcrConversion()) {
        return nullptr;
    }

    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;
    skgpu::SetupSamplerYcbcrConversionInfo(&ycbcrCreateInfo, conversionInfo);

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    if (conversionInfo.fExternalFormat) {
        // Format must not be specified for external images.
        SkASSERT(conversionInfo.fFormat == VK_FORMAT_UNDEFINED);
        externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.pNext = nullptr;
        externalFormat.externalFormat = conversionInfo.fExternalFormat;
        SkASSERT(ycbcrCreateInfo.pNext == nullptr);
        ycbcrCreateInfo.pNext = &externalFormat;
    }
#else
    // External images are supported only on Android.
    SkASSERT(!conversionInfo.fExternalFormat);
#endif

    if (!conversionInfo.fExternalFormat) {
        SkASSERT(conversionInfo.fFormat != VK_FORMAT_UNDEFINED);
    }

    VkSamplerYcbcrConversion conversion;
    VkResult result;
    VULKAN_CALL_RESULT(context,
                       result,
                       CreateSamplerYcbcrConversion(
                               context->device(), &ycbcrCreateInfo, nullptr, &conversion));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanSamplerYcbcrConversion>(
            new VulkanSamplerYcbcrConversion(context, conversion));
}

sk_sp<VulkanSamplerYcbcrConversion> VulkanSamplerYcbcrConversion::Make(
        const VulkanSharedContext* context,
        uint32_t nonFormatInfo,
        uint64_t format) {
    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;
    bool useExternalFormat =  static_cast<bool>(
            (nonFormatInfo & VulkanYcbcrConversionInfo::kUseExternalFormatMask) >>
                    VulkanYcbcrConversionInfo::kUsesExternalFormatShift);

    ycbcrCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    ycbcrCreateInfo.pNext = nullptr;
    ycbcrCreateInfo.format = useExternalFormat ? VK_FORMAT_UNDEFINED : static_cast<VkFormat>(format);

    ycbcrCreateInfo.ycbcrModel = static_cast<VkSamplerYcbcrModelConversion>(
                    (nonFormatInfo & VulkanYcbcrConversionInfo::kYcbcrModelMask) >>
                            VulkanYcbcrConversionInfo::kYcbcrModelShift);
    ycbcrCreateInfo.ycbcrRange = static_cast<VkSamplerYcbcrRange>(
                    (nonFormatInfo & VulkanYcbcrConversionInfo::kYcbcrRangeMask) >>
                            VulkanYcbcrConversionInfo::kYcbcrRangeShift);
    ycbcrCreateInfo.components = {
                static_cast<VkComponentSwizzle>(
                        (nonFormatInfo & VulkanYcbcrConversionInfo::kComponentRMask) >>
                                VulkanYcbcrConversionInfo::kComponentRShift),
                static_cast<VkComponentSwizzle>(
                        (nonFormatInfo & VulkanYcbcrConversionInfo::kComponentGMask) >>
                                VulkanYcbcrConversionInfo::kComponentGShift),
                static_cast<VkComponentSwizzle>(
                        (nonFormatInfo & VulkanYcbcrConversionInfo::kComponentBMask) >>
                                VulkanYcbcrConversionInfo::kComponentBShift),
                static_cast<VkComponentSwizzle>(
                        (nonFormatInfo & VulkanYcbcrConversionInfo::kComponentAMask) >>
                                VulkanYcbcrConversionInfo::kComponentAShift)};
    ycbcrCreateInfo.xChromaOffset = static_cast<VkChromaLocation>(
                    (nonFormatInfo & VulkanYcbcrConversionInfo::kXChromaOffsetMask) >>
                            VulkanYcbcrConversionInfo::kXChromaOffsetShift);
    ycbcrCreateInfo.yChromaOffset = static_cast<VkChromaLocation>(
                    (nonFormatInfo & VulkanYcbcrConversionInfo::kYChromaOffsetMask) >>
                            VulkanYcbcrConversionInfo::kYChromaOffsetShift);
    ycbcrCreateInfo.chromaFilter = static_cast<VkFilter>(
                    (nonFormatInfo & VulkanYcbcrConversionInfo::kChromaFilterMask) >>
                            VulkanYcbcrConversionInfo::kChromaFilterShift);
    ycbcrCreateInfo.forceExplicitReconstruction = static_cast<VkBool32>(
                    (nonFormatInfo & VulkanYcbcrConversionInfo::kForceExplicitReconMask) >>
                            VulkanYcbcrConversionInfo::kForceExplicitReconShift);

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    if (useExternalFormat) {
        externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.pNext = nullptr;
        externalFormat.externalFormat = format;
        SkASSERT(ycbcrCreateInfo.pNext == nullptr);
        ycbcrCreateInfo.pNext = &externalFormat;
    }
#endif

    VkSamplerYcbcrConversion conversion;
    VkResult result;
    VULKAN_CALL_RESULT(context,
                       result,
                       CreateSamplerYcbcrConversion(
                               context->device(), &ycbcrCreateInfo, nullptr, &conversion));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanSamplerYcbcrConversion>(
            new VulkanSamplerYcbcrConversion(context, conversion));
}

namespace {
// Define this anonymous helper to get a static resource type for YCbCr conversions regardless of
// which method is used to create it (from VulkanYcbcrConversionInfo or from SamplerDesc)
ResourceType conversion_rsrc_type() {
    static const ResourceType conversionType = GraphiteResourceKey::GenerateResourceType();
    return conversionType;
}
}
GraphiteResourceKey VulkanSamplerYcbcrConversion::MakeYcbcrConversionKey(
        const VulkanSharedContext* context, const VulkanYcbcrConversionInfo& info) {
    bool useExternalFormat = info.fFormat == VK_FORMAT_UNDEFINED;
    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, conversion_rsrc_type(), info.numInt32sNeeded(),
                                         Shareable::kYes);

    int i = 0;
    builder[i++] = info.nonFormatInfoAsUInt32();
    if (useExternalFormat) {
        builder[i++] = (uint32_t)info.fExternalFormat;
        builder[i++] = (uint32_t)(info.fExternalFormat >> 32);
    } else {
        builder[i++] = (uint32_t)info.fFormat;
    }
    SkASSERT(i == info.numInt32sNeeded());

    builder.finish();
    return key;
}

GraphiteResourceKey VulkanSamplerYcbcrConversion::GetKeyFromSamplerDesc(
        const SamplerDesc& samplerDesc) {
    GraphiteResourceKey key;

    uint32_t nonFormatYcbcrInfo =
            (uint32_t)(samplerDesc.desc() >> SamplerDesc::kImmutableSamplerInfoShift);
    SkASSERT(nonFormatYcbcrInfo != 0);

    bool usesExternalFormat =  static_cast<bool>(
                ((nonFormatYcbcrInfo & VulkanYcbcrConversionInfo::kUseExternalFormatMask) >>
                        VulkanYcbcrConversionInfo::kUsesExternalFormatShift));
    int num32DataCnt =
            usesExternalFormat ? VulkanYcbcrConversionInfo::kInt32sNeededExternalFormat
                               : VulkanYcbcrConversionInfo::kInt32sNeededKnownFormat;
    GraphiteResourceKey::Builder builder(&key, conversion_rsrc_type(), num32DataCnt,
                                         Shareable::kYes);

    int i = 0;
    builder[i++] = nonFormatYcbcrInfo;
    builder[i++] = (uint32_t)samplerDesc.format();
    if (usesExternalFormat) {
        builder[i++] = (uint32_t)(samplerDesc.format() >> 32);
    }
    SkASSERT(i == num32DataCnt);

    builder.finish();
    return key;
}

VulkanSamplerYcbcrConversion::VulkanSamplerYcbcrConversion(const VulkanSharedContext* context,
                                                           VkSamplerYcbcrConversion ycbcrConversion)
        : Resource(context,
                   Ownership::kOwned,
                   skgpu::Budgeted::kYes, // Shareable, so must be budgeted
                   /*gpuMemorySize=*/0,
                   /*label=*/"VulkanSamplerYcbcrConversion")
        , fYcbcrConversion (ycbcrConversion) {}

void VulkanSamplerYcbcrConversion::freeGpuData() {
    auto sharedContext = static_cast<const VulkanSharedContext*>(this->sharedContext());
    SkASSERT(fYcbcrConversion != VK_NULL_HANDLE);
    VULKAN_CALL(sharedContext->interface(),
                DestroySamplerYcbcrConversion(sharedContext->device(), fYcbcrConversion, nullptr));
}

} // namespace skgpu::graphite

