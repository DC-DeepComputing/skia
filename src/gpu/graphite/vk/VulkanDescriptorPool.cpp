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
    }
    SkUNREACHABLE;
}
} // anonymous namespace

sk_sp<VulkanDescriptorPool> VulkanDescriptorPool::Make(const VulkanSharedContext* context,
        std::map<DescriptorSetType, int> dsTypesAndCounts) {
    if (dsTypesAndCounts.empty()) {
        return nullptr;
    }

    // TODO: Remove/clean up this relatively hacky workaround for variable length arrays
    // (If it even works; testing on bots).
    const int arrLen = dsTypesAndCounts.size();
    VkDescriptorPoolSize poolSizeArr [arrLen];
    int poolSizeCount = 0;
    // For each DescriptorSetType, determine the appropriate VkDescriptorType. Each unique
    // VkDescriptorType necessitates a corresponding VkDescriptorPoolSize to specify descriptor type
    // and quantity for pool creation.
    for (auto descriptorSetType : dsTypesAndCounts) {
        VkDescriptorPoolSize poolSize = poolSizeArr[poolSizeCount++];
        memset(&poolSize, 0, sizeof(VkDescriptorPoolSize));
        poolSize.descriptorCount = descriptorSetType.second;
        poolSize.type = ds_type_enum_to_vk_ds(descriptorSetType.first);
    }

    VkDescriptorPoolCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    // Create a pool capable of supporting the maximum number of sets.
    createInfo.maxSets = kMaxNumSets;
    createInfo.poolSizeCount = dsTypesAndCounts.size();
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
    return sk_sp<VulkanDescriptorPool>(new VulkanDescriptorPool(context, pool, dsTypesAndCounts));
}

VulkanDescriptorPool::VulkanDescriptorPool(const VulkanSharedContext* context,
        VkDescriptorPool pool,
        std::map<DescriptorSetType, int> requestedDsTypesAndCounts)
        : fSharedContext(context)
        , fDsTypesAndCounts(requestedDsTypesAndCounts)
        , fDescPool(pool) {}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    // Destroying the VkDescriptorPool will automatically free and delete any VkDescriptorSets
    // allocated from the pool.
    VULKAN_CALL(fSharedContext->interface(),
                DestroyDescriptorPool(fSharedContext->device(), fDescPool, nullptr));
}

} // namespace skgpu::graphite
