/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// TODO(https://crbug.com/356927699): If Rust `png` needs to be used from C++
// clients other than Skia, then we should move the FFI glue to
// `third_party/rust/png/v0_x/ffi`.
#ifndef SkRustPngFFI_DEFINED
#define SkRustPngFFI_DEFINED

#include <stddef.h>
#include <stdint.h>

// TODO(https://crbug.com/356698922): Use a real `#include` if possible.
namespace rust {
inline namespace cxxbridge1 {
template <typename T> class Slice;
}  // namespace cxxbridge1
}  // namespace rust

namespace rust_png {

// Implementing the abstract C++ class below gives a rough equivalent of
// `dyn std::io::Read` from Rust.
class ReadTrait {
public:
    // The `virtual` method below is a rough equivalent of the
    // `std::io::Read::read` method in Rust. See
    // https://doc.rust-lang.org/nightly/std/io/trait.Read.html#tymethod.read
    // for guidance on the desired implementation and behavior of this method.
    //
    // Note that (unlike in Rust) this method is infallible.  This aspect of the
    // design is used tentatively and based on the following considerations:
    // * `SkStream::read` is also infallible
    // * FFI based on the `cxx` crate doesn't currently support Rust's
    //   `std::result::Result`, `std::option::Option`, nor C++'s
    //   `std::optional` or `std::expected`.
    virtual size_t read(rust::Slice<uint8_t> buffer) = 0;

    // `drop` is not part of the `std::io::Read` trait, but we expose the
    // destructor through public API to make it easier to manage ownership and
    // lifetimes.  In particular, `png::Decoder::new` takes ownership of the
    // reader and therefore the FFI layer exposes the parameter type as
    // `std::unique_ptr<ReadTrait>`.
    virtual ~ReadTrait() = default;

    // This type is non-copyable and non-movable.
    ReadTrait(const ReadTrait&) = delete;
    ReadTrait(ReadTrait&&) = delete;
    ReadTrait& operator=(const ReadTrait&) = delete;
    ReadTrait& operator=(ReadTrait&&) = delete;

protected:
    ReadTrait() = default;
};

}  // namespace rust_png

#endif  // SkRustPngFFI_DEFINED
