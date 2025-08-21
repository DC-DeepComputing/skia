/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/gpu/graphite/BackendTexturePriv.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"

#include <cstdint>

#if defined(SK_BUILD_FOR_ANDROID)
#include "include/android/AHardwareBufferUtils.h"
#include "include/core/SkColorType.h"

#include <optional>

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
// When building for the Android framework, there are formats defined outside of those publicly
// available in android/hardware_buffer.h.
#include <vndk/hardware_buffer.h>
#endif

#endif

namespace skgpu::graphite {

class VulkanBackendTextureData final : public BackendTextureData {
public:
    VulkanBackendTextureData(VulkanAlloc alloc,
                             sk_sp<skgpu::MutableTextureState> mts,
                             VkImage vImg
#if defined(SK_BUILD_FOR_ANDROID)
                             , std::optional<uint32_t> hwBufFormat
#endif
                            )
            : fMemoryAlloc(alloc)
            , fMutableState(mts)
            , fVkImage(vImg)
#if defined(SK_BUILD_FOR_ANDROID)
            , fAHwBufFormat(hwBufFormat)
#endif
            {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kVulkan; }
#endif

    VkImage image() const { return fVkImage; }
    VulkanAlloc memoryAllocator() const { return fMemoryAlloc; }
    sk_sp<skgpu::MutableTextureState> mutableState() const { return fMutableState; }

#if defined(SK_BUILD_FOR_ANDROID)
    SkColorType getSkColorType(bool usesExternalFormat) const {

#if __ANDROID_API__ >= 26
        // Determine the color type for the hardware buffer based on its format.
        //
        // In most cases, using `AHardwareBufferUtils::GetSkColorTypeFromBufferFormat` is
        // sufficient. However, it is possible that Skia determines that the AHwBuf should be
        // imported using an external format. In this case - even though we can never write to a
        // texture using an external format - Skia validity checks will expect the usage of
        // kExternalFormatColorType, so enforce that here.
        if (usesExternalFormat) {
            return AHardwareBufferUtils::kExternalFormatColorType;
        }

        if (fAHwBufFormat.has_value()) {
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
            // Check for a special case - the usage of AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM - when
            // building for the Android framework.
            //
            // Skia internal piping and validity checks rely upon SkSurfaces having a defined
            // SkColorType that is compatible with a texture's color information. b/431290055
            // exposed a case where we could end up using an SkColorType that is not compatible with
            // the BackendTexture's VkFormat.
            //
            // Essentially, the issue arose in a case where the driver may report that a certain
            // VkFormat can be used when importing the AHwBuf via
            // VkAndroidHardwareBufferFormatPropertiesANDROID.
            // When *creating* an SkSurface or SkImage from an AHwBuf-based BackendTexture, its
            // color type is determined by the AHwBuf format (using
            // `AHardwareBufferUtils::GetSkColorTypeFromBufferFormat`). However, when *validating*
            // those objects, Skia checks whether the color type is compatible with
            // the texture's VkFormat (which is a separate value from the AHwBuf format).
            //
            // Ideally, we would simply add the bug's AHwBuf format to
            // `GetSkColorTypeFromBufferFormat` and map to the expected SkColorType. However, this
            // utility is shared b/w ganesh and graphite, and adding this functionality introduced
            // complexity within ganesh. One such obstacle is that its
            // SkImages::DeferredFromAHardwareBuffer API does not know the AHwBuf format at
            // creation. While this could be worked around, the change would be fairly invasive for
            // minimal reward given that no clients have requested that we support this format in
            // ganesh.
            //
            // Therefore, simply manually check for this case within graphite.
            if (fAHwBufFormat.value() == AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM) {
                return SkColorType::kBGRA_8888_SkColorType;
            }
#endif // SK_BUILD_FOR_ANDROID_FRAMEWORK
            return AHardwareBufferUtils::GetSkColorTypeFromBufferFormat(fAHwBufFormat.value());
        }
#endif // __ANDROID_API__ >= 26

        return SkColorType::kUnknown_SkColorType;
    }
#endif // SK_BUILD_FOR_ANDROID

private:
    VulkanAlloc fMemoryAlloc;
    sk_sp<skgpu::MutableTextureState> fMutableState;
    VkImage fVkImage;

#if defined(SK_BUILD_FOR_ANDROID)
    std::optional<uint32_t> fAHwBufFormat;
#endif

    void copyTo(AnyBackendTextureData& dstData) const override {
        // Don't assert that dstData has a Vulkan type() because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<VulkanBackendTextureData>(fMemoryAlloc, fMutableState, fVkImage);
    }

    bool equal(const BackendTextureData* that) const override {
        SkASSERT(!that || that->type() == skgpu::BackendApi::kVulkan);
        if (auto otherVk = static_cast<const VulkanBackendTextureData*>(that)) {
            // We ignore the other two fields for the purpose of comparison.
            return fVkImage == otherVk->fVkImage;
        }
        return false;
    }
};

static const VulkanBackendTextureData* get_and_cast_data(const BackendTexture& tex) {
    auto data = BackendTexturePriv::GetData(tex);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kVulkan);
    return static_cast<const VulkanBackendTextureData*>(data);
}

static VulkanBackendTextureData* get_and_cast_data(BackendTexture* tex) {
    auto data = BackendTexturePriv::GetData(tex);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kVulkan);
    return static_cast<VulkanBackendTextureData*>(data);
}

namespace BackendTextures {
BackendTexture MakeVulkan(SkISize dimensions,
                          const VulkanTextureInfo& info,
                          VkImageLayout layout,
                          uint32_t queueFamilyIndex,
                          VkImage image,
                          VulkanAlloc vulkanMemoryAllocation
#if defined(SK_BUILD_FOR_ANDROID)
                          , std::optional<uint32_t> hwBufFormat
#endif
                        ) {
    return BackendTexturePriv::Make(
            dimensions,
            TextureInfos::MakeVulkan(info),
            VulkanBackendTextureData(
                    vulkanMemoryAllocation,
                    sk_make_sp<skgpu::MutableTextureState>(
                            skgpu::MutableTextureStates::MakeVulkan(layout, queueFamilyIndex)),
                    image
#if defined(SK_BUILD_FOR_ANDROID)
                    , hwBufFormat
#endif
                ));
}

VkImage GetVkImage(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kVulkan) {
        return VK_NULL_HANDLE;
    }
    const VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    return vkData->image();
}

VkImageLayout GetVkImageLayout(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kVulkan) {
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
    const VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    return skgpu::MutableTextureStates::GetVkImageLayout(vkData->mutableState().get());
}

uint32_t GetVkQueueFamilyIndex(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kVulkan) {
        return 0;
    }
    const VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    return skgpu::MutableTextureStates::GetVkQueueFamilyIndex(vkData->mutableState().get());
}

VulkanAlloc GetMemoryAlloc(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kVulkan) {
        return {};
    }
    const VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    return vkData->memoryAllocator();
}

sk_sp<skgpu::MutableTextureState> GetMutableState(const BackendTexture& tex) {
    if (!tex.isValid() || tex.backend() != skgpu::BackendApi::kVulkan) {
        return {};
    }
    const VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    return vkData->mutableState();
}

void SetMutableState(BackendTexture* tex, const skgpu::MutableTextureState& newState) {
    SkASSERT(tex);
    if (!tex->isValid() || tex->backend() != skgpu::BackendApi::kVulkan) {
        return;
    }
    VulkanBackendTextureData* vkData = get_and_cast_data(tex);
    SkASSERT(vkData);
    vkData->mutableState()->set(newState);
}

}  // namespace BackendTextures

}  // namespace skgpu::graphite
