/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef VulkanDescriptorSetManager_DEFINED
#define VulkanDescriptorSetManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"
#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

namespace skgpu::graphite {

class VulkanDescriptorSet;

/**
 * This class handles the allocation of descriptor sets for a given VulkanDescriptorSetLayout. It
 * attempts to reuse previously allocated descriptor sets if they are no longer in use by other
 * objects.
 * TODO: Imeplement.
 */
class VulkanDescriptorSetManager {
public:

private:
};

} // namespace skgpu::graphite

#endif // VulkanDescriptorSetManager_DEFINED
