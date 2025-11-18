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
    if (!stream) {
        *result = kInvalidInput;
        return nullptr;
    }

    SkASSERT_RELEASE(stream);

    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(stream.get());
    rust::Box<rust_bmp::ResultOfReader> resultOfReader =
            rust_bmp::new_reader(std::move(inputAdapter));

    rust_bmp::DecodingResult rustResult = resultOfReader->err();
    if (rustResult != rust_bmp::DecodingResult::Success) {
        *result = MapDecodingResult(rustResult);
        return nullptr;
    }

    rust::Box<rust_bmp::Reader> reader = resultOfReader->unwrap();

    // Get image info from reader
    uint32_t width = reader->width();
    uint32_t height = reader->height();
    rust_bmp::ColorType rustColorType = reader->color_type();
    rust_bmp::AlphaType rustAlphaType = reader->alpha_type();
    bool bottomUp = reader->bottom_up();

    SkEncodedInfo::Color color;
    SkEncodedInfo::Alpha alpha;
    int bitsPerComponent = 8;

    switch (rustColorType) {
        case rust_bmp::ColorType::Rgb:
            color = SkEncodedInfo::kRGB_Color;
            break;
        case rust_bmp::ColorType::Bgr:
            color = SkEncodedInfo::kBGR_Color;
            break;
        case rust_bmp::ColorType::Rgba:
            color = SkEncodedInfo::kRGBA_Color;
            break;
        case rust_bmp::ColorType::Bgra:
            color = SkEncodedInfo::kBGRA_Color;
            break;
        default:
            *result = kUnimplemented;
            return nullptr;
    }

    switch (rustAlphaType) {
        case rust_bmp::AlphaType::Opaque:
            alpha = SkEncodedInfo::kOpaque_Alpha;
            break;
        case rust_bmp::AlphaType::Unpremul:
            alpha = SkEncodedInfo::kUnpremul_Alpha;
            break;
        case rust_bmp::AlphaType::Premul:
            // BMP doesn't support premultiplied alpha; treat as unpremul
            alpha = SkEncodedInfo::kUnpremul_Alpha;
            break;
        default:
            alpha = SkEncodedInfo::kOpaque_Alpha;
            break;
    }

    // TODO: Add ICC profile support.
    //
    // BMP files can embed ICC color profiles (typically in BITMAPV5HEADER format).
    // To support this properly:
    // 1. Extract ICC profile bytes from the image-rs BmpDecoder (if available)
    // 2. Parse with skcms to create SkData for the profile
    // 3. Pass to SkEncodedInfo::Make() for proper color space handling
    //
    // Note: For a future Rust-based implementation, we could potentially use
    // moxcms (Rust port of lcms/skcms) for ICC profile parsing, avoiding
    // the need to pass raw bytes across the FFI boundary.
    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(
        width,
        height,
        color,
        alpha,
        bitsPerComponent
    );

    // Determine scanline order based on reader
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
    : INHERITED(std::move(encodedInfo), skcms_PixelFormat_BGR_888,
                // TODO(https://crbug.com/370522089): If/when `SkCodec` can
                // avoid unnecessary rewinding, then stop "hiding" our stream
                // from it.
                /* stream = */ nullptr)
    , fReader(std::move(reader))
    , fDecodingState(DecodingState{})
    , fPrivStream(std::move(stream))
    , fRowOrder(rowOrder) {
    SkASSERT_RELEASE(fPrivStream);
}

SkBmpRustCodec::~SkBmpRustCodec() = default;

bool SkBmpRustCodec::onRewind() {
    // Rewind the stream
    if (!fPrivStream->rewind()) {
        return false;
    }

    // Create a new reader from the rewound stream
    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(fPrivStream.get());
    rust::Box<rust_bmp::ResultOfReader> resultOfReader =
            rust_bmp::new_reader(std::move(inputAdapter));

    if (resultOfReader->err() != rust_bmp::DecodingResult::Success) {
        return false;
    }

    fReader = resultOfReader->unwrap();

    // Reset decoding state
    fDecodingState = DecodingState{};

    return true;
}

void SkBmpRustCodec::initializeSwizzler(const SkImageInfo& dstInfo, const Options& opts) {
    // Create swizzler to handle pixel format conversions (BGR -> RGBA, etc.)
    SkImageInfo swizzlerInfo = dstInfo;
    SkCodec::Options swizzlerOptions = opts;

    if (this->xformOnDecode()) {
        // When color transform is needed, swizzle to RGBA first, then apply color xform
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

    // Initialize swizzler for pixel format conversions
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

    // Decode using SkSwizzler for pixel format conversion and optionally
    // applyColorXform for color space transformation
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
            // Swizzle to xform buffer, then apply color transformation
            fSwizzler->swizzle(fXformBuffer.get(), srcRow.data());
            this->applyColorXform(dstRow, fXformBuffer.get(), width);
        } else {
            // Swizzle directly to destination
            fSwizzler->swizzle(dstRow, srcRow.data());
        }

        srcImage = srcImage.subspan(srcRowBytes);
    }

    // Return kIncompleteInput if we decoded with partial data
    return fDecodingState->fDataIsIncomplete ? kIncompleteInput : kSuccess;
}

SkCodec::Result SkBmpRustCodec::ensureDecodedData() {
    if (fDecodingState->fFullyDecoded) {
        return kSuccess; // Data already decoded
    }

    // Use streaming reader to decode row-by-row
    const int height = this->dimensions().height();
    const int width = this->dimensions().width();

    // Determine bytes per pixel from color type
    uint32_t bytesPerPixel = 3; // Default for BGR
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

    // Allocate buffer for all rows
    fDecodingState->fRustDecodedData.resize(totalSize);

    SkSpan<uint8_t> decodedImage = SkSpan(fDecodingState->fRustDecodedData);
    // Read all rows
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
    // Reset the Rust reader's decode state
    fReader->reset_decode_state();

    // Always reinitialize decoding state for scanline decoding
    fDecodingState = DecodingState{};

    // Determine bytes per pixel based on color type
    rust_bmp::ColorType colorType = fReader->color_type();

    if (colorType == rust_bmp::ColorType::Bgr) {
        fDecodingState->fBytesPerPixel = 3;
    } else if (colorType == rust_bmp::ColorType::Bgra) {
        fDecodingState->fBytesPerPixel = 4;
    } else if (colorType == rust_bmp::ColorType::Rgb) {
        fDecodingState->fBytesPerPixel = 3;
    } else if (colorType == rust_bmp::ColorType::Rgba) {
        fDecodingState->fBytesPerPixel = 4;
    } else {
        return kUnimplemented;
    }

    fDecodingState->fCurrentScanline = 0;
    fDecodingState->fFullyDecoded = false;
    fDecodingState->fDataIsIncomplete = false;
    fDecodingState->fRustDecodedData.clear();

    // Initialize color transformation if needed
    if (this->xformOnDecode()) {
        fXformBuffer.reset(new uint32_t[dstInfo.width()]);
    }

    // Initialize swizzler for pixel format conversions
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
        // Check if we already have this scanline decoded
        SkSafeMath loopSafe;
        size_t currentOffset = loopSafe.mul(fDecodingState->fCurrentScanline, srcRowBytes);
        size_t neededDataSize = loopSafe.add(currentOffset, srcRowBytes);
        if (!loopSafe.ok()) {
            fDecodingState->fDataIsIncomplete = true;
            break;
        }

        // If we don't have this scanline yet, decode it from the stream
        if (fDecodingState->fRustDecodedData.size() < neededDataSize) {
            // Ensure we have space for the new row
            fDecodingState->fRustDecodedData.resize(neededDataSize);

            // Decode one row from the Rust reader
            SkSpan<uint8_t> decodedImage = SkSpan(fDecodingState->fRustDecodedData);
            SkSpan<uint8_t> rowSpan = decodedImage.subspan(currentOffset, srcRowBytes);
            rust::Slice<uint8_t> row_span(rowSpan.data(), srcRowBytes);

            auto result = fReader->next_row(row_span);

            if (result != rust_bmp::DecodingResult::Success) {
                // Failed to decode row - we're done
                fDecodingState->fDataIsIncomplete = true;
                break;
            }
        }

        // Get the source row (reuse currentOffset calculated above)
        SkSpan<const uint8_t> srcImage = SkSpan(fDecodingState->fRustDecodedData);
        SkSpan<const uint8_t> srcRow = srcImage.subspan(currentOffset, srcRowBytes);

        // Use swizzler and color transform (same pattern as performFullDecode)
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

    // We need to actually decode (and discard) the skipped rows to advance the stream
    std::vector<uint8_t> tempBuffer(srcRowBytes);
    for (int i = 0; i < linesToSkip; ++i) {
        // Check if we already have this scanline decoded
        size_t neededDataSize = (fDecodingState->fCurrentScanline + 1) * srcRowBytes;

        if (fDecodingState->fRustDecodedData.size() < neededDataSize) {
            // Decode and discard the row
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
