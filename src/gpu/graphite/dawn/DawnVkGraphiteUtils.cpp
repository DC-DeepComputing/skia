/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/dawn/DawnVkQueueManager.h"
#include "src/gpu/graphite/dawn/DawnVkSharedContext.h"

#include "tools/library/LoadDynamicLibrary.h"

#ifndef SK_GPU_TOOLS_VK_LIBRARY_NAME_2
    #if defined _WIN32
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME_2 vulkan-1.dll
    #elif defined SK_BUILD_FOR_MAC
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME_2 libvk_swiftshader.dylib
    #else
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME_2        libvulkan.so
        //#define SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP_2 libvulkan.so.1
    #endif
#endif

#define STRINGIFY2a(S) #S
#define STRINGIFYa(S) STRINGIFY2a(S)

namespace {
bool LoadVkLibraryAndGetProcAddrFuncsLocal(PFN_vkGetInstanceProcAddr* instProc) {
    static void* vkLib = nullptr;
    static PFN_vkGetInstanceProcAddr localInstProc = nullptr;
    if (!vkLib) {
        vkLib = SkLoadDynamicLibrary(STRINGIFYa(SK_GPU_TOOLS_VK_LIBRARY_NAME_2));
        if (!vkLib) {
            // vulkaninfo tries to load the library from two places, so we do as well
            // https://github.com/KhronosGroup/Vulkan-Tools/blob/078d44e4664b7efa0b6c96ebced1995c4425d57a/vulkaninfo/vulkaninfo.h#L249
#ifdef SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP_2
            vkLib = SkLoadDynamicLibrary(STRINGIFY(SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP_2));
            if (!vkLib) {
                return false;
            }
#else
            return false;
#endif
        }
        localInstProc = (PFN_vkGetInstanceProcAddr) SkGetProcedureAddress(vkLib,
                                                                          "vkGetInstanceProcAddr");
    }
    if (!localInstProc) {
        return false;
    }
    *instProc = localInstProc;
    return true;
}
} // namespace: anonymous

namespace skgpu::graphite {

#define ACQUIRE_VK_INST_PROC_NOCHECK(name, instance) \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name))

std::unique_ptr<Context> MakeVulkanDawn(const DawnBackendContext& backendContext,
                                        const ContextOptions& options) {

    //id<MTLDevice> metalDevice = (__bridge id<MTLDevice>)backendContext.fDevice.GetMetalDeviceId();
    //id<MTLCommandQueue> metalQueue = (__bridge id<MTLCommandQueue>)backendContext.fDevice.GetMetalQueueId();


    VulkanBackendContext vkBEContext;
    vkBEContext.fInstance = reinterpret_cast<VkInstance>(backendContext.fDevice.GetVulkanInstance());
    vkBEContext.fPhysicalDevice = reinterpret_cast<VkPhysicalDevice>(backendContext.fDevice.GetVulkanPhysicalDevice());
    vkBEContext.fDevice = reinterpret_cast<VkDevice>(backendContext.fDevice.GetVulkanDevice());
    vkBEContext.fQueue = reinterpret_cast<VkQueue>(backendContext.fDevice.GetVulkanQueue());
    vkBEContext.fGraphicsQueueIndex = backendContext.fDevice.GetVulkanQueueIndex();
    vkBEContext.fMaxAPIVersion = VK_API_VERSION_1_1;
    vkBEContext.fVkExtensions = nullptr;
    vkBEContext.fDeviceFeatures2 = nullptr;

    PFN_vkGetInstanceProcAddr getInstProc;

    if (!LoadVkLibraryAndGetProcAddrFuncsLocal(&getInstProc)) {
        return nullptr;
    }

    ACQUIRE_VK_INST_PROC_NOCHECK(GetDeviceProcAddr, vkBEContext.fInstance);
    if (grVkGetDeviceProcAddr == nullptr) {
        SkDebugf("failed to get fn ptr for getDeviceProcAddr\n");
        return nullptr;
    }
    auto getProc = [getInstProc, grVkGetDeviceProcAddr](const char* proc_name,
                                                        VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return grVkGetDeviceProcAddr(device, proc_name);
        }
        return getInstProc(instance, proc_name);
    };

    vkBEContext.fGetProc = getProc;

    sk_sp<SharedContext> sharedContext = DawnVkSharedContext::Make(vkBEContext, backendContext,
                                                                   options);
    if (!sharedContext) {
        return nullptr;
    }

    auto queueManager =
            std::make_unique<DawnVkQueueManager>(vkBEContext.fQueue, sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    return ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                            std::move(queueManager),
                                            options);
}

}  // namespace skgpu::graphite
