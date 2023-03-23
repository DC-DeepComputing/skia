/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef VulkanDescriptorPool_DEFINED
#define VulkanDescriptorPool_DEFINED

#include "src/gpu/graphite/Resource.h"

#include <cinttypes>

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * We require that all descriptor sets are of a single descriptor type. We also use a pool to only
 * make one type of descriptor set. Thus a single VkDescriptorPool will only allocated space for
 * for one type of descriptor.
 */
class VulkanDescriptorPool : public Resource {
public:
    static VulkanDescriptorPool* Create(const VulkanSharedContext*, VkDescriptorType,
                                        uint32_t count);

    VkDescriptorPool descPool() const { return fDescPool; }

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

#endif // VulkanDescriptorPool_DEFINED
