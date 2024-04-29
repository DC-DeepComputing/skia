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

GraphiteResourceKey VulkanSamplerYcbcrConversion::MakeYcbcrConversionKey(
        const VulkanSharedContext* context, const VulkanYcbcrConversionInfo& info) {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    bool useExternalFormat = info.fFormat == VK_FORMAT_UNDEFINED;
    // 2 uint32s needed for the external format OR 1 for a known VkFormat. 1 uint32 can store all
    // other differentiating ycbcr information.
    const int num32DataCnt = useExternalFormat ? 3 : 2;
    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kYes);

    int i = 0;
    if (useExternalFormat) {
        builder[i++] = (uint32_t)info.fExternalFormat;
        builder[i++] = (uint32_t)(info.fExternalFormat >> 32);
    } else {
        builder[i++] = info.fFormat;
    }
    builder[i++] = info.nonFormatInfoAsUInt32();

    SkASSERT(i == num32DataCnt);

    builder.finish();
    return key;
}

// TODO: review this method.
GraphiteResourceKey VulkanSamplerYcbcrConversion::GetKeyFromSamplerDesc(
        const SamplerDesc& samplerDesc) {
    SkASSERT(samplerDesc.format());
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    GraphiteResourceKey key;
    int int32sNeeded = 1; // 1 needed for non-format information

    bool usesExternalFormat = (bool)(samplerDesc.desc() >>
            (SamplerDesc::kConversionInfoShift +
             VulkanYcbcrConversionInfo::kUsesExternalFormatShift));
    // If using an external format, we need to append more data thn if we are using a known one.
    // TODO: Make static constant vals?
    int32sNeeded += usesExternalFormat ? 2 : 1;

    GraphiteResourceKey::Builder builder(&key, kType, int32sNeeded, Shareable::kYes);

    int i = 0;
    builder[i++] = (uint32_t)(samplerDesc.desc() >> SamplerDesc::kConversionInfoShift);
    if (usesExternalFormat) {
        builder[i++] = (uint32_t)samplerDesc.format();
        builder[i++] = (uint32_t)(samplerDesc.format() >> 32);
    } else {
        builder[i++] = (uint32_t)samplerDesc.format();
    }
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

