/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/TextureInfo.h"

namespace skgpu {

bool TextureInfo::operator==(const TextureInfo& that) {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }
    switch (fBackend) {
#ifdef SK_METAL
        case BackendApi::kMetal:
            return fMtlSpec == that.fMtlSpec;
#endif
        default:
            return false;
    }
}

} // namespace skgpu

