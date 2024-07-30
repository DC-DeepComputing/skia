/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/SkPngRustCodec.h"

#include <utility>

#include "experimental/rust_png/SkPngRustCodecFFI.rs.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/src/skcms_public.h"
#include "src/codec/SkSwizzler.h"
#include "third_party/rust/cxx/v1/cxx.h"

namespace {

SkEncodedInfo::Alpha ToAlpha(SkEncodedInfo::Color color) {
    // Handling only colors that `SkPngRustCodecFFI.rs` can decode into.
    switch (color) {
        case SkEncodedInfo::kGray_Color:
        case SkEncodedInfo::kRGB_Color:
            return SkEncodedInfo::kOpaque_Alpha;
        case SkEncodedInfo::kGrayAlpha_Color:
        case SkEncodedInfo::kRGBA_Color:
            return SkEncodedInfo::kUnpremul_Alpha;
        default:
            SK_ABORT("Unexpected `ffi::ColorType`: %d", static_cast<int>(color));
    }
}

skcms_PixelFormat ToSkcmsPixelFormat(SkEncodedInfo::Color color) {
    // Handling only colors that `SkPngRustCodecFFI.rs` can decode into.
    switch (color) {
        case SkEncodedInfo::kGray_Color:
            return skcms_PixelFormat_G_8;
        case SkEncodedInfo::kRGB_Color:
            return skcms_PixelFormat_RGB_888;
        case SkEncodedInfo::kGrayAlpha_Color:
            return skcms_PixelFormat_G_8;  // DO NOT SUBMIT: GA_88
        case SkEncodedInfo::kRGBA_Color:
            return skcms_PixelFormat_RGBA_8888;
        default:
            SK_ABORT("Unexpected `ffi::ColorType`: %d", static_cast<int>(color));
    }
}

SkEncodedInfo CreateEncodedInfo(const ffi::DecodedImage& image) {
    return SkEncodedInfo::Make(
            image.width, image.height, image.color, ToAlpha(image.color), image.bits_per_component);
}

}  // namespace

// static
std::unique_ptr<SkCodec> SkPngRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                        Result* result) {
    SkASSERT(stream);
    SkASSERT(result);

    std::unique_ptr<ffi::DecodedImage> image = ffi::read_png(*stream);
    if (!image) {
        // TODO(https://crbug.com/1471546): Provide more granular conversion
        // from `png::DecodingError` to `SkCodec::Result`.
        *result = kErrorInInput;
        return nullptr;
    }

    std::unique_ptr<SkPngRustCodec> codec(new SkPngRustCodec(CreateEncodedInfo(*image),
                                                             ToSkcmsPixelFormat(image->color),
                                                             std::move(stream),
                                                             std::move(image->data)));
    *result = kSuccess;
    return codec;
}

SkCodec::Result SkPngRustCodec::onGetPixels(const SkImageInfo& dstInfo,
                                            void* dst,
                                            size_t rowBytes,
                                            const Options& options,
                                            int* rowsDecoded) {
    // TODO(https://crbug.com/1471546): Expose `png` crate's ability to decode
    // multiple frames.
    if (options.fFrameIndex != 0) {
        return kUnimplemented;
    }

    if (options.fSubset) {
        return kUnimplemented;
    }

    if ((dstInfo.width() != getEncodedInfo().width()) ||
        (dstInfo.height() != getEncodedInfo().height())) {
        return kInvalidParameters;
    }

    // Palette expansion currently takes place within the `png` crate, via
    // `png::Transformations::EXPAND`.
    SkPMColor* kColorTable = nullptr;

    std::unique_ptr<SkSwizzler> swizzler =
            SkSwizzler::Make(this->getEncodedInfo(), kColorTable, dstInfo, options);

    // Assertion is based on `png::Transformations::EXPAND`.
    SkASSERT(getEncodedInfo().bitsPerComponent() % 8 == 0);
    size_t srcRowSize =
            static_cast<size_t>(getEncodedInfo().bitsPerPixel()) / 8 * getEncodedInfo().width();

    SkSpan<const uint8_t> src = fDecodedData;
    void* dstRow = dst;
    for (int y = 0; y < getEncodedInfo().height(); ++y) {
        swizzler->swizzle(dstRow, src.data());
        dstRow = SkTAddOffset<void>(dstRow, rowBytes);
        src = src.subspan(srcRowSize);
    }

    return kSuccess;
}

SkPngRustCodec::SkPngRustCodec(SkEncodedInfo&& encodedInfo,
                               XformFormat srcFormat,
                               std::unique_ptr<SkStream> stream,
                               rust::Vec<uint8_t> decodedData)
        : INHERITED(std::move(encodedInfo), srcFormat, std::move(stream))
        , fDecodedData(std::move(decodedData)) {}

SkPngRustCodec::~SkPngRustCodec() = default;
