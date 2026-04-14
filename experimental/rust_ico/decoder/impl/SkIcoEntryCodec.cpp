/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_ico/decoder/impl/SkIcoEntryCodec.h"

#include "experimental/rust_ico/ffi/FFI.rs.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "rust/common/SkStreamAdapter.h"
#include "src/base/SkSafeMath.h"
#include "src/codec/SkSwizzler.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

// Static assertions to validate Rust enum values match SkEncodedInfo
static_assert(static_cast<int>(rust_ico::IcoColor::RGBA) ==
              static_cast<int>(SkEncodedInfo::kRGBA_Color),
              "IcoColor::RGBA must match SkEncodedInfo::kRGBA_Color");

static_assert(static_cast<int>(rust_ico::IcoAlpha::Opaque) ==
              static_cast<int>(SkEncodedInfo::kOpaque_Alpha),
              "IcoAlpha::Opaque must match SkEncodedInfo::kOpaque_Alpha");
static_assert(static_cast<int>(rust_ico::IcoAlpha::Unpremul) ==
              static_cast<int>(SkEncodedInfo::kUnpremul_Alpha),
              "IcoAlpha::Unpremul must match SkEncodedInfo::kUnpremul_Alpha");
static_assert(static_cast<int>(rust_ico::IcoAlpha::Binary) ==
              static_cast<int>(SkEncodedInfo::kBinary_Alpha),
              "IcoAlpha::Binary must match SkEncodedInfo::kBinary_Alpha");

namespace {

SkCodec::Result MapDecodingResult(rust_ico::DecodingResult rustResult) {
    switch (rustResult) {
        case rust_ico::DecodingResult::Success:
            return SkCodec::kSuccess;
        case rust_ico::DecodingResult::FormatError:
            return SkCodec::kErrorInInput;
        case rust_ico::DecodingResult::ParameterError:
            return SkCodec::kInvalidParameters;
        case rust_ico::DecodingResult::UnsupportedFeature:
            return SkCodec::kUnimplemented;
        case rust_ico::DecodingResult::IncompleteInput:
            return SkCodec::kIncompleteInput;
        case rust_ico::DecodingResult::MemoryError:
            return SkCodec::kInternalError;
        case rust_ico::DecodingResult::OtherError:
            return SkCodec::kErrorInInput;
    }
    return SkCodec::kInternalError;
}

}  // namespace

std::unique_ptr<SkIcoEntryCodec> SkIcoEntryCodec::Make(
        std::unique_ptr<SkStream> stream,
        rust_ico::EmbeddedFormat format,
        uint32_t entrySize,
        Result* result) {
    Result resultStorage;
    if (!result) {
        result = &resultStorage;
    }

    if (!stream) {
        *result = kInvalidInput;
        return nullptr;
    }

    // Create Rust reader using stream adapter
    auto streamAdapter = std::make_unique<rust::stream::SkStreamAdapter>(stream.get());
    rust::Box<rust_ico::ResultOfReader> resultOfReader =
            rust_ico::new_reader(std::move(streamAdapter), format, entrySize);

    rust_ico::DecodingResult rustResult = resultOfReader->err();
    if (rustResult != rust_ico::DecodingResult::Success) {
        *result = MapDecodingResult(rustResult);
        return nullptr;
    }

    rust::Box<rust_ico::Reader> reader = resultOfReader->unwrap();

    uint32_t width = reader->width();
    uint32_t height = reader->height();
    rust_ico::IcoColor rustColor = reader->color();
    rust_ico::IcoAlpha rustAlpha = reader->alpha();

    SkEncodedInfo::Color color = static_cast<SkEncodedInfo::Color>(rustColor);
    SkEncodedInfo::Alpha alpha = static_cast<SkEncodedInfo::Alpha>(rustAlpha);
    constexpr int kBitsPerComponent = 8;

    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(
            width, height, color, alpha, kBitsPerComponent);

    // Calculate row bytes (RGBA = 4 bytes per pixel)
    constexpr size_t kBytesPerPixel = 4;
    SkSafeMath safe;
    size_t srcRowBytes = safe.mul(safe.castTo<size_t>(width), kBytesPerPixel);
    (void)safe.mul(safe.castTo<size_t>(height), srcRowBytes);
    if (!safe.ok()) {
        *result = kInternalError;
        return nullptr;
    }

    *result = kSuccess;
    return std::unique_ptr<SkIcoEntryCodec>(new SkIcoEntryCodec(
            std::move(encodedInfo),
            std::move(stream),
            std::move(reader),
            format,
            srcRowBytes));
}

SkIcoEntryCodec::SkIcoEntryCodec(SkEncodedInfo&& encodedInfo,
                                 std::unique_ptr<SkStream> stream,
                                 rust::Box<rust_ico::Reader> reader,
                                 rust_ico::EmbeddedFormat format,
                                 size_t srcRowBytes)
        : SkCodec(std::move(encodedInfo), skcms_PixelFormat_RGBA_8888, nullptr)
        , fPrivStream(std::move(stream))
        , fReader(std::move(reader))
        , fFormat(format)
        , fSrcRowBytes(srcRowBytes) {
    SkASSERT_RELEASE(fPrivStream);
}

SkIcoEntryCodec::~SkIcoEntryCodec() = default;

SkCodec::Result SkIcoEntryCodec::initializeSwizzler(const SkImageInfo& dstInfo,
                                                     const Options& options) {
    fSwizzler = SkSwizzler::Make(this->getEncodedInfo(), nullptr, dstInfo, options);
    if (!fSwizzler) {
        return kInvalidConversion;
    }

    // Allocate source buffer for one row
    fSrcBuffer.resize(fSrcRowBytes);
    return kSuccess;
}

SkCodec::Result SkIcoEntryCodec::onGetPixels(const SkImageInfo& dstInfo,
                                              void* dst,
                                              size_t dstRowBytes,
                                              const Options& options,
                                              int* rowsDecoded) {
    // Reset decoder state for re-decode
    fReader->reset_decode_state();
    fCurrScanline = 0;

    Result result = this->initializeSwizzler(dstInfo, options);
    if (result != kSuccess) {
        return result;
    }

    const int32_t height = this->dimensions().height();

    rust::Slice<uint8_t> rowSlice(fSrcBuffer.data(), fSrcRowBytes);

    for (int32_t y = 0; y < height; ++y) {
        rust_ico::DecodingResult rustResult = fReader->next_row(rowSlice);
        if (rustResult != rust_ico::DecodingResult::Success) {
            if (rowsDecoded) {
                *rowsDecoded = y;
            }
            return MapDecodingResult(rustResult);
        }

        void* dstRow = static_cast<uint8_t*>(dst) + y * dstRowBytes;
        fSwizzler->swizzle(dstRow, fSrcBuffer.data());
    }

    if (rowsDecoded) {
        *rowsDecoded = height;
    }
    return kSuccess;
}

// ============================================================================
// Scanline decode support
// ============================================================================

SkCodec::Result SkIcoEntryCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
                                                        const Options& options) {
    // Reset decoder state
    fReader->reset_decode_state();
    fCurrScanline = 0;

    return this->initializeSwizzler(dstInfo, options);
}

int SkIcoEntryCodec::onGetScanlines(void* dst, int count, size_t rowBytes) {
    const int32_t height = this->dimensions().height();
    int rowsDecoded = 0;

    rust::Slice<uint8_t> rowSlice(fSrcBuffer.data(), fSrcRowBytes);

    for (int i = 0; i < count && fCurrScanline < height; ++i) {
        rust_ico::DecodingResult rustResult = fReader->next_row(rowSlice);
        if (rustResult != rust_ico::DecodingResult::Success) {
            return rowsDecoded;
        }

        void* dstRow = static_cast<uint8_t*>(dst) + i * rowBytes;
        fSwizzler->swizzle(dstRow, fSrcBuffer.data());

        fCurrScanline++;
        rowsDecoded++;
    }

    return rowsDecoded;
}

bool SkIcoEntryCodec::onSkipScanlines(int count) {
    const int32_t height = this->dimensions().height();

    rust::Slice<uint8_t> rowSlice(fSrcBuffer.data(), fSrcRowBytes);

    for (int i = 0; i < count && fCurrScanline < height; ++i) {
        rust_ico::DecodingResult rustResult = fReader->next_row(rowSlice);
        if (rustResult != rust_ico::DecodingResult::Success) {
            return false;
        }
        fCurrScanline++;
    }

    return true;
}

// ============================================================================
// Incremental decode support
// ============================================================================

SkCodec::Result SkIcoEntryCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                           void* dst,
                                                           size_t rowBytes,
                                                           const Options& options) {
    // Reset decoder state
    fReader->reset_decode_state();
    fCurrScanline = 0;

    Result result = this->initializeSwizzler(dstInfo, options);
    if (result != kSuccess) {
        return result;
    }

    fIncrementalDst = dst;
    fIncrementalRowBytes = rowBytes;

    return kSuccess;
}

SkCodec::Result SkIcoEntryCodec::onIncrementalDecode(int* rowsDecoded) {
    const int32_t height = this->dimensions().height();

    rust::Slice<uint8_t> rowSlice(fSrcBuffer.data(), fSrcRowBytes);

    while (fCurrScanline < height) {
        rust_ico::DecodingResult rustResult = fReader->next_row(rowSlice);
        if (rustResult == rust_ico::DecodingResult::IncompleteInput) {
            if (rowsDecoded) {
                *rowsDecoded = fCurrScanline;
            }
            return kIncompleteInput;
        }
        if (rustResult != rust_ico::DecodingResult::Success) {
            if (rowsDecoded) {
                *rowsDecoded = fCurrScanline;
            }
            return MapDecodingResult(rustResult);
        }

        void* dstRow = static_cast<uint8_t*>(fIncrementalDst) + fCurrScanline * fIncrementalRowBytes;
        fSwizzler->swizzle(dstRow, fSrcBuffer.data());

        fCurrScanline++;
    }

    if (rowsDecoded) {
        *rowsDecoded = height;
    }
    return kSuccess;
}

// ============================================================================
// Sampler support
// ============================================================================

SkSampler* SkIcoEntryCodec::getSampler(bool createIfNecessary) {
    if (fSwizzler) {
        return fSwizzler.get();
    }
    return nullptr;
}
