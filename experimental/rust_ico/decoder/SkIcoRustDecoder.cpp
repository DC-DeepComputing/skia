/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_ico/decoder/SkIcoRustDecoder.h"

#include <utility>

#include "experimental/rust_ico/ffi/FFI.rs.h"
#include "experimental/rust_ico/decoder/impl/SkIcoRustCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "rust/common/SkStreamAdapter.h"

namespace SkIcoRustDecoder {

bool IsIco(const void* buff, size_t bytesRead) {
    // Create a memory stream from the peeked buffer data
    auto data = SkData::MakeWithoutCopy(buff, bytesRead);
    auto stream = std::make_unique<SkMemoryStream>(std::move(data));
    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(stream.get());
    return rust_ico::is_ico(std::move(inputAdapter));
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* result,
                                SkCodecs::DecodeContext) {
    return SkIcoRustCodec::MakeFromStream(std::move(stream), result);
}

std::unique_ptr<SkCodec> Decode(sk_sp<const SkData> data,
                                SkCodec::Result* result,
                                SkCodecs::DecodeContext) {
    return Decode(SkMemoryStream::Make(std::move(data)), result);
}

}  // namespace SkIcoRustDecoder
