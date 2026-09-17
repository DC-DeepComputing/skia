/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

#include "include/private/SkTArray.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanDescriptorPool> VulkanDescriptorPool::Make(const VulkanSharedContext* context,
                                                       SkSpan<DescriptorData> requestedDescCounts,
                                                       VkDescriptorSetLayout layout,
                                                       uint32_t numSets) {

    if (requestedDescCounts.empty()) {
        return nullptr;
    }

    // Multiple bindings within a descriptor set may request the same descriptor type (e.g. multiple
    // storage buffers in compute steps). We consolidate identical descriptor types so each
    // VkDescriptorType appears at most once in poolSizes. This guarantees that poolSizes will
    // contain at most kDescriptorType entires, and we validate the cumulative per-set descriptor
    // count for each type against kMaxNumDescriptors, preventing the limit from being evaded across
    // multiple bindings.
    skia_private::STArray<kDescriptorTypeCount, VkDescriptorPoolSize> poolSizes;
    const uint32_t maxTotalDescriptors = kMaxNumDescriptors * numSets;

    auto checkCount = [&](VkDescriptorType type, uint32_t totalCount) -> bool {
        if (totalCount > maxTotalDescriptors) {
            SkDebugf("The cumulative number of descriptors requested for type %d exceeds "
                     "the maximum allowed (%d).\n",
                     type, kMaxNumDescriptors);
            return false;
        }
        return true;
    };

    for (const DescriptorData& desc : requestedDescCounts) {
        uint32_t count = desc.fCount;
        SkASSERT(count > 0);
        VkDescriptorType descType = DsTypeEnumToVkDs(desc.fType);
        bool found = false;
        for (auto& existing : poolSizes) {
            if (existing.type == descType) {
                existing.descriptorCount += count * numSets;
                if (!checkCount(descType, existing.descriptorCount)) {
                    return nullptr;
                }
                found = true;
                break;
            }
        }
        if (!found) {
            uint32_t total = count * numSets;
            if (!checkCount(descType, total)) {
                return nullptr;
            }
            VkDescriptorPoolSize& poolSize = poolSizes.push_back();
            poolSize = {};
            poolSize.type = descType;
            poolSize.descriptorCount = total;
        }
    }

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.maxSets = numSets;
    createInfo.poolSizeCount = poolSizes.size();
    createInfo.pPoolSizes = poolSizes.data();

    VkDescriptorPool pool;
    VkResult result;
    VULKAN_CALL_RESULT(context,
                       result,
                       CreateDescriptorPool(context->device(),
                                            &createInfo,
                                            /*const VkAllocationCallbacks*=*/nullptr,
                                            &pool));
    if (result != VK_SUCCESS) {
        VULKAN_CALL(context->interface(),
                    DestroyDescriptorSetLayout(context->device(), layout, nullptr));
        return nullptr;
    }
    return sk_sp<VulkanDescriptorPool>(new VulkanDescriptorPool(context, pool, layout));
}

VulkanDescriptorPool::VulkanDescriptorPool(const VulkanSharedContext* context,
                                           VkDescriptorPool pool,
                                           VkDescriptorSetLayout layout)
        : fSharedContext(context)
        , fDescPool(pool)
        , fDescSetLayout(layout) {}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    // Destroying the VkDescriptorPool will automatically free and delete any VkDescriptorSets
    // allocated from the pool.
    VULKAN_CALL(fSharedContext->interface(),
                DestroyDescriptorPool(fSharedContext->device(), fDescPool, nullptr));
    if (fDescSetLayout != VK_NULL_HANDLE) {
        VULKAN_CALL(fSharedContext->interface(),
                    DestroyDescriptorSetLayout(fSharedContext->device(), fDescSetLayout, nullptr));
        fDescSetLayout = VK_NULL_HANDLE;
    }
}

} // namespace skgpu::graphite
