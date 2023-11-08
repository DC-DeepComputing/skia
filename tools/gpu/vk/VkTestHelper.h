/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkTestHelper_DEFINED
#define VkTestHelper_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_VULKAN

#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/vk/VulkanBackendContext.h"

class GrDirectContext;
class SkSurface;

namespace skgpu::graphite {
    class Context;
}

namespace skiatest {
    enum class TestType : uint8_t;
}

#define DECLARE_VK_PROC(name) PFN_vk##name fVk##name

class VkTestHelper {
public:
    VkTestHelper(skiatest::TestType testType, bool isProtected)
            : fTestType(testType)
            , fIsProtected1(isProtected) {}

    ~VkTestHelper() {
        this->cleanup();
    }

    bool init();

    GrDirectContext* directContext1() { return fDirectContext1.get(); }

    skgpu::graphite::Context* graphiteContext() { return fGraphiteContext.get(); }

private:
    void cleanup();

    DECLARE_VK_PROC(DestroyInstance);
    DECLARE_VK_PROC(DeviceWaitIdle);
    DECLARE_VK_PROC(DestroyDevice);
    DECLARE_VK_PROC(GetDeviceProcAddr);

    DECLARE_VK_PROC(GetPhysicalDeviceFormatProperties);
    DECLARE_VK_PROC(GetPhysicalDeviceMemoryProperties);

    DECLARE_VK_PROC(CreateImage);
    DECLARE_VK_PROC(DestroyImage);
    DECLARE_VK_PROC(GetImageMemoryRequirements);
    DECLARE_VK_PROC(AllocateMemory);
    DECLARE_VK_PROC(FreeMemory);
    DECLARE_VK_PROC(BindImageMemory);
    DECLARE_VK_PROC(MapMemory);
    DECLARE_VK_PROC(UnmapMemory);
    DECLARE_VK_PROC(FlushMappedMemoryRanges);
    DECLARE_VK_PROC(GetImageSubresourceLayout);

    skiatest::TestType fTestType;
    bool fIsProtected1 = false;
    VkDevice fDevice = VK_NULL_HANDLE;

    skgpu::VulkanExtensions fExtensions;
    VkPhysicalDeviceFeatures2 fFeatures = {};
    VkDebugReportCallbackEXT fDebugCallback = VK_NULL_HANDLE;
    PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugCallback = nullptr;
    skgpu::VulkanBackendContext fBackendContext7b;

    sk_sp<GrDirectContext> fDirectContext1;
    std::unique_ptr<skgpu::graphite::Context> fGraphiteContext;
};

#undef DECLARE_VK_PROC

#endif // SK_VULKAN
#endif // VkTestHelper_DEFINED
