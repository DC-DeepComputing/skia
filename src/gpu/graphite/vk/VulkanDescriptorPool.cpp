/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

#include "src/gpu/graphite/vk/VulkanSharedContext.h"


VulkanDescriptorPool* VulkanDescriptorPool::Create(const VulkanSharedContext* context,
                                                   VkDescriptorType type,
                                                   uint32_t count) {
    VkDescriptorPoolSize poolSize;
    memset(&poolSize, 0, sizeof(VkDescriptorPoolSize));
    poolSize.descriptorCount = count;
    poolSize.type = type;

    VkDescriptorPoolCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    // This is an over/conservative estimate since each set may contain more than count descriptors.
    createInfo.maxSets = count;
    createInfo.poolSizeCount = 1;
    createInfo.pPoolSizes = &poolSize;

    VkDescriptorPool pool;
    VkResult result;
    VULKAN_CALL_RESULT(gpu, result, CreateDescriptorPool(context->device(), &createInfo, nullptr,
                                                        &pool));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return new VulkanDescriptorPool(context, pool, type, count);
}

// VulkanDescriptorPool::VulkanDescriptorPool(const GrVkGpu* gpu, VkDescriptorPool pool,
//                                        VkDescriptorType type, uint32_t count)
//         : INHERITED(gpu), fType(type), fCount(count), fDescPool(pool) {}

// bool VulkanDescriptorPool::isCompatible(VkDescriptorType type, uint32_t count) const {
//     return fType == type && count <= fCount;
// }

// void VulkanDescriptorPool::freeGPUData() const {
//     // Destroying the VkDescriptorPool will automatically free and delete any VkDescriptorSets
//     // allocated from the pool.
//     GR_VK_CALL(fGpu->vkInterface(), DestroyDescriptorPool(fGpu->device(), fDescPool, nullptr));
// }
