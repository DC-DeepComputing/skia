/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegSourceMgr.h"

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegSegmentScan.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkStream helpers.

/*
 * Class that will will rewind an SkStream, and then restore it to its original position when it
 * goes out of scope. If the SkStream is not seekable, then the stream will not be altered at all,
 * and will return false from canRestore.
 */

class ScopedSkStreamRestorer {
public:
    ScopedSkStreamRestorer(SkStream* stream)
            : fStream(stream), fPosition(stream->hasPosition() ? stream->getPosition() : 0) {
        if (canRestore()) {
            if (!fStream->rewind()) {
                SkCodecPrintf("Failed to rewind decoder stream.\n");
            }
        }
    }
    ~ScopedSkStreamRestorer() {
        if (canRestore()) {
            if (!fStream->seek(fPosition)) {
                SkCodecPrintf("Failed to restore decoder stream.\n");
            }
        }
    }
    bool canRestore() const { return fStream->hasPosition(); }

private:
    SkStream* const fStream;
    const size_t fPosition;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegMemorySourceMgr

class SkJpegMemorySourceMgr : public SkJpegSourceMgr {
public:
    SkJpegMemorySourceMgr(SkStream* stream) : fStream(stream) {}
    ~SkJpegMemorySourceMgr() override {}

    void initSource(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
        nextInputByte = reinterpret_cast<const uint8_t*>(fStream->getMemoryBase());
        bytesInBuffer = static_cast<size_t>(fStream->getLength());
    }
    bool fillInputBuffer(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
        // The whole JPEG data is expected to reside in the supplied memory buffer, so any request
        // for more data beyond the given buffer size is treated as an error.
        SkCodecPrintf("Asked to re-fill a memory-mapped stream.\n");
        return false;
    }
    bool skipInputBytes(size_t bytesToSkip,
                        const uint8_t*& nextInputByte,
                        size_t& bytesInBuffer) override {
        if (bytesToSkip > bytesInBuffer) {
            SkCodecPrintf("Asked to read past end of a memory-mapped stream.\n");
            return false;
        }
        nextInputByte += bytesToSkip;
        bytesInBuffer -= bytesToSkip;
        return true;
    }
    const std::vector<SkJpegSegment>& getAllSegments() override {
        if (!fScanner) {
            fScanner =
                    std::make_unique<SkJpegSegmentScanner>(SkJpegSegmentScanner::kMarkerEndOfImage);
            fScanner->onBytes(fStream->getMemoryBase(), fStream->getLength());
        }
        return fScanner->getSegments();
    }
    std::unique_ptr<SkStream> getSubsetStream(size_t offset, size_t size) override {
        if (offset > fStream->getLength() || size > fStream->getLength() - offset) {
            return nullptr;
        }
        return SkMemoryStream::MakeCopy(
                reinterpret_cast<const uint8_t*>(fStream->getMemoryBase()) + offset, size);
    }

private:
    SkStream* const fStream;  // unowned.
    std::unique_ptr<SkJpegSegmentScanner> fScanner;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegBufferedSourceMgr

class SkJpegBufferedSourceMgr : public SkJpegSourceMgr {
public:
    SkJpegBufferedSourceMgr(SkStream* stream) : fStream(stream) {}
    ~SkJpegBufferedSourceMgr() override {}

    void initSource(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
        constexpr size_t kBufferSize = 1024;
        fBuffer = SkData::MakeUninitialized(kBufferSize);
        nextInputByte = fBuffer->bytes();
        bytesInBuffer = 0;
    }
    bool fillInputBuffer(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
        size_t bytesRead = fStream->read(fBuffer->writable_data(), fBuffer->size());
        if (bytesRead == 0) {
            // Fail if we read zero bytes (libjpeg will accept any non-zero number of bytes).
            SkCodecPrintf("Hit end of file reading a buffered stream.\n");
            return false;
        }
        nextInputByte = fBuffer->bytes();
        bytesInBuffer = bytesRead;
        return true;
    }
    bool skipInputBytes(size_t bytesToSkip,
                        const uint8_t*& nextInputByte,
                        size_t& bytesInBuffer) override {
        // Skip through the already-read (or already in memory) buffer.
        if (bytesToSkip <= bytesInBuffer) {
            nextInputByte += bytesToSkip;
            bytesInBuffer -= bytesToSkip;
            return true;
        }
        bytesToSkip -= bytesInBuffer;

        // Fail if we skip past the end of the stream.
        if (fStream->skip(bytesToSkip) != bytesToSkip) {
            SkCodecPrintf("Failed to skip through buffered stream.\n");
            return false;
        }

        bytesInBuffer = 0;
        nextInputByte = fBuffer->bytes();
        return true;
    }
    const std::vector<SkJpegSegment>& getAllSegments() override {
        if (!fScanner) {
            fScanner =
                    std::make_unique<SkJpegSegmentScanner>(SkJpegSegmentScanner::kMarkerEndOfImage);
            size_t initialPosition = fStream->getPosition();
            fStream->rewind();
            while (!fScanner->isDone() && !fScanner->hadError()) {
                constexpr size_t kBufferSize = 1024;
                uint8_t buffer[kBufferSize];
                size_t bytesRead = fStream->read(buffer, kBufferSize);
                if (bytesRead == 0) {
                    SkCodecPrintf("Unexpected EOF.\n");
                    break;
                }
                fScanner->onBytes(buffer, bytesRead);
            }
            fStream->seek(initialPosition);
        }
        return fScanner->getSegments();
    }
    std::unique_ptr<SkStream> getSubsetStream(size_t offset, size_t size) override {
        size_t initialPosition = fStream->getPosition();
        if (!fStream->seek(offset)) {
            SkCodecPrintf("Failed to seek to subset stream position.\n");
            return nullptr;
        }
        sk_sp<SkData> data = SkData::MakeUninitialized(size);
        if (fStream->read(data->writable_data(), size) != size) {
            SkCodecPrintf("Failed to read subset stream data.\n");
            fStream->seek(initialPosition);
            return nullptr;
        }
        fStream->seek(initialPosition);
        return SkMemoryStream::Make(data);
    }

private:
    SkStream* const fStream;  // unowned.
    sk_sp<SkData> fBuffer;
    std::unique_ptr<SkJpegSegmentScanner> fScanner;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegSourceMgr

// static
std::unique_ptr<SkJpegSourceMgr> SkJpegSourceMgr::Make(SkStream* stream) {
    if (stream->hasLength() && stream->getMemoryBase()) {
        return std::make_unique<SkJpegMemorySourceMgr>(stream);
    }
    return std::make_unique<SkJpegBufferedSourceMgr>(stream);
}
