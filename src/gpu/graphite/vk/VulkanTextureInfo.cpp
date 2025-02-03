/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#include <cstdint>

namespace skgpu::graphite {

// namespace TextureInfos {
// VulkanTextureSpec GetVulkanTextureSpec(const TextureInfo&);
// VkFormat GetVkFormat(const TextureInfo&);
// VkImageUsageFlags GetVkUsageFlags(const TextureInfo&);
// VulkanYcbcrConversionInfo GetVulkanYcbcrConversionInfo(const TextureInfo&);
// }  // namespace TextureInfos

size_t VulkanTextureInfo::bytesPerPixel() const { return VkFormatBytesPerBlock(fFormat); }

SkTextureCompressionType VulkanTextureInfo::compressionType() const {
    return VkFormatToCompressionType(fFormat);
}

bool VulkanTextureInfo::isMemoryless() const {
    return fImageUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
}

SkString VulkanTextureInfo::toBackendString() const {
    return SkStringPrintf(
            "flags=0x%08X,format=%d,imageTiling=%d,usageFlags=0x%08X,sharingMode=%d,aspectMask=%u",
            fFlags,
            fFormat,
            fImageTiling,
            fImageUsageFlags,
            fSharingMode,
            fAspectMask);
}

bool VulkanTextureInfo::equals(const TextureInfo& that) const {
    const auto& vt = TextureInfoPriv::Get<VulkanTextureInfo>(that);
    return fFlags == vt.fFlags &&
           fFormat == vt.fFormat &&
           fImageTiling == vt.fImageTiling &&
           fImageUsageFlags == vt.fImageUsageFlags &&
           fSharingMode == vt.fSharingMode &&
           fAspectMask == vt.fAspectMask &&
           fYcbcrConversionInfo == vt.fYcbcrConversionInfo;
}

bool VulkanTextureInfo::isCompatible(const TextureInfo& that) const {
    const auto& vt = TextureInfoPriv::Get<VulkanTextureInfo>(that);

    // The usages may match or the usage passed in may be a superset of the usage stored within.
    return fFlags == vt.fFlags &&
           fFormat == vt.fFormat &&
           fImageTiling == vt.fImageTiling &&
           fSharingMode == vt.fSharingMode &&
           fAspectMask == vt.fAspectMask &&
           (fImageUsageFlags & vt.fImageUsageFlags) == fImageUsageFlags &&
           fYcbcrConversionInfo == vt.fYcbcrConversionInfo;
}

namespace TextureInfos {

TextureInfo MakeVulkan(const VulkanTextureInfo& vkInfo) {
    return TextureInfoPriv::Make(vkInfo);
}

bool GetVulkanTextureInfo(const TextureInfo& info, VulkanTextureInfo* out) {
    return TextureInfoPriv::Copy(info, out);
}

}  // namespace TextureInfos

}  // namespace skgpu::graphite
