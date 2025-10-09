/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRustBmpFFI_DEFINED
#define SkRustBmpFFI_DEFINED

#include <stddef.h>
#include <stdint.h>
#include "experimental/rust_bmp/ffi/UtilsForFFI.h"

// TODO(https://crbug.com/356698922): Use a real `#include` if possible.
namespace rust {
inline namespace cxxbridge1 {
template <typename T> class Slice;
}  // namespace cxxbridge1
}  // namespace rust

namespace rust_bmp {

// Implementing the abstract C++ class below gives a rough equivalent of
// `dyn std::io::Read + std::io::Seek` from Rust.
//
// This class provides the interface for reading BMP data from various sources
// (files, streams, memory buffers, etc.) in a way that's compatible with
// Skia's SkStream interface.
class ReadAndSeekTraits {
public:
    // The `virtual` method below is a rough equivalent of the
    // `std::io::Read::read` method in Rust. See
    // https://doc.rust-lang.org/nightly/std/io/trait.Read.html#tymethod.read
    // for guidance on the desired implementation and behavior of this method.
    //
    // Note that (unlike in Rust) this method is infallible.  This aspect of the
    // design is used tentatively and based on the following considerations:
    // * `SkStream::read` is also infallible
    // * `SkStream::read` communicates no-more-input by reporting that 0 bytes
    //   have been read (just like Rust's `Read` trait), and this condition
    //   corresponds to the only IO error covered by `SkCodec::Result`:
    //   `kIncompleteInput`.
    virtual bool read(rust::Slice<uint8_t> buffer) = 0;

    // The `virtual` method below is a rough equivalent of the
    // `std::io::Seek::seek_from_start` method in Rust.
    // Should return `true` when successful, and `false` upon failure.
    virtual bool seek_to_start() = 0;

    // The `virtual` method below allows seeking to a specific position.
    // Should return `true` when successful, and `false` upon failure.
    // When successful, should set `final_pos` to the actual position reached.
    virtual bool seek_to_position(uint64_t requested_pos, uint64_t& final_pos) = 0;

    // `drop` is not part of the `std::io::Read` nor `std::io::Seek` trait, but
    // we expose the destructor through public API to make it possible to pass
    // `std::unique_ptr<ReadAndSeekTraits>` over the FFI boundary.
    virtual ~ReadAndSeekTraits() = default;

    // This type is non-copyable and non-movable.
    ReadAndSeekTraits(const ReadAndSeekTraits&) = delete;
    ReadAndSeekTraits(ReadAndSeekTraits&&) = delete;
    ReadAndSeekTraits& operator=(const ReadAndSeekTraits&) = delete;
    ReadAndSeekTraits& operator=(ReadAndSeekTraits&&) = delete;

protected:
    ReadAndSeekTraits() = default;
};

}  // namespace rust_bmp

#endif  // SkRustBmpFFI_DEFINED
