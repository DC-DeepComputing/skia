/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendSemaphore.h"

namespace skgpu::graphite {

BackendSemaphore::BackendSemaphore() = default;

BackendSemaphore::~BackendSemaphore() = default;

BackendSemaphore::BackendSemaphore(const BackendSemaphore& that) {
    *this = that;
}

BackendSemaphore& BackendSemaphore::operator=(const BackendSemaphore& that) {
    bool valid = this->isValid();
    if (!that.isValid()) {
        fIsValid = false;
        return *this;
    } else if (valid && this->backend() != that.backend()) {
        valid = false;
    }
    fIsValid = that.fIsValid;
    fBackend = that.fBackend;

    switch (that.backend()) {
#ifdef SK_METAL
        case BackendApi::kMetal:
            fMtlEvent = that.fMtlEvent;
            fMtlValue = that.fMtlValue;
            break;
#endif
#ifdef SK_VULKAN
        case BackendApi::kVulkan:
            fVkSemaphore = that.fVkSemaphore;
            break;
#endif
        default:
            SK_ABORT("Unsupported Backend");
    }
    return *this;
}

}  // End of namespace skgpu::graphite
