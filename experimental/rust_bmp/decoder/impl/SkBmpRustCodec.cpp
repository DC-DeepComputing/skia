/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_bmp/decoder/impl/SkBmpRustCodec.h"

#include "include/codec/SkCodecAnimation.h"
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

    // In streaming mode, metadata might not be available yet if the stream
    // doesn't have enough data. Try to read metadata explicitly.
    if (!reader->metadata_loaded()) {
        rust_bmp::DecodingResult metadataResult = reader->read_metadata();

        if (metadataResult == rust_bmp::DecodingResult::IncompleteInput) {
            // Need more data for metadata
            *result = kIncompleteInput;
            return nullptr;
        } else if (metadataResult != rust_bmp::DecodingResult::Success) {
            // Error during metadata read
            *result = MapDecodingResult(metadataResult);
            return nullptr;
        }

        // Verify metadata is now loaded
        if (!reader->metadata_loaded()) {
            *result = kErrorInInput;
            return nullptr;
        }
    }

    uint32_t width = reader->width();
    uint32_t height = reader->height();
    bool isBottomUp = reader->is_bottom_up();
    rust_bmp::BmpColor rustColor = reader->color();
    rust_bmp::BmpAlpha rustAlpha = reader->alpha();

    // BmpColor/BmpAlpha values match SkEncodedInfo enums (validated by static_assert).
    SkEncodedInfo::Color color = static_cast<SkEncodedInfo::Color>(rustColor);
    SkEncodedInfo::Alpha alpha = static_cast<SkEncodedInfo::Alpha>(rustAlpha);
    constexpr int kBitsPerComponent = 8;

    // Extract and parse ICC profile if present
    std::unique_ptr<SkCodecs::ColorProfile> colorProfile;
    rust::Vec<uint8_t> profileData = reader->icc_profile();
    if (!profileData.empty()) {
        sk_sp<SkData> iccData = SkData::MakeWithCopy(profileData.data(), profileData.size());
        colorProfile = SkCodecs::ColorProfile::MakeICCProfile(std::move(iccData));
    }

    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(
        width,
        height,
        color,
        alpha,
        kBitsPerComponent,
        std::move(colorProfile)
    );

    // Pre-calculate srcRowBytes (width * bytesPerPixel) and check for overflow.
    // Also verify that the total image size (height * srcRowBytes) doesn't overflow.
    size_t bytesPerPixel;
    switch (color) {
        case SkEncodedInfo::kRGB_Color:
        case SkEncodedInfo::kBGR_Color:
            bytesPerPixel = 3;
            break;
        case SkEncodedInfo::kRGBA_Color:
        case SkEncodedInfo::kBGRA_Color:
        case SkEncodedInfo::kBGRX_Color:
            bytesPerPixel = 4;
            break;
        default:
            *result = kInvalidInput;
            return nullptr;
    }
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
        isBottomUp
    ));
}

SkBmpRustCodec::SkBmpRustCodec(SkEncodedInfo&& encodedInfo,
                               std::unique_ptr<SkStream> stream,
                               rust::Box<rust_bmp::Reader> reader,
                               bool isBottomUp)
    : SkCodec(std::move(encodedInfo), skcms_PixelFormat_RGB_888,
              // TODO(crbug.com/370522089): Pass stream to SkCodec once SkCodec
              // avoids unnecessary rewinding (which forces re-reading entire stream).
              /* stream = */ nullptr)
    , fReader(std::move(reader))
    , fPrivStream(std::move(stream))
    , fIsBottomUp(isBottomUp) {
    SkASSERT_RELEASE(fPrivStream);
}

SkBmpRustCodec::~SkBmpRustCodec() = default;

std::unique_ptr<SkStream> SkBmpRustCodec::getEncodedData() const {
    SkASSERT_RELEASE(fPrivStream);
    return fPrivStream->duplicate();
}

bool SkBmpRustCodec::onGetValidSubset(SkIRect* desiredSubset) const {
    if (!desiredSubset) {
        return false;
    }

    // Check that the subset is entirely within the image bounds
    if (!this->bounds().contains(*desiredSubset)) {
        return false;
    }

    // BMP codec supports arbitrary rectangular subsets with no restrictions
    return true;
}

bool SkBmpRustCodec::onDimensionsSupported(const SkISize& requested) {
    // BMP codec supports subset decoding, so any dimensions that fit within
    // the image bounds are supported. This is needed because when decoding
    // a subset, the requested dimensions are the subset size, not the full
    // image size.
    SkISize dims = this->dimensions();
    int w = requested.width();
    int h = requested.height();
    return 1 <= w && w <= dims.width() && 1 <= h && h <= dims.height();
}

bool SkBmpRustCodec::onGetFrameInfo(int index, FrameInfo* info) const {
    if (index != 0) {
        return false;  // BMP images only have one frame
    }
    if (info) {
        // BMP images are single-frame, so set frame metadata accordingly
        info->fRequiredFrame = SkCodec::kNoFrame;
        info->fDuration = 0;
        info->fFullyReceived = true;
        info->fAlphaType = this->getInfo().alphaType();
        info->fHasAlphaWithinBounds = info->fAlphaType != kOpaque_SkAlphaType;
        info->fDisposalMethod = SkCodecAnimation::DisposalMethod::kKeep;
        info->fBlend = SkCodecAnimation::Blend::kSrc;
        info->fFrameRect = SkIRect::MakeSize(this->dimensions());
    }
    return true;
}

bool SkBmpRustCodec::onRewind() {
    // Clear any incremental decoding state
    fIncrementalDecodingState.reset();

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

    // Read metadata for the new reader (required before read_image_data can work)
    if (!fReader->metadata_loaded()) {
        rust_bmp::DecodingResult metadataResult = fReader->read_metadata();
        if (metadataResult != rust_bmp::DecodingResult::Success) {
            return false;
        }
    }

    return true;
}

SkCodec::Result SkBmpRustCodec::initializeSwizzler(const SkImageInfo& dstInfo,
                                                    const Options& opts) {
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
    if (!fSwizzler) {
        // SkSwizzler doesn't support all possible destination color types (e.g.,
        // kBGRA_10101010_XR_SkColorType). Return kInvalidConversion if the requested
        // destination format is not supported.
        return kInvalidConversion;
    }
    return kSuccess;
}

void SkBmpRustCodec::swizzleRow(const uint8_t* srcRow, void* dstRow) {
    if (this->xformOnDecode()) {
        // Use swizzler's width, which accounts for subsets
        const int32_t width = fSwizzler->swizzleWidth();
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
    Result result = this->initializeSwizzler(info, options);
    if (result != kSuccess) {
        return result;
    }

    // If we have a subset, use incremental decode to efficiently decode only needed rows
    if (options.fSubset) {
        return this->performSubsetDecode(info, dst, dstRowStride, options);
    }

    return this->performFullDecode(info, dst, dstRowStride);
}

SkCodec::Result SkBmpRustCodec::performFullDecode(const SkImageInfo& dstInfo,
                                                  void* dst,
                                                  size_t dstRowStride) {
    // Reset the reader to start decoding from the beginning.
    fReader->reset_decode_state();

    // Read all image data into the Rust decoder's internal buffer.
    // This may return IncompleteInput if not enough data is available.
    rust_bmp::DecodingResult readResult = fReader->read_image_data();
    if (readResult == rust_bmp::DecodingResult::IncompleteInput) {
        return kIncompleteInput;
    }
    if (readResult != rust_bmp::DecodingResult::Success) {
        return MapDecodingResult(readResult);
    }

    const int32_t height = this->dimensions().height();
    const size_t srcRowBytes = fReader->row_bytes();

    // Get pointer to entire image buffer - single FFI call instead of per-row calls
    rust::Slice<const uint8_t> imageData = fReader->get_image_data();
    if (imageData.empty()) {
        return kErrorInInput;
    }

    const uint8_t* srcBase = imageData.data();

    // Optimization: Process rows in memory order to improve cache locality
    // For bottom-up BMPs (most common), we still need to write to reversed destination rows
    // but we read source rows sequentially for better cache performance.
    if (fIsBottomUp) {
        // Bottom-up: read source rows 0,1,2,... but write to dest rows height-1,height-2,...
        for (int32_t y = 0; y < height; ++y) {
            const uint8_t* srcRow = srcBase + (y * srcRowBytes);
            void* dstRow = SkTAddOffset<void>(dst, (height - 1 - y) * dstRowStride);
            this->swizzleRow(srcRow, dstRow);
        }
    } else {
        // Top-down: both source and dest are in same order - best cache locality
        for (int32_t y = 0; y < height; ++y) {
            const uint8_t* srcRow = srcBase + (y * srcRowBytes);
            void* dstRow = SkTAddOffset<void>(dst, y * dstRowStride);
            this->swizzleRow(srcRow, dstRow);
        }
    }

    return kSuccess;
}

SkCodec::Result SkBmpRustCodec::performSubsetDecode(const SkImageInfo& dstInfo,
                                                     void* dst,
                                                     size_t dstRowStride,
                                                     const Options& options) {
    // Reset decoder state
    fReader->reset_decode_state();

    // Read all image data into the Rust decoder's internal buffer.
    rust_bmp::DecodingResult readResult = fReader->read_image_data();
    if (readResult != rust_bmp::DecodingResult::Success) {
        return MapDecodingResult(readResult);
    }

    const int32_t height = this->dimensions().height();
    const SkIRect* subset = options.fSubset;
    const size_t srcRowBytes = fReader->row_bytes();

    // Get pointer to entire image buffer - single FFI call
    rust::Slice<const uint8_t> imageData = fReader->get_image_data();
    if (imageData.empty()) {
        return kErrorInInput;
    }

    const uint8_t* srcBase = imageData.data();

    // Calculate which rows we need to swizzle
    int32_t firstRow = subset->fTop;
    int32_t lastRow = subset->fBottom - 1;

    // Process all rows, but only swizzle the ones in our subset
    for (int32_t row = 0; row < height; ++row) {
        // Determine which image row this file row corresponds to
        int32_t imageRow = fIsBottomUp ? (height - 1 - row) : row;

        // Only swizzle if this image row is within our subset
        if (imageRow >= firstRow && imageRow <= lastRow) {
            const uint8_t* srcRow = srcBase + (row * srcRowBytes);

            int32_t dstRowIdx = imageRow - firstRow;
            void* dstRowPtr = SkTAddOffset<void>(dst, dstRowIdx * dstRowStride);
            this->swizzleRow(srcRow, dstRowPtr);
        }
    }

    return kSuccess;
}

SkCodec::Result SkBmpRustCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                        void* dst,
                                                        size_t dstRowBytes,
                                                        const Options& options) {
    // Validate and initialize swizzler
    Result result = this->initializeSwizzler(dstInfo, options);
    if (result != kSuccess) {
        return result;
    }

    // Reset the reader to start decoding from the beginning
    fReader->reset_decode_state();

    // Initialize decoding state
    DecodingState state;

    // Handle subset decoding - determine which rows we'll decode
    int firstRow, lastRow, numRows;
    if (options.fSubset) {
        firstRow = options.fSubset->top();
        lastRow = options.fSubset->bottom() - 1;
        numRows = lastRow - firstRow + 1;
    } else {
        firstRow = 0;
        lastRow = dstInfo.height() - 1;
        numRows = dstInfo.height();
    }

    state.fFirstRow = firstRow;
    state.fLastRow = lastRow;

    // Set up destination buffer information
    // Note: When decoding a subset, dst points to the first row of the subset in the
    // destination bitmap, so we calculate buffer size based on the number of rows we'll decode
    SkSafeMath safe;
    size_t totalDstSize = safe.mul(safe.castTo<size_t>(numRows), dstRowBytes);
    if (!safe.ok()) {
        return kInternalError;
    }

    state.fDecodingDstInfo.fDst = SkSpan(static_cast<uint8_t*>(dst), totalDstSize);
    state.fDecodingDstInfo.fDstRowStride = dstRowBytes;

    // Store state for subsequent incremental decode calls
    fIncrementalDecodingState = std::move(state);

    return kSuccess;
}

SkCodec::Result SkBmpRustCodec::onIncrementalDecode(int* rowsDecoded) {
    if (!fIncrementalDecodingState.has_value()) {
        return kInvalidParameters;
    }

    Result result = this->incrementalDecode(*fIncrementalDecodingState, rowsDecoded);

    // Clean up state when decoding completes or encounters non-recoverable error
    if (result == kSuccess || result != kIncompleteInput) {
        fIncrementalDecodingState.reset();
    }

    return result;
}

SkCodec::Result SkBmpRustCodec::incrementalDecode(DecodingState& state, int* rowsDecodedPtr) {
    int rowsDecoded = 0;
    DecodingState::DecodingDstInfo& dst = state.fDecodingDstInfo;

    // Attempt to read all image data if not already loaded.
    // This will return IncompleteInput if the stream doesn't have enough data yet.
    if (!fReader->image_data_loaded()) {
        rust_bmp::DecodingResult readResult = fReader->read_image_data();

        if (readResult == rust_bmp::DecodingResult::IncompleteInput) {
            // Not enough data available yet - return and let caller retry
            if (rowsDecodedPtr) {
                *rowsDecodedPtr = 0;
            }
            return kIncompleteInput;
        }

        if (readResult != rust_bmp::DecodingResult::Success) {
            return MapDecodingResult(readResult);
        }
    }

    // All data is now loaded - swizzle all rows in our subset
    const int32_t height = this->dimensions().height();
    const size_t srcRowBytes = fReader->row_bytes();

    // Get pointer to entire image buffer - single FFI call
    rust::Slice<const uint8_t> imageData = fReader->get_image_data();
    if (imageData.empty()) {
        return kErrorInInput;
    }

    const uint8_t* srcBase = imageData.data();

    for (int32_t fileRow = 0; fileRow < height; ++fileRow) {
        // Determine which image row this file row corresponds to
        int32_t imageRow = fIsBottomUp ? (height - 1 - fileRow) : fileRow;

        // Only swizzle rows within our subset range
        if (imageRow >= state.fFirstRow && imageRow <= state.fLastRow) {
            const uint8_t* srcRow = srcBase + (fileRow * srcRowBytes);

            int dstRowIndex = imageRow - state.fFirstRow;
            void* dstRow = dst.fDst.data() + (dstRowIndex * dst.fDstRowStride);
            this->swizzleRow(srcRow, dstRow);
            rowsDecoded++;
        }
    }

    if (rowsDecodedPtr) {
        *rowsDecodedPtr = rowsDecoded;
    }

    return kSuccess;
}
