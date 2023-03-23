/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanSharedContext* context,
                                         VulkanDescriptorSet descSet,
                                         VulkanDescriptorPool* pool,
                                         VulkanDescriptorSetManager* mgr)
    : Resource(context, Ownership::kOwned, Budgeted::kYes, sizeof(VkDescriptorSet))
    , fDescSet(descSet)
    , fPool(pool)
    , fDescriptorSetMgr(mgr) {
    fPool->ref();
}

void VulkanDescriptorSet::freeGpuData() {
    fPool->unref();
}

