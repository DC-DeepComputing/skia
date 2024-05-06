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

namespace {
    ResourceType conversion_rsrc_type() {
        static const ResourceType conversionType = GraphiteResourceKey::GenerateResourceType();
        return conversionType;
    }
}

GraphiteResourceKey VulkanSamplerYcbcrConversion::MakeYcbcrConversionKey(
        const VulkanSharedContext* context, const VulkanYcbcrConversionInfo& info) {
    bool useExternalFormat = info.fFormat == VK_FORMAT_UNDEFINED;
    // 2 uint32s needed for the external format OR 1 for a known VkFormat. 1 uint32 can store all
    // other differentiating ycbcr information.
    const int num32DataCnt = useExternalFormat ? 4 : 3;
    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, conversion_rsrc_type(), num32DataCnt, Shareable::kYes);

    int i = 0;
    builder[i++] = info.nonFormatInfoAsUInt32();
    if (useExternalFormat) {
        builder[i++] = (uint32_t)info.fExternalFormat;
        builder[i++] = (uint32_t)(info.fExternalFormat >> 32);
    } else {
        builder[i++] = info.fFormat;
    }
    builder[i++] = info.fFormatFeatures; // TODO: needed?

    SkASSERT(i == num32DataCnt);

    builder.finish();
    return key;
}

// TODO: review this method.
GraphiteResourceKey VulkanSamplerYcbcrConversion::GetKeyFromSamplerDesc(
        const SamplerDesc& samplerDesc) {
    SkASSERT(samplerDesc.format());

    GraphiteResourceKey key;
    int int32sNeeded = 1; // 1 needed for non-format information
    uint32_t nonFormatYcbcrInfo = (uint32_t)(
            samplerDesc.desc() >> SamplerDesc::kConversionInfoShift);
    bool usesYcbcrConversion = nonFormatYcbcrInfo != 0;

    bool usesExternalFormat = false;
    if (usesYcbcrConversion) {
        usesExternalFormat =  static_cast<bool>(
                ((nonFormatYcbcrInfo & VulkanYcbcrConversionInfo::kUseExternalFormatMask) >>
                        VulkanYcbcrConversionInfo::kUsesExternalFormatShift));
        // If using an external format, we need to append more data thn if we are using a known one.
        // TODO: Make static constant vals?
        int32sNeeded += usesExternalFormat ? 3 : 2;
    }

    GraphiteResourceKey::Builder builder(&key, conversion_rsrc_type(), int32sNeeded, Shareable::kYes);

    int i = 0;
    builder[i++] = nonFormatYcbcrInfo;
    builder[i++] = (uint32_t)samplerDesc.format();
    if (usesExternalFormat) {
        builder[i++] = (uint32_t)(samplerDesc.format() >> 32);
    }
    builder[i++] = samplerDesc.formatFeatures(); // TODO: needed?
    SkASSERT(i == int32sNeeded);

    builder.finish();
    return key;
}

VulkanSamplerYcbcrConversion::VulkanSamplerYcbcrConversion(
        const VulkanSharedContext* context, VkSamplerYcbcrConversion ycbcrConversion)
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

