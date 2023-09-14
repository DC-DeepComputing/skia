/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanAHardwareBufferUtils_DEFINED
#define skgpu_VulkanAHardwareBufferUtils_DEFINED


namespace skgpu {

// VkImage get_VkImage_from_AHardwareBuffer() {
//     VkResult result;
//     VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
//     hwbFormatProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
//     hwbFormatProps.pNext = nullptr;

//     VkAndroidHardwareBufferPropertiesANDROID hwbProps;
//     hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
//     hwbProps.pNext = &hwbFormatProps;

//     result = VULKAN_CALL(GetAndroidHardwareBufferProperties(device, hardwareBuffer, &hwbProps));
//     if (result != VK_SUCCESS) {
//         return {};
//     }
//     if (hwbFormatProps.format != format) {
//         SkDebugf("Queried format not consistent with expected format; got: %d, expected: %d",
//                  hwbFormatProps.format,
//                  format);
//         return {};
//     }
//     VkExternalFormatANDROID externalFormat;
//         externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
//         externalFormat.pNext = nullptr;
//         externalFormat.externalFormat = 0;  // If this is zero it is as if we aren't using this struct.

//    // TODO: Add YCBCR support
//     // const GrVkYcbcrConversionInfo* ycbcrConversion =
//     //         GrBackendFormats::GetVkYcbcrConversionInfo(backendFormat);
//     // if (!ycbcrConversion) {
//     //     return {};
//     // }

//     if (hwbFormatProps.format != VK_FORMAT_UNDEFINED) {
//         SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures) &&
//                  SkToBool(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & hwbFormatProps.formatFeatures) &&
//                  SkToBool(VK_FORMAT_FEATURE_TRANSFER_DST_BIT & hwbFormatProps.formatFeatures));
//         SkASSERT(!ycbcrConversion->isValid());
//     } else {
//         SkASSERT(ycbcrConversion->isValid());
//         // We have an external only format
//         SkASSERT(SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures));
//         SkASSERT(format == VK_FORMAT_UNDEFINED);
//         SkASSERT(hwbFormatProps.externalFormat == ycbcrConversion->fExternalFormat);
//         externalFormat.externalFormat = hwbFormatProps.externalFormat;
//     }

//     const VkExternalMemoryImageCreateInfo externalMemoryImageInfo{
//             VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,                 // sType
//             &externalFormat,                                                     // pNext
//             VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,  // handleTypes
//     };
//     VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
//     if (format != VK_FORMAT_UNDEFINED) {
//         usageFlags = usageFlags |
//                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
//                 VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//         if (isRenderable) {
//             usageFlags = usageFlags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//         }
//     }
//     VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL; // TODO: Query for tiling?
//     VkImageCreateFlags flags = isProtectedContent ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;

//     const VkImageCreateInfo imageCreateInfo = {
//         VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
//         &externalMemoryImageInfo,                    // pNext
//         flags,                                       // VkImageCreateFlags
//         VK_IMAGE_TYPE_2D,                            // VkImageType
//         format,                                      // VkFormat
//         { (uint32_t)width, (uint32_t)height, 1 },    // VkExtent3D
//         1,                                           // mipLevels
//         1,                                           // arrayLayers
//         VK_SAMPLE_COUNT_1_BIT,                       // samples
//         tiling,                                      // VkImageTiling
//         usageFlags,                                  // VkImageUsageFlags
//         VK_SHARING_MODE_EXCLUSIVE,                   // VkSharingMode
//         0,                                           // queueFamilyCount
//         nullptr,                                     // pQueueFamilyIndices
//         VK_IMAGE_LAYOUT_UNDEFINED,                   // initialLayout
//     };

//     VkImage image;
//     result = VULKAN_CALL(CreateImage(device, &imageCreateInfo, nullptr, &image));
//     if (result != VK_SUCCESS) {
//         return {};
//     }


//     // Fallback to use any available memory type for AHB
//     //
//     // For external memory import, compatible memory types are decided by the Vulkan driver since
//     // the memory has been allocated externally. There are usually special requirements against
//     // external memory. e.g. AHB allocated with CPU R/W often usage bits is only importable for
//     // non-device-local heap on some AMD systems.
//     if (!foundHeap && hwbProps.memoryTypeBits) {
//         typeIndex = ffs(hwbProps.memoryTypeBits) - 1;
//         foundHeap = true;
//     }

//     if (!foundHeap) {
//         VULKAN_CALL(DestroyImage(device, image, nullptr));
//         return {};
//     }

//     VkImportAndroidHardwareBufferInfoANDROID hwbImportInfo;
//     hwbImportInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
//     hwbImportInfo.pNext = nullptr;
//     hwbImportInfo.buffer = hardwareBuffer;

//     VkMemoryDedicatedAllocateInfo dedicatedAllocInfo;
//     dedicatedAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
//     dedicatedAllocInfo.pNext = &hwbImportInfo;
//     dedicatedAllocInfo.image = image;
//     dedicatedAllocInfo.buffer = VK_NULL_HANDLE;

//     VkMemoryAllocateInfo allocInfo = {
//         VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
//         &dedicatedAllocInfo,                         // pNext
//         hwbProps.allocationSize,                     // allocationSize
//         typeIndex,                                   // memoryTypeIndex
//     };

//     VkDeviceMemory memory;
//     result = VULKAN_CALL(AllocateMemory(device, &allocInfo, nullptr, &memory));
//     if (VK_SUCCESS != err) {
//         VULKAN_CALL(DestroyImage(device, image, nullptr));
//         return {};
//     }

//     VkBindImageMemoryInfo bindImageInfo;
//     bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
//     bindImageInfo.pNext = nullptr;
//     bindImageInfo.image = image;
//     bindImageInfo.memory = memory;
//     bindImageInfo.memoryOffset = 0;

//     result = VULKAN_CALL(BindImageMemory2(device, 1, &bindImageInfo));
//     if (VK_SUCCESS != err) {
//         VULKAN_CALL(DestroyImage(device, image, nullptr));
//         VULKAN_CALL(FreeMemory(device, memory, nullptr));
//         return {};
//     }

//     skgpu::VulkanAlloc alloc;
//     alloc.fMemory = memory;
//     alloc.fOffset = 0;
//     alloc.fSize = hwbProps.allocationSize;
//     alloc.fFlags = 0;
// }
} // namespace skgpu

#endif // skgpu_VulkanAHardwareBufferUtils_DEFINED
