/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPngRustCodecFFI_DEFINED
#define SkPngRustCodecFFI_DEFINED

#include <memory>

#include "include/codec/SkCodec.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "third_party/rust/cxx/v1/cxx.h"

namespace ffi {

// Type aliases below work around inability of `cxx`-based FFI to provide
// bindings for nested types.
using SkEncodedInfoColor = SkEncodedInfo::Color;

// An FFI-friendly wrapper around `SkStream::read`.
//
// Arguments against directly calling `SkStream::read` from Rust:
//
// * C++ and Rust represent empty spans/slices differently - naive
//   passing of the data pointer may run into issues.  See also:
//   https://davidben.net/2024/01/15/empty-slices.html
// * The `cxx` crate does not currently support passing `void*`.
//   See: https://github.com/dtolnay/cxx/issues/1049
//
// TODO(https://crbug.com/40278281): Consider removing this helper and replacing
// it with `SkStream::read` overload that takes `rust::Slice<uint8_t>` (behind a
// `ENABLE_RUST` or similar `#define`).
using SkStream = ::SkStream;
size_t SkStreamRead(SkStream& stream, rust::Slice<uint8_t> buffer);

}  // namespace ffi

#endif  // SkPngRustCodecFFI_DEFINED
