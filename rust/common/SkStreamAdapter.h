// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SkStreamAdapter_DEFINED
#define SkStreamAdapter_DEFINED

#include "include/core/SkStream.h"
#include "include/private/base/SkAssert.h"

// Forward declaration of rust::Slice from CXX generated code
namespace rust {
inline namespace cxxbridge1 {
template<typename T> class Slice;
}
}

namespace rust {
namespace stream {

// Adapter class for reading and seeking Skia SkStreams from Rust
class StreamAdapter {
public:
    explicit StreamAdapter(SkStream* stream) : fStream(stream) {
        SkASSERT_RELEASE(fStream);
    }

    virtual ~StreamAdapter() = default;

    // Non-copyable and non-movable (we want a stable `this` pointer)
    StreamAdapter(const StreamAdapter&) = delete;
    StreamAdapter& operator=(const StreamAdapter&) = delete;
    StreamAdapter(StreamAdapter&&) = delete;
    StreamAdapter& operator=(StreamAdapter&&) = delete;

    // Read from stream into provided buffer. Returns number of bytes actually read.
    virtual size_t read(rust::Slice<uint8_t> buffer);

    // Seek operations. Return true on success and set final_pos to actual position.
    virtual bool seek_from_start(uint64_t requested_pos, uint64_t& final_pos);
    virtual bool seek_from_end(int64_t requested_offset, uint64_t& final_pos);
    virtual bool seek_relative(int64_t requested_offset, uint64_t& final_pos);

private:
    SkStream* fStream;
};

}  // namespace stream
}  // namespace rust

#endif  // SkStreamAdapter_DEFINED
