/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"

#include <cstdint>

namespace skgpu::graphite {

class VulkanTextureInfoData final : public TextureInfoData {
public:
    VulkanTextureInfoData(MtlTextureSpec m) : fVkSpec(m) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kMetal; }
#endif

    VulkanTextureSpec spec() const { return fVkSpec; }

private:
    VulkanTextureSpec fVkSpec;

    size_t bytesPerPixel() const override {
        return VkFormatBytesPerBlock(static_cast<MTLPixelFormat>(fVkSpec.fFormat));
    }

    SkTextureCompressionType compressionType() const override {
        return VkFormatToCompressionType(static_cast<MTLPixelFormat>(fVkSpec.fFormat));
    }

    SkString toString() const override {
        return SkStringPrintf("Vulkan(%s,", fVkSpec.toString().c_str());
    }

    SkString toRPAttachmentString(uint32_t sampleCount) const override {
        return SkStringPrintf(
                "Vulkan(f=%u,s=%u)", static_cast<unsigned int>(fVkSpec.fFormat), sampleCount);
    }

    void copyTo(AnyTextureInfoData& dstData) const override {
        // Don't assert that dstData is a metal type because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<VulkanTextureInfoData>(fVkSpec);
    }

    bool equal(const TextureInfoData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kMetal);
        if (auto otherVk = static_cast<const VulkanTextureInfoData*>(that)) {
            return fVkSpec == otherVk->fVkSpec;
        }
        return false;
    }

    bool isCompatible(const TextureInfoData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kMetal);
        if (auto otherVk = static_cast<const VulkanTextureInfoData*>(that)) {
            return fVkSpec.isCompatible(otherVk->fVkSpec);
        }
        return false;
    }
};

static const VulkanTextureInfoData* get_and_cast_data(const TextureInfo& info) {
    auto data = TextureInfoPriv::GetData(info);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kMetal);
    return static_cast<const VulkanTextureInfoData*>(data);
}

namespace TextureInfos {
skgpu::graphite::TextureInfo MakeMetal(CFTypeRef mtlTexture) {
    return MakeMetal(VulkanTextureInfo(mtlTexture));
}

skgpu::graphite::TextureInfo MakeMetal(const VulkanTextureInfo& vkInfo) {
    skgpu::Protected p = Protected::kNo;
    if (vkInfo.fFlags & VK_IMAGE_CREATE_PROTECTED_BIT) {
        p = Protected::kYes;
    }
    return TextureInfoPriv::Make(skgpu::BackendApi::kVulkan,
                                 vkInfo.fSampleCount,
                                 vkInfo.fMipmapped,
                                 p,
                                 VulkanTextureInfoData(vkInfo));
}

bool GetVulkanTextureInfo(const TextureInfo& info, VulkanTextureInfo* out) {
    if (!info.isValid() || info.backend() != skgpu::BackendApi::kMetal) {
        return false;
    }
    SkASSERT(out);
    const VulkanTextureInfoData* mtlData = get_and_cast_data(info);
    SkASSERT(mtlData);
    *out = MtlTextureSpecToTextureInfo(mtlData->spec(), info.numSamples(), info.mipmapped());
    return true;
}

// This cannot return a const reference or we get a warning about returning
// a reference to a temporary local variable.
MtlTextureSpec GetMtlTextureSpec(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kMetal);
    const VulkanTextureInfoData* mtlData = get_and_cast_data(info);
    SkASSERT(mtlData);
    return mtlData->spec();
}

MTLPixelFormat GetMTLPixelFormat(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kMetal);
    const VulkanTextureInfoData* mtlData = get_and_cast_data(info);
    SkASSERT(mtlData);
    return mtlData->spec().fFormat;
}

MTLTextureUsage GetMTLTextureUsage(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kMetal);
    const VulkanTextureInfoData* mtlData = get_and_cast_data(info);
    SkASSERT(mtlData);
    return mtlData->spec().fUsage;
}

bool GetMtlFramebufferOnly(const TextureInfo& info) {
    SkASSERT(info.isValid() && info.backend() == skgpu::BackendApi::kMetal);
    const VulkanTextureInfoData* mtlData = get_and_cast_data(info);
    SkASSERT(mtlData);
    return mtlData->spec().fFramebufferOnly;
}

}  // namespace TextureInfos

#if defined(SK_METAL) && !defined(SK_DISABLE_LEGACY_TEXTURE_INFO_FUNCS)
TextureInfo::TextureInfo(const VulkanTextureInfo& mtlInfo)
        : fBackend(BackendApi::kMetal)
        , fValid(true)
        , fSampleCount(mtlInfo.fSampleCount)
        , fMipmapped(mtlInfo.fMipmapped)
        , fProtected(Protected::kNo) {
    fTextureInfoData.emplace<VulkanTextureInfoData>(VulkanTextureInfoData(mtlInfo));
}
#endif

}  // namespace skgpu::graphite
