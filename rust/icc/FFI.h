/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRustIccFFI_DEFINED
#define SkRustIccFFI_DEFINED

#include <stddef.h>
#include <stdint.h>

// TODO(https://crbug.com/356698922): Use a real `#include` if possible.
namespace rust {
inline namespace cxxbridge1 {
template <typename T> class Slice;
}  // namespace cxxbridge1
}  // namespace rust

namespace rust_icc {
// This header provides forward declarations for types used in the
// CXX bridge. The actual implementations are in the generated FFI.rs.h
}  // namespace rust_icc

#endif  // SkRustIccFFI_DEFINED
