/*
 * Copyright 2023 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/MutableTextureState.h"

MutableTextureState(const MutableTextureState& that) {
    SkASSERT(!fIsValid || fBackend == that.fBackend);
    fBackend = that.fBackend;
    fIsValid = that.fIsValid;
    fStateData.reset();
    if (!that.fIsValid) {
        return;
    }

    switch (fBackend) {
        case BackendApi::kVulkan:
            that.fStateData->copyTo(fStateData);
            break;
        default:
            SK_ABORT("Unknown BackendApi");
    }
}

MutableTextureState& operator=(const MutableTextureState& that) {
    if (this != &that) {
        this->~MutableTextureState();
        new (this) MutableTextureState(that);
    }
    return *this;
}
