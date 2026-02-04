/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnVkSharedContext.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnVkResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#if defined(SK_USE_VMA)
#include "src/gpu/vk/vulkanmemoryallocator/VulkanMemoryAllocatorPriv.h"
#endif

namespace skgpu::graphite {
struct ContextOptions;

sk_sp<SharedContext> DawnVkSharedContext::Make(const VulkanBackendContext& vkBackendContext,
                                               const DawnBackendContext& dawnBackendContext,
                                               const ContextOptions& options) {
    sk_sp<SharedContext> dawnSharedContext = DawnSharedContext::Make(dawnBackendContext, options);
    if (!dawnSharedContext) {
        return nullptr;
    }

    if (vkBackendContext.fInstance == VK_NULL_HANDLE ||
        vkBackendContext.fPhysicalDevice == VK_NULL_HANDLE ||
        vkBackendContext.fDevice == VK_NULL_HANDLE ||
        vkBackendContext.fQueue == VK_NULL_HANDLE) {
        SKGPU_LOG_E("Failed to create VulkanSharedContext because either fInstance,"
                    "fPhysicalDevice, fDevice, or fQueue in the VulkanBackendContext is"
                    "VK_NULL_HANDLE.");
        return nullptr;
    }
    if (!vkBackendContext.fGetProc) {
        SKGPU_LOG_E("Failed to create VulkanSharedContext because there is no valid VulkanGetProc"
                    "on the VulkanBackendContext");
        return nullptr;
    }
    // If no extensions are provided, make sure we don't have a null dereference downstream.
    skgpu::VulkanExtensions noExtensions;
    const skgpu::VulkanExtensions* extensions = &noExtensions;
    if (vkBackendContext.fVkExtensions) {
        extensions = vkBackendContext.fVkExtensions;
    }

    uint32_t physDevVersion = 0;
    sk_sp<const skgpu::VulkanInterface> interface =
            skgpu::MakeInterface(vkBackendContext, extensions, &physDevVersion, nullptr);
    if (!interface) {
        SKGPU_LOG_E("Failed to create VulkanInterface.");
        return nullptr;
    }

    VkPhysicalDeviceFeatures2 features;
    const VkPhysicalDeviceFeatures2* featuresPtr;
    // If fDeviceFeatures2 is not null, then we ignore fDeviceFeatures. If both are null, we assume
    // no features are enabled.
    if (!vkBackendContext.fDeviceFeatures2 && vkBackendContext.fDeviceFeatures) {
        features.pNext = nullptr;
        features.features = *vkBackendContext.fDeviceFeatures;
        featuresPtr = &features;
    } else {
        featuresPtr = vkBackendContext.fDeviceFeatures2;
    }

    std::unique_ptr<const VulkanCaps> caps(new VulkanCaps(options,
                                                          interface.get(),
                                                          vkBackendContext.fPhysicalDevice,
                                                          physDevVersion,
                                                          featuresPtr,
                                                          extensions,
                                                          vkBackendContext.fProtectedContext));

    sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator = vkBackendContext.fMemoryAllocator;
#if defined(SK_USE_VMA)
    if (!memoryAllocator) {
        SkDebugf("making my own allocator\n");
        // We were not given a memory allocator at creation
        skgpu::ThreadSafe threadSafe = options.fClientWillExternallySynchronizeAllThreads
                                               ? skgpu::ThreadSafe::kNo
                                               : skgpu::ThreadSafe::kYes;
        memoryAllocator = skgpu::VulkanMemoryAllocators::Make(vkBackendContext,
                                                              threadSafe,
                                                              options.fVulkanVMALargeHeapBlockSize);
    }
#endif
    if (!memoryAllocator) {
        SKGPU_LOG_E("No supplied vulkan memory allocator and unable to create one internally.");
        return nullptr;
    }

    return sk_sp<SharedContext>(new DawnVkSharedContext(vkBackendContext,
                                                        std::move(interface),
                                                        std::move(memoryAllocator),
                                                        std::move(caps),
                                                        options.fExecutor,
                                                        options.fPersistentPipelineStorage,
                                                        options.fUserDefinedKnownRuntimeEffects,
                                                        std::move(dawnSharedContext)));
}

DawnVkSharedContext::DawnVkSharedContext(
        const VulkanBackendContext& backendContext,
        sk_sp<const skgpu::VulkanInterface> interface,
        sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator,
        std::unique_ptr<const VulkanCaps> caps,
        SkExecutor* executor,
        PersistentPipelineStorage* persistentPipelineStorage,
        SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects,
        sk_sp<SharedContext> dawnSharedContext)
        : VulkanSharedContext(backendContext,
                              std::move(interface),
                              std::move(memoryAllocator),
                              std::move(caps),
                              executor,
                              persistentPipelineStorage,
                              userDefinedKnownRuntimeEffects)
        , fDawnSharedContext(std::move(dawnSharedContext)) {
}

DawnVkSharedContext::~DawnVkSharedContext() {}


void DawnVkSharedContext::checkBackends() {
/*
    id<MTLDevice> metalDevice = (__bridge id<MTLDevice>)this->dawnDevice().GetMetalDeviceId();
    id<MTLCommandQueue> metalQueue = (__bridge id<MTLCommandQueue>)this->dawnDevice().GetMetalQueueId();

    SkDebugf("Device name in checkBackends: %s, pointer: %p\n", metalDevice.name.UTF8String, metalDevice);
    SkDebugf("Queue pointer in checkBackends: %p\n", metalQueue);
    */
}

std::unique_ptr<ResourceProvider> DawnVkSharedContext::makeResourceProvider(
        SingleOwner* singleOwner,
        uint32_t recorderID,
        size_t resourceBudget) {
    return std::unique_ptr<ResourceProvider>(new DawnVkResourceProvider(this,
                                                                        singleOwner,
                                                                        recorderID,
                                                                        resourceBudget));
}

TextureInfo DawnVkSharedContext::getDefaultSampledTextureInfo(SkColorType ct,
                                                               Mipmapped mm,
                                                               Protected prot,
                                                               Renderable ren) const {
    return this->dawnSharedContext()->caps()->getDefaultSampledTextureInfo(ct, mm, prot, ren);
}

const wgpu::Device& DawnVkSharedContext::dawnDevice() const {
    return this->dawnSharedContext()->device();
}

DawnSharedContext* DawnVkSharedContext::dawnSharedContext() const {
    return static_cast<DawnSharedContext*>(fDawnSharedContext.get());
}

VulkanSharedContext::QueueSubmitResult DawnVkSharedContext::backendQueueSubmit(VkSubmitInfo* submitInfo,
                                                          VkFence* fence) const {
    bool result = this->dawnDevice().SubmitVulkanCommandBuffer(submitInfo, fence,
                                                               0, nullptr, nullptr,
                                                               0, nullptr, nullptr);
    return result ? VulkanSharedContext::QueueSubmitResult::kSuccess : VulkanSharedContext::QueueSubmitResult::kError;
}

std::optional<BackendTexture>
DawnVkSharedContext::convertBackendTextureIfNeeded(const BackendTexture& dawnBeTex) const {
    if (!dawnBeTex.isValid() || dawnBeTex.backend() != BackendApi::kDawn) {
        return {};
    }

    wgpu::Texture dawnTexture = BackendTextures::GetDawnTexturePtr(dawnBeTex);
    VkImage vkImage = *reinterpret_cast<VkImage*>(dawnTexture.GetVulkanImage());
    VulkanTextureInfo texInfo;
    texInfo.fFlags = 0;
    texInfo.fFormat = static_cast<VkFormat>(dawnTexture.GetVulkanImageFormat());
    texInfo.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    texInfo.fImageUsageFlags = static_cast<VkImageUsageFlags>(dawnTexture.GetVulkanImageUsage());
    texInfo.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    texInfo.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageLayout layout = static_cast<VkImageLayout>(dawnTexture.GetVulkanImageLayout());
    uint32_t queueFamilyIndex = this->queueIndex();
    VulkanAlloc vulkanAlloc;

    return BackendTextures::MakeVulkan(dawnBeTex.dimensions(), texInfo, layout, queueFamilyIndex,
                                       vkImage, vulkanAlloc);
}

std::optional<TextureInfo> DawnVkSharedContext::convertTextureInfo(const TextureInfo& info) const {
    if (!info.isValid() || info.backend() != BackendApi::kDawn) {
        return {};
    }

    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(info);

    VkFormat vkFormat = TextureFormatToVkFormat(DawnFormatToTextureFormat(dawnInfo.fFormat));

    VulkanTextureInfo vkTextureInfo;
    vkTextureInfo.fFlags = 0;
    vkTextureInfo.fFormat = vkFormat;
    vkTextureInfo.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    vkTextureInfo.fImageUsageFlags = 0;
    vkTextureInfo.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkTextureInfo.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //vkTextureInfo.fYcbcrConversionInfo;

    return TextureInfos::MakeVulkan(vkTextureInfo);
}

} // namespace skgpu::graphite

