/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/SkPngRustDecoder.h"

#include <utility>

#include "experimental/rust_png/impl/SkPngRustCodec.h"
#include "include/core/SkStream.h"

namespace SkPngRustDecoder {

bool IsPng(const void* buf, size_t size) {
    // From http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html#PNG-file-signature:
    static const std::array<uint8_t, 8> kPngSig = {137, 80, 78, 71, 13, 10, 26, 10};
    return size >= kPngSig.size() && !memcmp(buf, kPngSig.data(), kPngSig.size());
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* result,
                                SkCodecs::DecodeContext /* ignored */) {
    return SkPngRustCodec::MakeFromStream(std::move(stream), result);
}

}  // namespace SkPngRustDecoder
