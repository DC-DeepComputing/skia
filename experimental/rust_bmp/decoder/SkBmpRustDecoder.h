/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBmpRustDecoder_DEFINED
#define SkBmpRustDecoder_DEFINED

#include <memory>

#include "include/codec/SkCodec.h"
#include "include/private/base/SkAPI.h"

class SkStream;

namespace SkBmpRustDecoder {

/** Returns true if this data claims to be a BMP image. */
SK_API bool IsBmp(const void*, size_t);

SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);

inline constexpr SkCodecs::Decoder Decoder() { return {"bmp", IsBmp, Decode}; }

}  // namespace SkBmpRustDecoder

#endif  // SkBmpRustDecoder_DEFINED
