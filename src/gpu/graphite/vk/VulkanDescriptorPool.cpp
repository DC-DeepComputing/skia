/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

// std::pair<VkDescriptorPoolSize*, int poolSizeCount> VulkanDescriptorPool::getPoolSizesFromDescriptorSetTypes(
//     const std::unordered_map<DescriptorSetType, int count>& requestedDescriptorSetTypes) {

//     VkDescriptorPoolSize[requestedDescriptorSetTypes.size()] poolSizesArray;

//     for (auto descriptorSetType : requestedDescriptorSetTypes) {
//         VkDescriptorPoolSize poolSize;
//         memset(&poolSize, 0, sizeof(VkDescriptorPoolSize));
//         poolSize.descriptorCount = descriptorSetType.second;

//         switch (descriptorSetType) {
//             case kUniformBuffer:
//                 poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//                 break;
//             case kTextureSampler:
//                 poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLER;
//                 break;
//             case kTexture:
//                 poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
//                 break;
//             case kCombinedTextureSampler:
//                 poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//                 break;
//             case kStorageBuffer:
//                 poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//                 break;
//             case kInputAttachment:
//                 poolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
//                 break;
//             default:
//                 break;
//         };
//     }

//     return std::make_pair<VkDescriptorPoolSize*, int>(VkDescriptorPoolSize,
//                                                       requestedDescriptorSetTypes.size());
// }

VulkanDescriptorPool* VulkanDescriptorPool::Create(const VulkanSharedContext* context,
            std::unordered_map<DescriptorSetType, int> DSTypesAndCounts) {
    if (DSTypesAndCounts.empty()) {
        return nullptr;
    }

    // VkDescriptorPoolCreateInfo can take in an array of VkDescriptorPoolSizes.
    // # and type of descriptors is dependent upon the type of descriptor set requested.
    VkDescriptorPoolSize poolSizesArray [DSTypesAndCounts.size()];

    for (auto descriptorSetType : DSTypesAndCounts) {
        VkDescriptorPoolSize poolSize;
        memset(&poolSize, 0, sizeof(VkDescriptorPoolSize));
        poolSize.descriptorCount = descriptorSetType.second;

        switch (descriptorSetType.first) {
            case kUniformBuffer:
                poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case kTextureSampler:
                poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLER;
                break;
            case kTexture:
                poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                break;
            case kCombinedTextureSampler:
                poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
            case kStorageBuffer:
                poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            case kInputAttachment:
                poolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                break;
            default:
                break;
        };
    }
    std::pair<VkDescriptorPoolSize*, int> descriptorPoolSizesAndCount =
            std::make_pair<VkDescriptorPoolSize*, int>(poolSizesArray,
                                                        DSTypesAndCounts.size());

    VkDescriptorPoolCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    // This is an over/conservative estimate since each set may contain more than count descriptors.
    createInfo.maxSets = descriptorPoolSizesAndCount.second;//count;
    createInfo.poolSizeCount = descriptorPoolSizesAndCount.second;//1;
    createInfo.pPoolSizes = descriptorPoolSizesAndCount.first;

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
        std::unordered_map<DescriptorSetType, int> requestedDSTypesAndCounts)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kNo, /*gpuMemorySize=*/0)
        , fDSTypesAndCounts(requestedDSTypesAndCounts)
        , fDescPool(pool) {}

bool VulkanDescriptorPool::isCompatible(DescriptorSetType type, uint32_t count) const {
    // TODO: Implement. Compare maps to see if descriptor pool type is compatible.
    return false;
    // return fType == type && count <= fCount;
}

void VulkanDescriptorPool::freeGpuData() {
    // Destroying the VkDescriptorPool will automatically free and delete any VkDescriptorSets
    // allocated from the pool.
    auto ctxt = static_cast<const VulkanSharedContext*>(this->sharedContext());
    VULKAN_CALL(ctxt->interface(),
                DestroyDescriptorPool(ctxt->device(), fDescPool, nullptr));
}

} // namespace skgpu::graphite
