/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/SkPngRustCodec.h"

#include <utility>

#include "experimental/rust_png/ffi/FFI.rs.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/src/skcms_public.h"
#include "src/codec/SkSwizzler.h"
#include "third_party/rust/cxx/v1/cxx.h"

namespace {

SkEncodedInfo::Color ToColor(rust_png::ColorType color_type) {
    switch (color_type) {
        case rust_png::ColorType::Grayscale:
            return SkEncodedInfo::kGray_Color;
        case rust_png::ColorType::Rgb:
            return SkEncodedInfo::kRGB_Color;
        case rust_png::ColorType::GrayscaleAlpha:
            return SkEncodedInfo::kGrayAlpha_Color;
        case rust_png::ColorType::Rgba:
            return SkEncodedInfo::kRGBA_Color;
        // `Indexed` is impossible, because of `png::Transformations::EXPAND`.
        case rust_png::ColorType::Indexed:
            break;
    }
    SK_ABORT("Unexpected `rust_png::ColorType`: %d", static_cast<int>(color_type));
}

SkEncodedInfo::Alpha ToAlpha(rust_png::ColorType color_type) {
    switch (color_type) {
        case rust_png::ColorType::Grayscale:
        case rust_png::ColorType::Rgb:
            return SkEncodedInfo::kOpaque_Alpha;
        case rust_png::ColorType::GrayscaleAlpha:
        case rust_png::ColorType::Rgba:
            return SkEncodedInfo::kUnpremul_Alpha;
        // `Indexed` is impossible, because of `png::Transformations::EXPAND`.
        case rust_png::ColorType::Indexed:
            break;
    }
    SK_ABORT("Unexpected `rust_png::ColorType`: %d", static_cast<int>(color_type));
}

skcms_PixelFormat ToPixelFormat(rust_png::ColorType color_type) {
    // Handling only colors that `SkPngRustCodecFFI.rs` can decode into.
    switch (color_type) {
        case rust_png::ColorType::Grayscale:
            return skcms_PixelFormat_G_8;
        case rust_png::ColorType::Rgb:
            return skcms_PixelFormat_RGB_888;
        case rust_png::ColorType::GrayscaleAlpha:
            return skcms_PixelFormat_G_8;  // DO NOT SUBMIT: GA_88
        case rust_png::ColorType::Rgba:
            return skcms_PixelFormat_RGBA_8888;
        // `Indexed` is impossible, because of `png::Transformations::EXPAND`.
        case rust_png::ColorType::Indexed:
            break;
    }
    SK_ABORT("Unexpected `rust_png::ColorType`: %d", static_cast<int>(color_type));
}

SkEncodedInfo CreateEncodedInfo(const rust_png::DecodedImage& image) {
    return SkEncodedInfo::Make(image.width,
                               image.height,
                               ToColor(image.color),
                               ToAlpha(image.color),
                               image.bits_per_component);
}

// DO NOT SUBMIT?: This wrapper shouldn't be needed, but somehow without it we
// run into https://crbug.com/356632494
class ReadTraitWrapper : public rust_png::ReadTrait {
public:
    explicit ReadTraitWrapper(SkStream* stream) : fStream(stream) {}
    ~ReadTraitWrapper() = default;

    ReadTraitWrapper(const ReadTraitWrapper&) = delete;
    ReadTraitWrapper& operator=(const ReadTraitWrapper&) = delete;
    ReadTraitWrapper(ReadTraitWrapper&&) = delete;
    ReadTraitWrapper& operator=(ReadTraitWrapper&&) = delete;

    size_t read(rust::Slice<uint8_t> buffer) override {
        // Avoiding operating on `buffer.data()` if the slice is empty helps to avoid
        // UB risk described at https://davidben.net/2024/01/15/empty-slices.html.
        if (buffer.empty()) {
            return 0;
        }

        return fStream->read(buffer.data(), buffer.size());
    }

private:
    SkStream* fStream = nullptr;
};

}  // namespace

// static
std::unique_ptr<SkPngRustCodec> SkPngRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                               Result* result) {
    SkASSERT(stream);
    SkASSERT(result);

    // DO NOT SUBMIT
    ReadTraitWrapper wrapper(stream.get());
    rust_png::ReadTrait* read_trait_ptr = &wrapper;
    // Comment out the line above + uncomment the line below to hit
    // https://crbug.com/356632494.
    // rust_png::ReadTrait* read_trait_ptr = stream.get();

    std::unique_ptr<rust_png::DecodedImage> image = rust_png::read_png(*read_trait_ptr);
    if (!image) {
        // TODO(https://crbug.com/1471546): Provide more granular conversion
        // from `png::DecodingError` to `SkCodec::Result`.
        *result = kErrorInInput;
        return nullptr;
    }

    std::unique_ptr<SkPngRustCodec> codec(new SkPngRustCodec(CreateEncodedInfo(*image),
                                                             ToPixelFormat(image->color),
                                                             std::move(stream),
                                                             std::move(image->data)));
    *result = kSuccess;
    return codec;
}

SkPngRustCodec::~SkPngRustCodec() = default;

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
