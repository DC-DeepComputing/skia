/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngDetection_DEFINED
#define SkPngDetection_DEFINED

#include <stddef.h>
#include <stdint.h>

#include "include/core/SkSpan.h"

namespace SkPngDetection {

// Returns `true` if the first `bytesRead` of `buff` indicate that the buffer
// contains a PNG image.
//
// This is implemented in a private, separate compilation unit so that this
// doesn't depend on `libpng` nor Rust PNG (it only depends on the standard
// library).  In particular, `IsPng` needs to be used from `SkIcoCodec` without
// having a hardcoded/direct dependency on `SkPngDecoder` nor
// `SkPngRustDecoder`.
//
// This takes `void*` and `size_t` (rather than a `SkSpan`) for compatibility
// with `IsFormatCallback` from the `SkCodecs::Decoder` struct.
bool IsPng(const void* buff, size_t bytesRead);

// Convenience helper for using `SkSpan` as an argument.
inline bool IsPng(SkSpan<const uint8_t> buff) {
    return IsPng(buff.data(), buff.size());
}

}  // namespace SkPngDetection

#endif  // SkPngDetection_DEFINED
