/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/image/GrBitmap.h"

namespace skgpu::ganesh {

std::optional<Bitmap> Bitmap::Make(SkImageInfo ii, const void* pixels, size_t rowBytes, ReleaseProc proc, void* context) {
    SkBitmap bm;
    if (!bm.installPixels(ii, const_cast<void*>(pixels), rowBytes, proc, context)) {
        return {};
    }
    bm.setImmutable();
    return Bitmap(bm);
}

}  // namespace skgpu::ganesh