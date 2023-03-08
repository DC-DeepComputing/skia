/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/TextureInfo.h"

#ifdef SK_VULKAN
#include "src/gpu/vk/VulkanUtilsPriv.h"
#endif
#ifdef SK_METAL
#include "src/gpu/mtl/MtlCppUtilsPriv.h"
#endif

namespace skgpu::graphite {

TextureInfo& TextureInfo::operator=(const TextureInfo& that) {
    if (!that.isValid()) {
        fValid = false;
        return *this;
    }
    fBackend = that.fBackend;
    fSampleCount = that.fSampleCount;
    fMipmapped = that.fMipmapped;
    fProtected = that.fProtected;

    switch (that.backend()) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            fDawnSpec = that.fDawnSpec;
            break;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            fMtlSpec = that.fMtlSpec;
            break;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            // TODO: Actually fill this out
            break;
#endif
        default:
            SK_ABORT("Unsupport Backend");
    }

    fValid = true;
    return *this;
}

bool TextureInfo::operator==(const TextureInfo& that) const {
    if (!this->isValid() && !that.isValid()) {
        return true;
    }
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }

    if (fSampleCount != that.fSampleCount ||
        fMipmapped != that.fMipmapped ||
        fProtected != that.fProtected) {
        return false;
    }

    switch (fBackend) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            return fDawnSpec == that.fDawnSpec;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            return fMtlSpec == that.fMtlSpec;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            // TODO: Actually fill this out
            return false;
#endif
        default:
            return false;
    }
}

uint32_t TextureInfo::channelMask() const {
    if (!this->isValid()) {
        return 0;
    }
    switch (fBackend) {
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            return skgpu::VkFormatChannels(fVkSpec.fFormat);
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            return skgpu::MtlFormatChannels(fMtlSpec.fFormat);
#endif
#ifdef SK_DAWN
        case BackendApi::kDawn:
            // TODO: Move GrDawnFormatChannels() to shared loc and use
            return 0;
#endif

        default:
            return 0;
    }
}

} // namespace skgpu::graphite

