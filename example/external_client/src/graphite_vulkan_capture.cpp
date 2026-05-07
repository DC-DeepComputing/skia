/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <dlfcn.h>
#include <vulkan/vulkan.h>
#include <cstdio>
#include <vector>

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/vk/VulkanGraphiteContext.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "include/gpu/vk/VulkanPreferredFeatures.h"
#include "src/base/SkAutoMalloc.h"
#include "src/capture/SkCapture.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/vk/VulkanInterface.h"

#define WIDTH 200
#define HEIGHT 400

#define ACQUIRE_VK_INST_PROC_LOCAL(name, instance)                             \
    PFN_vk##name grVk##name =                                                  \
            reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name)); \
    if (!grVk##name) {                                                         \
        return false;                                                          \
    }

#define ACQUIRE_VK_INST_PROC(name, instance)                                   \
    PFN_vk##name grVk##name =                                                  \
            reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name)); \
    if (grVk##name == nullptr) {                                               \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);      \
        if (inst != VK_NULL_HANDLE) {                                          \
            destroy_instance(getInstProc, inst);                               \
        }                                                                      \
        return false;                                                          \
    }

#define ACQUIRE_VK_PROC(name, instance, device)                               \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(                 \
            getProc("vk" #name, instance, device));                           \
    do {                                                                      \
        if (grVk##name == nullptr) {                                          \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name); \
            if (inst != VK_NULL_HANDLE) {                                     \
                destroy_instance(getInstProc, inst);                          \
            }                                                                 \
            return false;                                                     \
        }                                                                     \
    } while (0)

static bool destroy_instance(PFN_vkGetInstanceProcAddr getInstProc,
                             VkInstance inst) {
    ACQUIRE_VK_INST_PROC_LOCAL(DestroyInstance, inst);
    grVkDestroyInstance(inst, nullptr);
    return true;
}

class SimpleVulkanMemoryAllocator : public skgpu::VulkanMemoryAllocator {
public:
    SimpleVulkanMemoryAllocator(
            VkDevice device,
            const VkPhysicalDeviceMemoryProperties& memProps,
            const skgpu::VulkanGetProc& getProc)
            : fDevice(device), fMemProps(memProps) {
        fAllocateMemory = (PFN_vkAllocateMemory)getProc(
                "vkAllocateMemory", VK_NULL_HANDLE, device);
        fFreeMemory = (PFN_vkFreeMemory)getProc(
                "vkFreeMemory", VK_NULL_HANDLE, device);
        fMapMemory =
                (PFN_vkMapMemory)getProc("vkMapMemory", VK_NULL_HANDLE, device);
        fUnmapMemory = (PFN_vkUnmapMemory)getProc(
                "vkUnmapMemory", VK_NULL_HANDLE, device);
        fFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)getProc(
                "vkFlushMappedMemoryRanges", VK_NULL_HANDLE, device);
        fInvalidateMappedMemoryRanges =
                (PFN_vkInvalidateMappedMemoryRanges)getProc(
                        "vkInvalidateMappedMemoryRanges",
                        VK_NULL_HANDLE,
                        device);
        fGetBufferMemoryRequirements =
                (PFN_vkGetBufferMemoryRequirements)getProc(
                        "vkGetBufferMemoryRequirements",
                        VK_NULL_HANDLE,
                        device);
        fGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)getProc(
                "vkGetImageMemoryRequirements", VK_NULL_HANDLE, device);
    }

    ~SimpleVulkanMemoryAllocator() override {}

    VkResult allocateImageMemory(
            VkImage image,
            uint32_t allocationPropertyFlags,
            skgpu::VulkanBackendMemory* backendMemory) override {
        VkMemoryRequirements memReqs;
        fGetImageMemoryRequirements(fDevice, image, &memReqs);

        VkMemoryPropertyFlags searchFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        if (allocationPropertyFlags & kLazyAllocation_AllocationPropertyFlag) {
            searchFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
        }
        if (allocationPropertyFlags & kProtected_AllocationPropertyFlag) {
            searchFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
        }

        return this->allocateMemory(
                memReqs, searchFlags, allocationPropertyFlags, backendMemory);
    }

    VkResult allocateBufferMemory(
            VkBuffer buffer,
            BufferUsage usage,
            uint32_t allocationPropertyFlags,
            skgpu::VulkanBackendMemory* backendMemory) override {
        VkMemoryRequirements memReqs;
        fGetBufferMemoryRequirements(fDevice, buffer, &memReqs);

        VkMemoryPropertyFlags searchFlags = 0;
        switch (usage) {
            case BufferUsage::kGpuOnly:
                searchFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                break;
            case BufferUsage::kCpuWritesGpuReads:
            case BufferUsage::kTransfersFromCpuToGpu:
                searchFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                break;
            case BufferUsage::kTransfersFromGpuToCpu:
                searchFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
                break;
        }
        if (allocationPropertyFlags & kProtected_AllocationPropertyFlag) {
            searchFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
        }

        return this->allocateMemory(
                memReqs, searchFlags, allocationPropertyFlags, backendMemory);
    }

    void freeMemory(const skgpu::VulkanBackendMemory& backendMemory) override {
        Alloc* alloc = reinterpret_cast<Alloc*>(backendMemory);
        fFreeMemory(fDevice, alloc->fMemory, nullptr);
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

    VkResult mapMemory(const skgpu::VulkanBackendMemory& backendMemory,
                       void** data) override {
        Alloc* alloc = reinterpret_cast<Alloc*>(backendMemory);
        return fMapMemory(fDevice, alloc->fMemory, 0, alloc->fSize, 0, data);
    }

    void unmapMemory(const skgpu::VulkanBackendMemory& backendMemory) override {
        Alloc* alloc = reinterpret_cast<Alloc*>(backendMemory);
        fUnmapMemory(fDevice, alloc->fMemory);
    }

    VkResult flushMemory(const skgpu::VulkanBackendMemory& backendMemory,
                         VkDeviceSize offset,
                         VkDeviceSize size) override {
        Alloc* alloc = reinterpret_cast<Alloc*>(backendMemory);
        VkMappedMemoryRange range = {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                                     nullptr,
                                     alloc->fMemory,
                                     offset,
                                     size};
        return fFlushMappedMemoryRanges(fDevice, 1, &range);
    }

    VkResult invalidateMemory(const skgpu::VulkanBackendMemory& backendMemory,
                              VkDeviceSize offset,
                              VkDeviceSize size) override {
        Alloc* alloc = reinterpret_cast<Alloc*>(backendMemory);
        VkMappedMemoryRange range = {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                                     nullptr,
                                     alloc->fMemory,
                                     offset,
                                     size};
        return fInvalidateMappedMemoryRanges(fDevice, 1, &range);
    }

    std::pair<uint64_t, uint64_t> totalAllocatedAndUsedMemory() const override {
        return {0, 0};
    }

private:
    struct Alloc {
        VkDeviceMemory fMemory;
        VkDeviceSize fSize;
        uint32_t fFlags;
    };

    VkResult allocateMemory(VkMemoryRequirements memReqs,
                            VkMemoryPropertyFlags searchFlags,
                            uint32_t allocationPropertyFlags,
                            skgpu::VulkanBackendMemory* backendMemory) {
        uint32_t typeIndex = 0;
        bool found = false;
        for (uint32_t i = 0; i < fMemProps.memoryTypeCount; ++i) {
            if ((memReqs.memoryTypeBits & (1 << i)) &&
                (fMemProps.memoryTypes[i].propertyFlags & searchFlags) ==
                        searchFlags) {
                typeIndex = i;
                found = true;
                break;
            }
        }

        if (!found) {
            // Fallback to just required searchFlags (minimal set)
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
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                nullptr,
                memReqs.size,
                typeIndex};

        VkDeviceMemory memory;
        VkResult result =
                fAllocateMemory(fDevice, &allocInfo, nullptr, &memory);
        if (result != VK_SUCCESS) {
            return result;
        }

        Alloc* alloc = new Alloc();
        alloc->fMemory = memory;
        alloc->fSize = memReqs.size;
        alloc->fFlags = 0;
        if (fMemProps.memoryTypes[typeIndex].propertyFlags &
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            alloc->fFlags |= skgpu::VulkanAlloc::kMappable_Flag;
        }
        if (!(fMemProps.memoryTypes[typeIndex].propertyFlags &
              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            alloc->fFlags |= skgpu::VulkanAlloc::kNoncoherent_Flag;
        }
        if (fMemProps.memoryTypes[typeIndex].propertyFlags &
            VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
            alloc->fFlags |= skgpu::VulkanAlloc::kLazilyAllocated_Flag;
        }

        *backendMemory = reinterpret_cast<skgpu::VulkanBackendMemory>(alloc);
        return VK_SUCCESS;
    }

    VkDevice fDevice;
    VkPhysicalDeviceMemoryProperties fMemProps;

    PFN_vkAllocateMemory fAllocateMemory;
    PFN_vkFreeMemory fFreeMemory;
    PFN_vkMapMemory fMapMemory;
    PFN_vkUnmapMemory fUnmapMemory;
    PFN_vkFlushMappedMemoryRanges fFlushMappedMemoryRanges;
    PFN_vkInvalidateMappedMemoryRanges fInvalidateMappedMemoryRanges;
    PFN_vkGetBufferMemoryRequirements fGetBufferMemoryRequirements;
    PFN_vkGetImageMemoryRequirements fGetImageMemoryRequirements;
};

bool CreateVkBackendContext(PFN_vkGetInstanceProcAddr getInstProc,
                            skgpu::VulkanBackendContext* backendContext,
                            skgpu::VulkanExtensions* extensions,
                            VkPhysicalDeviceFeatures2* features) {
    VkResult err;

    PFN_vkEnumerateInstanceVersion grVkEnumerateInstanceVersion =
            reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
                    getInstProc(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"));

    uint32_t instanceVersion = 0;

    if (!grVkEnumerateInstanceVersion) {
        instanceVersion = VK_MAKE_VERSION(1, 0, 0);
    } else {
        err = grVkEnumerateInstanceVersion(&instanceVersion);
        if (err) {
            SkDEBUGF("vkEnumerateInstanceVersion failed: %d\n", err);
            return false;
        }
    }

    SkASSERT(instanceVersion >= VK_MAKE_VERSION(1, 0, 0));

    uint32_t apiVersion = VK_MAKE_VERSION(1, 0, 0);
    if (instanceVersion >= VK_MAKE_VERSION(1, 1, 0)) {
        apiVersion = VK_MAKE_VERSION(1, 1, 0);
    }

    instanceVersion = std::min(instanceVersion, apiVersion);

    std::vector<VkPhysicalDevice> physDevs;
    VkDevice device;

    VkInstance inst = VK_NULL_HANDLE;

    const VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "graphite_vulkan_capture",
            .applicationVersion = 0,
            .pEngineName = "graphite_vulkan_capture",
            .engineVersion = 0,
            .apiVersion = apiVersion,
    };

    const VkInstanceCreateInfo instance_create_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = 0,
            .ppEnabledExtensionNames = nullptr,
    };

    ACQUIRE_VK_INST_PROC(CreateInstance, VK_NULL_HANDLE);

    err = grVkCreateInstance(&instance_create_info, nullptr, &inst);
    if (err < 0) {
        return false;
    }

    ACQUIRE_VK_INST_PROC(GetDeviceProcAddr, inst);
    auto getProc = [getInstProc, grVkGetDeviceProcAddr](const char* proc_name,
                                                        VkInstance instance,
                                                        VkDevice device) {
        if (device) {
            return grVkGetDeviceProcAddr(device, proc_name);
        }

        return getInstProc(instance, proc_name);
    };

    ACQUIRE_VK_PROC(EnumeratePhysicalDevices, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceProperties, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(
            GetPhysicalDeviceQueueFamilyProperties, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceFeatures, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceMemoryProperties, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(CreateDevice, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetDeviceQueue, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(DeviceWaitIdle, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(DestroyDevice, inst, VK_NULL_HANDLE);

    uint32_t gpuCount;

    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, nullptr);
    if (err) {
        SkDEBUGF("vkEnumeratePhysicalDevices failed: %d\n", err);
        destroy_instance(getInstProc, inst);
        return false;
    }

    if (!gpuCount) {
        SkDebugf("vkEnumeratePhysicalDevices returned no supported devices");
        destroy_instance(getInstProc, inst);
        return false;
    }

    physDevs.resize(gpuCount);
    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, physDevs.data());

    if (err) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", err);
        destroy_instance(getInstProc, inst);
        return false;
    }

    VkPhysicalDevice physDev = physDevs.front();

    VkPhysicalDeviceProperties physDeviceProperties;

    grVkGetPhysicalDeviceProperties(physDev, &physDeviceProperties);
    uint32_t physDeviceVersion =
            std::min(physDeviceProperties.apiVersion, apiVersion);

    uint32_t queueCount;
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);

    if (!queueCount) {
        SkDebugf(
                "vkGetPhysicalDeviceQueueFamilyProperties returned no "
                "supported queue families");
        destroy_instance(getInstProc, inst);
        return false;
    }

    SkAutoMalloc queuePropsAlloc(queueCount * sizeof(VkQueueFamilyProperties));

    VkQueueFamilyProperties* queueProps =
            static_cast<VkQueueFamilyProperties*>(queuePropsAlloc.get());

    grVkGetPhysicalDeviceQueueFamilyProperties(
            physDev, &queueCount, queueProps);

    uint32_t graphicsQueueIndex = queueCount;
    for (uint32_t i = 0; i < queueCount; ++i) {
        if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueIndex = i;
            break;
        }
    }

    if (graphicsQueueIndex == queueCount) {
        destroy_instance(getInstProc, inst);
        return false;
    }

    float queuePriorities[1] = {0.0};

    const VkDeviceQueueCreateInfo queueInfo[1] = {{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = graphicsQueueIndex,
            .queueCount = 1,
            .pQueuePriorities = queuePriorities,
    }};

    uint32_t queueInfoCount = 1;

    const VkDeviceCreateInfo deviceInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = features,
            .flags = 0,
            .queueCreateInfoCount = queueInfoCount,
            .pQueueCreateInfos = queueInfo,
    };

    err = grVkCreateDevice(physDev, &deviceInfo, nullptr, &device);
    if (err) {
        SkDebugf("vkCreateDevice failed: %d\n", err);
        destroy_instance(getInstProc, inst);
        return false;
    }

    VkQueue queue;
    grVkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

    extensions->init(getProc, inst, physDev, 0, nullptr, 0, nullptr);

    skgpu::VulkanInterface interface = skgpu::VulkanInterface(getProc,
                                                              inst,
                                                              device,
                                                              instanceVersion,
                                                              physDeviceVersion,
                                                              extensions);

    SkASSERT(
            interface.validate(instanceVersion, physDeviceVersion, extensions));

    VkPhysicalDeviceMemoryProperties memProps;
    grVkGetPhysicalDeviceMemoryProperties(physDev, &memProps);

    backendContext->fInstance = inst;
    backendContext->fPhysicalDevice = physDev;
    backendContext->fDevice = device;
    backendContext->fQueue = queue;
    backendContext->fGraphicsQueueIndex = graphicsQueueIndex;
    backendContext->fMaxAPIVersion = apiVersion;
    backendContext->fGetProc = getProc;
    backendContext->fVkExtensions = extensions;
    backendContext->fDeviceFeatures2 = features;

    backendContext->fMemoryAllocator = sk_sp<SimpleVulkanMemoryAllocator>(
            new SimpleVulkanMemoryAllocator(device, memProps, getProc));

    return backendContext->fMemoryAllocator != nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Specify a single write location.\n");
        return 1;
    }

    SkFILEWStream output(argv[1]);
    if (!output.isValid()) {
        printf("Cannot open output file %s\n", argv[1]);
        return 1;
    }

    /* LOAD VULKAN LOADER */
    void* libvulkan = dlopen("libvulkan.so.1", RTLD_NOW);
    if (!libvulkan) {
        libvulkan = dlopen("libvulkan.so", RTLD_NOW);
    }
    if (!libvulkan) {
        printf("Could not load libvulkan\n");
        return 1;
    }
    PFN_vkGetInstanceProcAddr getInstProc = (PFN_vkGetInstanceProcAddr)dlsym(
            libvulkan, "vkGetInstanceProcAddr");
    if (!getInstProc) {
        printf("Could not find vkGetInstanceProcAddr\n");
        return 1;
    }

    /* SET UP CONTEXT AND RECORDERS FOR DRAWING AND CAPTURE */

    skgpu::VulkanBackendContext backendContext;
    skgpu::VulkanExtensions extensions;
    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    if (!CreateVkBackendContext(
                getInstProc, &backendContext, &extensions, &features)) {
        printf("Could not create Vulkan backend context\n");
        return 1;
    }

    skgpu::graphite::ContextOptions options;
    options.fEnableCapture = true;

    std::unique_ptr<skgpu::graphite::Context> context =
            skgpu::graphite::ContextFactory::MakeVulkan(backendContext,
                                                        options);

    if (!context) {
        printf("Could not make Graphite Native Vulkan context (Vulkan not "
               "initialized)\n");
        return 1;
    }
    printf("Context made, now to make the surface\n");

    SkImageInfo imageInfo = SkImageInfo::Make(
            WIDTH, HEIGHT, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    std::unique_ptr<skgpu::graphite::Recorder> recorder =
            context->makeRecorder();
    if (!recorder) {
        printf("Could not make recorder\n");
        return 1;
    }
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(recorder.get(), imageInfo);
    if (!surface) {
        printf("Could not make surface from Vulkan Recorder\n");
        return 1;
    }

    sk_sp<SkSurface> surfaceB =
            SkSurfaces::RenderTarget(recorder.get(), imageInfo);
    if (!surfaceB) {
        printf("Could not make second surface from Vulkan Recorder\n");
        return 1;
    }

    /* DRAWING */
    context->startCapture();
    printf("Capture started, now to draw\n");

    SkCanvas* canvas = surface->getCanvas();
    SkCanvas* canvasB = surfaceB->getCanvas();

    // Canvas A
    canvas->clear(SK_ColorCYAN);
    SkRRect rrect =
            SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);

    canvas->drawRRect(rrect, paint);

    // Triggers a breakpoint in the capture
    auto contentImg = surface->makeImageSnapshot();
    canvas->drawCircle(50, 50, 30, paint);
    auto anotherContentImg = surface->makeImageSnapshot();
    paint.setColor(SK_ColorBLUE);
    canvas->drawCircle(100, 50, 30, paint);

    // Canvas B
    canvasB->clear(SK_ColorMAGENTA);
    paint.setColor(SK_ColorBLACK);
    canvasB->drawImage(contentImg, 0, 0);
    canvasB->drawCircle(10, 10, 5, paint);
    canvasB->drawImage(anotherContentImg, 0, 0);

    printf("ready to snap the GPU calls\n");
    // Now to send the draws to the GPU
    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    if (!recording) {
        printf("Could not create a recording\n");
        return 1;
    }
    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording.get();
    if (!context->insertRecording(info)) {
        printf("Context::insertRecording failed\n");
        return 1;
    }

    sk_sp<SkImage> img;
    auto callback = [](SkImage::ReadPixelsContext ctx,
                       std::unique_ptr<const SkImage::AsyncReadResult> result) {
        if (!result || result->count() != 1) {
            printf("Didn't load exactly one plane\n");
            return;
        }
        auto ii = SkImageInfo::Make(
                WIDTH, HEIGHT, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkImage>* output = reinterpret_cast<sk_sp<SkImage>*>(ctx);

        SkPixmap pm(ii, result->data(0), result->rowBytes(0));
        *output = SkImages::RasterFromPixmapCopy(pm);
    };

    printf("GPU operations modifying surface have been inserted in the command "
           "buffer,"
           "scheduling pixel readback\n");
    context->asyncRescaleAndReadPixels(surface.get(),
                                       imageInfo,
                                       SkIRect::MakeSize({WIDTH, HEIGHT}),
                                       SkImage::RescaleGamma::kSrc,
                                       SkImage::RescaleMode::kRepeatedCubic,
                                       callback,
                                       &img);

    printf("Submitting work to GPU and waiting for it to be done\n");
    context->submit(skgpu::graphite::SyncToCpu::kYes);
    if (context->hasUnfinishedGpuWork()) {
        printf("Sync with GPU completion failed\n");
        return 1;
    }

    auto capture = context->endCapture();
    if (!capture) {
        printf("No capture generated\n");
        return 1;
    }
    auto serializedCapture = capture->serializeCapture();

    /* WRITE DATA TO FILE LOCATION */
    if (serializedCapture) {
        output.write(serializedCapture->data(), serializedCapture->size());
        output.fsync();
        printf("Capture serialized and written to %s\n", argv[1]);
    } else {
        printf("Failed to serialize capture\n");
        return 1;
    }

    /* DESERIALIZE CAPTURE AND INSPECT CONTENTS */
    if (serializedCapture) {
        auto deserializedCapture = SkCapture::MakeFromData(serializedCapture);
        if (deserializedCapture) {
            printf("Successfully deserialized capture for inspection\n");
        }
    }

    printf("done\n");
    return 0;
}
