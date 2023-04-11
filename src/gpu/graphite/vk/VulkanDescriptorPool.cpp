/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

VkDescriptorType descriptorSetTypeToVkDescriptorType(VulkanDescriptorPool::DescriptorSetType type) {
    switch (type) {
        case VulkanDescriptorPool::kUniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case VulkanDescriptorPool::kTextureSampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case VulkanDescriptorPool::kTexture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case VulkanDescriptorPool::kCombinedTextureSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case VulkanDescriptorPool::kStorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case VulkanDescriptorPool::kInputAttachment:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    };
    SkUNREACHABLE;
}

VulkanDescriptorPool* VulkanDescriptorPool::Create(const VulkanSharedContext* context,
            std::map<DescriptorSetType, int> DSTypesAndCounts) {
    if (DSTypesAndCounts.empty()) {
        return nullptr;
    }

    // For each DescriptorSetType, determine the appropriate VkDescriptorType. Each unique
    // VkDescriptorType necessitates a corresponding VkDescriptorPoolSize to specify descriptor type
    // and quantity for pool creation.
    VkDescriptorPoolSize poolSizeArr [DSTypesAndCounts.size()];
    int i = 0;
    for (auto descriptorSetType : DSTypesAndCounts) {
        VkDescriptorPoolSize poolSize = poolSizeArr[i];
        memset(&poolSize, 0, sizeof(VkDescriptorPoolSize));
        poolSize.descriptorCount = descriptorSetType.second;
        poolSize.type = descriptorSetTypeToVkDescriptorType(descriptorSetType.first);
    }

    VkDescriptorPoolCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    // This is an over/conservative estimate since each set may contain more than count descriptors.
    createInfo.maxSets = DSTypesAndCounts.size();//count;
    createInfo.poolSizeCount = DSTypesAndCounts.size();//1;
    createInfo.pPoolSizes = poolSizeArr;

    VkDescriptorPool pool;
    VkResult result;
    VULKAN_CALL_RESULT(context->interface(), result, CreateDescriptorPool(context->device(),
                                                                          &createInfo,
                                                                          nullptr,
                                                                          &pool));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return new VulkanDescriptorPool(context, pool, DSTypesAndCounts);
}

VulkanDescriptorPool::VulkanDescriptorPool(const VulkanSharedContext* context,
        VkDescriptorPool pool,
        std::map<DescriptorSetType, int> requestedDSTypesAndCounts)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kNo, /*gpuMemorySize=*/0)
        , fDSTypesAndCounts(requestedDSTypesAndCounts)
        , fDescPool(pool) {}

bool VulkanDescriptorPool::isCompatible(std::map<DescriptorSetType, int> DSTypesAndCounts) const {
    return DSTypesAndCounts == fDSTypesAndCounts;
}

void VulkanDescriptorPool::freeGpuData() {
    // Destroying the VkDescriptorPool will automatically free and delete any VkDescriptorSets
    // allocated from the pool.
    auto ctxt = static_cast<const VulkanSharedContext*>(this->sharedContext());
    VULKAN_CALL(ctxt->interface(),
                DestroyDescriptorPool(ctxt->device(), fDescPool, nullptr));
}

} // namespace skgpu::graphite
