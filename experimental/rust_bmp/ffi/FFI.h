/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRustBmpFFI_DEFINED
#define SkRustBmpFFI_DEFINED

// This header is a placeholder for future FFI extensions.
// Currently, all embedded format decoding is handled by pure Rust
// implementations (zune-jpeg, png crate) for Rule of 2 compliance.
//
// Future: When Rust implementations of SkJpegCodec/SkPngCodec are available,
// processor delegation interfaces can be added here.

namespace rust_bmp {
// Reserved for future processor interfaces
}  // namespace rust_bmp

#endif  // SkRustBmpFFI_DEFINED
