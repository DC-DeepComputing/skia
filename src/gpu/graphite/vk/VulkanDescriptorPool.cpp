/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

#include "include/private/base/SkTArray.h"
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
    // Each unique descriptor type necessitates the creation of a VkDescriptorPoolSize struct which
    // specifies the descriptor type and quantity for descriptor pool creation. Initialize an
    // STArray with capacity for the maximum possible number of VkDescriptorPoolSizes (1 per
    // possible DescriptorSetType).
    skia_private::STArray<kDescriptorSetTypeCount, VkDescriptorPoolSize> poolSizes;
    int i = 0;
    for (auto descriptorSetType : dsTypesAndCounts) {
        VkDescriptorPoolSize* poolSize = &poolSizes.at(i++);
        memset(poolSize, 0, sizeof(VkDescriptorPoolSize));
        // Map each DescriptorSetType to the appropriate backend VkDescriptorType.
        poolSize->type = ds_type_enum_to_vk_ds(descriptorSetType.first);
        poolSize->descriptorCount = descriptorSetType.second;
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
