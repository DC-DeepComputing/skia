// /*
// * Copyright 2016 Google Inc.
// *
// * Use of this source code is governed by a BSD-style license that can be
// * found in the LICENSE file.
// */

// #include "src/gpu/graphite/vk/VulkanDescriptorSet.h"

// #include "src/gpu/ganesh/vk/GrVkDescriptorPool.h"
// #include "src/gpu/ganesh/vk/GrVkGpu.h"
// #include "src/gpu/ganesh/vk/GrVkResourceProvider.h"

// VulkanDescriptorSet::VulkanDescriptorSet(GrVkGpu* gpu,
//                                      VkDescriptorSet descSet,
//                                      GrVkDescriptorPool* pool,
//                                      VulkanDescriptorSetManager::Handle handle)
//     : INHERITED(gpu)
//     , fDescSet(descSet)
//     , fPool(pool)
//     , fHandle(handle) {
//     fPool->ref();
// }

// void VulkanDescriptorSet::freeGPUData() const {
//     fPool->unref();
// }

// void VulkanDescriptorSet::onRecycle() const {
//     fGpu->resourceProvider().recycleDescriptorSet(this, fHandle);
// }

