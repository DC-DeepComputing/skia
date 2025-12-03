/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_bmp/decoder/impl/SkBmpRustCodec.h"

#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "rust/common/SkStreamAdapter.h"
#include "rust/common/SpanUtils.h"
#include "src/base/SkSafeMath.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkStreamPriv.h"

// Color type to use when creating the swizzler for xform
//
// Note that `bitsPerComponent` is always 8 for all supported BMP images.
static constexpr SkColorType kXformSrcColorType = kRGBA_8888_SkColorType;

// Static assertions to validate that Rust BmpColor/BmpAlpha enum values
// match the corresponding SkEncodedInfo enum values. These assertions
// ensure type-safe casting between the enums.
static_assert(static_cast<int>(rust_bmp::BmpColor::RGB) ==
              static_cast<int>(SkEncodedInfo::kRGB_Color),
              "BmpColor::RGB must match SkEncodedInfo::kRGB_Color");
static_assert(static_cast<int>(rust_bmp::BmpColor::RGBA) ==
              static_cast<int>(SkEncodedInfo::kRGBA_Color),
              "BmpColor::RGBA must match SkEncodedInfo::kRGBA_Color");
static_assert(static_cast<int>(rust_bmp::BmpColor::BGR) ==
              static_cast<int>(SkEncodedInfo::kBGR_Color),
              "BmpColor::BGR must match SkEncodedInfo::kBGR_Color");
static_assert(static_cast<int>(rust_bmp::BmpColor::BGRA) ==
              static_cast<int>(SkEncodedInfo::kBGRA_Color),
              "BmpColor::BGRA must match SkEncodedInfo::kBGRA_Color");

static_assert(static_cast<int>(rust_bmp::BmpAlpha::Opaque) ==
              static_cast<int>(SkEncodedInfo::kOpaque_Alpha),
              "BmpAlpha::Opaque must match SkEncodedInfo::kOpaque_Alpha");
static_assert(static_cast<int>(rust_bmp::BmpAlpha::Unpremul) ==
              static_cast<int>(SkEncodedInfo::kUnpremul_Alpha),
              "BmpAlpha::Unpremul must match SkEncodedInfo::kUnpremul_Alpha");

namespace {

// Helper function to map Rust DecodingResult to SkCodec::Result
SkCodec::Result MapDecodingResult(rust_bmp::DecodingResult rustResult) {
    switch (rustResult) {
        case rust_bmp::DecodingResult::Success:
            return SkCodec::kSuccess;
        case rust_bmp::DecodingResult::FormatError:
            return SkCodec::kErrorInInput;
        case rust_bmp::DecodingResult::ParameterError:
            return SkCodec::kInvalidParameters;
        case rust_bmp::DecodingResult::UnsupportedFeature:
            return SkCodec::kUnimplemented;
        case rust_bmp::DecodingResult::IncompleteInput:
            return SkCodec::kIncompleteInput;
        case rust_bmp::DecodingResult::MemoryError:
            return SkCodec::kInternalError;
        case rust_bmp::DecodingResult::OtherError:
            return SkCodec::kErrorInInput;
    }
    SK_ABORT("Unexpected `rust_bmp::DecodingResult`: %d", static_cast<int>(rustResult));
}

}  // namespace

std::unique_ptr<SkBmpRustCodec> SkBmpRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                               Result* result) {
    Result resultStorage;
    if (!result) {
        result = &resultStorage;
    }

    if (!stream) {
        *result = kInvalidInput;
        return nullptr;
    }

    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(stream.get());
    rust::Box<rust_bmp::ResultOfReader> resultOfReader =
            rust_bmp::new_reader(std::move(inputAdapter));

    rust_bmp::DecodingResult rustResult = resultOfReader->err();
    if (rustResult != rust_bmp::DecodingResult::Success) {
        *result = MapDecodingResult(rustResult);
        return nullptr;
    }

    rust::Box<rust_bmp::Reader> reader = resultOfReader->unwrap();

    uint32_t width = reader->width();
    uint32_t height = reader->height();
    rust_bmp::BmpColor rustColor = reader->color();
    rust_bmp::BmpAlpha rustAlpha = reader->alpha();
    bool bottomUp = reader->bottom_up();

    // BmpColor/BmpAlpha values match SkEncodedInfo enums (validated by static_assert).
    SkEncodedInfo::Color color = static_cast<SkEncodedInfo::Color>(rustColor);
    SkEncodedInfo::Alpha alpha = static_cast<SkEncodedInfo::Alpha>(rustAlpha);
    constexpr int kBitsPerComponent = 8;

    // TODO(crbug.com/452666425): Add ICC profile support for color-managed decoding. BMP files
    // can embed ICC profiles (BITMAPV5HEADER format). Once rust/icc module
    // is available, extract profile bytes in Rust FFI, parse with skcms to
    // create sk_sp<SkData>, and pass to SkEncodedInfo::Make() below.
    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(
        width,
        height,
        color,
        alpha,
        kBitsPerComponent
    );

    SkCodec::SkScanlineOrder rowOrder = bottomUp ?
        SkCodec::kBottomUp_SkScanlineOrder : SkCodec::kTopDown_SkScanlineOrder;

    // Pre-calculate srcRowBytes (width * bytesPerPixel) and check for overflow.
    // Also verify that the total image size (height * srcRowBytes) doesn't overflow.
    size_t bytesPerPixel = (color == SkEncodedInfo::kBGR_Color) ? 3 : 4;
    SkSafeMath safe;
    size_t srcRowBytes = safe.mul(safe.castTo<size_t>(width), bytesPerPixel);
    (void)safe.mul(safe.castTo<size_t>(height), srcRowBytes);
    if (!safe.ok()) {
        *result = kInternalError;
        return nullptr;
    }

    *result = kSuccess;
    return std::unique_ptr<SkBmpRustCodec>(new SkBmpRustCodec(
        std::move(encodedInfo),
        std::move(stream),
        std::move(reader),
        rowOrder,
        srcRowBytes
    ));
}

SkBmpRustCodec::SkBmpRustCodec(SkEncodedInfo&& encodedInfo,
                               std::unique_ptr<SkStream> stream,
                               rust::Box<rust_bmp::Reader> reader,
                               SkCodec::SkScanlineOrder rowOrder,
                               size_t srcRowBytes)
    : SkCodec(std::move(encodedInfo), skcms_PixelFormat_BGR_888,
              // TODO(crbug.com/370522089): Pass stream to SkCodec once SkCodec
              // avoids unnecessary rewinding (which forces re-reading entire stream).
              /* stream = */ nullptr)
    , fReader(std::move(reader))
    , fDecodingState()
    , fPrivStream(std::move(stream))
    , fRowOrder(rowOrder)
    , fSrcRowBytes(srcRowBytes) {
    SkASSERT_RELEASE(fPrivStream);
}

SkBmpRustCodec::~SkBmpRustCodec() = default;

bool SkBmpRustCodec::onRewind() {
    if (!fPrivStream->rewind()) {
        return false;
    }

    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(fPrivStream.get());
    rust::Box<rust_bmp::ResultOfReader> resultOfReader =
            rust_bmp::new_reader(std::move(inputAdapter));

    if (resultOfReader->err() != rust_bmp::DecodingResult::Success) {
        return false;
    }

    fReader = resultOfReader->unwrap();
    fDecodingState.clear();

    return true;
}

void SkBmpRustCodec::initializeSwizzler(const SkImageInfo& dstInfo, const Options& opts) {
    SkImageInfo swizzlerInfo = dstInfo;
    SkCodec::Options swizzlerOptions = opts;

    if (this->xformOnDecode()) {
        fXformBuffer.reset(new uint32_t[dstInfo.width()]);
        swizzlerInfo = swizzlerInfo.makeColorType(kXformSrcColorType);
        if (kPremul_SkAlphaType == dstInfo.alphaType()) {
            swizzlerInfo = swizzlerInfo.makeAlphaType(kUnpremul_SkAlphaType);
        }
        swizzlerOptions.fZeroInitialized = kNo_ZeroInitialized;
    }

    fSwizzler = SkSwizzler::Make(this->getEncodedInfo(), nullptr, swizzlerInfo, swizzlerOptions);
    // BMP images only use kRGB_Color, kRGBA_Color, kBGR_Color, or kBGRA_Color (see FFI.rs
    // BmpColor enum). SkSwizzler supports all of these with standard destination color types,
    // so Make should always succeed.
    SkASSERT(fSwizzler);
}

void SkBmpRustCodec::swizzleRow(const uint8_t* srcRow, void* dstRow) {
    if (this->xformOnDecode()) {
        const int32_t width = this->dimensions().width();
        fSwizzler->swizzle(fXformBuffer.get(), srcRow);
        this->applyColorXform(dstRow, fXformBuffer.get(), width);
    } else {
        fSwizzler->swizzle(dstRow, srcRow);
    }
}

SkCodec::Result SkBmpRustCodec::onGetPixels(const SkImageInfo& info,
                                            void* dst,
                                            size_t dstRowStride,
                                            const Options& options,
                                            int* rowsDecoded) {
    this->initializeSwizzler(info, options);

    return this->performFullDecode(info, dst, dstRowStride);
}

SkCodec::Result SkBmpRustCodec::performFullDecode(const SkImageInfo& dstInfo,
                                                  void* dst,
                                                  size_t dstRowStride) {
    Result ensureResult = this->ensureDecodedData();
    if (ensureResult != kSuccess) {
        return ensureResult;
    }

    const int32_t height = this->dimensions().height();

    SkSpan<const uint8_t> srcImage = SkSpan(fDecodingState.fRustDecodedData);
    for (int y = 0; y < height; ++y) {
        SkSpan<const uint8_t> srcRow = srcImage.first(fSrcRowBytes);
        void* dstRow = SkTAddOffset<void>(dst, y * dstRowStride);

        this->swizzleRow(srcRow.data(), dstRow);

        srcImage = srcImage.subspan(fSrcRowBytes);
    }

    return fDecodingState.fDataIsIncomplete ? kIncompleteInput : kSuccess;
}

SkCodec::Result SkBmpRustCodec::ensureDecodedData() {
    if (fDecodingState.fFullyDecoded) {
        return kSuccess;
    }

    // Reset the reader to start decoding from the beginning. This is important when:
    // 1. getPixels returns kIncompleteInput but has done some decoding
    // 2. more data becomes available in SkStream
    // 3. getPixels is called again. To have proper decoding, reader state should be reset.
    fReader->reset_decode_state();

    const int32_t height = this->dimensions().height();

    SkSafeMath safe;
    size_t totalSize = safe.mul(fSrcRowBytes, safe.castTo<size_t>(height));
    if (!safe.ok()) {
        return kInternalError;
    }

    fDecodingState.fRustDecodedData.resize(totalSize);

    SkSpan<uint8_t> decodedImage = SkSpan(fDecodingState.fRustDecodedData);
    for (int y = 0; y < height; ++y) {
        SkSpan<uint8_t> rowSpan = decodedImage.first(fSrcRowBytes);
        rust::Slice<uint8_t> rowSlice(rowSpan.data(), fSrcRowBytes);

        rust_bmp::DecodingResult result = fReader->next_row(rowSlice);
        if (result == rust_bmp::DecodingResult::IncompleteInput) {
            fDecodingState.fDataIsIncomplete = true;
            return kIncompleteInput;
        }
        if (result != rust_bmp::DecodingResult::Success) {
            return kErrorInInput;
        }

        decodedImage = decodedImage.subspan(fSrcRowBytes);
    }

    fDecodingState.fFullyDecoded = true;
    fDecodingState.fDataIsIncomplete = false;

    return kSuccess;
}

SkCodec::Result SkBmpRustCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
                                                      const Options& options) {
    fReader->reset_decode_state();
    fDecodingState.clear();

    this->initializeSwizzler(dstInfo, options);

    return kSuccess;
}

int SkBmpRustCodec::onGetScanlines(void* dst, int count, size_t rowBytes) {
    const int32_t height = this->dimensions().height();

    int linesDecoded = 0;
    uint8_t* dstPtr = static_cast<uint8_t*>(dst);

    for (int i = 0; i < count && fDecodingState.fCurrentScanline < height; ++i) {
        SkSafeMath loopSafe;
        size_t currentOffset = loopSafe.mul(fDecodingState.fCurrentScanline, fSrcRowBytes);
        size_t neededDataSize = loopSafe.add(currentOffset, fSrcRowBytes);
        if (!loopSafe.ok()) {
            fDecodingState.fDataIsIncomplete = true;
            break;
        }

        if (fDecodingState.fRustDecodedData.size() < neededDataSize) {
            fDecodingState.fRustDecodedData.resize(neededDataSize);

            SkSpan<uint8_t> decodedImage = SkSpan(fDecodingState.fRustDecodedData);
            SkSpan<uint8_t> rowSpan = decodedImage.subspan(currentOffset, fSrcRowBytes);
            rust::Slice<uint8_t> row_span(rowSpan.data(), fSrcRowBytes);

            auto result = fReader->next_row(row_span);

            if (result != rust_bmp::DecodingResult::Success) {
                fDecodingState.fDataIsIncomplete = true;
                break;
            }
        }

        SkSpan<const uint8_t> srcImage = SkSpan(fDecodingState.fRustDecodedData);
        SkSpan<const uint8_t> srcRow = srcImage.subspan(currentOffset, fSrcRowBytes);

        this->swizzleRow(srcRow.data(), dstPtr);

        dstPtr += rowBytes;
        fDecodingState.fCurrentScanline++;
        linesDecoded++;
    }

    return linesDecoded;
}

bool SkBmpRustCodec::onSkipScanlines(int count) {
    const int32_t height = this->dimensions().height();

    int linesToSkip = std::min(count, height - fDecodingState.fCurrentScanline);

    std::vector<uint8_t> tempBuffer(fSrcRowBytes);
    for (int i = 0; i < linesToSkip; ++i) {
        rust::Slice<uint8_t> row_span(tempBuffer.data(), fSrcRowBytes);
        auto result = fReader->next_row(row_span);
        if (result != rust_bmp::DecodingResult::Success) {
            fDecodingState.fDataIsIncomplete = true;
            return false;
        }

        fDecodingState.fCurrentScanline++;
    }

    return linesToSkip == count;
}
