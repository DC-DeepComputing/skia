/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/AHardwareBufferVulkan.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

#include <android/hardware_buffer.h>

#define VK_CALL(X) gpu->vkInterface()->fFunctions.f##X

namespace skgpu::graphite {

namespace {
    // Create an actual BackendTexture.
    // FORMATINFO intead of TEXTUREINFO?????
    // VkFormat -> getFormatInfo ->
    static BackendTexture make_vk_backend_texture(SharedContext* context,
                                                  AHardwareBuffer* hardwareBuffer,
                                                  int width, int height,
                                                  DeleteImageProc* deleteProc,
                                                  UpdateImageProc* updateProc,
                                                  TexImageCtx* imageCtx,
                                                  bool isProtectedContent,
                                                  const TextureInfo& backendTextureInfo,
                                                  bool isRenderable,
                                                  bool fromAndroidWindow) {
    SkASSERT(context);
    SkASSERT(context->backend() == BackendApi::kVulkan);
    VulkanTextureInfo vkTexInfo;
    if (!backendTextureInfo.getVulkanTextureInfo(&vkTexInfo)) {
        // TODO: Add message
        return;
    }
    auto vkContext = static_cast<VulkanSharedContext*>(context);
    SkASSERT(!isProtectedContent || sharedContext->isProtected() == Protected::kYes);

    // Obtain devices somehow
    // vkContext->vulkanCaps()
    // VulkanSharedContext's ctor receives VulkanBackendContext which has VkPhysicalDevice
    // VkPhysicalDevice physicalDevice = gpu->physicalDevice();
    VkDevice device = vkContext->device();
    VkFormat format = backendTextureInfo.fFormat;

    VkResult result;
    VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
    hwbFormatProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    hwbFormatProps.pNext = nullptr;

    VkAndroidHardwareBufferPropertiesANDROID hwbProps;
    hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    hwbProps.pNext = &hwbFormatProps;

    result = VULKAN_CALL(GetAndroidHardwareBufferProperties(device, hardwareBuffer, &hwbProps));
    if (result != VK_SUCCESS) {
        return {};
    }
    if (hwbFormatProps.format != format) {
        SkDebugf("Queried format not consistent with expected format; got: %d, expected: %d",
                 hwbFormatProps.format,
                 format);
        return {};
    }

    VkExternalFormatANDROID externalFormat;
    externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormat.pNext = nullptr;
    externalFormat.externalFormat = 0;  // If this is zero it is as if we aren't using this struct.
    // TODO: Add YCBCR support
    // const GrVkYcbcrConversionInfo* ycbcrConversion =
    //         GrBackendFormats::GetVkYcbcrConversionInfo(backendFormat);
    // if (!ycbcrConversion) {
    //     return {};
    // }

    if (hwbFormatProps.format != VK_FORMAT_UNDEFINED) {
        SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures) &&
                 SkToBool(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & hwbFormatProps.formatFeatures) &&
                 SkToBool(VK_FORMAT_FEATURE_TRANSFER_DST_BIT & hwbFormatProps.formatFeatures));
        SkASSERT(!ycbcrConversion->isValid());
    } else {
        SkASSERT(ycbcrConversion->isValid());
        // We have an external only format
        SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures));
        SkASSERT(format == VK_FORMAT_UNDEFINED);
        SkASSERT(hwbFormatProps.externalFormat == ycbcrConversion->fExternalFormat);
        externalFormat.externalFormat = hwbFormatProps.externalFormat;
    }

    const VkExternalMemoryImageCreateInfo externalMemoryImageInfo{
            VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,                 // sType
            &externalFormat,                                                     // pNext
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,  // handleTypes
    };
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (format != VK_FORMAT_UNDEFINED) {
        usageFlags = usageFlags |
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (isRenderable) {
            usageFlags = usageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL; // TODO: Query for tiling?
    VkImageCreateFlags flags = isProtectedContent ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        &externalMemoryImageInfo,                    // pNext
        flags,                                       // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                            // VkImageType
        format,                                      // VkFormat
        { (uint32_t)width, (uint32_t)height, 1 },    // VkExtent3D
        1,                                           // mipLevels
        1,                                           // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                       // samples
        tiling,                                      // VkImageTiling
        usageFlags,                                  // VkImageUsageFlags
        VK_SHARING_MODE_EXCLUSIVE,                   // VkSharingMode
        0,                                           // queueFamilyCount
        nullptr,                                     // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED,                   // initialLayout
    };

    VkImage image;
    result = VULKAN_CALL(CreateImage(device, &imageCreateInfo, nullptr, &image));
    if (result != VK_SUCCESS) {
        return {};
    }

    // VkPhysicalDeviceMemoryProperties2 phyDevMemProps;
    // phyDevMemProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    // phyDevMemProps.pNext = nullptr;

    uint32_t typeIndex = 0;
    bool foundHeap = false;
    // VULKAN_CALL(GetPhysicalDeviceMemoryProperties2(physicalDevice, &phyDevMemProps));
    VkPhysicalDeviceMemoryProperties2 phyDevMemProps =
            vkContext->vulkanCaps()->physicalDeviceMemoryProperties2();
    uint32_t memTypeCnt = phyDevMemProps.memoryProperties.memoryTypeCount;
    for (uint32_t i = 0; i < memTypeCnt && !foundHeap; ++i) {
        if (hwbProps.memoryTypeBits & (1 << i)) {
            const VkPhysicalDeviceMemoryProperties& pdmp = phyDevMemProps.memoryProperties;
            uint32_t supportedFlags = pdmp.memoryTypes[i].propertyFlags &
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            if (supportedFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                typeIndex = i;
                foundHeap = true;
            }
        }
    }

    // Fallback to use any available memory type for AHB
    //
    // For external memory import, compatible memory types are decided by the Vulkan driver since
    // the memory has been allocated externally. There are usually special requirements against
    // external memory. e.g. AHB allocated with CPU R/W often usage bits is only importable for
    // non-device-local heap on some AMD systems.
    if (!foundHeap && hwbProps.memoryTypeBits) {
        typeIndex = ffs(hwbProps.memoryTypeBits) - 1;
        foundHeap = true;
    }

    if (!foundHeap) {
        VULKAN_CALL(DestroyImage(device, image, nullptr));
        return {};
    }

    VkImportAndroidHardwareBufferInfoANDROID hwbImportInfo;
    hwbImportInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    hwbImportInfo.pNext = nullptr;
    hwbImportInfo.buffer = hardwareBuffer;

    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo;
    dedicatedAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.pNext = &hwbImportInfo;
    dedicatedAllocInfo.image = image;
    dedicatedAllocInfo.buffer = VK_NULL_HANDLE;

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
        &dedicatedAllocInfo,                         // pNext
        hwbProps.allocationSize,                     // allocationSize
        typeIndex,                                   // memoryTypeIndex
    };

    VkDeviceMemory memory;
    result = VULKAN_CALL(AllocateMemory(device, &allocInfo, nullptr, &memory));
    if (VK_SUCCESS != err) {
        VULKAN_CALL(DestroyImage(device, image, nullptr));
        return {};
    }

    VkBindImageMemoryInfo bindImageInfo;
    bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindImageInfo.pNext = nullptr;
    bindImageInfo.image = image;
    bindImageInfo.memory = memory;
    bindImageInfo.memoryOffset = 0;

    result = VULKAN_CALL(BindImageMemory2(device, 1, &bindImageInfo));
    if (VK_SUCCESS != err) {
        VULKAN_CALL(DestroyImage(device, image, nullptr));
        VULKAN_CALL(FreeMemory(device, memory, nullptr));
        return {};
    }

    skgpu::VulkanAlloc alloc;
    alloc.fMemory = memory;
    alloc.fOffset = 0;
    alloc.fSize = hwbProps.allocationSize;
    alloc.fFlags = 0;

//     GrVkImageInfo imageInfo; // VKIMAGE INFO = VULKANTEXTURE INFO ???
//     imageInfo.fImage = image;
//     imageInfo.fAlloc = alloc;
//     imageInfo.fImageTiling = tiling;
//     imageInfo.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//     imageInfo.fFormat = format;
//     imageInfo.fLevelCount = 1;
//     // TODO: This should possibly be VK_QUEUE_FAMILY_FOREIGN_EXT but current Adreno devices do not
//     // support that extension. Or if we know the source of the AHardwareBuffer is not from a
//     // "foreign" device we can leave them as external.
//     imageInfo.fCurrentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
//     imageInfo.fProtected = isProtectedContent ? Protected::kYes : Protected::kNo;
//     imageInfo.fYcbcrConversionInfo = *ycbcrConversion;
//     imageInfo.fSharingMode = imageCreateInfo.sharingMode;
// #ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
//     imageInfo.fPartOfSwapchainOrAndroidWindow = fromAndroidWindow;
// #endif

    *deleteProc = delete_vk_image;
    *updateProc = update_vk_image;
    *imageCtx = new VulkanCleanupHelper(gpu, image, memory);

    return {{width, height}, vkTexInfo, VK_IMAGE_LAYOUT_UNDEFINED, VK_QUEUE_FAMILY_EXTERNAL, image};
    //return BackendTexture::MakeVk(width, height, imageInfo);
}
}

// Given a populated backend TextureInfo, create an actual BackendTexture.
BackendTexture MakeVulkanBackendTexture(SharedContext* context,
                                        AHardwareBuffer* hardwareBuffer,
                                        int width, int height,
                                        DeleteImageProc* deleteProc,
                                        UpdateImageProc* updateProc,
                                        TexImageCtx* imageCtx,
                                        bool isProtectedContent,
                                        const TextureInfo& backendTextureInfo,
                                        bool isRenderable,
                                        bool fromAndroidWindow) {
    SkASSERT(context);
    if (context->backend() != BackendApi::kVulkan) {
        return {};
    };
    // MAKE Vulkan shared context? VulkanGraphiteUtils::MakeVulkan
    // No, I think they do that. Create recorders, etc.
    // if (isProtectedContent && !context->protected) {
    //     return {};
    // }

    // Option A: Get private context -> resource provider -> onCreateBackendTexture
    // ContextPriv* contextPriv = context->priv();
    // contextPriv->resourceProvider();

    // Option B: Am I supposed to create/obtain a recorder? Recorder::createbackendtexture?
    // Option C: Cre
    return make_vk_backend_texture(context, hardwareBuffer, width, height, deleteProc,
                                   updateProc, imageCtx, isProtectedContent, backendTextureInfo,
                                   isRenderable, fromAndroidWindow);
}

// Given AHardwareBuffer information, populate and return a VulkanTextureInfo struct
TextureInfo GetVulkanTextureInfo(SharedContext* context, AHardwareBuffer* hardwareBuffer,
                                 uint32_t bufferFormat, bool requireKnownFormat) {
    if (context->backend() != BackendApi::kVulkan) {
        return {};
    };
    VkFormat format;
    switch (bufferFormat) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM: {
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        }
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT: {
            format = VK_FORMAT_R16G16B16A16_SFLOAT;
            break;
        }
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM: {
            format = VK_FORMAT_R5G6B5_UNORM_PACK16;
            break;
        }
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM: {
            format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            break;
        }
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM: {
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        }
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM: {
            format = VK_FORMAT_R8G8B8_UNORM;
            break;
        }
#if __ANDROID_API__ >= 33
        case AHARDWAREBUFFER_FORMAT_R8_UNORM: {
            format = VK_FORMAT_R8_UNORM;
            break;
        }
#endif
        default: {
            if (requireKnownFormat) {
                return {};
            } else {
                VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
                hwbFormatProps.sType =
                        VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
                hwbFormatProps.pNext = nullptr;
                VkAndroidHardwareBufferPropertiesANDROID hwbProps;
                hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
                hwbProps.pNext = &hwbFormatProps;

                VkResult result = VULKAN_CALL(GetAndroidHardwareBufferProperties(device,
                                                                                 hardwareBuffer,
                                                                                 &hwbProps));
                if (result != VK_SUCCESS) {
                    return {};
                } else if (hwbFormatProps.format != VK_FORMAT_UNDEFINED) {
                    return {};
                }

                // TODO: Use fetched  hardware properties to determine Ycbcr conversion information/
                // Pass the ycbcr conversion information into VulkanTextureInfo.
                // GrVkYcbcrConversionInfo ycbcrConversion;
                // ycbcrConversion.fYcbcrModel = hwbFormatProps.suggestedYcbcrModel;
                // ycbcrConversion.fYcbcrRange = hwbFormatProps.suggestedYcbcrRange;
                // ycbcrConversion.fXChromaOffset = hwbFormatProps.suggestedXChromaOffset;
                // ycbcrConversion.fYChromaOffset = hwbFormatProps.suggestedYChromaOffset;
                // ycbcrConversion.fForceExplicitReconstruction = VK_FALSE;
                // ycbcrConversion.fExternalFormat = hwbFormatProps.externalFormat;
                // ycbcrConversion.fFormatFeatures = hwbFormatProps.formatFeatures;
                // if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT &
                //     hwbFormatProps.formatFeatures) {
                //     ycbcrConversion.fChromaFilter = VK_FILTER_LINEAR;
                // } else {
                //     ycbcrConversion.fChromaFilter = VK_FILTER_NEAREST;
                // }

                VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
                if (format != VK_FORMAT_UNDEFINED) {
                    usageFlags = usageFlags |
                                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                    if (isRenderable) {
                        usageFlags = usageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                    }
                }
                VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL; // TODO: Query for tiling?
                VkImageCreateFlags imgCreateflags =
                        isProtectedContent ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;

                // TODO: Determine VkImageAspectFlags. If the VkImage is a Ycbcr format, the client
                //       can select a plane. For now, use the default VK_IMAGE_ASPECT_COLOR_BIT.
                VulkanTextureInfo vulkanTextureInfo {VK_SAMPLE_COUNT_1_BIT,
                                                     Mipmapped::kNo,
                                                     imgCreateflags,
                                                     format,
                                                     tiling,
                                                     usageFlags,
                                                     VK_SHARING_MODE_EXCLUSIVE,
                                                     VK_IMAGE_ASPECT_COLOR_BIT};
                return {vulkanTextureInfo};
            }
        }
    }
}

// TODO: This class has great potential to be shared between graphite & ganesh (gpu ptr...)
class VulkanCleanupHelper {
public:
    VulkanCleanupHelper(const VulkanSharedContext* ctxt, VkImage image, VkDeviceMemory memory)
        : fDevice(ctxt->device())
        , fImage(image)
        , fMemory(memory)
        , fDestroyImage(ctxt->interface()->fFunctions.fDestroyImage)
        , fFreeMemory(ctxt->interface()->fFunctions.fFreeMemory) {}
    ~VulkanCleanupHelper() {
        fDestroyImage(fDevice, fImage, nullptr);
        fFreeMemory(fDevice, fMemory, nullptr);
    }
private:
    VkDevice           fDevice;
    VkImage            fImage;
    VkDeviceMemory     fMemory;
    PFN_vkDestroyImage fDestroyImage;
    PFN_vkFreeMemory   fFreeMemory;
};

void delete_vk_image(void* context) {
    VulkanCleanupHelper* cleanupHelper = static_cast<VulkanCleanupHelper*>(context);
    delete cleanupHelper;
}

void update_vk_image(void* context, SharedContext* sharedContext) {
    // no op
}

}  // namespace skgpu::graphite

#endif
