/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vulkan_context_helper.h"

#include <dlfcn.h>

#include <algorithm>
#include <vector>

#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "src/gpu/vk/VulkanInterface.h"

class SimpleVulkanMemoryAllocator : public skgpu::VulkanMemoryAllocator {
public:
    SimpleVulkanMemoryAllocator(VkDevice device,
                                const VkPhysicalDeviceMemoryProperties& memProps,
                                sk_sp<const skgpu::VulkanInterface> interface)
            : fDevice(device), fMemProps(memProps), fInterface(std::move(interface)) {}

    VkResult allocateImageMemory(VkImage image,
                                 uint32_t allocationPropertyFlags,
                                 skgpu::VulkanBackendMemory* backendMemory) override {
        VkMemoryRequirements memReqs;
        fInterface->fFunctions.fGetImageMemoryRequirements(fDevice, image, &memReqs);

        VkMemoryPropertyFlags searchFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        if (allocationPropertyFlags & kLazyAllocation_AllocationPropertyFlag) {
            searchFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
        }
        if (allocationPropertyFlags & kProtected_AllocationPropertyFlag) {
            searchFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
        }

        return this->allocateMemory(memReqs, searchFlags, backendMemory);
    }

    VkResult allocateBufferMemory(VkBuffer buffer,
                                  BufferUsage usage,
                                  uint32_t allocationPropertyFlags,
                                  skgpu::VulkanBackendMemory* backendMemory) override {
        VkMemoryRequirements memReqs;
        fInterface->fFunctions.fGetBufferMemoryRequirements(fDevice, buffer, &memReqs);

        VkMemoryPropertyFlags searchFlags = 0;
        if (usage == BufferUsage::kGpuOnly) {
            searchFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        } else if (usage == BufferUsage::kTransfersFromGpuToCpu) {
            searchFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        } else {
            searchFlags =
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }

        if (allocationPropertyFlags & kProtected_AllocationPropertyFlag) {
            searchFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
        }

        return this->allocateMemory(memReqs, searchFlags, backendMemory);
    }

    void freeMemory(const skgpu::VulkanBackendMemory& backendMemory) override {
        Alloc* alloc = reinterpret_cast<Alloc*>(backendMemory);
        fInterface->fFunctions.fFreeMemory(fDevice, alloc->fMemory, nullptr);
        delete alloc;
    }

    void getAllocInfo(const skgpu::VulkanBackendMemory& backendMemory,
                      skgpu::VulkanAlloc* alloc) const override {
        Alloc* a = reinterpret_cast<Alloc*>(backendMemory);
        alloc->fMemory = a->fMemory;
        alloc->fOffset = 0;
        alloc->fSize = a->fSize;
        alloc->fFlags = a->fFlags;
        alloc->fBackendMemory = backendMemory;
    }

    VkResult mapMemory(const skgpu::VulkanBackendMemory& backendMemory, void** data) override {
        Alloc* alloc = reinterpret_cast<Alloc*>(backendMemory);
        return fInterface->fFunctions.fMapMemory(fDevice, alloc->fMemory, 0, alloc->fSize, 0, data);
    }

    void unmapMemory(const skgpu::VulkanBackendMemory& backendMemory) override {
        Alloc* alloc = reinterpret_cast<Alloc*>(backendMemory);
        fInterface->fFunctions.fUnmapMemory(fDevice, alloc->fMemory);
    }

    std::pair<uint64_t, uint64_t> totalAllocatedAndUsedMemory() const override { return {0, 0}; }

private:
    struct Alloc {
        VkDeviceMemory fMemory;
        VkDeviceSize fSize;
        uint32_t fFlags;
    };

    VkResult allocateMemory(VkMemoryRequirements memReqs,
                            VkMemoryPropertyFlags searchFlags,
                            skgpu::VulkanBackendMemory* backendMemory) {
        uint32_t typeIndex = 0;
        bool found = false;
        for (uint32_t i = 0; i < fMemProps.memoryTypeCount; ++i) {
            if ((memReqs.memoryTypeBits & (1 << i)) &&
                (fMemProps.memoryTypes[i].propertyFlags & searchFlags) == searchFlags) {
                typeIndex = i;
                found = true;
                break;
            }
        }

        if (!found) {
            for (uint32_t i = 0; i < fMemProps.memoryTypeCount; ++i) {
                if (memReqs.memoryTypeBits & (1 << i)) {
                    typeIndex = i;
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            return VK_ERROR_OUT_OF_DEVICE_MEMORY;
        }

        VkMemoryAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, memReqs.size, typeIndex};

        VkDeviceMemory memory;
        if (fInterface->fFunctions.fAllocateMemory(fDevice, &allocInfo, nullptr, &memory) !=
            VK_SUCCESS) {
            return VK_ERROR_OUT_OF_DEVICE_MEMORY;
        }

        uint32_t flags = 0;
        if (fMemProps.memoryTypes[typeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            flags |= skgpu::VulkanAlloc::kMappable_Flag;
        }
        if (!(fMemProps.memoryTypes[typeIndex].propertyFlags &
              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            flags |= skgpu::VulkanAlloc::kNoncoherent_Flag;
        }
        if (fMemProps.memoryTypes[typeIndex].propertyFlags &
            VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
            flags |= skgpu::VulkanAlloc::kLazilyAllocated_Flag;
        }

        *backendMemory = reinterpret_cast<skgpu::VulkanBackendMemory>(
                new Alloc{memory, memReqs.size, flags});
        return VK_SUCCESS;
    }

    VkDevice fDevice;
    VkPhysicalDeviceMemoryProperties fMemProps;
    sk_sp<const skgpu::VulkanInterface> fInterface;
};

VulkanContext::~VulkanContext() {
    if (fBackendContext.fDevice != VK_NULL_HANDLE) {
        auto waitIdle = (PFN_vkDeviceWaitIdle)fBackendContext.fGetProc(
                "vkDeviceWaitIdle", fBackendContext.fInstance, fBackendContext.fDevice);
        auto destroyDevice = (PFN_vkDestroyDevice)fBackendContext.fGetProc(
                "vkDestroyDevice", fBackendContext.fInstance, fBackendContext.fDevice);

        if (waitIdle) waitIdle(fBackendContext.fDevice);
        if (destroyDevice) destroyDevice(fBackendContext.fDevice, nullptr);
    }
    if (fBackendContext.fInstance != VK_NULL_HANDLE) {
        auto getInstProc = (PFN_vkGetInstanceProcAddr)dlsym(fLibVulkan, "vkGetInstanceProcAddr");
        if (getInstProc) {
            auto destroyInstance = (PFN_vkDestroyInstance)getInstProc(fBackendContext.fInstance,
                                                                      "vkDestroyInstance");
            if (destroyInstance) destroyInstance(fBackendContext.fInstance, nullptr);
        }
    }
    if (fLibVulkan) dlclose(fLibVulkan);
}

bool CreateVulkanContext(VulkanContext* ctx) {
    ctx->fLibVulkan = dlopen("libvulkan.so.1", RTLD_NOW);
    if (!ctx->fLibVulkan) ctx->fLibVulkan = dlopen("libvulkan.so", RTLD_NOW);
    if (!ctx->fLibVulkan) return false;

    auto getInstProc = (PFN_vkGetInstanceProcAddr)dlsym(ctx->fLibVulkan, "vkGetInstanceProcAddr");
    if (!getInstProc) return false;

    auto get = [&](const char* name, VkInstance inst = VK_NULL_HANDLE) {
        return getInstProc(inst, name);
    };

    uint32_t apiVersion = VK_MAKE_VERSION(1, 1, 0);
    VkInstance instance;
    VkApplicationInfo appInfo = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "skia", 0, "skia", 0, apiVersion};
    VkInstanceCreateInfo instInfo = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &appInfo, 0, nullptr, 0, nullptr};

    if (((PFN_vkCreateInstance)get("vkCreateInstance"))(&instInfo, nullptr, &instance) !=
        VK_SUCCESS) {
        return false;
    }

    auto getProc = [getInstProc](const char* n, VkInstance i, VkDevice d) -> PFN_vkVoidFunction {
        if (d) {
            auto getDevProc = (PFN_vkGetDeviceProcAddr)getInstProc(i, "vkGetDeviceProcAddr");
            return getDevProc(d, n);
        }
        return getInstProc(i, n);
    };

    uint32_t gpuCount;
    ((PFN_vkEnumeratePhysicalDevices)get("vkEnumeratePhysicalDevices", instance))(
            instance, &gpuCount, nullptr);
    if (!gpuCount) return false;

    std::vector<VkPhysicalDevice> devs(gpuCount);
    ((PFN_vkEnumeratePhysicalDevices)get("vkEnumeratePhysicalDevices", instance))(
            instance, &gpuCount, devs.data());
    VkPhysicalDevice physicalDevice = devs[0];

    uint32_t queueCount;
    ((PFN_vkGetPhysicalDeviceQueueFamilyProperties)get("vkGetPhysicalDeviceQueueFamilyProperties",
                                                       instance))(
            physicalDevice, &queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    ((PFN_vkGetPhysicalDeviceQueueFamilyProperties)get("vkGetPhysicalDeviceQueueFamilyProperties",
                                                       instance))(
            physicalDevice, &queueCount, queueProps.data());

    uint32_t graphicsQueueIndex = queueCount;
    for (uint32_t i = 0; i < queueCount; ++i) {
        if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueIndex = i;
            break;
        }
    }
    if (graphicsQueueIndex == queueCount) return false;

    float queuePriority = 0.0f;
    VkDeviceQueueCreateInfo qInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                     nullptr,
                                     0,
                                     graphicsQueueIndex,
                                     1,
                                     &queuePriority};
    VkDeviceCreateInfo devInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1, &qInfo, 0, nullptr, 0, nullptr};

    VkDevice device;
    if (((PFN_vkCreateDevice)get("vkCreateDevice", instance))(
                physicalDevice, &devInfo, nullptr, &device) != VK_SUCCESS) {
        return false;
    }

    VkQueue queue;
    ((PFN_vkGetDeviceQueue)getProc("vkGetDeviceQueue", instance, device))(
            device, graphicsQueueIndex, 0, &queue);

    ctx->fExtensions.init(getProc, instance, physicalDevice, 0, nullptr, 0, nullptr);
    auto interface = sk_make_sp<skgpu::VulkanInterface>(
            getProc, instance, device, apiVersion, apiVersion, &ctx->fExtensions);

    VkPhysicalDeviceMemoryProperties memProps;
    ((PFN_vkGetPhysicalDeviceMemoryProperties)get("vkGetPhysicalDeviceMemoryProperties", instance))(
            physicalDevice, &memProps);

    ctx->fBackendContext.fInstance = instance;
    ctx->fBackendContext.fPhysicalDevice = physicalDevice;
    ctx->fBackendContext.fDevice = device;
    ctx->fBackendContext.fQueue = queue;
    ctx->fBackendContext.fGraphicsQueueIndex = graphicsQueueIndex;
    ctx->fBackendContext.fMaxAPIVersion = apiVersion;
    ctx->fBackendContext.fVkExtensions = &ctx->fExtensions;
    ctx->fBackendContext.fGetProc = getProc;
    ctx->fBackendContext.fMemoryAllocator =
            sk_make_sp<SimpleVulkanMemoryAllocator>(device, memProps, interface);

    return true;
}
