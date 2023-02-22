/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "src/codec/SkCodecPriv.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

// We always include and compile in these BMP codecs
#include "src/codec/SkBmpCodec.h"
#include "src/codec/SkWbmpCodec.h"

class SkPngChunkReader;

#ifdef SK_CODEC_DECODES_AVIF
#include "src/codec/SkAvifCodec.h"
#endif

#ifdef SK_HAS_HEIF_LIBRARY
#include "src/codec/SkHeifCodec.h"
#endif

#ifdef SK_CODEC_DECODES_JPEG
#include "src/codec/SkJpegCodec.h"
#endif

#ifdef SK_CODEC_DECODES_JPEGXL
#include "src/codec/SkJpegxlCodec.h"
#endif

#ifdef SK_CODEC_DECODES_PNG
#include "src/codec/SkIcoCodec.h"
#include "src/codec/SkPngCodec.h"
#endif

#ifdef SK_CODEC_DECODES_RAW
#include "src/codec/SkRawCodec.h"
#endif

#ifdef SK_CODEC_DECODES_WEBP
#include "src/codec/SkWebpCodec.h"
#endif

#ifdef SK_HAS_WUFFS_LIBRARY
#include "src/codec/SkWuffsCodec.h"
#endif

namespace {

struct DecoderProc {
    bool (*IsFormat)(const void*, size_t);
    std::unique_ptr<SkCodec> (*MakeFromStream)(std::unique_ptr<SkStream>, SkCodec::Result*);
};

std::vector<DecoderProc>* decoders() {
    static auto* decoders = new std::vector<DecoderProc> {
    #ifdef SK_CODEC_DECODES_JPEG
        { SkJpegCodec::IsJpeg, SkJpegCodec::MakeFromStream },
    #endif
    #ifdef SK_CODEC_DECODES_WEBP
        { SkWebpCodec::IsWebp, SkWebpCodec::MakeFromStream },
    #endif
    #ifdef SK_HAS_WUFFS_LIBRARY
        { SkWuffsCodec_IsFormat, SkWuffsCodec_MakeFromStream },
    #endif
    #ifdef SK_CODEC_DECODES_PNG
        { SkIcoCodec::IsIco, SkIcoCodec::MakeFromStream },
    #endif
        { SkBmpCodec::IsBmp, SkBmpCodec::MakeFromStream },
        { SkWbmpCodec::IsWbmp, SkWbmpCodec::MakeFromStream },
    #ifdef SK_CODEC_DECODES_AVIF
        { SkAvifCodec::IsAvif, SkAvifCodec::MakeFromStream },
    #endif
    #ifdef SK_CODEC_DECODES_JPEGXL
        { SkJpegxlCodec::IsJpegxl, SkJpegxlCodec::MakeFromStream },
    #endif
    };
    return decoders;
}

}  // namespace

void SkCodec::Register(
            bool                     (*peek)(const void*, size_t),
            std::unique_ptr<SkCodec> (*make)(std::unique_ptr<SkStream>, SkCodec::Result*)) {
    decoders()->push_back(DecoderProc{peek, make});
}

std::unique_ptr<SkCodec> SkCodec::MakeFromStream(
        std::unique_ptr<SkStream> stream, Result* outResult,
        SkPngChunkReader* chunkReader, SelectionPolicy selectionPolicy) {
    Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }

    if (!stream) {
        *outResult = kInvalidInput;
        return nullptr;
    }

    if (selectionPolicy != SelectionPolicy::kPreferStillImage
            && selectionPolicy != SelectionPolicy::kPreferAnimation) {
        *outResult = kInvalidParameters;
        return nullptr;
    }

    constexpr size_t bytesToRead = MinBufferedBytesNeeded();

    char buffer[bytesToRead];
    size_t bytesRead = stream->peek(buffer, bytesToRead);

    // It is also possible to have a complete image less than bytesToRead bytes
    // (e.g. a 1 x 1 wbmp), meaning peek() would return less than bytesToRead.
    // Assume that if bytesRead < bytesToRead, but > 0, the stream is shorter
    // than bytesToRead, so pass that directly to the decoder.
    // It also is possible the stream uses too small a buffer for peeking, but
    // we trust the caller to use a large enough buffer.

    if (0 == bytesRead) {
        // TODO: After implementing peek in CreateJavaOutputStreamAdaptor.cpp, this
        // printf could be useful to notice failures.
        // SkCodecPrintf("Encoded image data failed to peek!\n");

        // It is possible the stream does not support peeking, but does support
        // rewinding.
        // Attempt to read() and pass the actual amount read to the decoder.
        bytesRead = stream->read(buffer, bytesToRead);
        if (!stream->rewind()) {
            SkCodecPrintf("Encoded image data could not peek or rewind to determine format!\n");
            *outResult = kCouldNotRewind;
            return nullptr;
        }
    }

    // PNG is special, since we want to be able to supply an SkPngChunkReader.
    // But this code follows the same pattern as the loop.
#ifdef SK_CODEC_DECODES_PNG
    if (SkPngCodec::IsPng(buffer, bytesRead)) {
        return SkPngCodec::MakeFromStream(std::move(stream), outResult, chunkReader);
    }
#endif

    for (DecoderProc proc : *decoders()) {
        if (proc.IsFormat(buffer, bytesRead)) {
            return proc.MakeFromStream(std::move(stream), outResult);
        }
    }

#ifdef SK_HAS_HEIF_LIBRARY
    SkEncodedImageFormat format;
    if (SkHeifCodec::IsSupported(buffer, bytesRead, &format)) {
        return SkHeifCodec::MakeFromStream(std::move(stream), selectionPolicy,
                format, outResult);
    }
#endif

#ifdef SK_CODEC_DECODES_RAW
    // Try to treat the input as RAW if all the other checks failed.
    return SkRawCodec::MakeFromStream(std::move(stream), outResult);
#else
    if (bytesRead < bytesToRead) {
        *outResult = kIncompleteInput;
    } else {
        *outResult = kUnimplemented;
    }

    return nullptr;
#endif
}

std::unique_ptr<SkCodec> SkCodec::MakeFromData(sk_sp<SkData> data, SkPngChunkReader* reader) {
    if (!data) {
        return nullptr;
    }
    return MakeFromStream(SkMemoryStream::Make(std::move(data)), nullptr, reader);
}
