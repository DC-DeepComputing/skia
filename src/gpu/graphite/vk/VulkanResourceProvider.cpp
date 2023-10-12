/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanResourceProvider.h"

#include "include/core/SkSpan.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/MutableTextureStateRef.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanFramebuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSamplerYcbcrConversion.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"
#include "src/gpu/vk/VulkanMemory.h"
#include "src/sksl/SkSLCompiler.h"
#ifdef  SK_BUILD_FOR_ANDROID // needed, or check in file?
#include "include/gpu/vk/VulkanAHardwareBufferUtils.h"
#endif

namespace skgpu::graphite {

GraphiteResourceKey build_desc_set_key(const SkSpan<DescriptorData>& requestedDescriptors,
                                       const uint32_t uniqueId) {
    // TODO(nicolettep): Finalize & optimize key structure. Refactor to have the order of the
    // requested descriptors be irrelevant.
    // For now, to place some kind of upper limit on key size, limit a key to only containing
    // information for up to 9 descriptors. This number was selected due to having a maximum of 3
    // uniform buffer descriptors and observationally only encountering up to 6 texture/samplers for
    // our testing use cases. The 10th uint32 is reserved for housing a unique descriptor set ID.
    static const int kMaxDescriptorQuantity = 9;
    static const int kNum32DataCnt = kMaxDescriptorQuantity + 1;
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, kNum32DataCnt, Shareable::kNo);

    if (requestedDescriptors.size() > kMaxDescriptorQuantity) {
        SKGPU_LOG_E("%d descriptors requested, but graphite currently only supports creating"
                    "descriptor set keys for up to %d. The key will only take the first %d into"
                    " account.", static_cast<int>(requestedDescriptors.size()),
                    kMaxDescriptorQuantity, kMaxDescriptorQuantity);
    }

    for (size_t i = 0; i < kNum32DataCnt; i++) {
        if (i < requestedDescriptors.size()) {
            // TODO: Consider making the DescriptorData struct itself just use uint16_t.
            uint16_t smallerCount = static_cast<uint16_t>(requestedDescriptors[i].count);
            builder[i] =  static_cast<uint8_t>(requestedDescriptors[i].type) << 24
                          | requestedDescriptors[i].bindingIndex << 16
                          | smallerCount;
        } else {
            // Populate reminaing key components with 0.
            builder[i] = 0;
        }
    }
    builder[kNum32DataCnt - 1] = uniqueId;
    builder.finish();
    return key;
}

VulkanResourceProvider::VulkanResourceProvider(SharedContext* sharedContext,
                                               SingleOwner* singleOwner,
                                               uint32_t recorderID,
                                               size_t resourceBudget)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget) {}

VulkanResourceProvider::~VulkanResourceProvider() {
    if (fPipelineCache != VK_NULL_HANDLE) {
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyPipelineCache(this->vulkanSharedContext()->device(),
                                         fPipelineCache,
                                         nullptr));
    }
}

const VulkanSharedContext* VulkanResourceProvider::vulkanSharedContext() const {
    return static_cast<const VulkanSharedContext*>(fSharedContext);
}

sk_sp<Texture> VulkanResourceProvider::createWrappedTexture(const BackendTexture& texture) {
    VulkanTextureInfo vkTexInfo;
    if (!texture.info().getVulkanTextureInfo(&vkTexInfo)) {
        return nullptr;
    }
    auto ycbcrConversion =
            this->findOrCreateCompatibleSamplerYcbcrConversion(vkTexInfo.fYcbcrConversionInfo);
    return VulkanTexture::MakeWrapped(this->vulkanSharedContext(),
                                      texture.dimensions(),
                                      texture.info(),
                                      texture.getMutableState(),
                                      texture.getVkImage(),
                                      /*alloc=*/{}, // Skia does not own wrapped texture memory
                                      ycbcrConversion); // TODO track lifetime of conversion
}

sk_sp<GraphicsPipeline> VulkanResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    SkSL::Compiler skslCompiler(fSharedContext->caps()->shaderCaps());
    auto compatibleRenderPass =
            this->findOrCreateRenderPass(renderPassDesc, /*compatibleOnly=*/true);
    return VulkanGraphicsPipeline::Make(this->vulkanSharedContext(),
                                        &skslCompiler,
                                        runtimeDict,
                                        pipelineDesc,
                                        renderPassDesc,
                                        compatibleRenderPass,
                                        this->pipelineCache());
}

sk_sp<ComputePipeline> VulkanResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    return nullptr;
}

sk_sp<Texture> VulkanResourceProvider::createTexture(SkISize size, const TextureInfo& info,
                                                     skgpu::Budgeted budgeted) {
    VulkanTextureInfo vkTexInfo;
    if (!info.getVulkanTextureInfo(&vkTexInfo)) {
        return nullptr;
    }
    auto ycbcrConversion =
            this->findOrCreateCompatibleSamplerYcbcrConversion(vkTexInfo.fYcbcrConversionInfo);

    return VulkanTexture::Make(this->vulkanSharedContext(), size, info, budgeted,
                               std::move(ycbcrConversion));
}

sk_sp<Buffer> VulkanResourceProvider::createBuffer(size_t size,
                                                   BufferType type,
                                                   AccessPattern accessPattern) {
    return VulkanBuffer::Make(this->vulkanSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> VulkanResourceProvider::createSampler(const SkSamplingOptions& samplingOptions,
                                                     SkTileMode xTileMode,
                                                     SkTileMode yTileMode) {
    return VulkanSampler::Make(this->vulkanSharedContext(), samplingOptions, xTileMode, yTileMode);
}

BackendTexture VulkanResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                              const TextureInfo& info) {
    VulkanTextureInfo vkTexInfo;
    if (!info.getVulkanTextureInfo(&vkTexInfo)) {
        return {};
    }
    VulkanTexture::CreatedImageInfo createdTextureInfo;
    if (!VulkanTexture::MakeVkImage(this->vulkanSharedContext(), dimensions, info,
                                    &createdTextureInfo)) {
        return {};
    }
    return {dimensions,
            vkTexInfo,
            createdTextureInfo.fMutableState->getImageLayout(),
            createdTextureInfo.fMutableState->getQueueFamilyIndex(),
            createdTextureInfo.fImage,
            createdTextureInfo.fMemoryAlloc};
}

sk_sp<VulkanDescriptorSet> VulkanResourceProvider::findOrCreateDescriptorSet(
        SkSpan<DescriptorData> requestedDescriptors) {
    if (requestedDescriptors.empty()) {
        return nullptr;
    }
    // Search for available descriptor sets by assembling a key based upon the set's structure with
    // a unique set ID (which ranges from 0 to kMaxNumSets - 1). Start the search at 0 and continue
    // until an available set is found.
    // TODO(nicolettep): Explore ways to optimize this traversal.
    GraphiteResourceKey descSetKeys [VulkanDescriptorPool::kMaxNumSets];
    for (uint32_t i = 0; i < VulkanDescriptorPool::kMaxNumSets; i++) {
        GraphiteResourceKey key = build_desc_set_key(requestedDescriptors, i);
        if (auto descSet = fResourceCache->findAndRefResource(key, skgpu::Budgeted::kNo)) {
            // A non-null resource pointer indicates we have found an available descriptor set.
            return sk_sp<VulkanDescriptorSet>(static_cast<VulkanDescriptorSet*>(descSet));
        }
        descSetKeys[i] = key;
    }

    // If we did not find an existing avilable desc set, allocate sets with the appropriate layout
    // and add them to the cache.
    VkDescriptorSetLayout layout;
    DescriptorDataToVkDescSetLayout(this->vulkanSharedContext(), requestedDescriptors, &layout);
    if (!layout) {
        return nullptr;
    }
    auto pool = VulkanDescriptorPool::Make(this->vulkanSharedContext(),
                                           requestedDescriptors,
                                           layout);
    if (!pool) {
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyDescriptorSetLayout(this->vulkanSharedContext()->device(), layout,
                                               nullptr));
        return nullptr;
    }

    // First allocate one descriptor set to enforce that at least one set is successfully created.
    // If not, we can return early before attempting to allocate multiple sets. Having a defined ptr
    // to the first set also makes it such that it can be returned without having to perform a find
    // operation on the cache.
    auto firstDescSet = VulkanDescriptorSet::Make(this->vulkanSharedContext(), pool, layout);
    if (!firstDescSet) {
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyDescriptorSetLayout(this->vulkanSharedContext()->device(), layout,
                                               nullptr));
        VULKAN_CALL(this->vulkanSharedContext()->interface(),
                    DestroyDescriptorPool(this->vulkanSharedContext()->device(), pool->descPool(),
                                          nullptr));
        return nullptr;
    }
    firstDescSet->setKey(descSetKeys[0]);
    fResourceCache->insertResource(firstDescSet.get());

    // Continue to allocate & cache the maximum number of sets so they can be easily accessed as
    // they're needed.
    for (int i = 1; i < VulkanDescriptorPool::kMaxNumSets ; i++) {
        auto descSet = VulkanDescriptorSet::Make(this->vulkanSharedContext(), pool, layout);
        descSet->setKey(descSetKeys[i]);
        fResourceCache->insertResource(descSet.get());
    }

    return firstDescSet;
}

sk_sp<VulkanRenderPass> VulkanResourceProvider::findOrCreateRenderPass(
        const RenderPassDesc& renderPassDesc, bool compatibleOnly) {
    auto renderPassKey = VulkanRenderPass::MakeRenderPassKey(renderPassDesc, compatibleOnly);

    if (Resource* resource =
            fResourceCache->findAndRefResource(renderPassKey, skgpu::Budgeted::kYes)) {
        return sk_sp<VulkanRenderPass>(static_cast<VulkanRenderPass*>(resource));
        // TODO confirm budgeted
    }

    auto renderPass = VulkanRenderPass::MakeRenderPass(this->vulkanSharedContext(), renderPassDesc,
                                                       compatibleOnly);
    if (!renderPass) {
        return nullptr;
    }

    renderPass->setKey(renderPassKey);
    fResourceCache->insertResource(renderPass.get());

    return renderPass;
}

VkPipelineCache VulkanResourceProvider::pipelineCache() {
    if (fPipelineCache == VK_NULL_HANDLE) {
        VkPipelineCacheCreateInfo createInfo;
        memset(&createInfo, 0, sizeof(VkPipelineCacheCreateInfo));
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.initialDataSize = 0;
        createInfo.pInitialData = nullptr;
        VkResult result;
        VULKAN_CALL_RESULT(this->vulkanSharedContext()->interface(),
                           result,
                           CreatePipelineCache(this->vulkanSharedContext()->device(),
                                               &createInfo,
                                               nullptr,
                                               &fPipelineCache));
        if (VK_SUCCESS != result) {
            fPipelineCache = VK_NULL_HANDLE;
        }
    }
    return fPipelineCache;
}

sk_sp<VulkanFramebuffer> VulkanResourceProvider::createFramebuffer(
        const VulkanSharedContext* context,
        const skia_private::TArray<VkImageView>& attachmentViews,
        const VulkanRenderPass& renderPass,
        const int width,
        const int height) {
    // TODO: Consider caching these in the future. If we pursue that, it may make more sense to
    // use a compatible renderpass rather than a full one to make each frame buffer more versatile.
    VkFramebufferCreateInfo framebufferInfo;
    memset(&framebufferInfo, 0, sizeof(VkFramebufferCreateInfo));
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0;
    framebufferInfo.renderPass = renderPass.renderPass();
    framebufferInfo.attachmentCount = attachmentViews.size();
    framebufferInfo.pAttachments = attachmentViews.begin();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;
    return VulkanFramebuffer::Make(context, framebufferInfo);
}

void VulkanResourceProvider::onDeleteBackendTexture(const BackendTexture& texture) {
    SkASSERT(texture.isValid());
    SkASSERT(texture.backend() == BackendApi::kVulkan);

    if (texture.getMemoryAlloc() && texture.getMemoryAlloc()->fBackendMemory) {
        skgpu::VulkanMemory::FreeImageMemory(
                this->vulkanSharedContext()->memoryAllocator(), *(texture.getMemoryAlloc()));
    }

    VULKAN_CALL(this->vulkanSharedContext()->interface(),
                DestroyImage(this->vulkanSharedContext()->device(), texture.getVkImage(), nullptr));
}

sk_sp<VulkanSamplerYcbcrConversion>
        VulkanResourceProvider::findOrCreateCompatibleSamplerYcbcrConversion(
                const VulkanYcbcrConversionInfo& ycbcrInfo) const {
    if (!ycbcrInfo.isValid()) {
        return nullptr;
    }
    auto ycbcrConversionKey = VulkanSamplerYcbcrConversion::MakeYcbcrConversionKey(
            this->vulkanSharedContext(), ycbcrInfo);

    if (Resource* resource = fResourceCache->findAndRefResource(ycbcrConversionKey,
                                                                skgpu::Budgeted::kNo)) {
        return sk_sp<VulkanSamplerYcbcrConversion>(
                static_cast<VulkanSamplerYcbcrConversion*>(resource));
    }

    auto ycbcrConversion = VulkanSamplerYcbcrConversion::Make(this->vulkanSharedContext(),
                                                              ycbcrInfo);
    if (!ycbcrConversion) {
        return nullptr;
    }

    ycbcrConversion->setKey(ycbcrConversionKey);
    fResourceCache->insertResource(ycbcrConversion.get());

    return ycbcrConversion;
}

#ifdef SK_BUILD_FOR_ANDROID
TextureInfo VulkanResourceProvider::getTextureInfoFromAHardwareBuffer(
        AHardwareBuffer* hardwareBuffer,
        uint32_t bufferFormat,
        bool requireKnownFormat,
        bool isRenderable,
        bool isProtectedContent,
        bool fromAndroidWindow) const {
    const VulkanSharedContext* vkContext = this->vulkanSharedContext();
    VkDevice device = vkContext->device();

    VkFormat format = get_vk_format_from_ahardwarebuffer_format(bufferFormat);
    if (requireKnownFormat && format == VK_FORMAT_UNDEFINED) {
        SkDebugf("Encountered unknown format, but require known format.\n");
        return {};
    }

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    VulkanYcbcrConversionInfo ycbcrInfo;
    // VK_FORMAT_UNDEFINED indicates we have an external-only format.
    if (format == VK_FORMAT_UNDEFINED) {
        VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
        hwbFormatProps.sType =
                VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
        hwbFormatProps.pNext = nullptr;
        VkAndroidHardwareBufferPropertiesANDROID hwbProps;
        hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
        hwbProps.pNext = &hwbFormatProps;

        VkResult result = VULKAN_CALL(vkContext->interface(),
                                    GetAndroidHardwareBufferProperties(device,
                                                                        hardwareBuffer,
                                                                        &hwbProps));
        if (result != VK_SUCCESS) {
            SkDebugf("Failed to get AndroidHardwareBufferProperties\n");
            return {};
        }

        get_ycbcr_conversion_info_from_format_props(hwbFormatProps, &ycbcrInfo);
    } else {
        usageFlags = usageFlags |
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (isRenderable) {
            usageFlags = usageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }

    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL; // TODO: Query for tiling?
    VkImageCreateFlags imgCreateflags =
            isProtectedContent ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;

    // TODO: Determine VkImageAspectFlags. If the VkImage is a Ycbcr format, the client can select a
    // plane. For now, use the default VK_IMAGE_ASPECT_COLOR_BIT.
    VulkanTextureInfo vulkanTextureInfo {VK_SAMPLE_COUNT_1_BIT,
                                         Mipmapped::kNo,
                                         imgCreateflags,
                                         format,
                                         tiling,
                                         usageFlags,
                                         VK_SHARING_MODE_EXCLUSIVE,
                                         VK_IMAGE_ASPECT_COLOR_BIT,
                                         ycbcrInfo};
    return {vulkanTextureInfo};
}

BackendTexture VulkanResourceProvider::createBackendTexture(
        AHardwareBuffer* hardwareBuffer,
        int width, int height,
        const TextureInfo& textureInfo) const {
    VulkanTextureInfo vkTexInfo;
    if (!textureInfo.getVulkanTextureInfo(&vkTexInfo)) {
        SKGPU_LOG_W("Failed to fetch Vulkan texture information");
        return {};
    }

    const VulkanSharedContext* vkContext = this->vulkanSharedContext();
    SkASSERT(!(vkTexInfo.fFlags & VK_IMAGE_CREATE_PROTECTED_BIT) ||
             fSharedContext->isProtected() == Protected::kYes);

    VkDevice device = vkContext->device();

    VkResult result;
    VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
    hwbFormatProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    hwbFormatProps.pNext = nullptr;

    VkAndroidHardwareBufferPropertiesANDROID hwbProps;
    hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    hwbProps.pNext = &hwbFormatProps;

    result = VULKAN_CALL(vkContext->interface(),
                         GetAndroidHardwareBufferProperties(device, hardwareBuffer, &hwbProps));
    if (result != VK_SUCCESS) {
        return {};
    }
    if (hwbFormatProps.format != vkTexInfo.fFormat) {
        SkDebugf("Queried format not consistent with expected format; got: %d, expected: %d",
                 hwbFormatProps.format,
                 vkTexInfo.fFormat);
        return {};
    }

    VkExternalFormatANDROID externalFormat;
    externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormat.pNext = nullptr;
    externalFormat.externalFormat = 0;  // If this is zero it is as if we aren't using this struct.

    if (hwbFormatProps.format != VK_FORMAT_UNDEFINED) {
        SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures) &&
                 SkToBool(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & hwbFormatProps.formatFeatures) &&
                 SkToBool(VK_FORMAT_FEATURE_TRANSFER_DST_BIT & hwbFormatProps.formatFeatures));
        SkASSERT(!vkTexInfo.fYcbcrConversionInfo.isValid());
    } else {
        // We have an external only format.
        SkASSERT(vkTexInfo.fYcbcrConversionInfo.isValid());
        SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures));
        SkASSERT(vkTexInfo.fFormat == VK_FORMAT_UNDEFINED);
        externalFormat.externalFormat = hwbFormatProps.externalFormat;
    }

    const VkExternalMemoryImageCreateInfo externalMemoryImageInfo{
            VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,                 // sType
            &externalFormat,                                                     // pNext
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,  // handleTypes
    };

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        &externalMemoryImageInfo,                    // pNext
        vkTexInfo.fFlags,                            // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                            // VkImageType
        vkTexInfo.fFormat,                           // VkFormat
        { (uint32_t)width, (uint32_t)height, 1 },    // VkExtent3D
        1,                                           // mipLevels
        1,                                           // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                       // samples
        vkTexInfo.fImageTiling,                      // VkImageTiling
        vkTexInfo.fImageUsageFlags,                  // VkImageUsageFlags
        vkTexInfo.fSharingMode,                      // VkSharingMode
        0,                                           // queueFamilyCount
        nullptr,                                     // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED,                   // initialLayout
    };

    VkImage image;
    result = VULKAN_CALL(vkContext->interface(),
                         CreateImage(device, &imageCreateInfo, nullptr, &image));
    if (result != VK_SUCCESS) {
        return {};
    }

    uint32_t typeIndex = 0;
    bool foundHeap = false;
    VkPhysicalDeviceMemoryProperties2 phyDevMemProps =
            vkContext->vulkanCaps().physicalDeviceMemoryProperties2();
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
        VULKAN_CALL(vkContext->interface(), DestroyImage(device, image, nullptr));
        return {};
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
    result = VULKAN_CALL(vkContext->interface(),
                         AllocateMemory(device, &allocInfo, nullptr, &memory));
    if (result != VK_SUCCESS) {
        VULKAN_CALL(vkContext->interface(), DestroyImage(device, image, nullptr));
        return {};
    }

    VkBindImageMemoryInfo bindImageInfo;
    bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindImageInfo.pNext = nullptr;
    bindImageInfo.image = image;
    bindImageInfo.memory = memory;
    bindImageInfo.memoryOffset = 0;

    result = VULKAN_CALL(vkContext->interface(), BindImageMemory2(device, 1, &bindImageInfo));
    if (result != VK_SUCCESS) {
        VULKAN_CALL(vkContext->interface(), DestroyImage(device, image, nullptr));
        VULKAN_CALL(vkContext->interface(), FreeMemory(device, memory, nullptr));
        return {};
    }

    skgpu::VulkanAlloc alloc;
    alloc.fMemory = memory;
    alloc.fOffset = 0;
    alloc.fSize = hwbProps.allocationSize;
    alloc.fFlags = 0;

    return {{width, height}, vkTexInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_QUEUE_FAMILY_EXTERNAL,
             image, alloc};
}
#endif // SK_BUILD_FOR_ANDROID

} // namespace skgpu::graphite
