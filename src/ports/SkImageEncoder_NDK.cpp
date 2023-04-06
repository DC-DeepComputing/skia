/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "src/encode/SkImageEncoderPriv.h"
#include "src/ports/SkNDKConversions.h"


static AndroidBitmapInfo info_for_pixmap(const SkPixmap& pmap) {
    // If any of these values is invalid (e.g. set to zero), the info will be rejected by
    // AndroidBitmap_compress.
    AndroidBitmapInfo info {
        .width  = SkTFitsIn<uint32_t>(pmap.width())    ? SkToU32(pmap.width())    : 0,
        .height = SkTFitsIn<uint32_t>(pmap.height())   ? SkToU32(pmap.height())   : 0,
        .stride = SkTFitsIn<uint32_t>(pmap.rowBytes()) ? SkToU32(pmap.rowBytes()) : 0,
        .format = SkNDKConversions::toAndroidBitmapFormat(pmap.colorType())
    };

    switch (pmap.alphaType()) {
        case kPremul_SkAlphaType:
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_PREMUL;
            break;
        case kOpaque_SkAlphaType:
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE;
            break;
        case kUnpremul_SkAlphaType:
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_UNPREMUL;
            break;
        default:
            SkDEBUGFAIL("unspecified alphaType");
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE;
            break;
    }
    return info;
}

static bool write_image_to_stream(SkWStream* stream, const SkPixmap& pmap,
                                  AndroidBitmapCompressFormat androidFormat, int quality) {
    AndroidBitmapInfo info = info_for_pixmap(pmap);

    auto write_to_stream = [](void* userContext, const void* data, size_t size) {
        return reinterpret_cast<SkWStream*>(userContext)->write(data, size);
    };

    return ANDROID_BITMAP_RESULT_SUCCESS == AndroidBitmap_compress(&info,
            SkNDKConversions::toDataSpace(pmap.colorSpace()), pmap.addr(), androidFormat, quality,
            reinterpret_cast<void*>(stream), write_to_stream);
}

namespace SkPngEncoder {
std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkPixmap& src,
                                const Options& options) {
    SkDEBUGFAIL("Making an encoder is not supported via the NDK");
    return nullptr;
}

bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    return write_image_to_stream(dst, src, ANDROID_BITMAP_COMPRESS_FORMAT_PNG, 100);
}
}

namespace SkJpegEncoder {

bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    return write_image_to_stream(dst, src, ANDROID_BITMAP_COMPRESS_FORMAT_JPEG, options.fQuality);
}

bool Encode(SkWStream* dst, const SkYUVAPixmaps& src, const SkColorSpace* srcColorSpace,
            const Options& options) {
    SkDEBUGFAIL("encoding a YUVA pixmap is not supported via the NDK");
    return false;
}

std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkPixmap& src, const Options& options) {
    SkDEBUGFAIL("Making an encoder is not supported via the NDK");
    return nullptr;
}

std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkYUVAPixmaps& src,
                                const SkColorSpace* srcColorSpace, const Options& options) {
    SkDEBUGFAIL("Making an encoder is not supported via the NDK");
    return nullptr
}

} // namespace SkJpegEncoder


namespace SkWebpEncoder {

bool SkWebpEncoder::Encode(SkWStream* stream, const SkPixmap& pixmap, const Options& opts) {
    if (opts.quality >= 100.0f) {
        // Mimic the behavior of SkImageEncoder.cpp. In LOSSLESS mode, libwebp
        // interprets quality as the amount of effort (time) to spend making
        // the encoded image smaller, while the visual quality remains constant.
        // This value of 75 (on a scale of 0 - 100, where 100 spends the most
        // time for the smallest encoding) matches WebPConfigInit.
        androidFormat = ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSLESS;
        quality = 75;
        return write_image_to_stream(dst, src, ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSLESS, 75);
    }
    return write_image_to_stream(dst, src, ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSY, 75);
}

bool EncodeAnimated(SkWStream* stream, SkSpan<const SkEncoder::Frame> frames, const Options& opts) {
    SkDEBUGFAIL("Encoding Animated WebP images is not supported with the NDK.");
    return nullptr
}  // SkWebpEncoder
