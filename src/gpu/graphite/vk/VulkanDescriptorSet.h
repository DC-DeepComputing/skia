/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef VulkanDescriptorSet_DEFINED
#define VulkanDescriptorSet_DEFINED

// #include "include/gpu/vk/GrVkTypes.h"
// #include "src/gpu/ganesh/vk/GrVkManagedResource.h"
// #include "src/gpu/graphite/vk/VulkanDescriptorSetManager.h"

// #include <cinttypes>

class GrVkDescriptorPool;
// class GrVkGpu;

class VulkanDescriptorSet /*: public GrVkRecycledResource*/ {
public:
    VulkanDescriptorSet(const VulkanSharedContext*, VkDescriptorSet, VulkanDescriptorPool,
                        VulkanDescriptorSetManager::Handle handle);

//     ~VulkanDescriptorSet() override {}

    const VkDescriptorSet* descriptorSet() const { return &fDescSet; }

// #ifdef SK_TRACE_MANAGED_RESOURCES
//     void dumpInfo() const override {
//         SkDebugf("VulkanDescriptorSet: %" PRIdPTR " (%d refs)\n", (intptr_t)fDescSet,
//                  this->getRefCnt());
//     }
// #endif

private:
//     void freeGPUData() const;
//     void onRecycle() const ;

    VkDescriptorSet fDescSet;
//     SkDEBUGCODE(mutable) GrVkDescriptorPool* fPool;
//     VulkanDescriptorSetManager::Handle         fHandle;
};

#endif
