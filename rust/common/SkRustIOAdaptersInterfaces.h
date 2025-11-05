// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SkRustIOAdaptersInterfaces_DEFINED
#define SkRustIOAdaptersInterfaces_DEFINED

// Forward declarations for FFI boundary.
// The actual implementation is in SkRustIOAdapters.h/cpp.
// The CXX-generated code will provide the full rust:: namespace types.

namespace rust {
namespace stream {

// StreamAdapter provides Read + Seek operations for Rust codecs.
// This type is shared across codec implementations (PNG, BMP, etc.)
// via CXX's extern type unification mechanism.
class StreamAdapter;

// WriteAdapter provides Write + Flush operations for Rust codec encoders.
// This type is shared across encoder implementations (PNG, JPEG, etc.)
// via CXX's extern type unification mechanism.
class WriteAdapter;

}  // namespace stream
}  // namespace rust

#endif  // SkRustIOAdaptersInterfaces_DEFINED
