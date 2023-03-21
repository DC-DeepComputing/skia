/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkPngDecoder.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/codec/SkPngCodec.h"

std::unique_ptr<SkCodec> SkPngDecoder::Decode(std::unique_ptr<SkStream> stream,
                                              SkCodec::Result* outResult,
                                              SkPngChunkReader* reader) {
    SkCodec::Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }

    constexpr size_t bytesToRead = SkCodec::MinBufferedBytesNeeded();
    uint8_t buffer[bytesToRead];
    size_t bytesRead = SkCodec::ReadHeader(std::move(stream), buffer);
    if (SkPngCodec::IsPng(buffer, bytesRead)) {
        return SkPngCodec::MakeFromStream(std::move(stream), outResult, reader);
    }
    *outResult = SkCodec::kInvalidInput;
    return nullptr;
}

std::unique_ptr<SkCodec> SkPngDecoder::Decode(sk_sp<SkData> data, SkCodec::Result* outResult,
                                              SkPngChunkReader* reader) {
    if (!data) {
        if (outResult) {
            *outResult = SkCodec::kInvalidInput;
        }
        return nullptr;
    }
    return Decode(SkMemoryStream::Make(std::move(data)), outResult, reader);
}
