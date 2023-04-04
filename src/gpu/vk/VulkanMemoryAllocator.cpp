/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"

#include <stdio.h>

namespace skgpu {

void VulkanMemoryAllocator::addImageToDefragList(GrVkImage* gr_image, VulkanBackendMemory memory) {
    auto iter = defrag_resource_map_.find(gr_image);
    SkASSERTF(iter == defrag_resource_map_.end(),
              "Trying to insert resource already in defrag map. Was the resource not freed "
              "correctly?");
    defrag_resource_map_.emplace_hint(iter, gr_image, memory);
}

void VulkanMemoryAllocator::removeImageFromDefragList(GrVkImage* gr_image) {
    auto iter = defrag_resource_map_.find(gr_image);
    SkASSERTF(iter != defrag_resource_map_.end(),
              "Trying to remove resource that was not in defrag map.");
    defrag_resource_map_.erase(iter);
}

}  // namespace skgpu
