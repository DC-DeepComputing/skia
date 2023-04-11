/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

namespace { // anonymous namespace
VkDescriptorType ds_type_enum_to_vk_ds(VulkanDescriptorPool::DescriptorSetType type) {
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
} // anonymous namespace

VulkanDescriptorPool* VulkanDescriptorPool::Create(const VulkanSharedContext* context,
        std::map<DescriptorSetType, int> DsTypesAndCounts) {
    if (DsTypesAndCounts.empty()) {
        return nullptr;
    }

    // For each DescriptorSetType, determine the appropriate VkDescriptorType. Each unique
    // VkDescriptorType necessitates a corresponding VkDescriptorPoolSize to specify descriptor type
    // and quantity for pool creation.
    VkDescriptorPoolSize poolSizeArr [DsTypesAndCounts.size()];
    int poolSizeCount = 0;
    // Track the maximum requested quantity of descriptors for use as a conservative upper bounds
    // for the max. number of sets that can be allocated from this pool.
    int maxTypeCount = 0;
    for (auto descriptorSetType : DsTypesAndCounts) {
        VkDescriptorPoolSize poolSize = poolSizeArr[poolSizeCount++];
        memset(&poolSize, 0, sizeof(VkDescriptorPoolSize));
        poolSize.descriptorCount = descriptorSetType.second;
        if (descriptorSetType.second > maxTypeCount) {
            maxTypeCount = descriptorSetType.second;
        }
        poolSize.type = ds_type_enum_to_vk_ds(descriptorSetType.first);
    }

    VkDescriptorPoolCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    // This is an over/conservative estimate since each set may contain > maxTypeCount descriptors.
    createInfo.maxSets = maxTypeCount;
    createInfo.poolSizeCount = DsTypesAndCounts.size();
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
    return new VulkanDescriptorPool(context, pool, DsTypesAndCounts);
}

VulkanDescriptorPool::VulkanDescriptorPool(const VulkanSharedContext* context,
        VkDescriptorPool pool,
        std::map<DescriptorSetType, int> requestedDsTypesAndCounts)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kNo, /*gpuMemorySize=*/0)
        , fDsTypesAndCounts(requestedDsTypesAndCounts)
        , fDescPool(pool) {}

bool VulkanDescriptorPool::isCompatible(const std::map<DescriptorSetType, int>& DsTypesAndCounts)
        const {
    return DsTypesAndCounts == fDsTypesAndCounts;
}

void VulkanDescriptorPool::freeGpuData() {
    // Destroying the VkDescriptorPool will automatically free and delete any VkDescriptorSets
    // allocated from the pool.
    auto ctxt = static_cast<const VulkanSharedContext*>(this->sharedContext());
    VULKAN_CALL(ctxt->interface(),
                DestroyDescriptorPool(ctxt->device(), fDescPool, nullptr));
}

} // namespace skgpu::graphite
