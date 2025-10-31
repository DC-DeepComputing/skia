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
#include "src/core/SkStreamPriv.h"

namespace {

// Number of bytes in .BMP used to store the file header. This is effectively
// `sizeof(BITMAPFILEHEADER)`, as defined in
// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
static constexpr size_t kSizeOfFileHeader = 14;

}

std::unique_ptr<SkBmpRustCodec> SkBmpRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                               Result* result) {
    if (!stream) {
        *result = kInvalidInput;
        return nullptr;
    }

    // Either wrap or copy stream data.
    sk_sp<SkData> data = nullptr;
    if (stream->getMemoryBase()) {
        data = SkData::MakeWithoutCopy(stream->getMemoryBase(), stream->getLength());
    } else {
        data = SkCopyStreamToData(stream.get());
        // Data is copied; stream can be released now.
        stream.reset(nullptr);
    }

    if (!data || data->size() < kSizeOfFileHeader) {
        *result = kIncompleteInput;
        return nullptr;
    }

    auto rust_decoder = rust_bmp::create_decoder();

    // Pass data to Rust for ALL parsing and validation
    rust::Slice<const uint8_t> data_slice{
        reinterpret_cast<const uint8_t*>(data->data()),
        data->size()
    };

    auto decode_result = rust_decoder->decode_info(data_slice);
    if (!decode_result.success) {
        *result = kInvalidInput;
        return nullptr;
    }

    SkEncodedInfo::Color color;
    SkEncodedInfo::Alpha alpha;
    int bitsPerComponent;

    switch (decode_result.color_type) {
        case rust_bmp::ColorType::Rgb:
            color = SkEncodedInfo::kRGB_Color;
            alpha = SkEncodedInfo::kOpaque_Alpha;
            bitsPerComponent = 8;
            break;
        case rust_bmp::ColorType::Bgr:
            color = SkEncodedInfo::kBGR_Color;
            alpha = SkEncodedInfo::kOpaque_Alpha;
            bitsPerComponent = 8;
            break;
        case rust_bmp::ColorType::Rgba:
            color = SkEncodedInfo::kRGBA_Color;
            alpha = SkEncodedInfo::kUnpremul_Alpha;
            bitsPerComponent = 8;
            break;
        default:
            *result = kUnimplemented;
            return nullptr;
    }

    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(
        decode_result.width,
        decode_result.height,
        color,
        alpha,
        bitsPerComponent
    );

    // Determine scanline order (top-down if height was negative in BMP header)
    SkCodec::SkScanlineOrder rowOrder = SkCodec::kBottomUp_SkScanlineOrder;

    // Check the raw BMP height value to determine row order
    // BMP height is at offset 22 (after 14-byte file header + 4-byte DIB header size + 4-byte width)
    if (data->size() >= 26) {
        const uint8_t* rawData = reinterpret_cast<const uint8_t*>(data->data());
        int32_t heightSigned = static_cast<int32_t>(
            rawData[22] | (rawData[23] << 8) | (rawData[24] << 16) | (rawData[25] << 24)
        );
        if (heightSigned < 0) {
            rowOrder = SkCodec::kTopDown_SkScanlineOrder;
        }
    }

    *result = kSuccess;
    return std::unique_ptr<SkBmpRustCodec>(new SkBmpRustCodec(
        std::move(encodedInfo),
        std::move(stream),
        std::move(rust_decoder),
        data,  // Cache the data we read
        rowOrder
    ));
}

SkBmpRustCodec::SkBmpRustCodec(SkEncodedInfo&& encodedInfo,
                               std::unique_ptr<SkStream> stream,
                               rust::Box<rust_bmp::BmpDecoder> rustDecoder,
                               sk_sp<SkData> cachedData,
                               SkCodec::SkScanlineOrder rowOrder)
    : INHERITED(std::move(encodedInfo), skcms_PixelFormat_BGR_888, std::move(stream))
    , fRustDecoder(std::move(rustDecoder))
    , fDecodingState(std::make_unique<DecodingState>())
    , fCachedData(std::move(cachedData))
    , fRowOrder(rowOrder) {}

SkBmpRustCodec::~SkBmpRustCodec() = default;

bool SkBmpRustCodec::onRewind() {
    // Reset decoding state
    fDecodingState = std::make_unique<DecodingState>();

    // Create a new Rust decoder instance
    fRustDecoder = rust_bmp::create_decoder();

    // Verify that the cached data is still valid by re-decoding the info
    if (!fCachedData || fCachedData->size() < kSizeOfFileHeader) {
        return false;
    }

    rust::Slice<const uint8_t> data_slice{
        reinterpret_cast<const uint8_t*>(fCachedData->data()),
        fCachedData->size()
    };

    auto decode_result = fRustDecoder->decode_info(data_slice);
    return decode_result.success;
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
    const bool srcIsRGBA = (this->getEncodedInfo().color() == SkEncodedInfo::kRGBA_Color);

    // Handle format conversion based on destination color type
    for (int y = 0; y < height; ++y) {
        const uint8_t* srcRow = fDecodingState->fRustDecodedData.data() + y * srcRowBytes;
        uint8_t* dstRow = static_cast<uint8_t*>(dst) + y * dstRowBytes;

        if (dstInfo.colorType() == kRGBA_8888_SkColorType) {
            if (srcIsRGBA && srcBytesPerPixel == 4) {
                // Source is already RGBA (from embedded JPEG/PNG processors)
                // Direct copy
                memcpy(dstRow, srcRow, width * 4);
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
            if (srcIsRGBA && srcBytesPerPixel == 4) {
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

    // Use cached data if available, otherwise read from stream
    sk_sp<SkData> data;
    if (fCachedData) {
        data = fCachedData;
    } else {
        // Rewind the stream to the beginning
        if (!this->stream()->rewind()) {
            return kErrorInInput;
        }

        data = SkCopyStreamToData(this->stream());
        if (!data || data->isEmpty()) {
            return kIncompleteInput;
        }
    }

    rust::Slice<const uint8_t> data_slice{
        reinterpret_cast<const uint8_t*>(data->data()),
        data->size()
    };

    auto decode_result = fRustDecoder->decode_pixels(data_slice);
    if (!decode_result.success) {
        return decode_result.error_is_incomplete ? kIncompleteInput : kErrorInInput;
    }

    // Cache the decoded data
    fDecodingState->fRustDecodedData.assign(
        decode_result.pixels.begin(),
        decode_result.pixels.end()
    );
    fDecodingState->fBytesPerPixel = decode_result.bytes_per_pixel;
    fDecodingState->fDataIsIncomplete = decode_result.data_is_incomplete;
    fDecodingState->fFullyDecoded = true;

    return kSuccess;
}

SkCodec::Result SkBmpRustCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
                                                      const Options& options) {
    // Ensure we have decoded data available
    Result ensureResult = this->ensureDecodedData();
    if (ensureResult != kSuccess) {
        return ensureResult;
    }

    fDecodingState->fCurrentScanline = 0;
    return kSuccess;
}

int SkBmpRustCodec::onGetScanlines(void* dst, int count, size_t rowBytes) {
    if (!fDecodingState->fFullyDecoded) {
        return 0;
    }

    const int width = this->dimensions().width();
    const int height = this->dimensions().height();
    const size_t srcRowBytes = width * fDecodingState->fBytesPerPixel;

    int linesDecoded = 0;
    uint8_t* dstPtr = static_cast<uint8_t*>(dst);

    for (int i = 0; i < count && fDecodingState->fCurrentScanline < height; ++i) {
        const uint8_t* srcRow = fDecodingState->fRustDecodedData.data() +
                                fDecodingState->fCurrentScanline * srcRowBytes;

        memcpy(dstPtr, srcRow, std::min(srcRowBytes, rowBytes));

        dstPtr += rowBytes;
        fDecodingState->fCurrentScanline++;
        linesDecoded++;
    }

    return linesDecoded;
}

bool SkBmpRustCodec::onSkipScanlines(int count) {
    if (!fDecodingState->fFullyDecoded) {
        return false;
    }

    const int height = this->dimensions().height();
    int linesToSkip = std::min(count, height - fDecodingState->fCurrentScanline);
    fDecodingState->fCurrentScanline += linesToSkip;

    return linesToSkip == count;
}


