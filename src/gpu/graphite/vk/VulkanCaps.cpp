/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCaps.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"

namespace skgpu::graphite {

VulkanCaps::VulkanCaps() : Caps() {}

VulkanCaps::~VulkanCaps() {}

TextureInfo VulkanCaps::getDefaultSampledTextureInfo(SkColorType colorType,
                                                     uint32_t levelCount,
                                                     Protected,
                                                     Renderable renderable) const {
    // if (renderable == Renderable::kYes) {

    // }
    // get format from color type; return nothing if invalid; assign to fFormat otherwise

    VulkanTextureInfo info {};
    info.fLevelCount = levelCount;

    return info;
}

TextureInfo VulkanCaps::getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                                  Discardable discardable) const {
    return {};
}

TextureInfo VulkanCaps::getDefaultDepthStencilTextureInfo(SkEnumBitMask<DepthStencilFlags>,
                                                          uint32_t sampleCount,
                                                          Protected) const {
    return {};
}

} // namespace skgpu::graphite

