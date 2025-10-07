/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GraphiteVulkanWindowContext_DEFINED
#define GraphiteVulkanWindowContext_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_VULKAN

#include "tools/gpu/vk/VkTestUtils.h"
#include "tools/window/WindowContext.h"
namespace skgpu {
struct VulkanInterface;
}
namespace skwindow::internal {

class GraphiteVulkanWindowContext : public WindowContext {
public:
    ~GraphiteVulkanWindowContext() override;

    sk_sp<SkSurface> getBackbufferSurface() override;

    bool isValid() override { return fDevice != VK_NULL_HANDLE; }

    void resize(int w, int h) override { this->createSwapchain(w, h); }

    void setDisplayParams(std::unique_ptr<const DisplayParams> params) override {
        this->destroyContext();
        fDisplayParams = std::move(params);
        this->initializeContext();
    }

    /* Platform specific function that creates a VkSurfaceKHR for a window */
    using CreateVkSurfaceFn = std::function<VkSurfaceKHR(VkInstance)>;
    /* Platform specific function that determines whether presentation will succeed. */
    using CanPresentFn = sk_gpu_test::CanPresentFn;

    GraphiteVulkanWindowContext(std::unique_ptr<const DisplayParams>,
                                CreateVkSurfaceFn,
                                CanPresentFn,
                                PFN_vkGetInstanceProcAddr);

private:
    void initializeContext();
    void destroyContext();

    bool createSwapchain(int width, int height);
    bool createBuffers(VkFormat format, VkImageUsageFlags, SkColorType colorType, VkSharingMode);
    void destroyBuffers();
    void onSwapBuffers() override;

    /**
     * Define an abstraction around each backbuffer (i.e., frame). Each backbuffer is associated
     * with a swapchain image and will have an associated SkSurface.
     */
    struct BackbufferRepresentation {
        uint32_t fSwapchainImgRepIdx; /* Index of SwapchainImage associated with this backbuffer */
        sk_sp<SkSurface> fSurface; /* Surface client renders to (may not be based on rts) */
    };
    BackbufferRepresentation* getAvailableBackbuffer();
    /**
     * Define private method to submit work to the GPU (rather than using Window::submitToGpu) in
     * order to handle semaphores. Return false upon failure.
     */
    bool submitToGpu(const BackbufferRepresentation&);

    /**
     * Define an abstraction around each swapchain image. Each one has an image layout, a
     * semaphore used to signal rendering completion of the image, and an index that can be used
     * to obtain the associated native VkImage.
     *
     * We store a VkImage index rather than raw VkImages because the Vulkan driver calls expect
     * VkImages to be passed in as an array. Therefore, it is most convenient to store those
     * independently of any abstractions or wrappers. Instead, simply store the index of the VkImage
     * that's associated with a swapchain image.
     *
     * We require a rendering completion semaphore for each swapchain image rather than each
     * backbuffer -  see [TODO:LINK] for details.
     */
    struct SwapchainImageRepresentation {
        // uint32_t fVkImageIndex; /* Index to use for getting VkImage fom fSwapchainVkImages */
        VkImageLayout fImageLayout; /* Layout of image when not color attachment */
        VkSemaphore fRenderingCompleteSemaphore;
    };

    uint32_t fSwapchainImgCount;
    SwapchainImageRepresentation* fSwapchainImgReps; /* Array of swapchain image representations */
    BackbufferRepresentation* fBackbufferReps; /* Array of backbuffer/frame representations */
    uint32_t fBackbufferIndex; /* In-flight backbuffer/frame index */
    sk_sp<const skgpu::VulkanInterface> fInterface;

    /* Native Vulkan objects + info */
    VkSemaphore fAcquireSemaphore = VK_NULL_HANDLE; /* Semaphore for signaling image acquisition */
    VkImage* fSwapchainVkImages; /* Array of native VkImages. Useful for driver calls. */

    VkInstance fInstance = VK_NULL_HANDLE;
    VkPhysicalDevice fPhysicalDevice = VK_NULL_HANDLE;
    VkDevice fDevice = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT fDebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR fSurface;
    VkSwapchainKHR fSwapchain;
    uint32_t fGraphicsQueueIndex;
    VkQueue fGraphicsQueue;
    uint32_t fPresentQueueIndex;
    VkQueue fPresentQueue;

    /* Create functions */
    CreateVkSurfaceFn fCreateVkSurfaceFn;
    CanPresentFn fCanPresentFn;
    PFN_vkGetInstanceProcAddr fGetInstanceProcAddr = nullptr;

    /* WSI interface functions */
    PFN_vkDestroySurfaceKHR fDestroySurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR fGetPhysicalDeviceSurfaceSupportKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fGetPhysicalDeviceSurfaceCapabilitiesKHR =
            nullptr;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fGetPhysicalDeviceSurfacePresentModesKHR =
            nullptr;
    PFN_vkCreateSwapchainKHR fCreateSwapchainKHR = nullptr;
    PFN_vkDestroySwapchainKHR fDestroySwapchainKHR = nullptr;
    PFN_vkGetSwapchainImagesKHR fGetSwapchainImagesKHR = nullptr;
    PFN_vkAcquireNextImageKHR fAcquireNextImageKHR = nullptr;
    PFN_vkQueuePresentKHR fQueuePresentKHR = nullptr;

    PFN_vkDestroyInstance fDestroyInstance = nullptr;
    PFN_vkDeviceWaitIdle fDeviceWaitIdle = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT fDestroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkQueueWaitIdle fQueueWaitIdle = nullptr;
    PFN_vkDestroyDevice fDestroyDevice = nullptr;
    PFN_vkGetDeviceQueue fGetDeviceQueue = nullptr;
};

}  // namespace skwindow::internal

#endif  // SK_VULKAN

#endif  // GraphiteVulkanWindowContext_DEFINED
