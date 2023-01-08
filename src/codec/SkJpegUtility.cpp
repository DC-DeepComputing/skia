/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegUtility.h"

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegPriv.h"

#include <csetjmp>
#include <cstring>
#include <utility>

extern "C" {
    #include "jmorecfg.h"
    #include "jpeglib.h"
}

/*
 * Call longjmp to continue execution on an error
 */
void skjpeg_err_exit(j_common_ptr dinfo) {
    // Simply return to Skia client code
    // JpegDecoderMgr will take care of freeing memory
    skjpeg_error_mgr* error = (skjpeg_error_mgr*) dinfo->err;
    (*error->output_message) (dinfo);
    if (error->fJmpBufStack.empty()) {
        SK_ABORT("JPEG error with no jmp_buf set.");
    }
    longjmp(*error->fJmpBufStack.back(), 1);
}

// Functions for buffered sources //

/*
 * Initialize the buffered source manager
 */
static void sk_init_buffered_source(j_decompress_ptr dinfo) {
    skjpeg_source_mgr* src = (skjpeg_source_mgr*) dinfo->src;
    src->next_input_byte = (const JOCTET*) src->fBuffer;
    src->bytes_in_buffer = 0;
}

/*
 * Fill the input buffer from the stream
 */
static boolean sk_fill_buffered_input_buffer(j_decompress_ptr dinfo) {
    skjpeg_source_mgr* src = (skjpeg_source_mgr*) dinfo->src;
    size_t bytes = src->fStream->read(src->fBuffer, skjpeg_source_mgr::kBufferSize);

    // libjpeg is still happy with a less than full read, as long as the result is non-zero
    if (bytes == 0) {
        // Let libjpeg know that the buffer needs to be refilled
        src->next_input_byte = nullptr;
        src->bytes_in_buffer = 0;
        return false;
    }

    src->next_input_byte = (const JOCTET*) src->fBuffer;
    src->bytes_in_buffer = bytes;
    return true;
}

/*
 * Skip a certain number of bytes in the stream
 */
static void sk_skip_buffered_input_data(j_decompress_ptr dinfo, long numBytes) {
    skjpeg_source_mgr* src = (skjpeg_source_mgr*) dinfo->src;
    size_t bytes = (size_t) numBytes;

    if (bytes > src->bytes_in_buffer) {
        size_t bytesToSkip = bytes - src->bytes_in_buffer;
        if (bytesToSkip != src->fStream->skip(bytesToSkip)) {
            SkCodecPrintf("Failure to skip.\n");
            dinfo->err->error_exit((j_common_ptr) dinfo);
            return;
        }

        src->next_input_byte = (const JOCTET*) src->fBuffer;
        src->bytes_in_buffer = 0;
    } else {
        src->next_input_byte += numBytes;
        src->bytes_in_buffer -= numBytes;
    }
}

/*
 * We do not need to do anything to terminate our stream
 */
static void sk_term_source(j_decompress_ptr dinfo)
{
    // The current implementation of SkJpegCodec does not call
    // jpeg_finish_decompress(), so this function is never called.
    // If we want to modify this function to do something, we also
    // need to modify SkJpegCodec to call jpeg_finish_decompress().
}

// Functions for memory backed sources //

/*
 * Initialize the mem backed source manager
 */
static void sk_init_mem_source(j_decompress_ptr dinfo) {
    /* no work necessary here, all things are done in constructor */
}

static void sk_skip_mem_input_data (j_decompress_ptr cinfo, long num_bytes) {
    jpeg_source_mgr* src = cinfo->src;
    size_t bytes = static_cast<size_t>(num_bytes);
    if(bytes > src->bytes_in_buffer) {
        src->next_input_byte = nullptr;
        src->bytes_in_buffer = 0;
    } else {
        src->next_input_byte += bytes;
        src->bytes_in_buffer -= bytes;
    }
}

static boolean sk_fill_mem_input_buffer (j_decompress_ptr cinfo) {
    /* The whole JPEG data is expected to reside in the supplied memory,
     * buffer, so any request for more data beyond the given buffer size
     * is treated as an error.
     */
    return false;
}

/*
 * Constructor for the source manager that we provide to libjpeg
 * We provide skia implementations of all of the stream processing functions required by libjpeg
 */
skjpeg_source_mgr::skjpeg_source_mgr(SkStream* stream)
    : fStream(stream)
{
    if (stream->hasLength() && stream->getMemoryBase()) {
        init_source = sk_init_mem_source;
        fill_input_buffer = sk_fill_mem_input_buffer;
        skip_input_data = sk_skip_mem_input_data;
        resync_to_restart = jpeg_resync_to_restart;
        term_source = sk_term_source;
        bytes_in_buffer = static_cast<size_t>(stream->getLength());
        next_input_byte = static_cast<const JOCTET*>(stream->getMemoryBase());
    } else {
        init_source = sk_init_buffered_source;
        fill_input_buffer = sk_fill_buffered_input_buffer;
        skip_input_data = sk_skip_buffered_input_data;
        resync_to_restart = jpeg_resync_to_restart;
        term_source = sk_term_source;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JpegSegmentScan

JpegSegmentScan::JpegSegmentScan(SkStream* stream,
                                 size_t initialPosition,
                                 std::vector<Segment> segments)
        : fStream(stream), fInitialPosition(initialPosition), fSegments(std::move(segments)) {}

// static
bool JpegSegmentScan::MarkerIsValid(const uint8_t markerCode[kMarkerCodeSize]) {
    if (markerCode[0] != 0xFF) return false;
    if (markerCode[1] == 0x00 || markerCode[1] == 0xFF) return false;
    return true;
}

// static
bool JpegSegmentScan::MarkerStandsAlone(uint8_t marker) {
    // These markers are TEM (0x01), RSTm (0xD0 through 0xD7), SOI (0xD8), and
    // EOI (0xD9). See section B.1.1.3, Marker assignments.
    return marker == 0x01 || (marker >= 0xD0 && marker <= 0xD9);
}

std::unique_ptr<JpegSegmentScan> JpegSegmentScan::Create(SkStream* stream, const Options& options) {
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
        if (stream->peek(signature, sizeof(kJpegSig)) != sizeof(kJpegSig)) {
            SkCodecPrintf("Failed to peek Jpeg signature.\n");
            return nullptr;
        }
        if (memcmp(signature, kJpegSig, sizeof(kJpegSig))) {
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
            SkCodecPrintf("Unspected EOF at first marker code.\n");
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
            if (stream->peek(parameterLength, kParameterLengthSize) != kParameterLengthSize) {
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

    return std::unique_ptr<JpegSegmentScan>(
            new JpegSegmentScan(stream, initialPosition, std::move(segments)));
}

bool JpegSegmentScan::SkipPastEntropyCodedData(SkStream* stream) {
    uint8_t markerCode[kMarkerCodeSize] = {0};

    while (1) {
        // Peek at the two bytes for the marker.
        if (stream->peek(markerCode, kMarkerCodeSize) != kMarkerCodeSize) {
            // Assume to be EOF.
            SkCodecPrintf("Failed to peek two ECD bytes (unexpected EOF?).\n");
            return false;
        }

        // If the first byte is not 0xFF, it's part of the entropy-coded data,
        // so skip to the next byte.
        if (markerCode[0] != 0xFF) {
            if (stream->skip(1) != 1) {
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
            if (stream->skip(kMarkerCodeSize) != kMarkerCodeSize) {
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
        if (stream->skip(1) != 1) {
            SkCodecPrintf("Failed to skip post-ECD fill.\n");
            return false;
        }
        if (stream->peek(markerCode, kMarkerCodeSize) != kMarkerCodeSize) {
            SkCodecPrintf("Failed to peek past post-ECD fill (unexpected EOF?).\n");
            return false;
        }
    }

    return true;
}

sk_sp<SkData> JpegSegmentScan::CopyParameters(const Segment& segment,
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
        return nullptr;
    }
    if (memcmp(segmentSignature->data(), signature, signatureLength)) {
        return nullptr;
    }

    // Finally, read the remainder of the segment.
    auto result = SkData::MakeUninitialized(sizeToRead);
    if (fStream->read(result->writable_data(), sizeToRead) != sizeToRead) {
        return nullptr;
    }

    return result;
}
