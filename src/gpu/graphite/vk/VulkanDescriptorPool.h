/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanDescriptorPool_DEFINED
#define skgpu_graphite_VulkanDescriptorPool_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include <unordered_map>

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * Graphite allows for multiple descriptor types to exist within a single descriptor set. Given a
 * DescriptorSetType, Graphite will correlate the type to an appropriate DescriptorSetLayout and
 * DescriptorSet. A DescriptorPool will be created for each uniquely-encountered DescriptorSetLayout.
 */
class VulkanDescriptorPool : public Resource {
public:
    /**
     * Descriptor sets can be requested for the following structures.
     * TODO: Elevate this structure to be higher-up in graphite code so other backends can
     * utilize it.
    */
    enum DescriptorSetType {
        kUniformBuffer = 0,
        kTextureSampler,
        kTexture,
        kCombinedTextureSampler,
        kStorageBuffer,
        kInputAttachment,
    };

    // struct DescriptorSetTypes {
    //     DescriptorType* nextType;
    //     DescriptorSetType descriptorSetType;
    // };

    static VulkanDescriptorPool* Create(const VulkanSharedContext*,
            std::unordered_map<DescriptorSetType, int> DSTypesAndCounts);

    VkDescriptorPool descPool() const { return fDescPool; }

    // Returns whether or not this descriptor pool could be used, assuming it gets fully reset and
    // not in use by another draw, to support the requested type and count.
    bool isCompatible(DescriptorSetType type, uint32_t count) const;

private:
    VulkanDescriptorPool(const VulkanSharedContext*, VkDescriptorPool,
            std::unordered_map<DescriptorSetType, int> requestedDSTypesAndCounts);
    void freeGpuData() override;

    std::unordered_map<DescriptorSetType, int> fDSTypesAndCounts;
    VkDescriptorPool     fDescPool;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorPool_DEFINED
