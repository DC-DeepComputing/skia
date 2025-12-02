/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkJxlRsDecoder_DEFINED
#define SkJxlRsDecoder_DEFINED

#include <memory>

#include "include/codec/SkCodec.h"
#include "include/private/base/SkAPI.h"

class SkStream;

namespace SkJxlRsDecoder {

/** Returns true if this data claims to be a JXL (JPEG XL) image. */
SK_API bool IsJxl(const void*, size_t);

SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);

inline constexpr SkCodecs::Decoder Decoder() { return {"jxl", IsJxl, Decode}; }

}  // namespace SkJxlRsDecoder

#endif  // SkJxlRsDecoder_DEFINED
