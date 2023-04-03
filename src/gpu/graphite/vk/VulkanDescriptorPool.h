/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanDescriptorPool_DEFINED
#define skgpu_graphite_VulkanDescriptorPool_DEFINED

#include "src/gpu/graphite/Resource.h"

#include <cinttypes>

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * For now, graphite requires that all descriptor sets are of a single descriptor type. We also only
 * make one type of descriptor set from a pool. Thus, a single VkDescriptorPool will only
 * allocate space for one type of descriptor.
 *
 * TODO(b/237106461): Enable supporting multiple descriptor types within descriptor sets.
 */
class VulkanDescriptorPool : public Resource {
public:
    static VulkanDescriptorPool* Create(const VulkanSharedContext*, VkDescriptorType,
                                        uint32_t count);

    VkDescriptorPool descPool() const { return fDescPool; }

    // Returns whether or not this descriptor pool could be used, assuming it gets fully reset and
    // not in use by another draw, to support the requested type and count.
    bool isCompatible(VkDescriptorType type, uint32_t count) const;

private:
    VulkanDescriptorPool(const VulkanSharedContext*, VkDescriptorPool, VkDescriptorType,
                         uint32_t count);
    ~VulkanDescriptorPool() override;

    void freeGpuData() override;

    VkDescriptorType     fType;
    uint32_t             fCount;
    VkDescriptorPool     fDescPool;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanDescriptorPool_DEFINED
