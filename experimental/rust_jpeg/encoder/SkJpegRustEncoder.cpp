/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_jpeg/encoder/SkJpegRustEncoder.h"

#include "experimental/rust_jpeg/ffi/FFI.rs.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "third_party/rust/cxx/v1/cxx.h"

namespace SkJpegRustEncoder {

bool Encode(SkWStream* dst, const SkPixmap& src,
            const SkJpegEncoder::Options& options) {
    if (!dst) {
        return false;
    }

    const SkImageInfo& info = src.info();
    const uint32_t width = info.width();
    const uint32_t height = info.height();

    rust_jpeg::JpegEncodeColor colorType;
    switch (info.colorType()) {
        case kRGB_888x_SkColorType:
            colorType = rust_jpeg::JpegEncodeColor::RGB;
            break;
        case kRGBA_8888_SkColorType:
            colorType = rust_jpeg::JpegEncodeColor::RGBA;
            break;
        case kBGRA_8888_SkColorType:
            colorType = rust_jpeg::JpegEncodeColor::BGRA;
            break;
        case kGray_8_SkColorType:
            colorType = rust_jpeg::JpegEncodeColor::Grayscale;
            break;
        default:
            return false;
    }

    rust_jpeg::JpegEncodeAlpha alphaOption;
    switch (options.fAlphaOption) {
        case SkJpegEncoder::AlphaOption::kBlendOnBlack:
            alphaOption = rust_jpeg::JpegEncodeAlpha::BlendOnBlack;
            break;
        default:
            alphaOption = rust_jpeg::JpegEncodeAlpha::Ignore;
            break;
    }

    const uint32_t quality = static_cast<uint32_t>(
        std::max(0, std::min(100, options.fQuality)));
    const uint32_t srcRowBytes = static_cast<uint32_t>(src.rowBytes());

    const uint8_t* pixels = static_cast<const uint8_t*>(src.addr());
    const size_t totalBytes = static_cast<size_t>(height) * src.rowBytes();
    rust::Slice<const uint8_t> pixelSlice(pixels, totalBytes);

    rust::Vec<uint8_t> output;
    rust_jpeg::EncodingResult result = rust_jpeg::encode_jpeg(
        pixelSlice, width, height, srcRowBytes, colorType, alphaOption,
        quality, output);

    if (result != rust_jpeg::EncodingResult::Success) {
        return false;
    }

    // Write encoded JPEG data, then append XMP if provided.
    // The image crate writes a complete JPEG. For XMP, we insert an
    // APP1 marker segment right after the SOI + APP0 (JFIF) markers.
    if (options.xmpMetadata && options.xmpMetadata->size() > 0) {
        // JPEG structure: SOI (2 bytes), then APP0/JFIF segment, then rest.
        // We find the end of the first APP marker and inject APP1 XMP there.
        const uint8_t* data = output.data();
        const size_t size = output.size();

        // Find insertion point after SOI + first APP0 segment.
        size_t insertPos = 2;  // After SOI (0xFF 0xD8)
        if (size > 4 && data[2] == 0xFF && (data[3] & 0xF0) == 0xE0) {
            // APP0 segment: skip marker (2 bytes) + length (2 bytes) + data
            uint16_t segLen = (static_cast<uint16_t>(data[4]) << 8) | data[5];
            insertPos = 4 + segLen;  // 2 (marker) + 2+segLen (segment)
        }

        // Build APP1 XMP segment:
        // Marker: FF E1
        // Length: 2 + namespace.size() + 1 + xmp.size()
        static const char kXmpNamespace[] = "http://ns.adobe.com/xap/1.0/";
        const size_t nsLen = sizeof(kXmpNamespace);  // includes null terminator
        const size_t xmpSize = options.xmpMetadata->size();
        const uint16_t app1Len = static_cast<uint16_t>(2 + nsLen + xmpSize);

        // Write: before insertion, APP1 marker, rest
        dst->write(data, insertPos);
        uint8_t app1Header[4] = {
            0xFF, 0xE1,
            static_cast<uint8_t>(app1Len >> 8),
            static_cast<uint8_t>(app1Len & 0xFF)
        };
        dst->write(app1Header, 4);
        dst->write(kXmpNamespace, nsLen);
        dst->write(options.xmpMetadata->data(), xmpSize);
        dst->write(data + insertPos, size - insertPos);
    } else {
        dst->write(output.data(), output.size());
    }

    return true;
}

}  // namespace SkJpegRustEncoder
