/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendTexture.h"

namespace skgpu::graphite {

BackendTexture::~BackendTexture() {
#ifdef SK_DAWN
    if (this->backend() == BackendApi::kDawn) {
        // Only one of fDawnTexture and fDawnTextureView can be non null.
        SkASSERT(!(fDawnTexture && fDawnTextureView));
        // Release reference.
        fDawnTexture = nullptr;
        fDawnTextureView = nullptr;
    }
#endif
}

BackendTexture::BackendTexture(const BackendTexture& that) {
    *this = that;
}

BackendTexture& BackendTexture::operator=(const BackendTexture& that) {
    bool valid = this->isValid();
    if (!that.isValid()) {
        fInfo = {};
        return *this;
    } else if (valid && this->backend() != that.backend()) {
        valid = false;
    }
    fDimensions = that.fDimensions;
    fInfo = that.fInfo;

    switch (that.backend()) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            fDawnTexture     = that.fDawnTexture;
            fDawnTextureView = that.fDawnTextureView;
            break;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            fMtlTexture = that.fMtlTexture;
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
    return *this;
}

bool BackendTexture::operator==(const BackendTexture& that) const {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fDimensions != that.fDimensions || fInfo != that.fInfo) {
        return false;
    }

    switch (that.backend()) {
#ifdef SK_DAWN
        case BackendApi::kDawn:
            if (fDawnTexture.Get() != that.fDawnTexture.Get()) {
                return false;
            }
            if (fDawnTextureView.Get() != that.fDawnTextureView.Get()) {
                return false;
            }
            break;
#endif
#ifdef SK_METAL
        case BackendApi::kMetal:
            if (fMtlTexture != that.fMtlTexture) {
                return false;
            }
            break;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            // TODO: Actually fill this out
            return false;
#endif
        default:
            SK_ABORT("Unsupport Backend");
    }
    return true;
}

#ifdef SK_DAWN
BackendTexture::BackendTexture(wgpu::Texture texture)
    : fDimensions{static_cast<int32_t>(texture.GetWidth()),
                  static_cast<int32_t>(texture.GetHeight())}
    , fInfo(DawnTextureInfo(texture)) {}

BackendTexture::BackendTexture(SkISize dimensions,
                               wgpu::TextureView textureView,
                               const DawnTextureInfo& info)
        : fDimensions(dimensions)
        , fInfo(info)
        , fDawnTextureView(std::move(textureView)) {}

wgpu::Texture BackendTexture::getDawnTexture() const {
    if (this->isValid() && this->backend() == BackendApi::kDawn) {
        return fDawnTexture;
    }
    return {};
}

wgpu::TextureView BackendTexture::getDawnTextureView() const {
    if (this->isValid() && this->backend() == BackendApi::kDawn) {
        return fDawnTextureView;
    }
    return {};
}
#endif

#ifdef SK_METAL
BackendTexture::BackendTexture(SkISize dimensions, MtlHandle mtlTexture)
        : fDimensions(dimensions)
        , fInfo(MtlTextureInfo(mtlTexture))
        , fMtlTexture(mtlTexture) {}

MtlHandle BackendTexture::getMtlTexture() const {
    if (this->isValid() && this->backend() == BackendApi::kMetal) {
        return fMtlTexture;
    }
    return nullptr;
}

#endif

} // namespace skgpu::graphite

