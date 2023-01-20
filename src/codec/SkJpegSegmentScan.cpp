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

#include <cstring>
#include <utility>

// Some seekable SkStream sub-classes do not implement peek. Manually implement it via
// setting and restoring the stream's position.
static size_t peek(SkStream* stream, void* data, size_t size) {
    size_t initialPosition = stream->getPosition();
    size_t result = stream->read(data, size);
    if (!stream->seek(initialPosition)) {
        SkCodecPrintf("Failed to restore position during peek.\n");
        return 0;
    }
    return result;
}

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
    size_t initialPosition = stream->getPosition();

    // This implementation relies on using getPosition. It is possible to implement this with just
    // rewind.
    if (!stream->hasPosition()) {
        SkCodecPrintf("Cannot scan stream that doesn't have a position.\n");
        return nullptr;
    }

    // First peek to see the Jpeg signature.
    {
        uint8_t signature[sizeof(kJpegSig)];
        if (peek(stream, signature, sizeof(kJpegSig)) != sizeof(kJpegSig)) {
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
        const size_t offset = stream->getPosition() - initialPosition;
        uint8_t markerCode[kMarkerCodeSize] = {0};
        if (stream->read(markerCode, kMarkerCodeSize) != kMarkerCodeSize) {
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
        segment.offset = offset;

        // Some markers stand alone, which means that they do not have parameters, see Marker
        // assignments, in section B.1.1.3.
        if (!MarkerStandsAlone(segment.marker)) {
            uint8_t parameterLength[kParameterLengthSize] = {0};
            // Read the length for the parameters. See section B.1.1.4: A marker segment consists of
            // a marker followed by a sequence of related parameters. The first parameter in a
            // marker segment is the two-byte length parameter. This length parameter encodes the
            // number of bytes in the marker segment, including the length parameter and excluding
            // the two-byte marker.
            if (peek(stream, parameterLength, kParameterLengthSize) != kParameterLengthSize) {
                SkCodecPrintf("Failed to peek segment parameter length.\n");
                return nullptr;
            }
            segment.parameterLength = 256 * parameterLength[0] + parameterLength[1];
            if (stream->skip(segment.parameterLength) != segment.parameterLength) {
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
        if (!SkipPastEntropyCodedData(stream)) {
            SkCodecPrintf("Failed to seek past entropy coded data.\n");
            return nullptr;
        }
    }

    return std::unique_ptr<SkJpegSegmentScan>(
            new SkJpegSegmentScan(stream, initialPosition, std::move(segments)));
}

enum class EntropyBufferResult {
    kUnexpectedEOF,
    kNeedsMoreBuffer,
    kEndOfECD,
};

EntropyBufferResult EntropyBufferSeek(uint8_t* buffer, int32_t bufferSize, int32_t& position) {
    position = 0;
    while (position < bufferSize) {
        // Seek to the next 0xFF.
        uint8_t* found =
                reinterpret_cast<uint8_t*>(memchr(buffer + position, 0xFF, bufferSize - position));
        if (!found) {
            position = bufferSize;
            return EntropyBufferResult::kNeedsMoreBuffer;
        }
        position += found - (buffer + position);
        SkASSERT(buffer[position] == 0xFF);

        // The last character of the stream is an 0xFF.
        if (bufferSize - position == 1) {
            // If the buffer was size 1, then we're at the EOF (we read bigger chunks than that).
            if (bufferSize == 1) {
                return EntropyBufferResult::kUnexpectedEOF;
            }
            // Otherwise, read another chunk (we'll start at this 0xFF).
            return EntropyBufferResult::kNeedsMoreBuffer;
        }

        // If the byte after the 0xFF is 0x00, assume we are in an entropy-coding
        // segment and that this is a byte-stuffed representation of 0xFF. See the
        // text of F.1.2.3: Whenever the byte value 0xFF is created in the code
        // string, a 0x00 byte is stuffed into the code string. If a 0x00 byte is
        // detected after a 0xFF byte, the decoder must discard it.
        if (buffer[position + 1] == 0x00) {
            position += 2;
            continue;
        }

        // We are at the end of the entropy coded data (but we may have more 0xFF stuffing before we
        // get to the next marker).
        return EntropyBufferResult::kEndOfECD;
    }

    // We ran out of buffer. Get more.
    return EntropyBufferResult::kNeedsMoreBuffer;
}

bool SkJpegSegmentScan::SkipPastEntropyCodedData(SkStream* stream) {
    // Seek through entropy-coded data in reads of 1k (reading byte by byte is very slow).
    while (1) {
        constexpr size_t kBufferSize = 1024;
        uint8_t buffer[kBufferSize];
        size_t bytesRead = stream->read(buffer, kBufferSize);
        if (bytesRead == 0) {
            return false;
        }
        int32_t position = 0;
        int32_t bufferSize = static_cast<int32_t>(bytesRead);
        auto result = EntropyBufferSeek(buffer, bufferSize, position);
        if (result == EntropyBufferResult::kUnexpectedEOF) {
            SkCodecPrintf("Unexpected end of file\n");
            return false;
        }

        // If we stopped before the end of the buffer, seek backward.
        if (position != bufferSize) {
            stream->move(position - bufferSize);
        }

        // If we're at the end of the Entropy Coded Data, stop.
        if (result == EntropyBufferResult::kEndOfECD) {
            break;
        }

        // Otherwise, keep reading chunks.
        SkASSERT(result == EntropyBufferResult::kNeedsMoreBuffer);
    }

    // See section B.1.1.3: Any marker may optionally be preceded by any number
    // of fill bytes, which are bytes assigned code 0xFF. Skip past any 0xFF
    // fill bytes that may be present at the end of the entropy-coded data.
    uint8_t markerCode[2];
    if (peek(stream, markerCode, kMarkerCodeSize) != kMarkerCodeSize) {
        return false;
    }
    while (markerCode[1] == 0xFF) {
        if (stream->skip(1) != 1) {
            SkCodecPrintf("Failed to skip post-ECD fill.\n");
            return false;
        }
        if (peek(stream, markerCode, kMarkerCodeSize) != kMarkerCodeSize) {
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
    if (fStream->read(data->writable_data(), size) != size) {
        SkCodecPrintf("Failed to read subset stream data.\n");
        return nullptr;
    }

    return SkMemoryStream::Make(data);
}
