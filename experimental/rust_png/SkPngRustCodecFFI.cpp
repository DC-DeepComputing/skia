/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/SkPngRustCodecFFI.h"

#include "include/core/SkSpan.h"

namespace ffi {

namespace {

// TODO(https://crbug.com/40278281): Consider moving this helper into
// `SkSpan_impl.h` / `.cpp` (behind a `ENABLE_RUST` or similar `#define`;
// maybe exposing it as a explicit conversion constructor).
template <typename T> SkSpan<T> ConvertToSkSpan(rust::Slice<T> slice) {
    // C++ and Rust represent empty spans/slices differently - we have to
    // explicitly check if `slice` is empty.  See also:
    // https://davidben.net/2024/01/15/empty-slices.html
    if (slice.empty()) {
        return SkSpan<T>();
    }
    return SkSpan<T>(slice.data(), slice.size());
}

}  // namespace

size_t SkStreamRead(SkStream& stream, rust::Slice<uint8_t> buffer) {
    SkSpan<uint8_t> span = ConvertToSkSpan(buffer);
    return stream.read(span.data(), span.size());
}

}  // namespace ffi
