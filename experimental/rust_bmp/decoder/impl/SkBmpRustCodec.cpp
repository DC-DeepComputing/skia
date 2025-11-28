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
        default:
            return SkCodec::kErrorInInput;
    }
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
    int bitsPerComponent = 8;

    // TODO(crbug.com/452666425): Add ICC profile support for color-managed decoding. BMP files
    // can embed ICC profiles (BITMAPV5HEADER format). Once rust/icc module
    // is available, extract profile bytes in Rust FFI, parse with skcms to
    // create sk_sp<SkData>, and pass to SkEncodedInfo::Make() below.
    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(
        width,
        height,
        color,
        alpha,
        bitsPerComponent
    );

    SkCodec::SkScanlineOrder rowOrder = bottomUp ?
        SkCodec::kBottomUp_SkScanlineOrder : SkCodec::kTopDown_SkScanlineOrder;

    *result = kSuccess;
    return std::unique_ptr<SkBmpRustCodec>(new SkBmpRustCodec(
        std::move(encodedInfo),
        std::move(stream),
        std::move(reader),
        rowOrder
    ));
}

SkBmpRustCodec::SkBmpRustCodec(SkEncodedInfo&& encodedInfo,
                               std::unique_ptr<SkStream> stream,
                               rust::Box<rust_bmp::Reader> reader,
                               SkCodec::SkScanlineOrder rowOrder)
    : SkCodec(std::move(encodedInfo), skcms_PixelFormat_BGR_888,
              // TODO(crbug.com/370522089): Pass stream to SkCodec once SkCodec
              // avoids unnecessary rewinding (which forces re-reading entire stream).
              /* stream = */ nullptr)
    , fReader(std::move(reader))
    , fDecodingState(DecodingState{})
    , fPrivStream(std::move(stream))
    , fRowOrder(rowOrder) {
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
    fDecodingState = DecodingState{};

    return true;
}

void SkBmpRustCodec::initializeSwizzler(const SkImageInfo& dstInfo, const Options& opts) {
    SkImageInfo swizzlerInfo = dstInfo;
    SkCodec::Options swizzlerOptions = opts;

    if (this->xformOnDecode()) {
        swizzlerInfo = swizzlerInfo.makeColorType(kXformSrcColorType);
        if (kPremul_SkAlphaType == dstInfo.alphaType()) {
            swizzlerInfo = swizzlerInfo.makeAlphaType(kUnpremul_SkAlphaType);
        }
        swizzlerOptions.fZeroInitialized = kNo_ZeroInitialized;
    }

    fSwizzler = SkSwizzler::Make(this->getEncodedInfo(), nullptr, swizzlerInfo, swizzlerOptions);
    SkASSERT(fSwizzler);
}

SkCodec::Result SkBmpRustCodec::onGetPixels(const SkImageInfo& info,
                                            void* dst,
                                            size_t dstRowStride,
                                            const Options& options,
                                            int* rowsDecoded) {
    if (this->xformOnDecode()) {
        fXformBuffer.reset(new uint32_t[info.width()]);
    }

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

    const int width = this->dimensions().width();
    const int height = this->dimensions().height();

    SkSafeMath safe;
    const size_t srcRowBytes = safe.mul(width, fDecodingState->fBytesPerPixel);
    if (!safe.ok()) {
        return kInternalError;
    }

    SkSpan<const uint8_t> srcImage = SkSpan(fDecodingState->fRustDecodedData);
    for (int y = 0; y < height; ++y) {
        SkSpan<const uint8_t> srcRow = srcImage.first(srcRowBytes);
        void* dstRow = SkTAddOffset<void>(dst, y * dstRowStride);

        if (this->xformOnDecode()) {
            fSwizzler->swizzle(fXformBuffer.get(), srcRow.data());
            this->applyColorXform(dstRow, fXformBuffer.get(), width);
        } else {
            fSwizzler->swizzle(dstRow, srcRow.data());
        }

        srcImage = srcImage.subspan(srcRowBytes);
    }

    return fDecodingState->fDataIsIncomplete ? kIncompleteInput : kSuccess;
}

SkCodec::Result SkBmpRustCodec::ensureDecodedData() {
    if (fDecodingState->fFullyDecoded) {
        return kSuccess;
    }

    const int height = this->dimensions().height();
    const int width = this->dimensions().width();

    uint32_t bytesPerPixel = 3;
    if (this->getEncodedInfo().color() == SkEncodedInfo::kRGBA_Color ||
        this->getEncodedInfo().color() == SkEncodedInfo::kBGRA_Color) {
        bytesPerPixel = 4;
    }

    fDecodingState->fBytesPerPixel = bytesPerPixel;

    SkSafeMath safe;
    size_t rowBytes = safe.mul(width, bytesPerPixel);
    size_t totalSize = safe.mul(rowBytes, height);
    if (!safe.ok()) {
        return kInternalError;
    }

    fDecodingState->fRustDecodedData.resize(totalSize);

    SkSpan<uint8_t> decodedImage = SkSpan(fDecodingState->fRustDecodedData);
    for (int y = 0; y < height; ++y) {
        SkSpan<uint8_t> rowSpan = decodedImage.first(rowBytes);
        rust::Slice<uint8_t> rowSlice(rowSpan.data(), rowBytes);

        rust_bmp::DecodingResult result = fReader->next_row(rowSlice);
        if (result == rust_bmp::DecodingResult::IncompleteInput) {
            fDecodingState->fDataIsIncomplete = true;
            return kIncompleteInput;
        }
        if (result != rust_bmp::DecodingResult::Success) {
            return kErrorInInput;
        }

        decodedImage = decodedImage.subspan(rowBytes);
    }

    fDecodingState->fFullyDecoded = true;
    fDecodingState->fDataIsIncomplete = false;

    return kSuccess;
}

SkCodec::Result SkBmpRustCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
                                                      const Options& options) {
    fReader->reset_decode_state();
    fDecodingState = DecodingState{};

    rust_bmp::BmpColor color = fReader->color();

    if (color == rust_bmp::BmpColor::BGR || color == rust_bmp::BmpColor::RGB) {
        fDecodingState->fBytesPerPixel = 3;
    } else if (color == rust_bmp::BmpColor::BGRA || color == rust_bmp::BmpColor::RGBA) {
        fDecodingState->fBytesPerPixel = 4;
    } else {
        return kUnimplemented;
    }

    fDecodingState->fCurrentScanline = 0;
    fDecodingState->fFullyDecoded = false;
    fDecodingState->fDataIsIncomplete = false;
    fDecodingState->fRustDecodedData.clear();

    if (this->xformOnDecode()) {
        fXformBuffer.reset(new uint32_t[dstInfo.width()]);
    }

    this->initializeSwizzler(dstInfo, options);

    return kSuccess;
}

int SkBmpRustCodec::onGetScanlines(void* dst, int count, size_t rowBytes) {
    const int width = this->dimensions().width();
    const int height = this->dimensions().height();

    SkSafeMath safe;
    const size_t srcRowBytes = safe.mul(width, fDecodingState->fBytesPerPixel);
    if (!safe.ok()) {
        return 0;  // Return 0 lines decoded on overflow
    }

    int linesDecoded = 0;
    uint8_t* dstPtr = static_cast<uint8_t*>(dst);

    for (int i = 0; i < count && fDecodingState->fCurrentScanline < height; ++i) {
        SkSafeMath loopSafe;
        size_t currentOffset = loopSafe.mul(fDecodingState->fCurrentScanline, srcRowBytes);
        size_t neededDataSize = loopSafe.add(currentOffset, srcRowBytes);
        if (!loopSafe.ok()) {
            fDecodingState->fDataIsIncomplete = true;
            break;
        }

        if (fDecodingState->fRustDecodedData.size() < neededDataSize) {
            fDecodingState->fRustDecodedData.resize(neededDataSize);

            SkSpan<uint8_t> decodedImage = SkSpan(fDecodingState->fRustDecodedData);
            SkSpan<uint8_t> rowSpan = decodedImage.subspan(currentOffset, srcRowBytes);
            rust::Slice<uint8_t> row_span(rowSpan.data(), srcRowBytes);

            auto result = fReader->next_row(row_span);

            if (result != rust_bmp::DecodingResult::Success) {
                fDecodingState->fDataIsIncomplete = true;
                break;
            }
        }

        SkSpan<const uint8_t> srcImage = SkSpan(fDecodingState->fRustDecodedData);
        SkSpan<const uint8_t> srcRow = srcImage.subspan(currentOffset, srcRowBytes);

        if (this->xformOnDecode()) {
            fSwizzler->swizzle(fXformBuffer.get(), srcRow.data());
            this->applyColorXform(dstPtr, fXformBuffer.get(), width);
        } else {
            fSwizzler->swizzle(dstPtr, srcRow.data());
        }

        dstPtr += rowBytes;
        fDecodingState->fCurrentScanline++;
        linesDecoded++;
    }

    return linesDecoded;
}

bool SkBmpRustCodec::onSkipScanlines(int count) {
    const int height = this->dimensions().height();
    const int width = this->dimensions().width();
    const size_t srcRowBytes = width * fDecodingState->fBytesPerPixel;

    int linesToSkip = std::min(count, height - fDecodingState->fCurrentScanline);

    std::vector<uint8_t> tempBuffer(srcRowBytes);
    for (int i = 0; i < linesToSkip; ++i) {
        size_t neededDataSize = (fDecodingState->fCurrentScanline + 1) * srcRowBytes;

        if (fDecodingState->fRustDecodedData.size() < neededDataSize) {
            rust::Slice<uint8_t> row_span(tempBuffer.data(), srcRowBytes);
            auto result = fReader->next_row(row_span);
            if (result != rust_bmp::DecodingResult::Success) {
                fDecodingState->fDataIsIncomplete = true;
                return false;
            }
        }

        fDecodingState->fCurrentScanline++;
    }

    return linesToSkip == count;
}
