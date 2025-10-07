/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/GraphiteNativeVulkanWindowContext.h"

#include "include/core/SkSurface.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/BackendSemaphore.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/vk/VulkanGraphiteContext.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/base/SkAutoMalloc.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/gpu/vk/vulkanmemoryallocator/VulkanAMDMemoryAllocator.h"
#include "tools/ToolUtils.h"
#include "tools/graphite/GraphiteToolUtils.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif

#define GET_PROC(F) f##F = (PFN_vk##F)backendContext.fGetProc("vk" #F, fInstance, VK_NULL_HANDLE)
#define GET_DEV_PROC(F) f##F = (PFN_vk##F)backendContext.fGetProc("vk" #F, VK_NULL_HANDLE, fDevice)

namespace skwindow::internal {

GraphiteVulkanWindowContext::GraphiteVulkanWindowContext(
        std::unique_ptr<const DisplayParams> params,
        CreateVkSurfaceFn createVkSurface,
        CanPresentFn canPresent,
        PFN_vkGetInstanceProcAddr instProc)
        : WindowContext(std::move(params))
        , fBackbuffers(nullptr)
        , fVkImages(nullptr)
        , fSwapchainImgInfo(nullptr)
        , fDeviceSurface(VK_NULL_HANDLE)
        , fSwapchain(VK_NULL_HANDLE)
        , fCreateVkSurfaceFn(std::move(createVkSurface))
        , fCanPresentFn(std::move(canPresent)) {
    fGetInstanceProcAddr = instProc;
    this->initializeContext();
}

void GraphiteVulkanWindowContext::initializeContext() {
    SkASSERT(!fGraphiteContext && !fGraphiteRecorder);
    // Any config code here (particularly for msaa)?

    PFN_vkGetInstanceProcAddr getInstanceProc = fGetInstanceProcAddr;
    skgpu::VulkanBackendContext backendContext;
    skgpu::VulkanExtensions extensions;
    sk_gpu_test::TestVkFeatures features;
    if (!sk_gpu_test::CreateVkBackendContext(getInstanceProc,
                                             &backendContext,
                                             &extensions,
                                             &features,
                                             &fDebugMessenger,
                                             &fPresentQueueIndex,
                                             fCanPresentFn,
                                             fDisplayParams->createProtectedNativeBackend())) {
        return;
    }

    if (!extensions.hasExtension(VK_KHR_SURFACE_EXTENSION_NAME, 25) ||
        !extensions.hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, 68)) {
        return;
    }

    fInstance = backendContext.fInstance;
    fPhysicalDevice = backendContext.fPhysicalDevice;
    fDevice = backendContext.fDevice;
    fGraphicsQueueIndex = backendContext.fGraphicsQueueIndex;
    fGraphicsQueue = backendContext.fQueue;

    PFN_vkGetPhysicalDeviceProperties localGetPhysicalDeviceProperties =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(backendContext.fGetProc(
                    "vkGetPhysicalDeviceProperties", backendContext.fInstance, VK_NULL_HANDLE));
    if (!localGetPhysicalDeviceProperties) {
        return;
    }
    VkPhysicalDeviceProperties physDeviceProperties;
    localGetPhysicalDeviceProperties(backendContext.fPhysicalDevice, &physDeviceProperties);
    uint32_t physDevVersion = physDeviceProperties.apiVersion;

    fInterface.reset(new skgpu::VulkanInterface(backendContext.fGetProc,
                                                fInstance,
                                                fDevice,
                                                backendContext.fMaxAPIVersion,
                                                physDevVersion,
                                                &extensions));

    GET_PROC(DestroyInstance);
    if (fDebugMessenger != VK_NULL_HANDLE) {
        GET_PROC(DestroyDebugUtilsMessengerEXT);
    }
    GET_PROC(DestroySurfaceKHR);
    GET_PROC(GetPhysicalDeviceSurfaceSupportKHR);
    GET_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_PROC(GetPhysicalDeviceSurfaceFormatsKHR);
    GET_PROC(GetPhysicalDeviceSurfacePresentModesKHR);
    GET_DEV_PROC(DeviceWaitIdle);
    GET_DEV_PROC(QueueWaitIdle);
    GET_DEV_PROC(DestroyDevice);
    GET_DEV_PROC(CreateSwapchainKHR);
    GET_DEV_PROC(DestroySwapchainKHR);
    GET_DEV_PROC(GetSwapchainImagesKHR);
    GET_DEV_PROC(AcquireNextImageKHR);
    GET_DEV_PROC(QueuePresentKHR);
    GET_DEV_PROC(GetDeviceQueue);

    skgpu::graphite::ContextOptions contextOptions;
    skgpu::graphite::ContextOptionsPriv contextOptionsPriv;
    // Needed to make synchronous readPixels work
    contextOptionsPriv.fStoreContextRefInRecorder = true;
    contextOptions.fOptionsPriv = &contextOptionsPriv;
    fGraphiteContext = skgpu::graphite::ContextFactory::MakeVulkan(backendContext, contextOptions);
    fGraphiteRecorder = fGraphiteContext->makeRecorder(ToolUtils::CreateTestingRecorderOptions());

    fDeviceSurface = fCreateVkSurfaceFn(fInstance);
    if (VK_NULL_HANDLE == fDeviceSurface) {
        this->destroyContext();
        return;
    }

    VkBool32 supported;
    VkResult res = fGetPhysicalDeviceSurfaceSupportKHR(
            fPhysicalDevice, fPresentQueueIndex, fDeviceSurface, &supported);
    if (VK_SUCCESS != res) {
        this->destroyContext();
        return;
    }

    if (!this->createSwapchain(-1, -1)) {
        this->destroyContext();
        return;
    }

    fGetDeviceQueue(fDevice, fPresentQueueIndex, /*queueIndex=*/0, &fPresentQueue);
}

bool GraphiteVulkanWindowContext::createSwapchain(int width, int height) {
    // Check surface capabilities
    VkSurfaceCapabilitiesKHR caps;
    VkResult res = fGetPhysicalDeviceSurfaceCapabilitiesKHR(fPhysicalDevice, fDeviceSurface, &caps);
    if (VK_SUCCESS != res) {
        return false;
    }

    uint32_t surfaceFormatCount;
    res = fGetPhysicalDeviceSurfaceFormatsKHR(
            fPhysicalDevice, fDeviceSurface, &surfaceFormatCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }

    SkAutoMalloc surfaceFormatAlloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
    VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)surfaceFormatAlloc.get();
    res = fGetPhysicalDeviceSurfaceFormatsKHR(
            fPhysicalDevice, fDeviceSurface, &surfaceFormatCount, surfaceFormats);
    if (VK_SUCCESS != res) {
        return false;
    }

    uint32_t presentModeCount;
    res = fGetPhysicalDeviceSurfacePresentModesKHR(
            fPhysicalDevice, fDeviceSurface, &presentModeCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }

    SkAutoMalloc presentModeAlloc(presentModeCount * sizeof(VkPresentModeKHR));
    VkPresentModeKHR* presentModes = (VkPresentModeKHR*)presentModeAlloc.get();
    res = fGetPhysicalDeviceSurfacePresentModesKHR(
            fPhysicalDevice, fDeviceSurface, &presentModeCount, presentModes);
    if (VK_SUCCESS != res) {
        return false;
    }

    VkExtent2D extent = caps.currentExtent;
    // Use the hints for width + height
    if (extent.width == (uint32_t)-1) {
        extent.width = width;
        extent.height = height;
    }
    // Clamp the values to protect from broken hints
    if (extent.width < caps.minImageExtent.width) {
        extent.width = caps.minImageExtent.width;
    } else if (extent.width > caps.maxImageExtent.width) {
        extent.width = caps.maxImageExtent.width;
    }
    if (extent.height < caps.minImageExtent.height) {
        extent.height = caps.minImageExtent.height;
    } else if (extent.height > caps.maxImageExtent.height) {
        extent.height = caps.maxImageExtent.height;
    }
    fWidth = (int)extent.width;
    fHeight = (int)extent.height;

    uint32_t imageCount = caps.minImageCount + 2;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
        // Application must settle for fewer images than desired:
        imageCount = caps.maxImageCount;
    }

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    SkASSERT((caps.supportedUsageFlags & usageFlags) == usageFlags);
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
        usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
        usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    SkASSERT(caps.supportedTransforms & caps.currentTransform);
    SkASSERT(caps.supportedCompositeAlpha &
             (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));
    VkCompositeAlphaFlagBitsKHR composite_alpha =
            (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
                    ? VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
                    : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Pick our surface format.
    VkFormat surfaceFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    for (uint32_t i = 0; i < surfaceFormatCount; ++i) {
        VkFormat localFormat = surfaceFormats[i].format;
        skgpu::graphite::TextureFormat format =
            skgpu::graphite::VkFormatToTextureFormat(localFormat);
        // Skip unsupported and HW sRGB formats. We can technically render to the sRGB formats
        // but it requires the SkColorSpace to have a linear gamut. Viewer needs to be able to
        // set the dst color space for legacy color management and various other modes, so we
        // skip those formats here for compatibility.
        if (format != skgpu::graphite::TextureFormat::kUnsupported &&
            format != skgpu::graphite::TextureFormat::kRGBA8_sRGB &&
            format != skgpu::graphite::TextureFormat::kBGRA8_sRGB) {
            surfaceFormat = localFormat;
            colorSpace = surfaceFormats[i].colorSpace;
            break;
        }
    }
    fSampleCount = std::max(1, fDisplayParams->msaaSampleCount());
    fStencilBits = 8;

    if (VK_FORMAT_UNDEFINED == surfaceFormat) {
        return false;
    }

    SkColorType colorType;
    switch (surfaceFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            colorType = kRGBA_8888_SkColorType;
            break;
        case VK_FORMAT_B8G8R8A8_UNORM:
            colorType = kBGRA_8888_SkColorType;
            break;
        default:
            return false;
    }

    // If mailbox mode is available, use it, as it is the lowest-latency non-tearing mode. If not,
    // fall back to FIFO which is always available.
    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
    bool hasImmediate = false;
    for (uint32_t i = 0; i < presentModeCount; ++i) {
        // use mailbox
        if (VK_PRESENT_MODE_MAILBOX_KHR == presentModes[i]) {
            mode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
        if (VK_PRESENT_MODE_IMMEDIATE_KHR == presentModes[i]) {
            hasImmediate = true;
        }
    }
    if (fDisplayParams->disableVsync() && hasImmediate) {
        mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    memset(&swapchainCreateInfo, 0, sizeof(VkSwapchainCreateInfoKHR));
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.flags = fDisplayParams->createProtectedNativeBackend()
                                        ? VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR
                                        : 0;
    swapchainCreateInfo.surface = fDeviceSurface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat;
    swapchainCreateInfo.imageColorSpace = colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = usageFlags;

    uint32_t queueFamilies[] = {fGraphicsQueueIndex, fPresentQueueIndex};
    if (fGraphicsQueueIndex != fPresentQueueIndex) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilies;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha = composite_alpha;
    swapchainCreateInfo.presentMode = mode;
    swapchainCreateInfo.clipped = true;
    swapchainCreateInfo.oldSwapchain = fSwapchain;

    res = fCreateSwapchainKHR(fDevice, &swapchainCreateInfo, nullptr, &fSwapchain);
    if (VK_SUCCESS != res) {
        return false;
    }

    // Destroy the old swapchain
    if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE) {
        fDeviceWaitIdle(fDevice);
        this->destroyBuffers();
        fDestroySwapchainKHR(fDevice, swapchainCreateInfo.oldSwapchain, nullptr);
        swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    }
    // If buffer creation fails, destroy the swapchain.
    if (!this->createBuffers(swapchainCreateInfo.imageFormat,
                             usageFlags,
                             colorType,
                             swapchainCreateInfo.imageSharingMode)) {
        fDeviceWaitIdle(fDevice);
        this->destroyBuffers();
        fDestroySwapchainKHR(fDevice, swapchainCreateInfo.oldSwapchain, nullptr);
        swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

bool GraphiteVulkanWindowContext::createBuffers(VkFormat format,
                                                VkImageUsageFlags usageFlags,
                                                SkColorType colorType,
                                                VkSharingMode sharingMode) {
    // Determine number of swapchain images
    fGetSwapchainImagesKHR(fDevice, fSwapchain, &fSwapchainImgCount, /*pSwapchainImages*/nullptr);
    SkASSERT(fSwapchainImgCount);

    // Obtain the VkImages.
    fVkImages = new VkImage[fSwapchainImgCount];
    fGetSwapchainImagesKHR(fDevice, fSwapchain, &fSwapchainImgCount, fVkImages);

    // Define static semaphore creation info for all render completion semaphores
    static const VkSemaphoreCreateInfo submitSemInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                                                        /*pNext=*/nullptr,
                                                        /*flags=*/0};

    // Initialize array of backbuffer image indices. Create one additional backbuffer in order to
    // give the command buffers they contain a chance to finish before we cycle back.
    const uint32_t backbufferCount = fSwapchainImgCount + 1;
    fBackbuffers = new Backbuffer[backbufferCount];

    // Populate all swapchain image representations.
    fSwapchainImgInfo = new SwapchainImageInfo[fSwapchainImgCount];
    VkResult result;
    for (uint32_t i = 0; i < fSwapchainImgCount; ++i) {
        // All images start in undefined layout
        fSwapchainImgInfo[i].fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        // Create the semaphore that will be signaled once the image done being rendered
        VULKAN_CALL_RESULT_NOCHECK(
                fInterface,
                result,
                CreateSemaphore(fDevice,
                                &submitSemInfo,
                                /*pAllocator=*/nullptr,
                                &fSwapchainImgInfo[i].fRenderCompletionSemaphore));

        // Create a Surface associated with each image for presentation
        skgpu::graphite::VulkanTextureInfo info;
        info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
        info.fFormat = format;
        info.fImageUsageFlags = usageFlags;
        info.fSharingMode = sharingMode;
        info.fFlags =
                fDisplayParams->createProtectedNativeBackend() ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
        auto backendTex = skgpu::graphite::BackendTextures::MakeVulkan(this->dimensions(),
                                                                       info,
                                                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                                                       fPresentQueueIndex,
                                                                       fVkImages[i],
                                                                       skgpu::VulkanAlloc());
        fSwapchainImgInfo[i].fSurface = SkSurfaces::WrapBackendTexture(
                this->graphiteRecorder(),
                backendTex,
                colorType,
                fDisplayParams->colorSpace(),
                &fDisplayParams->surfaceProps());

        if (!fSwapchainImgInfo[i].fSurface) {
            // Clean up any previously-created components before returning false to indicate failure
            SKGPU_LOG_W("Failed to create Surface for swapchain image");
            this->destroyBuffers();
            return false;
        }
    }
    return true;
}

void GraphiteVulkanWindowContext::destroyBuffers() {
    if (fSwapchainImgInfo) {
        for (uint32_t i = 0; i < fSwapchainImgCount; ++i) {
            VULKAN_CALL(fInterface,
                        DestroySemaphore(fDevice, fSwapchainImgInfo[i].fRenderCompletionSemaphore,
                                         nullptr));
        }
    }
    delete[] fBackbuffers;
    fBackbuffers = nullptr;
    delete[] fSwapchainImgInfo;
    fSwapchainImgInfo = nullptr;
    delete[] fVkImages;
    fVkImages = nullptr;
}

GraphiteVulkanWindowContext::~GraphiteVulkanWindowContext() { this->destroyContext(); }

void GraphiteVulkanWindowContext::destroyContext() {
    if (this->isValid()) {
        if (fPresentQueue != VK_NULL_HANDLE) {
            fQueueWaitIdle(fPresentQueue);
        }
        fDeviceWaitIdle(fDevice);

        if (fAcquireSemaphore != VK_NULL_HANDLE) {
            VULKAN_CALL(fInterface,
                        DestroySemaphore(fDevice, fAcquireSemaphore, /*pAllocator=*/nullptr));
            fAcquireSemaphore = VK_NULL_HANDLE;
        }

        this->destroyBuffers();

        if (fSwapchain != VK_NULL_HANDLE) {
            fDestroySwapchainKHR(fDevice, fSwapchain, /*pAllocator=*/nullptr);
            fSwapchain = VK_NULL_HANDLE;
        }

        if (fDeviceSurface != VK_NULL_HANDLE) {
            fDestroySurfaceKHR(fInstance, fDeviceSurface, /*pAllocator=*/nullptr);
            fDeviceSurface = VK_NULL_HANDLE;
        }
    }

    if (fGraphiteContext) {
        fGraphiteRecorder.reset();
        fGraphiteContext.reset();
    }
    fInterface.reset();

    if (fDevice != VK_NULL_HANDLE) {
        fDestroyDevice(fDevice, /*pAllocator=*/nullptr);
        fDevice = VK_NULL_HANDLE;
    }

#ifdef SK_ENABLE_VK_LAYERS
    if (fDebugMessenger != VK_NULL_HANDLE) {
        fDestroyDebugUtilsMessengerEXT(fInstance, fDebugMessenger, /*pAllocator=*/nullptr);
    }
#endif

    fPhysicalDevice = VK_NULL_HANDLE;

    if (fInstance != VK_NULL_HANDLE) {
        fDestroyInstance(fInstance, /*pAllocator=*/nullptr);
        fInstance = VK_NULL_HANDLE;
    }
}

bool GraphiteVulkanWindowContext::submitToGpu(Backbuffer* backbuffer) {
    if (!fGraphiteContext) {
        SKGPU_LOG_W("Cannot have null graphite context when submitting work to the GPU for "
                    "presentation.");
        return false;
    }
    SkASSERT(fGraphiteRecorder);

    std::unique_ptr<skgpu::graphite::Recording> recording = fGraphiteRecorder->snap();
    if (!recording) {
        SKGPU_LOG_W("Failed to snap recording for submitting work to the GPU for presentation. "
                    "Will not submit the present operation to the present queue.");
        return false;
    }

    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording.get();

    // Set up surface for layout transition
    info.fTargetSurface = fSwapchainImgInfo[backbuffer->fImageIndex].fSurface.get();
    skgpu::MutableTextureState presentState = skgpu::MutableTextureStates::MakeVulkan(
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, fPresentQueueIndex);
    info.fTargetTextureState = &presentState;

    // Populate InsertRecordingInfo semaphores by wrapping VkSemaphores in graphite representation.
    // The recorder should wait upon swapchain image acquisition:
    info.fNumWaitSemaphores = 1;
    SkASSERT(fAcquireSemaphore != VK_NULL_HANDLE);
    auto beAcquireSemaphore = skgpu::graphite::BackendSemaphores::MakeVulkan(fAcquireSemaphore);
    info.fWaitSemaphores = &beAcquireSemaphore;
    // The recorder should signal render completion for its associated image:
    info.fNumSignalSemaphores = 1;
    VkSemaphore renderCompletionSemaphore =
            fSwapchainImgInfo[backbuffer->fImageIndex].fRenderCompletionSemaphore;
    auto beRenderingCompleteSemaphore =
            skgpu::graphite::BackendSemaphores::MakeVulkan(renderCompletionSemaphore);
    info.fSignalSemaphores = &beRenderingCompleteSemaphore;

    // Insert finishedProc to delete the wait semaphore when done. The signal semaphore used for
    // rendering completion is handled by destroyBuffers().
    struct FinishContext {
        sk_sp<const skgpu::VulkanInterface> fInterface;
        VkDevice fDevice;
        VkSemaphore fWaitSemaphore;
    };
    auto* finishContext = new FinishContext{fInterface, fDevice, fAcquireSemaphore};
    skgpu::graphite::GpuFinishedProc finishCallback = [](skgpu::graphite::GpuFinishedContext c,
                                                         skgpu::CallbackResult status) {
        // Regardless of the status, we need to destroy the waitSemaphore
        if (status != skgpu::CallbackResult::kSuccess) {
            SKGPU_LOG_W("Recording insertion failed.");
        }
        const auto* context = reinterpret_cast<const FinishContext*>(c);
        VULKAN_CALL(context->fInterface, DestroySemaphore(context->fDevice,
                                                          context->fWaitSemaphore,
                                                          /*pAllocator=*/nullptr));
    };
    info.fFinishedContext = finishContext;
    info.fFinishedProc = finishCallback;

    fGraphiteContext->insertRecording(info);
    fGraphiteContext->submit(skgpu::graphite::SyncToCpu::kNo);
    fAcquireSemaphore = VK_NULL_HANDLE;  // FinishCallback will destroy this
    return true;
}

GraphiteVulkanWindowContext::Backbuffer* GraphiteVulkanWindowContext::getAvailableBackbuffer() {
    SkASSERT(fBackbuffers);

    ++fCurrentBackbufferIndex;
    if (fCurrentBackbufferIndex > fSwapchainImgCount) {
        fCurrentBackbufferIndex = 0;
    }

    return &fBackbuffers[fCurrentBackbufferIndex];
}

sk_sp<SkSurface> GraphiteVulkanWindowContext::getBackbufferSurface() {
    Backbuffer* availableBackbuffer = this->getAvailableBackbuffer();
    SkASSERT(availableBackbuffer);

    // Create a new, unsignaled semaphore to be signaled upon swapchain image acquisition.
    static const VkSemaphoreCreateInfo acquireSemInfo =
            { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, /*pNext=*/nullptr, /*flags=*/0 };
    VkResult result;
    VULKAN_CALL_RESULT_NOCHECK(fInterface, result, CreateSemaphore(fDevice,
                                                                   &acquireSemInfo,
                                                                   /*pAllocator=*/nullptr,
                                                                   &fAcquireSemaphore));
    SkASSERT(fAcquireSemaphore != VK_NULL_HANDLE);
    // Acquire the next available presentable image. Update the backbuffer's image idx accordingly.
    VkResult res = fAcquireNextImageKHR(fDevice,
                                        fSwapchain,
                                        UINT64_MAX,
                                        fAcquireSemaphore,
                                        /*VkFence=*/VK_NULL_HANDLE,
                                        &availableBackbuffer->fImageIndex);
    if (VK_ERROR_SURFACE_LOST_KHR == res) {
        // TODO: Recreate fDeviceSurface using fCreateVkSurfaceFn, and then rebuild the swapchain
        VULKAN_CALL(fInterface,
                    DestroySemaphore(fDevice, fAcquireSemaphore, /*pAllocator=*/nullptr));
        return nullptr;
    }
    if (VK_ERROR_OUT_OF_DATE_KHR == res) {
        SKGPU_LOG_W("Failed to acquire next image for swapchain. Tearing down and trying again.");
        if (!this->createSwapchain(-1, -1)) {
            VULKAN_CALL(fInterface,
                        DestroySemaphore(fDevice, fAcquireSemaphore, /*pAllocator=*/nullptr));
            return nullptr;
        }
        availableBackbuffer = this->getAvailableBackbuffer();

        res = fAcquireNextImageKHR(fDevice,
                                   fSwapchain,
                                   UINT64_MAX,
                                   fAcquireSemaphore,
                                   /*VkFence=*/VK_NULL_HANDLE,
                                   &availableBackbuffer->fImageIndex);
        if (VK_SUCCESS != res) {
            VULKAN_CALL(fInterface,
                        DestroySemaphore(fDevice, fAcquireSemaphore, /*pAllocator=*/nullptr));
            return nullptr;
        }
    }

    // Get the Surface associated with this backbuffer's VkImage.
    SkSurface* surface = fSwapchainImgInfo[availableBackbuffer->fImageIndex].fSurface.get();
    return sk_ref_sp(surface);
}

void GraphiteVulkanWindowContext::onSwapBuffers() {
    // Submit the GPU work associated with the current backbuffer. If submission fails, exit early.
    Backbuffer* backbuffer = fBackbuffers + fCurrentBackbufferIndex;
    if (!this->submitToGpu(backbuffer)) {
        return;
    }

    SwapchainImageInfo* swapchainImgRep = &fSwapchainImgInfo[backbuffer->fImageIndex];

    // Submit present operation to present queue. Use backbuffer's swapchain image representation
    // index for updates. See docs (TODO: nikipre add note)

    // Submit present operation to present queue
    const VkPresentInfoKHR presentInfo = {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                       // sType
            nullptr,                                                  // pNext
            1,                                                        // waitSemaphoreCount
            &swapchainImgRep->fRenderCompletionSemaphore,             // pWaitSemaphores
            1,                                                        // swapchainCount
            &fSwapchain,                                              // pSwapchains
            &backbuffer->fImageIndex,                                 // pImageIndices
            nullptr                                                   // pResults
    };
    fQueuePresentKHR(fPresentQueue, &presentInfo);
}

}  // namespace skwindow::internal
