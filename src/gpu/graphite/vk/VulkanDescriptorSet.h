/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef VulkanDescriptorSet_DEFINED
#define VulkanDescriptorSet_DEFINED

#include "src/gpu/graphite/Resource.h"
// #include "include/gpu/vk/GrVkTypes.h"
// #include "src/gpu/ganesh/vk/GrVkManagedResource.h"
// #include "src/gpu/graphite/vk/VulkanDescriptorSetManager.h"

// #include <cinttypes>
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

// #ifdef SK_TRACE_MANAGED_RESOURCES
//     void dumpInfo() const override {
//         SkDebugf("VulkanDescriptorSet: %" PRIdPTR " (%d refs)\n", (intptr_t)fDescSet,
//                  this->getRefCnt());
//     }
// #endif

private:
    void freeGpuData() override;

    const VulkanSharedContext* fContext;
    VkDescriptorSet fDescSet;
    VulkanDescriptorPool* fPool;
    VulkanDescriptorSetManager* fDescriptorSetMgr;
};

} // namespace skgpu::graphite

#endif // VulkanDescriptorSet_DEFINED
