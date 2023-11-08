/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/vk/VkTestHelper.h"

#ifdef SK_VULKAN

#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "tools/gpu/vk/VkTestUtils.h"

#define ACQUIRE_INST_VK_PROC(name)                                                               \
    fVk##name = reinterpret_cast<PFN_vk##name>(instProc(fBackendContext1.fInstance, "vk" #name)); \
    if (fVk##name == nullptr) {                                                                  \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);                        \
        return false;                                                                            \
    }

#define ACQUIRE_DEVICE_VK_PROC(name)                                                          \
    fVk##name = reinterpret_cast<PFN_vk##name>(fVkGetDeviceProcAddr(fDevice, "vk" #name));    \
    if (fVk##name == nullptr) {                                                               \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);                     \
        return false;                                                                         \
    }

bool VkTestHelper::init() {
    PFN_vkGetInstanceProcAddr instProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
        return false;
    }

    fFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    fFeatures.pNext = nullptr;

    fBackendContext1.fInstance = VK_NULL_HANDLE;
    fBackendContext1.fDevice = VK_NULL_HANDLE;

    if (!sk_gpu_test::CreateVkBackendContext(instProc, &fBackendContext1, &fExtensions,
                                             &fFeatures, &fDebugCallback, nullptr,
                                             sk_gpu_test::CanPresentFn(), fIsProtected)) {
        return false;
    }
    fDevice = fBackendContext1.fDevice;

    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                instProc(fBackendContext1.fInstance, "vkDestroyDebugReportCallbackEXT"));
    }
    ACQUIRE_INST_VK_PROC(DestroyInstance)
    ACQUIRE_INST_VK_PROC(DeviceWaitIdle)
    ACQUIRE_INST_VK_PROC(DestroyDevice)

    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceFormatProperties)
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceMemoryProperties)

    ACQUIRE_INST_VK_PROC(GetDeviceProcAddr)

    ACQUIRE_DEVICE_VK_PROC(CreateImage)
    ACQUIRE_DEVICE_VK_PROC(DestroyImage)
    ACQUIRE_DEVICE_VK_PROC(GetImageMemoryRequirements)
    ACQUIRE_DEVICE_VK_PROC(AllocateMemory)
    ACQUIRE_DEVICE_VK_PROC(FreeMemory)
    ACQUIRE_DEVICE_VK_PROC(BindImageMemory)
    ACQUIRE_DEVICE_VK_PROC(MapMemory)
    ACQUIRE_DEVICE_VK_PROC(UnmapMemory)
    ACQUIRE_DEVICE_VK_PROC(FlushMappedMemoryRanges)
    ACQUIRE_DEVICE_VK_PROC(GetImageSubresourceLayout)

    GrVkBackendContext gr;
    sk_gpu_test::ConvertBackendContext(fBackendContext1, &gr);
    fDirectContext = GrDirectContexts::MakeVulkan(gr);
    if (!fDirectContext) {
        return false;
    }

    SkASSERT(fDirectContext->supportsProtectedContent() == fIsProtected);
    return true;
}

void VkTestHelper::cleanup() {
    // Make sure any work, release procs, etc left on the context are finished with before we start
    // tearing everything down.
    if (fDirectContext) {
        fDirectContext->flushAndSubmit(GrSyncCpu::kYes);
    }

    fDirectContext.reset();

    fBackendContext1.fMemoryAllocator.reset();
    if (fDevice != VK_NULL_HANDLE) {
        fVkDeviceWaitIdle(fDevice);
        fVkDestroyDevice(fDevice, nullptr);
        fDevice = VK_NULL_HANDLE;
    }
    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback(fBackendContext1.fInstance, fDebugCallback, nullptr);
    }

    if (fBackendContext1.fInstance != VK_NULL_HANDLE) {
        fVkDestroyInstance(fBackendContext1.fInstance, nullptr);
        fBackendContext1.fInstance = VK_NULL_HANDLE;
    }

    sk_gpu_test::FreeVulkanFeaturesStructs(&fFeatures);
}

#endif // SK_VULKAN
