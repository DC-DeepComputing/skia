/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// TODO(https://crbug.com/40278281): Once the FFI API matures we should move the
// `experimental/rust_png/ffi` directory to `third_party/rust/png/v0_x/ffi`.
// This directory is temporarily in Skia to avoid having to coordinate changes
// across Skia and Chromium repos.
#ifndef SkRustPngFFI_DEFINED
#define SkRustPngFFI_DEFINED

#include <stddef.h>
#include <stdint.h>

#include "third_party/rust/cxx/v1/cxx.h"

namespace rust_png {

// Implementing the abstract interface below gives a rough equivalent of
// `dyn std::io::Read` from Rust.
class ReadTrait {
public:
    // This is a rough equivalent of the `std::io::Read::read` method in Rust.
    // See https://doc.rust-lang.org/nightly/std/io/trait.Read.html#tymethod.read
    // for guidance on the desired implementation and behavior of this method.
    //
    // Note that (unlike in Rust) this method is infallible.  This aspect of the
    // design is used tentatively and based on the following considerations:
    // * `SkStream::read` is also infallible
    // * FFI based on the `cxx` crate doesn't currently support Rust's
    //   `std::result::Result`, `std::option::Option`, nor C++'s
    //   `std::optional`.
    virtual size_t read(rust::Slice<uint8_t> buffer) = 0;

protected:
    // The destructor is *not* part of this abstract interface and therefore it is
    // non-`virtual` and `protected`.
    ReadTrait() {}
};

}  // namespace rust_png

#endif  // SkRustPngFFI_DEFINED
