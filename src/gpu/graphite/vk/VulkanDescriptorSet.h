/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanDescriptorSet_DEFINED
#define skgpu_graphite_VulkanDescriptorSet_DEFINED

#include "src/gpu/graphite/Resource.h"

namespace skgpu::graphite {

class VulkanDescriptorPool;
class VulkanDescriptorSetManager;
class VulkanSharedContext;

class VulkanDescriptorSet : public Resource {
public:
    VulkanDescriptorSet(const VulkanSharedContext*, VkDescriptorSet, VulkanDescriptorPool,
                        VulkanDescriptorSetManager*);

    ~VulkanDescriptorSet() override {}

    const VkDescriptorSet* descriptorSet() const { return &fDescSet; }

private:
    void freeGpuData() override;

    const VulkanSharedContext* fContext;
    VkDescriptorSet fDescSet;
    VulkanDescriptorPool* fPool;
    VulkanDescriptorSetManager* fDescriptorSetMgr;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorSet_DEFINED
