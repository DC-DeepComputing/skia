/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkPngDetection.h"

#include <string.h>

namespace SkPngDetection {

bool IsPng(const void* buff, size_t bytesRead) {
    // `pngSignature` has been taken from
    // https://www.w3.org/TR/png-3/#3PNGsignature
    static constexpr unsigned char pngSignature[] = {
            0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    if (bytesRead < sizeof(pngSignature)) {
        return false;
    }

    return memcmp(buff, pngSignature, sizeof(pngSignature)) == 0;
}

}  // namespace SkPngDetection
