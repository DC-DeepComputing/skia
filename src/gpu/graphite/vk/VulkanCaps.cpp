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

const Caps::ColorTypeInfo* VulkanCaps::getColorTypeInfo(
        SkColorType ct, const TextureInfo& textureInfo) const {

    VkFormat vkFormat = static_cast<VkFormat>(textureInfo.vulkanTextureSpec().fFormat);
    if (vkFormat == VK_FORMAT_UNDEFINED) {
        return nullptr;
    }

    const FormatInfo& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const ColorTypeInfo& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == ct) {
            return &ctInfo;
        }
    }

    return nullptr;
}

// These are all the valid VkFormats that we support in Skia. They are roughly ordered from most
// frequently used to least to improve look up times in arrays.
static constexpr VkFormat kVkFormats[] = {
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_R8_UNORM,
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R5G6B5_UNORM_PACK16,
    VK_FORMAT_R16G16B16A16_SFLOAT,
    VK_FORMAT_R16_SFLOAT,
    VK_FORMAT_R8G8B8_UNORM,
    VK_FORMAT_R8G8_UNORM,
    VK_FORMAT_A2B10G10R10_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    VK_FORMAT_B4G4R4A4_UNORM_PACK16,
    VK_FORMAT_R4G4B4A4_UNORM_PACK16,
    VK_FORMAT_R8G8B8A8_SRGB,
    VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
    VK_FORMAT_BC1_RGB_UNORM_BLOCK,
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
    VK_FORMAT_R16_UNORM,
    VK_FORMAT_R16G16_UNORM,
    VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
    VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
    VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16_SFLOAT,
};

void VulkanCaps::setColorType(SkColorType colorType, std::initializer_list<VkFormat> formats) {
#ifdef SK_DEBUG
    for (size_t i = 0; i < kNumVkFormats; ++i) {
        const auto& formatInfo = fFormatTable[i];
        for (int j = 0; j < formatInfo.fColorTypeInfoCount; ++j) {
            const auto& ctInfo = formatInfo.fColorTypeInfos[j];
            if (ctInfo.fColorType == colorType &&
                !SkToBool(ctInfo.fFlags & ColorTypeInfo::kWrappedOnly_Flag)) {
                bool found = false;
                for (auto it = formats.begin(); it != formats.end(); ++it) {
                    if (kVkFormats[i] == *it) {
                        found = true;
                    }
                }
                SkASSERT(found);
            }
        }
    }
#endif
    int idx = static_cast<int>(colorType);
    for (auto it = formats.begin(); it != formats.end(); ++it) {
        const auto& info = this->getFormatInfo(*it);
        for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
            if (info.fColorTypeInfos[i].fColorType == colorType) {
                fColorTypeToFormatTable[idx] = *it;
                return;
            }
        }
    }
}

const VulkanCaps::FormatInfo& VulkanCaps::getFormatInfo(VkFormat format) const {
    static_assert(std::size(kVkFormats) == VulkanCaps::kNumVkFormats,
                    "Size of VkFormats array must match static value in header");

    for (size_t i = 0; i < std::size(kVkFormats); ++i) {
        if (kVkFormats[i] == format) {
            return fFormatTable[i];
        }
    }
    static FormatInfo kInvalidFormat;
    return kInvalidFormat;
}

} // namespace skgpu::graphite

