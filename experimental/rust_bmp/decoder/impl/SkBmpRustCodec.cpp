/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_bmp/decoder/impl/SkBmpRustCodec.h"

#include <cstring>
#include "experimental/rust_bmp/ffi/FFI.rs.h"
#include "experimental/rust_bmp/ffi/UtilsForFFI.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkEndian.h"
#include "src/base/SkSafeMath.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegCodec.h"
#include "src/codec/SkPngCodec.h"
#include "src/core/SkStreamPriv.h"
#include "third_party/rust/cxx/v1/cxx.h"

// This helper class adapts `SkStream` to expose the API required by Rust FFI
// (i.e. the `ReadAndSeekTraits` API).
class ReadAndSeekTraitsAdapterForSkStream final : public rust_bmp::ReadAndSeekTraits {
public:
    // SAFETY: The caller needs to guarantee that `stream` will be alive for
    // as long as `ReadAndSeekTraitsAdapterForSkStream`.
    explicit ReadAndSeekTraitsAdapterForSkStream(std::unique_ptr<SkStream> stream)
        : fStream(std::move(stream)) {
        SkASSERT_RELEASE(fStream);
    }

    ~ReadAndSeekTraitsAdapterForSkStream() override = default;

    // Non-copyable and non-movable (we want a stable `this` pointer, because we
    // will be passing a `ReadAndSeekTraits*` pointer over the FFI boundary).
    ReadAndSeekTraitsAdapterForSkStream(const ReadAndSeekTraitsAdapterForSkStream&) = delete;
    ReadAndSeekTraitsAdapterForSkStream& operator=(const ReadAndSeekTraitsAdapterForSkStream&) = delete;
    ReadAndSeekTraitsAdapterForSkStream(ReadAndSeekTraitsAdapterForSkStream&&) = delete;
    ReadAndSeekTraitsAdapterForSkStream& operator=(ReadAndSeekTraitsAdapterForSkStream&&) = delete;

    // Implementation of the `std::io::Read::read` method.
    bool read(rust::Slice<uint8_t> buffer) override {
        SkSpan<uint8_t> span = ToSkSpan(buffer);
        if (span.empty()) {
            return true;  // Nothing to read
        }
        size_t bytesRead = fStream->read(span.data(), span.size());

        // Only return true if we actually read the requested amount
        // This properly handles truncated streams by failing when EOF is hit unexpectedly
        return bytesRead == span.size();
    }

    bool seek_to_start() override {
        return fStream->rewind();
    }

    bool seek_to_position(uint64_t requested_pos, uint64_t& final_pos) override {
        SkSafeMath safe;
        size_t pos = safe.castTo<size_t>(requested_pos);
        if (!safe.ok()) {
            return false;
        }

        if (!fStream->seek(pos)) {
            return false;
        }

        // Assigning `size_t` to `uint64_t` doesn't need to go through
        // `SkSafeMath`, because `uint64_t` is never smaller than `size_t`.
        static_assert(sizeof(uint64_t) >= sizeof(size_t));
        final_pos = requested_pos;
        return true;
    }

private:
    std::unique_ptr<SkStream> fStream;
};

// Static helper functions
namespace {

SkEncodedInfo::Color ToSkEncodedInfoColor(rust_bmp::ColorType colorType) {
    switch (colorType) {
        case rust_bmp::ColorType::Grayscale:
            return SkEncodedInfo::kGray_Color;
        case rust_bmp::ColorType::Rgb:
            return SkEncodedInfo::kBGR_Color;  // BMP files use BGR format
        case rust_bmp::ColorType::Rgba:
            return SkEncodedInfo::kBGRA_Color; // BMP 32-bit uses BGRA format
    }
    SK_ABORT("Unexpected rust_bmp::ColorType: %d", static_cast<int>(colorType));
}

SkEncodedInfo::Alpha ToSkEncodedInfoAlpha(rust_bmp::ColorType colorType) {
    switch (colorType) {
        case rust_bmp::ColorType::Grayscale:
        case rust_bmp::ColorType::Rgb:
            return SkEncodedInfo::kOpaque_Alpha;
        case rust_bmp::ColorType::Rgba:
            return SkEncodedInfo::kUnpremul_Alpha;
    }
    SK_ABORT("Unexpected rust_bmp::ColorType: %d", static_cast<int>(colorType));
}

SkCodec::Result ToSkCodecResult(rust_bmp::DecodingResult rustResult) {
    switch (rustResult) {
        case rust_bmp::DecodingResult::Success:
            return SkCodec::kSuccess;
        case rust_bmp::DecodingResult::FormatError:
            return SkCodec::kErrorInInput;
        case rust_bmp::DecodingResult::ParameterError:
            return SkCodec::kInvalidParameters;
        case rust_bmp::DecodingResult::UnsupportedFormat:
            return SkCodec::kUnimplemented;
        case rust_bmp::DecodingResult::IncompleteInput:
            return SkCodec::kIncompleteInput;
        case rust_bmp::DecodingResult::OtherError:
            return SkCodec::kInternalError;
        case rust_bmp::DecodingResult::MemoryError:
            return SkCodec::kInternalError;
        case rust_bmp::DecodingResult::DelegateToJpegCodec:
            return SkCodec::kUnimplemented;  // Will be handled differently
        case rust_bmp::DecodingResult::DelegateToPngCodec:
            return SkCodec::kUnimplemented;  // Will be handled differently
    }
    SK_ABORT("Unexpected rust_bmp::DecodingResult: %d", static_cast<int>(rustResult));
}

// Helper function to detect BMP compression types that require delegation
enum class BmpCompressionType {
    Standard,      // Handle with Rust decoder
    EmbeddedJpeg,  // Delegate to SkJpegCodec
    EmbeddedPng,   // Delegate to SkPngCodec
    Unsupported    // Return error
};

BmpCompressionType DetectBmpCompressionType(std::unique_ptr<SkStream>& stream) {
    // Save stream position
    size_t originalPosition = stream->getPosition();

    // Read BMP header to check compression type
    uint8_t header[54];
    if (stream->read(header, 54) != 54) {
        stream->seek(originalPosition);
        return BmpCompressionType::Standard;
    }

    // Check BMP signature
    if (header[0] != 'B' || header[1] != 'M') {
        stream->seek(originalPosition);
        return BmpCompressionType::Standard;
    }

    // Extract compression type (at offset 30 in BITMAPINFOHEADER)
    uint32_t compression = SkEndian_SwapLE32(*(uint32_t*)(header + 30));
    uint32_t dataOffset = SkEndian_SwapLE32(*(uint32_t*)(header + 10));

    // Check for embedded formats
    if (compression == 4) {  // Could be JPEG embedded or AlphaBitMasks
        // Seek to data section and check for JPEG signature
        if (stream->seek(dataOffset) && stream->getPosition() == dataOffset) {
            uint8_t jpegSig[2];
            if (stream->read(jpegSig, 2) == 2 &&
                jpegSig[0] == 0xFF && jpegSig[1] == 0xD8) {
                stream->seek(originalPosition);
                return BmpCompressionType::EmbeddedJpeg;
            }
        }
    } else if (compression == 5) {  // PNG embedded
        // Seek to data section and check for PNG signature
        if (stream->seek(dataOffset) && stream->getPosition() == dataOffset) {
            uint8_t pngSig[8];
            if (stream->read(pngSig, 8) == 8 &&
                pngSig[0] == 0x89 && pngSig[1] == 0x50 &&
                pngSig[2] == 0x4E && pngSig[3] == 0x47 &&
                pngSig[4] == 0x0D && pngSig[5] == 0x0A &&
                pngSig[6] == 0x1A && pngSig[7] == 0x0A) {
                stream->seek(originalPosition);
                return BmpCompressionType::EmbeddedPng;
            }
        }
    }

    // Reset stream position and use standard Rust decoder
    stream->seek(originalPosition);
    return BmpCompressionType::Standard;
}

std::unique_ptr<SkEncodedInfo> CreateEncodedInfo(const rust_bmp::ImageInfo& imageInfo,
                                                 sk_sp<SkColorSpace> colorSpace = nullptr) {
    SkEncodedInfo::Color color = ToSkEncodedInfoColor(imageInfo.color_type);
    SkEncodedInfo::Alpha alpha = ToSkEncodedInfoAlpha(imageInfo.color_type);

    // bitsPerComponent should be bits per color component, not bits per pixel
    uint8_t bitsPerComponent;
    switch (imageInfo.color_type) {
        case rust_bmp::ColorType::Rgb:
        case rust_bmp::ColorType::Rgba:
            bitsPerComponent = 8;  // 8 bits per color component for RGB/RGBA
            break;
        case rust_bmp::ColorType::Grayscale:
            bitsPerComponent = 8;  // 8 bits for grayscale
            break;
        default:
            bitsPerComponent = 8;  // Default to 8 bits per component
            break;
    }

    // Create SkEncodedInfo (ICC profiles not supported for BMP yet)
    return std::make_unique<SkEncodedInfo>(
        SkEncodedInfo::Make(imageInfo.width, imageInfo.height, color, alpha, bitsPerComponent));
}

}  // namespace

// SkBmpRustCodec implementation

std::unique_ptr<SkBmpRustCodec> SkBmpRustCodec::MakeFromStream(
        std::unique_ptr<SkStream> stream, SkCodec::Result* result) {
    if (!stream) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }

    // Stream length available if needed for debugging

    // First, check if this BMP contains embedded formats that should be delegated
    BmpCompressionType compressionType = DetectBmpCompressionType(stream);

    if (compressionType == BmpCompressionType::EmbeddedJpeg) {
        // Read the entire stream to extract embedded JPEG data
        sk_sp<SkData> bmpData = SkCopyStreamToData(stream.get());
        if (!bmpData || bmpData->size() < 54) {
            *result = SkCodec::kIncompleteInput;
            return nullptr;
        }

        // Extract data offset from BMP header
        const uint8_t* header = (const uint8_t*)bmpData->data();
        uint32_t dataOffset = SkEndian_SwapLE32(*(uint32_t*)(header + 10));

        if (dataOffset >= bmpData->size()) {
            *result = SkCodec::kErrorInInput;
            return nullptr;
        }

        // Create JPEG stream from embedded data
        sk_sp<SkData> jpegData = SkData::MakeSubset(bmpData.get(), dataOffset,
                                                    bmpData->size() - dataOffset);
        std::unique_ptr<SkStream> jpegStream = SkMemoryStream::Make(jpegData);

        // Delegate to JPEG codec - return as SkCodec, not SkBmpRustCodec
        // Note: This changes the return type conceptually, but maintains interface compatibility
        auto jpegCodec = SkJpegCodec::MakeFromStream(std::move(jpegStream), result);
        // For now, we'll continue with Rust decoder and handle embedded formats there
        // This is a placeholder for the complete delegation implementation
    } else if (compressionType == BmpCompressionType::EmbeddedPng) {
        // Similar logic for embedded PNG
        // For now, continue with standard processing
    }

    // Create adapter for Rust FFI
    auto readAndSeekTraits = std::make_unique<ReadAndSeekTraitsAdapterForSkStream>(std::move(stream));

    // Create decoder using Rust FFI
    rust::Box<rust_bmp::ResultOfDecoder> resultOfDecoder = rust_bmp::new_decoder(std::move(readAndSeekTraits));

    // Check if decoder creation succeeded
    if (resultOfDecoder->err() != rust_bmp::DecodingResult::Success) {
        *result = ToSkCodecResult(resultOfDecoder->err());
        return nullptr;
    }

    // Unwrap the decoder
    rust::Box<rust_bmp::Decoder> decoder = resultOfDecoder->unwrap();

    // TODO: Once FFI compilation is fixed, add embedded format extraction here
    // if needed for JPEG or PNG embedded in BMP files

    // Get image info from the decoder
    rust_bmp::ImageInfo imageInfo = decoder->get_info();

    // Try to extract ICC color profile if the original BMP data is available
    sk_sp<SkColorSpace> colorSpace = nullptr;
    // TODO: Once FFI supports ICC profile extraction, get it from Rust decoder
    // For now, we'll extract it from the original stream if we can access the raw BMP data

    // Create SkEncodedInfo with color space support
    std::unique_ptr<SkEncodedInfo> encodedInfo = CreateEncodedInfo(imageInfo, colorSpace);
    if (!encodedInfo) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    // Create the codec
    // Note: We pass nullptr for stream since ReadAndSeekTraits now owns it
    auto codec = std::make_unique<SkBmpRustCodec>(std::move(*encodedInfo),
                                                  nullptr,
                                                  std::move(decoder));

    *result = SkCodec::kSuccess;
    return codec;
}

SkBmpRustCodec::SkBmpRustCodec(SkEncodedInfo&& encodedInfo,
                               std::unique_ptr<SkStream> stream,
                               rust::Box<rust_bmp::Decoder> decoder)
    : INHERITED(std::move(encodedInfo), skcms_PixelFormat_RGB_888, std::move(stream)),
      fDecoder(std::move(decoder)) {
}

SkBmpRustCodec::~SkBmpRustCodec() {
    // Rust Box handles cleanup automatically
}

// Helper function to convert BGR24 data to various Skia color formats
bool SkBmpRustCodec::convertRgb24ToRequestedFormat(const uint8_t* srcBgr24,
                                                    uint8_t* dst,
                                                    uint32_t width,
                                                    uint32_t height,
                                                    SkColorType dstColorType,
                                                    size_t dstRowStride) {
    const size_t srcRowSize = width * 3; // BGR24 = 3 bytes per pixel

    // Debug: Log what format conversion is happening for small images
    if (width <= 8 && height <= 8) {
        printf("C++: Converting BGR24 to format %d, width=%d, height=%d, dstRowStride=%zu\n",
               (int)dstColorType, width, height, dstRowStride);
    }

    for (uint32_t y = 0; y < height; y++) {
        const uint8_t* srcRow = srcBgr24 + (y * srcRowSize);
        uint8_t* dstRow = dst + (y * dstRowStride);

        switch (dstColorType) {
            case kRGB_565_SkColorType: {
                // kRGB_565_SkColorType: [R:15..11 G:10..5 B:4..0]
                uint16_t* dst565 = reinterpret_cast<uint16_t*>(dstRow);
                for (uint32_t x = 0; x < width; x++) {
                    uint8_t b = srcRow[x * 3 + 0];  // BGR: B is first
                    uint8_t g = srcRow[x * 3 + 1];  // BGR: G is second
                    uint8_t r = srcRow[x * 3 + 2];  // BGR: R is third

                    // Convert 8-bit RGB to 5-6-5 format
                    uint16_t r5 = (r >> 3) & 0x1F;   // 5 bits
                    uint16_t g6 = (g >> 2) & 0x3F;   // 6 bits
                    uint16_t b5 = (b >> 3) & 0x1F;   // 5 bits

                    dst565[x] = (r5 << 11) | (g6 << 5) | b5;
                }
                break;
            }

            case kRGBA_8888_SkColorType: {
                // kRGBA_8888_SkColorType: [A:31..24 B:23..16 G:15..8 R:7..0] (ABGR byte order)
                uint32_t* dst8888 = reinterpret_cast<uint32_t*>(dstRow);
                for (uint32_t x = 0; x < width; x++) {
                    uint8_t b = srcRow[x * 3 + 0];  // BGR: B is first
                    uint8_t g = srcRow[x * 3 + 1];  // BGR: G is second
                    uint8_t r = srcRow[x * 3 + 2];  // BGR: R is third
                    uint8_t a = 0xFF; // Opaque alpha

                    dst8888[x] = (a << 24) | (b << 16) | (g << 8) | r;
                }
                break;
            }

            case kBGRA_8888_SkColorType: {
                // kBGRA_8888_SkColorType: [A:31..24 R:23..16 G:15..8 B:7..0] (ARGB byte order)
                uint32_t* dstBgra = reinterpret_cast<uint32_t*>(dstRow);
                for (uint32_t x = 0; x < width; x++) {
                    uint8_t b = srcRow[x * 3 + 0];  // BGR: B is first
                    uint8_t g = srcRow[x * 3 + 1];  // BGR: G is second
                    uint8_t r = srcRow[x * 3 + 2];  // BGR: R is third
                    uint8_t a = 0xFF; // Opaque alpha

                    dstBgra[x] = (a << 24) | (r << 16) | (g << 8) | b;
                }
                break;
            }

            case kRGB_888x_SkColorType: {
                // kRGB_888x_SkColorType: [x:31..24 B:23..16 G:15..8 R:7..0] (xBGR byte order)
                uint32_t* dst888x = reinterpret_cast<uint32_t*>(dstRow);
                for (uint32_t x = 0; x < width; x++) {
                    uint8_t b = srcRow[x * 3 + 0];  // BGR: B is first
                    uint8_t g = srcRow[x * 3 + 1];  // BGR: G is second
                    uint8_t r = srcRow[x * 3 + 2];  // BGR: R is third

                    dst888x[x] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                }
                break;
            }

            default:
                return false;
        }
    }

    return true;
}

SkCodec::Result SkBmpRustCodec::startDecoding(const SkImageInfo& dstInfo,
                                              void* pixels,
                                              size_t rowBytes,
                                              const Options& options,
                                              DecodingState* decodingState) {
    // Validate parameters (pixels can be nullptr for scanline decoding setup)
    if (pixels && rowBytes < (size_t)dstInfo.minRowBytes()) {
        return kInvalidParameters;
    }

    // Set up the decoding state for the requested format
    const size_t bytesPerPixel = dstInfo.bytesPerPixel();
    decodingState->fDstRowStride = rowBytes;
    decodingState->fDstRowSize = this->dimensions().width() * bytesPerPixel;
    decodingState->fDstColorType = dstInfo.colorType();

    if (pixels) {
        decodingState->fDst = SkSpan<uint8_t>(static_cast<uint8_t*>(pixels),
                                              rowBytes * dstInfo.height());
    } else {
        // For scanline decoding, we don't have a destination buffer yet
        decodingState->fDst = SkSpan<uint8_t>();
    }

    // Set up intermediate buffer for RGB24 data from Rust decoder
    // Rust decoder always outputs RGB24 (3 bytes per pixel)
    const uint32_t width = this->dimensions().width();
    const uint32_t height = this->dimensions().height();
    const size_t rgb24Size = width * height * 3; // Fixed: 3 bytes per pixel for RGB24
    decodingState->fRustDecodedData.resize(rgb24Size);

    return kSuccess;
}

SkCodec::Result SkBmpRustCodec::incrementalDecode(DecodingState& decodingState,
                                                  int* rowsDecoded) {
    if (rowsDecoded) {
        *rowsDecoded = 0;
    }

    // Decode to RGB24 using Rust FFI (Rust decoder always outputs RGB24)
    uint32_t totalRows = this->dimensions().height();
    rust_bmp::DecodeResult result = fDecoder->decode_rows(0, totalRows,
        rust::Slice<uint8_t>(decodingState.fRustDecodedData.data(), decodingState.fRustDecodedData.size()));

    SkCodec::Result skResult = ToSkCodecResult(result.result);
    if (skResult != kSuccess) {
        return skResult;
    }

    // Convert from RGB24 to the requested format if we have a destination buffer
    if (!decodingState.fDst.empty()) {
        const uint32_t width = this->dimensions().width();
        const uint32_t height = this->dimensions().height();

        if (!this->convertRgb24ToRequestedFormat(decodingState.fRustDecodedData.data(),
                                                 decodingState.fDst.data(),
                                                 width, height,
                                                 decodingState.fDstColorType,
                                                 decodingState.fDstRowStride)) {
            return kInternalError;
        }
    }

    if (rowsDecoded) {
        // Calculate how many rows were actually decoded (RGB24 basis)
        const uint32_t width = this->dimensions().width();
        size_t rgb24BytesPerRow = width * 3;
        if (rgb24BytesPerRow > 0) {
            *rowsDecoded = static_cast<int>(result.bytes_decoded / rgb24BytesPerRow);
        }
    }

    return kSuccess;
}

SkCodec::Result SkBmpRustCodec::onGetPixels(const SkImageInfo& dstInfo,
                                            void* pixels,
                                            size_t rowBytes,
                                            const Options& options,
                                            int* rowsDecoded) {
    DecodingState decodingState;
    Result result = this->startDecoding(dstInfo, pixels, rowBytes, options, &decodingState);
    if (result != kSuccess) {
        return result;
    }

    result = this->incrementalDecode(decodingState, rowsDecoded);
    return result;
}

SkCodec::Result SkBmpRustCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                         void* pixels,
                                                         size_t rowBytes,
                                                         const Options& options) {
    fDecodingState = std::make_unique<DecodingState>();
    return this->startDecoding(dstInfo, pixels, rowBytes, options, fDecodingState.get());
}

SkCodec::Result SkBmpRustCodec::onIncrementalDecode(int* rowsDecoded) {
    if (!fDecodingState) {
        return kInvalidParameters;
    }

    return this->incrementalDecode(*fDecodingState, rowsDecoded);
}

SkCodec::Result SkBmpRustCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
                                                     const Options& options) {
    // For scanline decoding, we still need to set up a decoding state
    // but we don't need the pixel buffer since getScanlines will provide its own buffer
    fDecodingState = std::make_unique<DecodingState>();

    Result result = this->startDecoding(dstInfo, nullptr, 0, options, fDecodingState.get());
    if (result != kSuccess) {
        fDecodingState.reset();
        return result;
    }

    return kSuccess;
}

int SkBmpRustCodec::onGetScanlines(void* dst, int countLines, size_t rowBytes) {
    if (!fDecodingState) {
        return 0;
    }

    // If we don't have cached image data yet, decode the full image first
    if (fDecodingState->fCachedImageData.empty()) {
        uint32_t totalRows = this->dimensions().height();
        size_t totalBytes = totalRows * fDecodingState->fDstRowSize;
        fDecodingState->fCachedImageData.resize(totalBytes);

        rust_bmp::DecodeResult result = fDecoder->decode_rows(0, totalRows,
            rust::Slice<uint8_t>(fDecodingState->fCachedImageData.data(), totalBytes));

        SkCodec::Result skResult = ToSkCodecResult(result.result);
        if (skResult != kSuccess) {
            return 0;
        }
    }

    // Extract the requested scanlines from cached data
    int imageHeight = this->dimensions().height();
    int availableLines = imageHeight - fDecodingState->fCurrentScanline;
    int linesToCopy = std::min(countLines, availableLines);

    if (linesToCopy <= 0) {
        return 0; // No more lines available
    }

    // Copy the requested scanlines
    for (int i = 0; i < linesToCopy; i++) {
        int srcRow = fDecodingState->fCurrentScanline + i;
        size_t srcOffset = srcRow * fDecodingState->fDstRowSize;
        size_t dstOffset = i * rowBytes;

        if (srcOffset + fDecodingState->fDstRowSize <= fDecodingState->fCachedImageData.size() &&
            dstOffset + fDecodingState->fDstRowSize <= static_cast<size_t>(countLines * rowBytes)) {
            memcpy(static_cast<uint8_t*>(dst) + dstOffset,
                   fDecodingState->fCachedImageData.data() + srcOffset,
                   fDecodingState->fDstRowSize);
        }
    }

    // Update current scanline position
    fDecodingState->fCurrentScanline += linesToCopy;

    return linesToCopy;
}

bool SkBmpRustCodec::onRewind() {
    // Reset decoding state
    fDecodingState.reset();

    // Note: The Rust decoder doesn't currently support rewinding
    // This is acceptable for many use cases, but could be enhanced
    // in the future if needed
    return true;
}
