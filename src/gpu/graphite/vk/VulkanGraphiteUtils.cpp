/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"

namespace skgpu::graphite {

VkFormat VulkanDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags> mask) {
    if (mask == DepthStencilFlags::kDepth) {
        return VK_FORMAT_D32_SFLOAT;
    } else if (mask == DepthStencilFlags::kStencil) {
        return VK_FORMAT_S8_UINT;
    } else if (mask == DepthStencilFlags::kDepthStencil) {
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    }
    SkASSERT(false);
    return VK_FORMAT_UNDEFINED;
}

} // namespace skgpu::graphite
