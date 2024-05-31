/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * TODO(b/309785258) delete this after types are aligned.
 */
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "include/gpu/vk/VulkanMemoryAllocatorUtil.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"
#include "src/gpu/vk/vulkanmemoryallocator/VulkanAMDMemoryAllocator.h"

namespace skgpu::VulkanMemoryAllocators {
sk_sp<VulkanMemoryAllocator> Make(const GrVkBackendContext& oldContext, ThreadSafe threadSafe) {
    SkASSERT(oldContext.fInstance != VK_NULL_HANDLE);
    SkASSERT(oldContext.fPhysicalDevice != VK_NULL_HANDLE);
    SkASSERT(oldContext.fDevice != VK_NULL_HANDLE);
    SkASSERT(oldContext.fQueue != VK_NULL_HANDLE);
    SkASSERT(oldContext.fGetProc);
    SkASSERT(oldContext.fVkExtensions);

    skgpu::VulkanBackendContext backendContext;
    backendContext.fInstance = oldContext.fInstance;
    backendContext.fPhysicalDevice = oldContext.fPhysicalDevice;
    backendContext.fDevice = oldContext.fDevice;
    backendContext.fQueue = oldContext.fQueue;
    backendContext.fGraphicsQueueIndex = oldContext.fGraphicsQueueIndex;
    backendContext.fMaxAPIVersion = oldContext.fMaxAPIVersion;
    backendContext.fVkExtensions = oldContext.fVkExtensions;
    backendContext.fDeviceFeatures = oldContext.fDeviceFeatures;
    backendContext.fDeviceFeatures2 = oldContext.fDeviceFeatures2;
    backendContext.fGetProc = oldContext.fGetProc;
    backendContext.fProtectedContext = oldContext.fProtectedContext;

    uint32_t physDevVersion = 0;
    sk_sp<const skgpu::VulkanInterface> interface =
            skgpu::MakeInterface(backendContext, nullptr, &physDevVersion, nullptr);
    if (!interface) {
        return nullptr;
    }

    return VulkanAMDMemoryAllocator::Make(backendContext.fInstance,
                                          backendContext.fPhysicalDevice,
                                          backendContext.fDevice,
                                          physDevVersion,
                                          backendContext.fVkExtensions,
                                          interface.get(),
                                          threadSafe);
}
}  // namespace skgpu::VulkanMemoryAllocators

namespace skgpu {
sk_sp<skgpu::VulkanInterface> MakeInterface(const GrVkBackendContext& oldContext,
                                            const skgpu::VulkanExtensions* extOverride,
                                            uint32_t* physDevVersionOut,
                                            uint32_t* instanceVersionOut) {
    skgpu::VulkanBackendContext backendContext;
    backendContext.fInstance = oldContext.fInstance;
    backendContext.fPhysicalDevice = oldContext.fPhysicalDevice;
    backendContext.fDevice = oldContext.fDevice;
    backendContext.fQueue = oldContext.fQueue;
    backendContext.fGraphicsQueueIndex = oldContext.fGraphicsQueueIndex;
    backendContext.fMaxAPIVersion = oldContext.fMaxAPIVersion;
    backendContext.fVkExtensions = oldContext.fVkExtensions;
    backendContext.fDeviceFeatures = oldContext.fDeviceFeatures;
    backendContext.fDeviceFeatures2 = oldContext.fDeviceFeatures2;
    backendContext.fGetProc = oldContext.fGetProc;
    backendContext.fProtectedContext = oldContext.fProtectedContext;

    return skgpu::MakeInterface(backendContext, extOverride, physDevVersionOut, instanceVersionOut);
}
}  // namespace skgpu
