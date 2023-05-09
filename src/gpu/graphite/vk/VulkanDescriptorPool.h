/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanDescriptorPool_DEFINED
#define skgpu_graphite_VulkanDescriptorPool_DEFINED

#include "include/core/SkRefCnt.h"

#include "src/gpu/graphite/Descriptors.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanDescriptorPool : public SkRefCnt {
public:
    // Conservative upper bound of number of sets supported per pool.
    static constexpr int kMaxNumSets = 512;
    /**
     * For each unique descriptor set layout needed, VulkanDescriptorPool::Make will be given an
     * array of integers. Each index of the array corresponds to a DescriptorType enum value, and
     * the integer at that index represents descriptor quantity. For example, accessing the value of
     * the array at the index [DescriptorType::kUniformBuffer] will yield the requested quantity of
     * uniform buffer descriptors.
     *
     * A pool is then created which houses enough of the descriptor types and quantities to
     * allocate the maximum number of descriptor sets possible (kMaxNumSets).
    */
    static sk_sp<VulkanDescriptorPool> Make(const VulkanSharedContext*,
                                            int descriptorCounts [kDescriptorTypeCount]);

    VkDescriptorPool* descPool() { return &fDescPool; }

private:
    // Conservative overestimation of a maximum number of descriptors of any given type that can be
    // requested.
    static constexpr int kMaxNumDescriptors = 1024;

    VulkanDescriptorPool(const VulkanSharedContext*,
                         VkDescriptorPool);
    ~VulkanDescriptorPool() override;

    const VulkanSharedContext*       fSharedContext;
    VkDescriptorPool                 fDescPool;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorPool_DEFINED
