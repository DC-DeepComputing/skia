/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanDescriptorPool> VulkanDescriptorPool::Make(const VulkanSharedContext* context,
        int requestedDescCounts [kDescriptorTypeCount]) {
    // Each unique descriptor type necessitates the creation of a VkDescriptorPoolSize struct which
    // specifies the descriptor type and quantity for descriptor pool creation. Initialize an
    // STArray with capacity for the maximum possible number of VkDescriptorPoolSizes (1 per
    // possible DescriptorSetType).
    skia_private::STArray<kDescriptorTypeCount, VkDescriptorPoolSize> poolSizes;
    for (int i = 0; i < kDescriptorTypeCount; i++) {
        if (requestedDescCounts[i] == 0) {
            continue;
        } else if (requestedDescCounts[i] > kMaxNumDescriptors) {
            SkDebugf("The number of descriptors requested, %d, exceeds the maximum allowed (%d).\n",
                     requestedDescCounts[i],
                     kMaxNumDescriptors);
            return nullptr;
        }
        VkDescriptorPoolSize* poolSize = &poolSizes.at(i++);
        memset(poolSize, 0, sizeof(VkDescriptorPoolSize));
        // Map each DescriptorSetType to the appropriate backend VkDescriptorType
        poolSize->type = VulkanDescriptorSet::dsTypeEnumToVkDs(static_cast<DescriptorType>(i));
        // Create a pool large enough to accommodate the maximum possible number of descriptor sets
        poolSize->descriptorCount = requestedDescCounts[i] * kMaxNumSets;
    }

    if (poolSizes.empty()) {
        return nullptr;
    }

    VkDescriptorPoolCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.maxSets = kMaxNumSets;
    createInfo.poolSizeCount = poolSizes.size();
    createInfo.pPoolSizes = &poolSizes.front();

    VkDescriptorPool pool;
    VkResult result;
    VULKAN_CALL_RESULT(context->interface(), result, CreateDescriptorPool(context->device(),
                                                                          &createInfo,
                                                                          nullptr,
                                                                          &pool));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanDescriptorPool>(new VulkanDescriptorPool(context, pool));
}

VulkanDescriptorPool::VulkanDescriptorPool(const VulkanSharedContext* context,
                                           VkDescriptorPool pool)
        : fSharedContext(context)
        , fDescPool(pool) {}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    // Destroying the VkDescriptorPool will automatically free and delete any VkDescriptorSets
    // allocated from the pool.
    VULKAN_CALL(fSharedContext->interface(),
                DestroyDescriptorPool(fSharedContext->device(), fDescPool, nullptr));
}

} // namespace skgpu::graphite
