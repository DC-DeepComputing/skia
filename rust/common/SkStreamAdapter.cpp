// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Common implementation of StreamAdapter.
// This file must be compiled with access to CXX bridge headers.

// Note: This file is compiled in the context of codec packages (e.g., PNG)
// which provide the necessary CXX bridge headers for rust::Slice.
#include "rust/png/FFI.rs.h"  // Provides CXX bridge types including rust::Slice
#include "rust/common/SkStreamAdapter.h"
#include "rust/common/SpanUtils.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkSafeMath.h"
#include <limits>

namespace rust {
namespace stream {


size_t StreamAdapter::read(rust::Slice<uint8_t> buffer) {
    SkASSERT_RELEASE(fStream);
    SkSpan<uint8_t> span = ToSkSpan(buffer);
    return fStream->read(span.data(), span.size());
}

bool StreamAdapter::seek_from_start(uint64_t requested_pos, uint64_t& final_pos) {
    SkASSERT_RELEASE(fStream);

    SkSafeMath safe;
    size_t pos = safe.castTo<size_t>(requested_pos);
    if (!safe.ok()) {
        return false;
    }

    if (!fStream->seek(pos)) {
        return false;
    }
    SkASSERT_RELEASE(!fStream->hasPosition() || fStream->getPosition() == requested_pos);

    // Assigning `size_t` to `uint64_t` doesn't need to go through
    // `SkSafeMath`, because `uint64_t` is never smaller than `size_t`.
    static_assert(sizeof(uint64_t) >= sizeof(size_t));
    final_pos = requested_pos;

    return true;
}

bool StreamAdapter::seek_from_end(int64_t requested_offset, uint64_t& final_pos) {
    SkASSERT_RELEASE(fStream);

    if (!fStream->hasLength()) {
        return false;
    }
    size_t length = fStream->getLength();

    SkSafeMath safe;
    uint64_t endPos = safe.castTo<uint64_t>(length);
    if (requested_offset > 0) {
        // IIUC `SkStream` doesn't support reading beyond the current length.
        return false;
    }
    if (requested_offset == std::numeric_limits<int64_t>::min()) {
        // `-requested_offset` below wouldn't work.
        return false;
    }
    uint64_t offset = safe.castTo<uint64_t>(-requested_offset);
    if (!safe.ok()) {
        return false;
    }
    if (offset > endPos) {
        // `endPos - offset` below wouldn't work.
        return false;
    }

    return this->seek_from_start(endPos - offset, final_pos);
}

bool StreamAdapter::seek_relative(int64_t requested_offset, uint64_t& final_pos) {
    SkASSERT_RELEASE(fStream);

    if (!fStream->hasPosition()) {
        return false;
    }

    SkSafeMath safe;
    long offset = safe.castTo<long>(requested_offset);
    if (!safe.ok()) {
        return false;
    }

    if (!fStream->move(offset)) {
        return false;
    }

    final_pos = safe.castTo<uint64_t>(fStream->getPosition());
    if (!safe.ok()) {
        return false;
    }
    return true;
}

}  // namespace stream
}  // namespace rust
