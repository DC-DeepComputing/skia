/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/BackendTexture.h"

namespace skgpu {

BackendTexture::~BackendTexture() {
    this->cleanup();
}

void BackendTexture::cleanup() {
#ifdef SK_METAL
    if (this->isValid() && this->backend() == BackendApi::kMetal) {
        fMtlTexture.reset();
    }
#endif
}

BackendTexture::BackendTexture(const BackendTexture& that) {
    *this = that;
}

BackendTexture& BackendTexture::operator=(const BackendTexture& that) {
    bool valid = this->isValid();
    if (!that.isValid()) {
        this->cleanup();
        fInfo = {};
        return *this;
    } else if (valid && this->backend() != that.backend()) {
        this->cleanup();
        valid = false;
    }
    fDimensions = that.fDimensions;
    fInfo = that.fInfo;

    switch (that.backend()) {
#ifdef SK_METAL
        case BackendApi::kMetal:
            if (!valid) {
                // We may have garbage sitting in the fMtlTexture data of the union. We need to
                // make sure we don't try to call SkCFSafeRelease on the grabage sk_cfp when we do
                // the assignment operator below. Thus we call release first to null out the data.
                auto ignoredPtr SK_MAYBE_UNUSED = fMtlTexture.release();
            }
            fMtlTexture = that.fMtlTexture;
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
#ifdef SK_METAL
        case BackendApi::kMetal:
            if (fMtlTexture != that.fMtlTexture) {
                return false;
            }
            break;
#endif
        default:
            SK_ABORT("Unsupport Backend");
    }
    return true;
}

#ifdef SK_METAL
BackendTexture::BackendTexture(SkISize dimensions, sk_cfp<mtl::Handle> mtlTexture)
        : fDimensions(dimensions)
        , fInfo(mtl::TextureInfo(mtlTexture.get()))
        , fMtlTexture(std::move(mtlTexture)) {}

sk_cfp<mtl::Handle> BackendTexture::getMtlTexture() const {
    if (this->isValid() && this->backend() == BackendApi::kMetal) {
        return fMtlTexture;
    }
    return nullptr;
}

#endif

} // namespace skgpu

