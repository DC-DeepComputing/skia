/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkTestUtils_DEFINED
#define VkTestUtils_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_VULKAN

#include "include/gpu/vk/VulkanBackendContext.h"
#include "tools/gpu/vk/GrVulkanDefines.h"
#include <functional>

namespace skgpu {
struct VulkanBackendContext;
class VulkanExtensions;
}

namespace sk_gpu_test {
    struct TestVkFeatures {
        VkPhysicalDeviceFeatures2 features;

        // Note: Any struct that CreateVkBackendContext may chain to VkPhysicalDeviceFeatures2
        // should remain in scope together with the physical device features.
        skgpu::VulkanPreferredFeatures skiaFeatures;
        VkPhysicalDeviceProtectedMemoryFeatures protectedMemoryFeatures;
    };

    bool LoadVkLibraryAndGetProcAddrFuncs(PFN_vkGetInstanceProcAddr*);

    using CanPresentFn = std::function<bool(VkInstance, VkPhysicalDevice,
                                            uint32_t queueFamilyIndex)>;

    bool CreateVkBackendContext(PFN_vkGetInstanceProcAddr getInstProc,
                                skgpu::VulkanBackendContext* ctx,
                                skgpu::VulkanExtensions*,
                                TestVkFeatures*,
                                VkDebugUtilsMessengerEXT* debugMessenger,
                                uint32_t* presentQueueIndexPtr = nullptr,
                                const CanPresentFn& canPresent = CanPresentFn(),
                                bool isProtected = false);

}  // namespace sk_gpu_test

#endif
#endif
