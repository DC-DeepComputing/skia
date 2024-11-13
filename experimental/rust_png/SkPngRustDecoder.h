/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustDecoder_DEFINED
#define SkPngRustDecoder_DEFINED

#include <memory>

#include "include/codec/SkCodec.h"
#include "include/private/base/SkAPI.h"

class SkStream;

namespace SkPngRustDecoder {

/** Returns true if this data claims to be a PNG image. */
bool IsPng(const void* buf, size_t size);

/**
 *  Attempts to decode the given bytes as a PNG.
 *
 *  If the bytes are not a PNG, returns nullptr.
 *
 *  DecodeContext, should be null.
 */
SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);

inline constexpr SkCodecs::Decoder Decoder() { return {"png", IsPng, Decode}; }

}  // namespace SkPngRustDecoder

#endif  // SkPngRustDecoder_DEFINED
