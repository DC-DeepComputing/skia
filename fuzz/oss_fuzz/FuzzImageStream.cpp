/*
 * Copyright 2026 Microsoft Corporation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// The other image fuzzers decode from an SkMemoryStream, which codecs can read in place. That
// means they never run the code that first copies a stream into memory, or what happens when a
// stream delivers fewer bytes than it reported. This fuzzer decodes from a stream that is not
// backed by memory, and whose capabilities and reported length are controlled by the input.

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace {

class FuzzedStream : public SkStream {
public:
    FuzzedStream(
            const uint8_t* data, size_t size, bool hasPosition, bool hasLength, size_t extraLength)
            : fStream(data, size)
            , fHasPosition(hasPosition)
            , fHasLength(hasLength)
            , fExtraLength(extraLength) {}

    bool hasPosition() const override { return fHasPosition; }
    size_t getPosition() const override { return fStream.getPosition(); }
    bool hasLength() const override { return fHasLength; }
    // The length can be larger than the data, so reading all of it comes up short.
    size_t getLength() const override { return fStream.getLength() + fExtraLength; }
    size_t peek(void* buf, size_t bytes) const override { return fStream.peek(buf, bytes); }
    size_t read(void* buf, size_t bytes) override { return fStream.read(buf, bytes); }
    bool rewind() override { return fStream.rewind(); }
    bool isAtEnd() const override { return fStream.isAtEnd(); }
    bool seek(size_t position) override { return fHasPosition && fStream.seek(position); }
    bool move(long offset) override { return fHasPosition && fStream.move(offset); }

private:
    SkMemoryStream fStream;
    const bool fHasPosition;
    const bool fHasLength;
    const size_t fExtraLength;
};

}  // namespace

// The first byte selects how the stream behaves, the rest is the image.
bool FuzzImageDecodeStream(const uint8_t* data, size_t size) {
    if (size < 1) {
        return false;
    }
    const uint8_t mode = data[0];
    data++;
    size--;

    const bool hasPosition = mode & 0x01;
    const bool hasLength = mode & 0x02;
    const auto policy = (mode & 0x04) ? SkCodec::SelectionPolicy::kPreferAnimation
                                      : SkCodec::SelectionPolicy::kPreferStillImage;
    const size_t extraLength = hasLength ? (mode >> 3) : 0;

    SkCodec::Result result;
    auto codec = SkCodec::MakeFromStream(
            std::make_unique<FuzzedStream>(data, size, hasPosition, hasLength, extraLength),
            &result,
            /*chunkReader=*/nullptr,
            policy);
    if (!codec) {
        return false;
    }

    SkBitmap bm;
    if (!bm.tryAllocPixels(codec->getInfo())) {
        // May fail in memory-constrained fuzzing environments
        return false;
    }

    result = codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes());
    return result == SkCodec::kSuccess || result == SkCodec::kIncompleteInput ||
           result == SkCodec::kErrorInInput;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size > 10240) {
        return 0;
    }
    FuzzImageDecodeStream(data, size);
    return 0;
}
#endif
