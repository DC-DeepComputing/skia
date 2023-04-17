/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanDescriptorPool_DEFINED
#define skgpu_graphite_VulkanDescriptorPool_DEFINED

#include "include/core/SkRefCnt.h"

#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include <map>

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * Graphite's Vulkan backend allows for multiple descriptor types to exist within a single
 * descriptor set. Given a map of DescriptorSetType(s) to their requested quantity, a DescriptorPool
 * will be created with the appropriate VkDescriptorTypes and quantities.
 *
 * For example, if you want a descriptor pool that has descriptors to support a set of 1 sampler and
 * 1 texture, you can call VulkanDescriptorPool::Create with {{kTexture, 1}, {kTextureSampler, 1}}.
 */
class VulkanDescriptorPool : public SkRefCnt {
public:
    /**
     * Descriptor sets can be requested for the following structures.
     * TODO: Optionally elevate this structure to be higher-up in graphite code so other backends
     * can utilize it.
    */
    enum DescriptorSetType {
        kUniformBuffer = 0,
        kTextureSampler,
        kTexture,
        kCombinedTextureSampler,
        kStorageBuffer,
        kInputAttachment,
    };

    static sk_sp<VulkanDescriptorPool> Create(const VulkanSharedContext*,
                                        std::map<DescriptorSetType, int> DsTypesAndCounts);

    VkDescriptorPool descPool() const { return fDescPool; }

    // Returns whether or not this descriptor pool could be used, assuming it gets fully reset and
    // not in use by another draw, to support the requested types and counts.
    bool isCompatible(const std::map<DescriptorSetType, int>& DsTypesAndCounts) const;

private:
    // Conservative upper bound of number of sets supported per pool.
    static constexpr int kMaxNumSets = 512;

    VulkanDescriptorPool(const VulkanSharedContext*,
                         VkDescriptorPool,
                         std::map<DescriptorSetType, int> requestedDsTypesAndCounts);
    ~VulkanDescriptorPool() override;

    const VulkanSharedContext*       fSharedContext;
    std::map<DescriptorSetType, int> fDsTypesAndCounts;
    VkDescriptorPool                 fDescPool;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorPool_DEFINED
