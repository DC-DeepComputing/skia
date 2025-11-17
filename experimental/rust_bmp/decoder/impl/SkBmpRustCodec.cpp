/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_bmp/decoder/impl/SkBmpRustCodec.h"

#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "rust/common/SkStreamAdapter.h"
#include "rust/common/SpanUtils.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkStreamPriv.h"

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
    , fDecodingState(std::make_unique<DecodingState>())
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
    fDecodingState = std::make_unique<DecodingState>();

    return true;
}

SkCodec::Result SkBmpRustCodec::onGetPixels(const SkImageInfo& info,
                                            void* dst,
                                            size_t dstRowStride,
                                            const Options& options,
                                            int* rowsDecoded) {
    return this->performFullDecode(info, dst, dstRowStride);
}

SkCodec::Result SkBmpRustCodec::performFullDecode(const SkImageInfo& dstInfo,
                                                  void* dst,
                                                  size_t dstRowStride) {
    Result ensureResult = this->ensureDecodedData();
    if (ensureResult != kSuccess) {
        return ensureResult;
    }

    // Copy decoded data to destination with format conversion
    // Use cached decoded data
    const size_t srcRowBytes = this->dimensions().width() * fDecodingState->fBytesPerPixel;
    const size_t dstRowBytes = dstRowStride;
    const int width = this->dimensions().width();
    const int height = this->dimensions().height();
    const int srcBytesPerPixel = fDecodingState->fBytesPerPixel;

    // Determine source format from encoded info
    const bool srcIsBgr = (this->getEncodedInfo().color() == SkEncodedInfo::kBGR_Color);
    const bool srcIsBgra = (this->getEncodedInfo().color() == SkEncodedInfo::kBGRA_Color);
    const bool srcIsRGBA = (this->getEncodedInfo().color() == SkEncodedInfo::kRGBA_Color);

    // Handle format conversion based on destination color type
    for (int y = 0; y < height; ++y) {
        const uint8_t* srcRow = fDecodingState->fRustDecodedData.data() + y * srcRowBytes;
        uint8_t* dstRow = static_cast<uint8_t*>(dst) + y * dstRowBytes;

        if (dstInfo.colorType() == kRGBA_8888_SkColorType) {
            if (srcIsRGBA && srcBytesPerPixel == 4) {
                // Source is already RGBA - direct copy
                memcpy(dstRow, srcRow, width * 4);
            } else if (srcIsBgra && srcBytesPerPixel == 4) {
                // Source is BGRA -> RGBA (swap R and B)
                for (int x = 0; x < width; ++x) {
                    dstRow[x * 4 + 0] = srcRow[x * 4 + 2]; // R
                    dstRow[x * 4 + 1] = srcRow[x * 4 + 1]; // G
                    dstRow[x * 4 + 2] = srcRow[x * 4 + 0]; // B
                    dstRow[x * 4 + 3] = srcRow[x * 4 + 3]; // A
                }
            } else if (srcBytesPerPixel == 3) {
                // Convert BGR/RGB -> RGBA
                for (int x = 0; x < width; ++x) {
                    uint8_t c0 = srcRow[x * 3 + 0];
                    uint8_t c1 = srcRow[x * 3 + 1];
                    uint8_t c2 = srcRow[x * 3 + 2];

                    if (srcIsBgr) {
                        // Source is BGR
                        dstRow[x * 4 + 0] = c2; // R
                        dstRow[x * 4 + 1] = c1; // G
                        dstRow[x * 4 + 2] = c0; // B
                    } else {
                        // Source is RGB
                        dstRow[x * 4 + 0] = c0; // R
                        dstRow[x * 4 + 1] = c1; // G
                        dstRow[x * 4 + 2] = c2; // B
                    }
                    dstRow[x * 4 + 3] = 255; // A (opaque)
                }
            }
        } else if (dstInfo.colorType() == kBGRA_8888_SkColorType) {
            if (srcIsBgra && srcBytesPerPixel == 4) {
                // Source is BGRA, destination is BGRA - direct copy
                memcpy(dstRow, srcRow, width * 4);
            } else if (srcIsRGBA && srcBytesPerPixel == 4) {
                // Source is RGBA, destination is BGRA - swap R and B
                for (int x = 0; x < width; ++x) {
                    dstRow[x * 4 + 0] = srcRow[x * 4 + 2]; // B
                    dstRow[x * 4 + 1] = srcRow[x * 4 + 1]; // G
                    dstRow[x * 4 + 2] = srcRow[x * 4 + 0]; // R
                    dstRow[x * 4 + 3] = srcRow[x * 4 + 3]; // A
                }
            } else if (srcBytesPerPixel == 3) {
                // Convert BGR/RGB -> BGRA
                for (int x = 0; x < width; ++x) {
                    uint8_t c0 = srcRow[x * 3 + 0];
                    uint8_t c1 = srcRow[x * 3 + 1];
                    uint8_t c2 = srcRow[x * 3 + 2];

                    if (srcIsBgr) {
                        // Source is BGR
                        dstRow[x * 4 + 0] = c0; // B
                        dstRow[x * 4 + 1] = c1; // G
                        dstRow[x * 4 + 2] = c2; // R
                    } else {
                        // Source is RGB, need to swap
                        dstRow[x * 4 + 0] = c2; // B
                        dstRow[x * 4 + 1] = c1; // G
                        dstRow[x * 4 + 2] = c0; // R
                    }
                    dstRow[x * 4 + 3] = 255; // A (opaque)
                }
            }
        } else if (dstInfo.colorType() == kRGB_565_SkColorType) {
            // Convert BGR/RGB -> RGB_565 (16-bit)
            uint16_t* dst16 = reinterpret_cast<uint16_t*>(dstRow);
            for (int x = 0; x < width; ++x) {
                uint8_t c0 = srcRow[x * 3 + 0];
                uint8_t c1 = srcRow[x * 3 + 1];
                uint8_t c2 = srcRow[x * 3 + 2];

                uint8_t r, g, b;
                if (srcIsBgr) {
                    b = c0; g = c1; r = c2;
                } else {
                    r = c0; g = c1; b = c2;
                }

                // Pack into 16-bit RGB_565: [R:15..11 G:10..5 B:4..0]
                uint16_t rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
                dst16[x] = rgb565;
            }
        } else {
            // Fallback: simple copy for matching formats
            memcpy(dstRow, srcRow, std::min(srcRowBytes, dstRowBytes));
        }
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
    size_t rowBytes = width * bytesPerPixel;
    size_t totalSize = rowBytes * height;

    // Allocate buffer for all rows
    fDecodingState->fRustDecodedData.resize(totalSize);

    // Read all rows
    for (int y = 0; y < height; ++y) {
        uint8_t* rowPtr = fDecodingState->fRustDecodedData.data() + y * rowBytes;
        rust::Slice<uint8_t> rowSlice(rowPtr, rowBytes);

        rust_bmp::DecodingResult result = fReader->next_row(rowSlice);
        if (result == rust_bmp::DecodingResult::IncompleteInput) {
            fDecodingState->fDataIsIncomplete = true;
            return kIncompleteInput;
        }
        if (result != rust_bmp::DecodingResult::Success) {
            return kErrorInInput;
        }
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
    fDecodingState = std::make_unique<DecodingState>();

    // Determine bytes per pixel and BGR flag based on color type
    rust_bmp::ColorType colorType = fReader->color_type();

    if (colorType == rust_bmp::ColorType::Bgr) {
        // 24-bit BGR
        fDecodingState->fBytesPerPixel = 3;
        fDecodingState->fIsBgr = true;
    } else if (colorType == rust_bmp::ColorType::Bgra) {
        // 32-bit BGRA
        fDecodingState->fBytesPerPixel = 4;
        fDecodingState->fIsBgr = true;
    } else if (colorType == rust_bmp::ColorType::Rgb) {
        // Palette formats output RGB
        fDecodingState->fBytesPerPixel = 3;
        fDecodingState->fIsBgr = false;
    } else if (colorType == rust_bmp::ColorType::Rgba) {
        // RGBA
        fDecodingState->fBytesPerPixel = 4;
        fDecodingState->fIsBgr = false;
    } else {
        // Unsupported color type
        return kUnimplemented;
    }

    fDecodingState->fCurrentScanline = 0;
    fDecodingState->fFullyDecoded = false;
    fDecodingState->fDataIsIncomplete = false;
    fDecodingState->fRustDecodedData.clear();

    return kSuccess;
}

int SkBmpRustCodec::onGetScanlines(void* dst, int count, size_t rowBytes) {
    const int width = this->dimensions().width();
    const int height = this->dimensions().height();
    const size_t srcRowBytes = width * fDecodingState->fBytesPerPixel;
    const int srcBytesPerPixel = fDecodingState->fBytesPerPixel;
    const bool srcIsBgr = fDecodingState->fIsBgr;

    int linesDecoded = 0;
    uint8_t* dstPtr = static_cast<uint8_t*>(dst);

    for (int i = 0; i < count && fDecodingState->fCurrentScanline < height; ++i) {
        // Check if we already have this scanline decoded
        size_t neededDataSize = (fDecodingState->fCurrentScanline + 1) * srcRowBytes;

        // If we don't have this scanline yet, decode it from the stream
        if (fDecodingState->fRustDecodedData.size() < neededDataSize) {
            // Ensure we have space for the new row
            fDecodingState->fRustDecodedData.resize(neededDataSize);

            // Decode one row from the Rust reader
            uint8_t* rowDst = fDecodingState->fRustDecodedData.data() +
                             (fDecodingState->fCurrentScanline * srcRowBytes);
            rust::Slice<uint8_t> row_span(rowDst, srcRowBytes);

            auto result = fReader->next_row(row_span);

            if (result != rust_bmp::DecodingResult::Success) {
                // Failed to decode row - we're done
                fDecodingState->fDataIsIncomplete = true;
                break;
            }
        }

        // Get the source row
        const uint8_t* srcRow = fDecodingState->fRustDecodedData.data() +
                                fDecodingState->fCurrentScanline * srcRowBytes;

        // Perform color conversion (same logic as performFullDecode)
        // Note: For proper integration, this should use SkSwizzler, but for now
        // we do manual conversion to match the full decode behavior
        // TODO: Integrate with SkSwizzler for proper color management

        if (srcBytesPerPixel == 3) {
            // 24-bit RGB/BGR -> RGBA conversion
            uint8_t* dstRow = dstPtr;
            for (int x = 0; x < width; ++x) {
                uint8_t c0 = srcRow[x * 3 + 0];
                uint8_t c1 = srcRow[x * 3 + 1];
                uint8_t c2 = srcRow[x * 3 + 2];

                if (srcIsBgr) {
                    // Source is BGR, convert to RGBA
                    dstRow[x * 4 + 0] = c2; // R
                    dstRow[x * 4 + 1] = c1; // G
                    dstRow[x * 4 + 2] = c0; // B
                } else {
                    // Source is RGB
                    dstRow[x * 4 + 0] = c0; // R
                    dstRow[x * 4 + 1] = c1; // G
                    dstRow[x * 4 + 2] = c2; // B
                }
                dstRow[x * 4 + 3] = 255; // A (opaque)
            }
        } else if (srcBytesPerPixel == 4) {
            // 32-bit RGBA/BGRA -> RGBA conversion
            uint8_t* dstRow = dstPtr;
            if (srcIsBgr) {
                // BGRA -> RGBA
                for (int x = 0; x < width; ++x) {
                    dstRow[x * 4 + 0] = srcRow[x * 4 + 2]; // R
                    dstRow[x * 4 + 1] = srcRow[x * 4 + 1]; // G
                    dstRow[x * 4 + 2] = srcRow[x * 4 + 0]; // B
                    dstRow[x * 4 + 3] = srcRow[x * 4 + 3]; // A
                }
            } else {
                // RGBA -> RGBA (direct copy)
                memcpy(dstRow, srcRow, width * 4);
            }
        } else {
            // For other formats, just copy
            memcpy(dstPtr, srcRow, std::min(srcRowBytes, rowBytes));
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


