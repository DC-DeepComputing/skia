/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkJxlRsDecoder.h"

#include <utility>

#include "include/core/SkStream.h"
#include "src/codec/SkJxlRsCodec.h"

namespace SkJxlRsDecoder {

bool IsJxl(const void* data, size_t size) {
    return SkJxlRsCodec::IsJxl(data, size);
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* result,
                                SkCodecs::DecodeContext) {
    SkCodec::Result resultStorage;
    if (!result) {
        result = &resultStorage;
    }
    return SkJxlRsCodec::MakeFromStream(std::move(stream), result);
}

}  // namespace SkJxlRsDecoder
