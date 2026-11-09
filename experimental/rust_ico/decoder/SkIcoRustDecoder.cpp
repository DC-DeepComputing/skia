/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_ico/decoder/SkIcoRustDecoder.h"

#include <utility>

#include "experimental/rust_ico/decoder/impl/SkIcoRustCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"

namespace SkIcoRustDecoder {

bool IsIco(const void* buff, size_t bytesRead) { return SkIcoRustCodec::IsIco(buff, bytesRead); }

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

std::optional<SkIPoint> GetHotSpot(const SkCodec* codec, int frameIndex) {
    if (!codec) {
        return std::nullopt;
    }
    return static_cast<const SkIcoRustCodec*>(codec)->getHotSpot(frameIndex);
}

bool GetHotSpot(const SkCodec* codec, int frameIndex, SkIPoint* hotSpot) {
    if (!hotSpot) {
        return false;
    }
    const auto point = GetHotSpot(codec, frameIndex);
    if (!point) {
        return false;
    }
    *hotSpot = *point;
    return true;
}

std::optional<DefaultImageInfo> GetDefaultImageInfo(const SkCodec* codec) {
    if (!codec) {
        return std::nullopt;
    }
    const auto* ico = static_cast<const SkIcoRustCodec*>(codec);
    return DefaultImageInfo{ico->defaultDimensions(),
                            ico->getDefaultHotSpot(),
                            ico->defaultImageIsAvailable()};
}

}  // namespace SkIcoRustDecoder
