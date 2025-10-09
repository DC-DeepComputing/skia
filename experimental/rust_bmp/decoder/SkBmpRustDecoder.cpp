/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_bmp/decoder/SkBmpRustDecoder.h"

#include <utility>

#include "experimental/rust_bmp/decoder/impl/SkBmpRustCodec.h"
#include "include/core/SkStream.h"

namespace SkBmpRustDecoder {

bool IsBmp(const void* buff, size_t bytesRead) {
    // BMP files start with the signature "BM" (0x42, 0x4D)
    static constexpr unsigned char bmpSignature[] = {0x42, 0x4D};
    if (bytesRead < sizeof(bmpSignature)) {
        return false;
    }

    return memcmp(buff, bmpSignature, sizeof(bmpSignature)) == 0;
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* result,
                                SkCodecs::DecodeContext) {
    return SkBmpRustCodec::MakeFromStream(std::move(stream), result);
}

}  // namespace SkBmpRustDecoder
