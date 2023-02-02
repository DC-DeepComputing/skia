/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegSegmentScan.h"

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkAssert.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegPriv.h"

#include <algorithm>
#include <cstring>
#include <utility>

class SkJpegSegmentScan::ForwardScanAdapter {
    // Size we're targeting to read from the base stream
    static constexpr size_t kMaxReadSize = 8192;
    // "shrug" number; big enough we can reliably do 8192 size reads
    static constexpr size_t kBufferSize = 9216;
public:
    explicit ForwardScanAdapter(SkStream* stream) : fStream(stream) {}

    size_t peek(void* buffer, size_t size) {
        fillIfNeeded(size);
        auto toCopy = std::min(fSizeRemaining, size);
        memcpy(buffer, fBuffer + fReadHead, toCopy);
        return toCopy;
    }

    size_t read(void* buffer, size_t size) {
        size = peek(buffer, size);
        fReadHead += size;
        fSizeRemaining -= size;
        fOffsetFromStart += size;
        return size;
    }

    size_t skip(size_t size) {
        size_t startPos = fOffsetFromStart;
        if (size > fSizeRemaining) {
            fOffsetFromStart += fSizeRemaining;
            size -= fSizeRemaining;
            fSizeRemaining = 0;
            fReadHead = 0;
            fOffsetFromStart += fStream->skip(size);
        } else {
            fSizeRemaining -= size;
            fReadHead += size;
            fOffsetFromStart += size;
        }
        return fOffsetFromStart - startPos;
    }

    size_t offsetFromStart() const {
        return fOffsetFromStart;
    }

    // Returns any unread data back to the stream
    void refund() {
        fStream->seek(fStream->getPosition() - fSizeRemaining);
        fSizeRemaining = 0;
        fReadHead = 0;
    }

private:
    SkStream* fStream;
    char fBuffer[kBufferSize];
    size_t fSizeRemaining = 0;
    size_t fReadHead = 0;
    size_t fOffsetFromStart = 0;

    void fillIfNeeded(size_t toAccess) {
        if (fSizeRemaining < toAccess) {
            memcpy(fBuffer, fBuffer + fReadHead, fSizeRemaining);
            fReadHead = 0;
            size_t toRead = std::min(kMaxReadSize, kBufferSize - fSizeRemaining);
            fSizeRemaining += fStream->read(fBuffer + fSizeRemaining, toRead);
        }
    }
};

SkJpegSegmentScan::SkJpegSegmentScan(SkStream* stream,
                                     size_t initialPosition,
                                     std::vector<Segment>&& segments)
        : fStream(stream), fInitialPosition(initialPosition), fSegments(segments) {}

// static
bool SkJpegSegmentScan::MarkerIsValid(const uint8_t markerCode[kMarkerCodeSize]) {
    if (markerCode[0] != 0xFF) return false;
    if (markerCode[1] == 0x00 || markerCode[1] == 0xFF) return false;
    return true;
}

// static
bool SkJpegSegmentScan::MarkerStandsAlone(uint8_t marker) {
    // These markers are TEM (0x01), RSTm (0xD0 through 0xD7), SOI (0xD8), and
    // EOI (0xD9). See section B.1.1.3, Marker assignments.
    return marker == 0x01 || (marker >= 0xD0 && marker <= 0xD9);
}

std::unique_ptr<SkJpegSegmentScan> SkJpegSegmentScan::Create(SkStream* stream,
                                                             const Options& options) {
    size_t startOfImageCount = 0;
    size_t endOfImageCount = 0;
    std::vector<Segment> segments;

    // This implementation relies on using getPosition. It is possible to implement this with just
    // rewind.
    if (!stream->hasPosition()) {
        SkCodecPrintf("Cannot scan stream that doesn't have a position.\n");
        return nullptr;
    }

    size_t initialPosition = stream->getPosition();
    ForwardScanAdapter adapter(stream);

    // First peek to see the Jpeg signature.
    {
        uint8_t signature[sizeof(kJpegSig)];
        if (adapter.peek(signature, sizeof(kJpegSig)) != sizeof(kJpegSig)) {
            SkCodecPrintf("Failed to peek Jpeg signature.\n");
            return nullptr;
        }
        if (memcmp(signature, kJpegSig, sizeof(kJpegSig)) != 0) {
            SkCodecPrintf("Stream does not have Jpeg signature.\n");
            return nullptr;
        }
    }

    while (1) {
        // Read each segment, starting with its marker. See section B.1.1.3: All markers are
        // assigned two-byte codes: a 0xFF byte followed by a byte which is not equal to 0x00 or
        // 0xFF.
        uint8_t markerCode[kMarkerCodeSize] = {0};
        if (adapter.read(markerCode, kMarkerCodeSize) != kMarkerCodeSize) {
            SkCodecPrintf("Unexpected EOF at first marker code.\n");
            break;
        }
        if (!MarkerIsValid(markerCode)) {
            SkCodecPrintf("Invalid marker code %02x%02x.\n", markerCode[0], markerCode[1]);
            return nullptr;
        }

        // We are at a marker for the beginning of a segment.
        Segment segment;
        segment.marker = markerCode[1];
        segment.offset = adapter.offsetFromStart() - kMarkerCodeSize;

        // Some markers stand alone, which means that they do not have parameters, see Marker
        // assignments, in section B.1.1.3.
        if (!MarkerStandsAlone(segment.marker)) {
            uint8_t parameterLength[kParameterLengthSize] = {0};
            // Read the length for the parameters. See section B.1.1.4: A marker segment consists of
            // a marker followed by a sequence of related parameters. The first parameter in a
            // marker segment is the two-byte length parameter. This length parameter encodes the
            // number of bytes in the marker segment, including the length parameter and excluding
            // the two-byte marker.
            if (adapter.peek(parameterLength, kParameterLengthSize) != kParameterLengthSize) {
                SkCodecPrintf("Failed to peek segment parameter length.\n");
                return nullptr;
            }
            segment.parameterLength = 256 * parameterLength[0] + parameterLength[1];
            if (adapter.skip(segment.parameterLength) != segment.parameterLength) {
                SkCodecPrintf("Failed to seek past segment parameters.\n");
                return nullptr;
            }
        }
        segments.push_back(segment);

        // Validate StartOfImage and EndOfImage marker pairs, and stop reading when our termination
        // conditions are met.
        if (segment.marker == kMarkerStartOfImage) {
            startOfImageCount += 1;
            if (startOfImageCount != endOfImageCount + 1) {
                SkCodecPrintf("Encountered new StartOfImage without EndOfImage.\n");
                return nullptr;
            }
        } else if (segment.marker == kMarkerEndOfImage) {
            endOfImageCount += 1;
            if (endOfImageCount > startOfImageCount) {
                SkCodecPrintf("EndOfImage without StartOfImage.\n");
                return nullptr;
            }
            if (options.stopOnEndOfImageCount == endOfImageCount) {
                break;
            }
        } else if (segment.marker == kMarkerStartOfScan) {
            if (options.stopOnStartOfScan) {
                break;
            }
        }

        // Allow entroy-coded data to follow any segment.
        if (!SkipPastEntropyCodedData(adapter)) {
            SkCodecPrintf("Failed to seek past entropy coded data.\n");
            return nullptr;
        }
    }
    adapter.refund();

    return std::unique_ptr<SkJpegSegmentScan>(
            new SkJpegSegmentScan(stream, initialPosition, std::move(segments)));
}

bool SkJpegSegmentScan::SkipPastEntropyCodedData(ForwardScanAdapter& adapter) {
    uint8_t markerCode[kMarkerCodeSize] = {0};

    while (1) {
        // Peek at the two bytes for the marker.
        if (adapter.peek(markerCode, kMarkerCodeSize) != kMarkerCodeSize) {
            // Assume to be EOF.
            SkCodecPrintf("Failed to peek two ECD bytes (unexpected EOF?).\n");
            return false;
        }

        // If the first byte is not 0xFF, it's part of the entropy-coded data,
        // so skip to the next byte.
        if (markerCode[0] != 0xFF) {
            if (adapter.skip(1) != 1) {
                SkCodecPrintf("Failed to skip single ECD byte.\n");
                return false;
            }
            continue;
        }

        // If the byte after the 0xFF is 0x00, assume we are in an entropy-coding
        // segment and that this is a byte-stuffed representation of 0xFF. See the
        // text of F.1.2.3: Whenever the byte value 0xFF is created in the code
        // string, a 0x00 byte is stuffed into the code string. If a 0x00 byte is
        // detected after a 0xFF byte, the decoder must discard it.
        if (markerCode[1] == 0x00) {
            if (adapter.skip(kMarkerCodeSize) != kMarkerCodeSize) {
                SkCodecPrintf("Failed to skip stuffed ECD byte.\n");
                return false;
            }
            continue;
        }

        // We are at the end of the entropy-coded data.
        break;
    }

    // See section B.1.1.3: Any marker may optionally be preceded by any number
    // of fill bytes, which are bytes assigned code 0xFF. Skip past any 0xFF
    // fill bytes that may be present at the end of the entropy-coded data.
    while (markerCode[1] == 0xFF) {
        if (adapter.skip(1) != 1) {
            SkCodecPrintf("Failed to skip post-ECD fill.\n");
            return false;
        }
        if (adapter.peek(markerCode, kMarkerCodeSize) != kMarkerCodeSize) {
            SkCodecPrintf("Failed to peek past post-ECD fill (unexpected EOF?).\n");
            return false;
        }
    }

    return true;
}

sk_sp<SkData> SkJpegSegmentScan::copyParameters(const Segment& segment,
                                                const void* signature,
                                                const size_t signatureLength) {
    // If the segment's parameter length isn't long enough for the signature and the length,
    // early-out.
    if (segment.parameterLength < signatureLength + kParameterLengthSize) {
        return nullptr;
    }
    size_t sizeToRead = segment.parameterLength - signatureLength - kParameterLengthSize;

    // Seek to the start of the segment.
    if (!fStream->seek(fInitialPosition + segment.offset)) {
        SkCodecPrintf("Failed to seek to segment\n");
        return nullptr;
    }

    // Read the marker and verify it matches `segment`.
    uint8_t markerCode[kMarkerCodeSize] = {0};
    if (fStream->read(markerCode, kMarkerCodeSize) != kMarkerCodeSize) {
        SkCodecPrintf("Failed to read segment marker code\n");
        return nullptr;
    }
    SkASSERT(markerCode[0] == 0xFF);
    SkASSERT(markerCode[1] == segment.marker);

    // Read the parameter length and verify it matches `segment`.
    uint8_t parameterLength[kParameterLengthSize] = {0};
    if (fStream->read(parameterLength, kParameterLengthSize) != kParameterLengthSize) {
        SkCodecPrintf("Failed to read parameter length\n");
        return nullptr;
    }
    SkASSERT(256 * parameterLength[0] + parameterLength[1] == segment.parameterLength);

    // Check the next bytes against `signature`.
    auto segmentSignature = SkData::MakeUninitialized(signatureLength);
    if (fStream->read(segmentSignature->writable_data(), segmentSignature->size()) !=
        segmentSignature->size()) {
        SkCodecPrintf("Failed to read parameters\n");
        return nullptr;
    }
    if (memcmp(segmentSignature->data(), signature, signatureLength) != 0) {
        return nullptr;
    }

    // Finally, read the remainder of the segment.
    auto result = SkData::MakeUninitialized(sizeToRead);
    if (fStream->read(result->writable_data(), sizeToRead) != sizeToRead) {
        return nullptr;
    }

    return result;
}

std::unique_ptr<SkStream> SkJpegSegmentScan::getSubsetStream(size_t offset, size_t size) {
    // Read the image's data. It would be better to fork `stream` and limit its position and size.
    if (!fStream->seek(fInitialPosition + offset)) {
        SkCodecPrintf("Failed to seek to subset stream position.\n");
        return nullptr;
    }

    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    auto read = fStream->read(data->writable_data(), size);
    if (read != size) {
        SkCodecPrintf("Failed to read subset stream data; wanted %zu got %zu.\n", size, read);
        return nullptr;
    }

    return SkMemoryStream::Make(data);
}
