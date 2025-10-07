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
     * For readability, define a structure to represent a backbuffer (i.e. frame). Each backbuffer
     * simply contains an index which points to a swapchain image.
     *
     * By default, assign this index to an invalid value. It will be updated whenever
     * vkAcquireNextImageKHR is used to obtain the index of the next available presentable image.
     */
    struct Backbuffer {
        uint32_t fImageIndex = -1;
    };
    Backbuffer* fBackbuffers; /* Array representing frames */
    uint32_t fCurrentBackbufferIndex; /* Current ("in-flight") backbuffer/frame index */
    VkSemaphore fAcquireSemaphore = VK_NULL_HANDLE; /* Semaphore to signal image acquisition */

    /**
     * Determine + return a backbuffer which is ready to be presented.
     */
    Backbuffer* getAvailableBackbuffer();

    /**
     * Define private method to submit work to the GPU (rather than using Window::submitToGpu) in
     * order to properly configure semaphores. Return false upon failure.
     */
    bool submitToGpu(Backbuffer*);

    /**
     * Define a struct to house information pertinent to each swapchain image. VkImages themselves
     * are stored in a separate array - this is because vkGetSwapchainImagesKHR expects the VkImages
     * to be passed in this way.
     *
     * A VkImage's associated SwapchainImageInfo is always located at the same index as the VkImage
     * within fVkImages (meaning a backbuffer's image index is used when indexing into both the
     * VkImage array and the array of image information).
     *
     * Each swapchain image has an image layout while not a color attachment (which we initialize as
     * VK_IMAGE_LAYOUT_UNDEFINED), a semaphore for signaling render completion, and an SkSurface the
     * client renders to. Note that, without the use of VK_EXT_swapchain_maintenance1 which enables
     * using fences with present queues, a semaphore is required for each swapchain image rather
     * than for each backbuffer - see
     * https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html for details.
     */
    struct SwapchainImageInfo {
        VkImageLayout fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkSemaphore fRenderCompletionSemaphore;
        sk_sp<SkSurface> fSurface; /* May not be based on rts */
    };

    uint32_t fSwapchainImgCount;
    VkImage* fVkImages; /* Array of native VkImages - useful for driver calls. */
    SwapchainImageInfo* fSwapchainImgInfo; /* Array of swapchain image representations */

    sk_sp<const skgpu::VulkanInterface> fInterface;

    VkInstance fInstance = VK_NULL_HANDLE;
    VkPhysicalDevice fPhysicalDevice = VK_NULL_HANDLE;
    VkDevice fDevice = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT fDebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR fDeviceSurface;
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
