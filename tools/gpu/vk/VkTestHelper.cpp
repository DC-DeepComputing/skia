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
#include "tests/Test.h"
#include "tools/gpu/vk/VkTestUtils.h"

#include "include/gpu/graphite/vk/VulkanGraphiteUtils.h"

#define ACQUIRE_INST_VK_PROC(name)                                                               \
    fVk##name = reinterpret_cast<PFN_vk##name>(instProc(fBackendContext7b.fInstance, "vk" #name)); \
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

std::unique_ptr<VkTestHelper> VkTestHelper::Make(skiatest::TestType testType, bool isProtected) {
    std::unique_ptr<VkTestHelper> helper;

    if (testType == skiatest::TestType::kGanesh) {
        helper = std::make_unique<VkTestHelper>(/* isProtected= */ false, true);
    }

    if (!helper->init()) {
        return nullptr;
    }

    return helper;
}

bool VkTestHelper::init() {
    PFN_vkGetInstanceProcAddr instProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
        return false;
    }

    fFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    fFeatures.pNext = nullptr;

    fBackendContext7b.fInstance = VK_NULL_HANDLE;
    fBackendContext7b.fDevice = VK_NULL_HANDLE;

    if (!sk_gpu_test::CreateVkBackendContext(instProc, &fBackendContext7b, &fExtensions,
                                             &fFeatures, &fDebugCallback, nullptr,
                                             sk_gpu_test::CanPresentFn(), fIsProtected1)) {
        return false;
    }
    fDevice = fBackendContext7b.fDevice;

    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                instProc(fBackendContext7b.fInstance, "vkDestroyDebugReportCallbackEXT"));
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
    sk_gpu_test::ConvertBackendContext(fBackendContext7b, &gr);
    fDirectContext1 = GrDirectContexts::MakeVulkan(gr);
    if (!fDirectContext1) {
        return false;
    }

    SkASSERT(fDirectContext1->supportsProtectedContent() == fIsProtected1);

    skgpu::graphite::ContextOptions contextOptions;

    fGraphiteContext = skgpu::graphite::ContextFactory::MakeVulkan2(fBackendContext7b, contextOptions); /*$$$*/

    return true;
}

void VkTestHelper::cleanup() {
    // Make sure any work, release procs, etc left on the context are finished with before we start
    // tearing everything down.
    if (fDirectContext1) {
        fDirectContext1->flushAndSubmit(GrSyncCpu::kYes);
        fDirectContext1.reset();
    }

    if (fGraphiteContext) {
        fGraphiteContext.reset();
    }

    fBackendContext7b.fMemoryAllocator.reset();
    if (fDevice != VK_NULL_HANDLE) {
        fVkDeviceWaitIdle(fDevice);
        fVkDestroyDevice(fDevice, nullptr);
        fDevice = VK_NULL_HANDLE;
    }
    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback(fBackendContext7b.fInstance, fDebugCallback, nullptr);
    }

    if (fBackendContext7b.fInstance != VK_NULL_HANDLE) {
        fVkDestroyInstance(fBackendContext7b.fInstance, nullptr);
        fBackendContext7b.fInstance = VK_NULL_HANDLE;
    }

    sk_gpu_test::FreeVulkanFeaturesStructs(&fFeatures);
}

#endif // SK_VULKAN
