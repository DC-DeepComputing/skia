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
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include <map>

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * Graphite's Vulkan backend allows for multiple descriptor types to exist within a single
 * descriptor set. Given a map of DescriptorType(s) to their requested quantity, a DescriptorPool
 * will be created with the appropriate VkDescriptorTypes and quantities.
 *
 * For example, if you want a descriptor pool that has descriptors to support a set of 1 sampler and
 * 1 texture, you can call VulkanDescriptorPool::Create with {{kTexture, 1}, {kTextureSampler, 1}}.
 */
class VulkanDescriptorPool : public SkRefCnt {
public:
    static sk_sp<VulkanDescriptorPool> Make(const VulkanSharedContext*,
            std::map<DescriptorType, int> dsTypesAndCounts);

    VkDescriptorPool descPool() const { return fDescPool; }

private:
    // Conservative upper bound of number of sets supported per pool.
    static constexpr int kMaxNumSets = 512;

    VulkanDescriptorPool(const VulkanSharedContext*,
            VkDescriptorPool,
            std::map<DescriptorType, int> requestedDsTypesAndCounts);
    ~VulkanDescriptorPool() override;

    const VulkanSharedContext*                      fSharedContext;
    std::map<DescriptorType, int>                   fDsTypesAndCounts;
    VkDescriptorPool                                fDescPool;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorPool_DEFINED
