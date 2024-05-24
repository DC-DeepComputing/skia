/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStreamPriv_DEFINED
#define SkStreamPriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "src/base/SkEndian.h"

#include <cstdint>

class SkData;

/**
 *  Copy the provided stream to an SkData variable.
 *
 *  Note: Assumes the stream is at the beginning. If it has a length,
 *  but is not at the beginning, this call will fail (return NULL).
 *
 *  @param stream SkStream to be copied into data.
 *  @return The resulting SkData after the copy, nullptr on failure.
 */
sk_sp<SkData> SkCopyStreamToData(SkStream* stream);

/**
 *  Copies the input stream from the current position to the end.
 *  Does not rewind the input stream.
 */
bool SkStreamCopy(SkWStream* out, SkStream* input);

/** A SkWStream that writes all output to SkDebugf, for debugging purposes. */
class SkDebugfStream final : public SkWStream {
public:
    bool write(const void* buffer, size_t size) override;
    size_t bytesWritten() const override;

private:
    size_t fBytesWritten = 0;
};

// A SkDynamicMemoryWStream that provides helper functions for writing big-endian data.
class SkDynamicMemoryEndianWStream final : public SkDynamicMemoryWStream {
public:
    // Write the specified value as big-endian.
    bool writeU16BE(uint16_t value) { return write16(SkEndian_SwapBE16(value)); }
    bool writeU32BE(uint32_t value) { return write32(SkEndian_SwapBE32(value)); }
    bool writeS32BE(int32_t value) { return write32(SkEndian_SwapBE32(value)); }
};

// A SkMemoryStream that provides helper functions for reading big-endian data, or
// data with parameterized endian-ness.
class SkMemoryEndianStream final : public SkMemoryStream {
public:
    SkMemoryEndianStream(sk_sp<SkData> data) : SkMemoryStream(std::move(data)) {}
    SkMemoryEndianStream(const void* data, size_t size, bool copyData = false)
            : SkMemoryStream(data, size, copyData) {}

    [[nodiscard]] bool readU16(uint16_t* value, bool littleEndian) {
        if (SkMemoryStream::readU16(value)) {
            *value = littleEndian ? SkEndian_SwapLE16(*value) : SkEndian_SwapBE16(*value);
            return true;
        }
        return false;
    }
    [[nodiscard]] bool readU32(uint32_t* value, bool littleEndian) {
        if (SkMemoryStream::readU32(value)) {
            *value = littleEndian ? SkEndian_SwapLE32(*value) : SkEndian_SwapBE32(*value);
            return true;
        }
        return false;
    }
    [[nodiscard]] bool readS32(int32_t* value, bool littleEndian) {
        if (SkMemoryStream::readS32(value)) {
            *value = littleEndian ? SkEndian_SwapLE32(*value) : SkEndian_SwapBE32(*value);
            return true;
        }
        return false;
    }

    [[nodiscard]] bool readU16BE(uint16_t* value) { return readU16(value, false); }
    [[nodiscard]] bool readU32BE(uint32_t* value) { return readU32(value, false); }
    [[nodiscard]] bool readS32BE(int32_t* value) { return readS32(value, false); }
};

// If the stream supports identifying the current position and total length, this returns
// true if there are not enough bytes in the stream to fulfill a read of the given length.
// Otherwise, it returns false.
// False does *not* mean a read will succeed of the given length, but true means we are
// certain it will fail.
bool StreamRemainingLengthIsBelow(SkStream* stream, size_t len);

#endif  // SkStreamPriv_DEFINED
