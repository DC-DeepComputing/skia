/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRawDecoder_DEFINED
#define SkRawDecoder_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkRefCnt.h"

class SkData;
class SkStream;

#include <memory>

namespace SkRawDecoder {

/** Returns true if this data claims to be a raw image which Skia can decode.*/
SK_API bool IsRaw(const void*, size_t);

/**
 *  Attempts to decode the given bytes as a raw image.
 *
 *  If the bytes are not a raw, returns nullptr.
 *
 *  DecodeContext is ignored
 */
SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<SkData>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);

inline SkCodecs::Decoder Decoder() {
    return { "raw", IsRaw, Decode };
}

}  // namespace SkRawDecoder

#endif  // SkRawDecoder_DEFINED
