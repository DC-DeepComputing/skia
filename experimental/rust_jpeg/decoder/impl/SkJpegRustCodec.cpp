/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_jpeg/decoder/impl/SkJpegRustCodec.h"

#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkJpegMetadataDecoder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "rust/common/SkStreamAdapter.h"
#include "rust/common/SpanUtils.h"
#include "src/base/SkSafeMath.h"
#include "src/codec/SkParseEncodedOrigin.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkStreamPriv.h"

static constexpr SkColorType kXformSrcColorType = kRGBA_8888_SkColorType;

// Static assertions to validate that Rust JpegColor/JpegAlpha enum values
// match the corresponding SkEncodedInfo enum values.
static_assert(static_cast<int>(rust_jpeg::JpegColor::RGB) ==
              static_cast<int>(SkEncodedInfo::kRGB_Color),
              "JpegColor::RGB must match SkEncodedInfo::kRGB_Color");
static_assert(static_cast<int>(rust_jpeg::JpegColor::Grayscale) ==
              static_cast<int>(SkEncodedInfo::kGray_Color),
              "JpegColor::Grayscale must match SkEncodedInfo::kGray_Color");

static_assert(static_cast<int>(rust_jpeg::JpegAlpha::Opaque) ==
              static_cast<int>(SkEncodedInfo::kOpaque_Alpha),
              "JpegAlpha::Opaque must match SkEncodedInfo::kOpaque_Alpha");

namespace {

SkCodec::Result MapDecodingResult(rust_jpeg::DecodingResult rustResult) {
    switch (rustResult) {
        case rust_jpeg::DecodingResult::Success:
            return SkCodec::kSuccess;
        case rust_jpeg::DecodingResult::FormatError:
            return SkCodec::kErrorInInput;
        case rust_jpeg::DecodingResult::ParameterError:
            return SkCodec::kInvalidParameters;
        case rust_jpeg::DecodingResult::UnsupportedFeature:
            return SkCodec::kUnimplemented;
        case rust_jpeg::DecodingResult::IncompleteInput:
            return SkCodec::kIncompleteInput;
        case rust_jpeg::DecodingResult::MemoryError:
            return SkCodec::kInternalError;
        case rust_jpeg::DecodingResult::OtherError:
            return SkCodec::kErrorInInput;
    }
    SK_ABORT("Unexpected `rust_jpeg::DecodingResult`: %d", static_cast<int>(rustResult));
}

}  // namespace

std::unique_ptr<SkJpegRustCodec> SkJpegRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
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
    rust::Box<rust_jpeg::ResultOfReader> resultOfReader =
            rust_jpeg::new_reader(std::move(inputAdapter));

    rust_jpeg::DecodingResult rustResult = resultOfReader->err();
    if (rustResult != rust_jpeg::DecodingResult::Success) {
        *result = MapDecodingResult(rustResult);
        return nullptr;
    }

    rust::Box<rust_jpeg::Reader> reader = resultOfReader->unwrap();

    if (!reader->metadata_loaded()) {
        rust_jpeg::DecodingResult metadataResult = reader->read_metadata();

        if (metadataResult == rust_jpeg::DecodingResult::IncompleteInput) {
            *result = kIncompleteInput;
            return nullptr;
        } else if (metadataResult != rust_jpeg::DecodingResult::Success) {
            *result = MapDecodingResult(metadataResult);
            return nullptr;
        }

        SkASSERT_RELEASE(reader->metadata_loaded());
    }

    uint32_t width = reader->width();
    uint32_t height = reader->height();
    rust_jpeg::JpegColor rustColor = reader->color();
    rust_jpeg::JpegAlpha rustAlpha = reader->alpha();

    SkEncodedInfo::Color color = static_cast<SkEncodedInfo::Color>(rustColor);
    SkEncodedInfo::Alpha alpha = static_cast<SkEncodedInfo::Alpha>(rustAlpha);
    constexpr int kBitsPerComponent = 8;

    std::unique_ptr<SkCodecs::ColorProfile> colorProfile;
    rust::Vec<uint8_t> profileData = reader->icc_profile();
    if (!profileData.empty()) {
        sk_sp<SkData> iccData = SkData::MakeWithCopy(profileData.data(), profileData.size());
        colorProfile = SkCodecs::ColorProfile::MakeICCProfile(std::move(iccData));
    }

    // Read EXIF orientation.
    SkEncodedOrigin orientation = kDefault_SkEncodedOrigin;
    rust::Vec<uint8_t> exifData = reader->exif_data();
    if (!exifData.empty()) {
        SkParseEncodedOrigin(exifData.data(), exifData.size(), &orientation);
    }

    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(
        width,
        height,
        color,
        alpha,
        kBitsPerComponent,
        std::move(colorProfile)
    );

    // Guard against overflow: reject images whose raw pixel buffer would
    // exceed size_t range.
    size_t bytesPerPixel;
    switch (color) {
        case SkEncodedInfo::kRGB_Color:
            bytesPerPixel = 3;
            break;
        case SkEncodedInfo::kGray_Color:
            bytesPerPixel = 1;
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
    return std::unique_ptr<SkJpegRustCodec>(new SkJpegRustCodec(
        std::move(encodedInfo),
        std::move(stream),
        std::move(reader),
        orientation
    ));
}

SkJpegRustCodec::SkJpegRustCodec(SkEncodedInfo&& encodedInfo,
                                  std::unique_ptr<SkStream> stream,
                                  rust::Box<rust_jpeg::Reader> reader,
                                  SkEncodedOrigin origin)
    : SkCodec(std::move(encodedInfo), skcms_PixelFormat_RGB_888,
              /* stream = */ nullptr, origin)
    , fReader(std::move(reader))
    , fPrivStream(std::move(stream)) {
    SkASSERT_RELEASE(fPrivStream);
}

SkJpegRustCodec::~SkJpegRustCodec() = default;

sk_sp<const SkData> SkJpegRustCodec::getEncodedData() const {
    SkASSERT_RELEASE(fPrivStream);
    sk_sp<const SkData> data = fPrivStream->getData();
    if (data) {
        return data;
    }
    auto dStream = fPrivStream->duplicate();
    if (!dStream->hasLength()) {
        return nullptr;
    }
    return SkData::MakeFromStream(dStream.get(), dStream->getLength());
}

bool SkJpegRustCodec::onGetFrameInfo(int index, FrameInfo* info) const {
    if (index != 0) {
        return false;
    }
    if (info) {
        info->fRequiredFrame = SkCodec::kNoFrame;
        info->fDuration = 0;
        info->fFullyReceived = fReader->image_data_loaded();
        info->fAlphaType = this->getInfo().alphaType();
        info->fHasAlphaWithinBounds = false;  // JPEG never has alpha
        info->fDisposalMethod = SkCodecAnimation::DisposalMethod::kKeep;
        info->fBlend = SkCodecAnimation::Blend::kSrc;
        info->fFrameRect = SkIRect::MakeSize(this->dimensions());
    }
    return true;
}

bool SkJpegRustCodec::onGetGainmapInfo(SkGainmapInfo* info,
                                        std::unique_ptr<SkStream>* gainmapImageStream) {
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    sk_sp<const SkData> encodedData = this->getEncodedData();
    if (!encodedData) {
        return false;
    }

    auto metadataDecoder = SkJpegMetadataDecoder::Make(encodedData);
    if (!metadataDecoder || !metadataDecoder->mightHaveGainmapImage()) {
        return false;
    }

    auto [gainmapData, gainmapInfo] = metadataDecoder->findGainmapImage(encodedData);
    if (!gainmapData) {
        return false;
    }

    *info = gainmapInfo;
    *gainmapImageStream = SkMemoryStream::Make(gainmapData);
    return true;
#else
    return false;
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS
}

bool SkJpegRustCodec::onRewind() {
    fIncrementalDecodingState.reset();

    if (!fPrivStream->rewind()) {
        return false;
    }

    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(fPrivStream.get());
    rust::Box<rust_jpeg::ResultOfReader> resultOfReader =
            rust_jpeg::new_reader(std::move(inputAdapter));

    if (resultOfReader->err() != rust_jpeg::DecodingResult::Success) {
        return false;
    }

    fReader = resultOfReader->unwrap();

    if (!fReader->metadata_loaded()) {
        rust_jpeg::DecodingResult metadataResult = fReader->read_metadata();
        if (metadataResult != rust_jpeg::DecodingResult::Success) {
            return false;
        }
    }

    return true;
}

SkCodec::Result SkJpegRustCodec::initializeSwizzler(const SkImageInfo& dstInfo,
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
        return kInvalidConversion;
    }
    return kSuccess;
}

void SkJpegRustCodec::swizzleRow(const uint8_t* srcRow, void* dstRow) {
    if (this->xformOnDecode()) {
        const int32_t width = fSwizzler->swizzleWidth();
        fSwizzler->swizzle(fXformBuffer.get(), srcRow);
        this->applyColorXform(dstRow, fXformBuffer.get(), width);
    } else {
        fSwizzler->swizzle(dstRow, srcRow);
    }
}

SkCodec::Result SkJpegRustCodec::onGetPixels(const SkImageInfo& info,
                                              void* dst,
                                              size_t dstRowStride,
                                              const Options& options,
                                              int* rowsDecoded) {
    Result result = this->initializeSwizzler(info, options);
    if (result != kSuccess) {
        return result;
    }

    int decodedRows = 0;
    result = this->performFullDecode(info, dst, dstRowStride, &decodedRows);
    if (rowsDecoded) {
        *rowsDecoded = decodedRows;
    }
    return result;
}

SkCodec::Result SkJpegRustCodec::performFullDecode(const SkImageInfo& dstInfo,
                                                    void* dst,
                                                    size_t dstRowStride,
                                                    int* rowsDecoded) {
    fReader->reset_decode_state();

    rust_jpeg::DecodingResult readResult = fReader->read_image_data();
    if (readResult != rust_jpeg::DecodingResult::Success) {
        return MapDecodingResult(readResult);
    }

    const size_t srcRowBytes = fReader->row_bytes();

    rust::Slice<const uint8_t> imageData;
    rust_jpeg::DecodedRowsInfo rowsInfo = fReader->get_next_rows(imageData);

    if (imageData.empty()) {
        return kErrorInInput;
    }

    SkSpan<const uint8_t> srcImage(imageData.data(), imageData.size());
    SkSpan<uint8_t> dstImage(static_cast<uint8_t*>(dst), dstInfo.height() * dstRowStride);
    const uint32_t rowCount = rowsInfo.row_count;

    for (uint32_t y = 0; y < rowCount; ++y) {
        SkSpan<const uint8_t> srcRow = srcImage.subspan(y * srcRowBytes, srcRowBytes);
        SkSpan<uint8_t> dstRow = dstImage.subspan(y * dstRowStride, dstRowStride);
        this->swizzleRow(srcRow.data(), dstRow.data());
    }

    if (rowsDecoded) {
        *rowsDecoded = static_cast<int>(rowCount);
    }

    return (rowCount == static_cast<uint32_t>(this->dimensions().height())) ? kSuccess
                                                                             : kIncompleteInput;
}

SkCodec::Result SkJpegRustCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                          void* dst,
                                                          size_t dstRowBytes,
                                                          const Options& options) {
    Result result = this->initializeSwizzler(dstInfo, options);
    if (result != kSuccess) {
        return result;
    }

    fReader->reset_decode_state();

    DecodingState state;

    const int32_t height = dstInfo.height();
    SkSafeMath safe;
    size_t totalDstSize = safe.mul(safe.castTo<size_t>(height), dstRowBytes);
    if (!safe.ok()) {
        return kInternalError;
    }

    state.fDst = SkSpan(static_cast<uint8_t*>(dst), totalDstSize);
    state.fDstRowStride = dstRowBytes;

    fIncrementalDecodingState = std::move(state);

    return kSuccess;
}

SkCodec::Result SkJpegRustCodec::onIncrementalDecode(int* rowsDecoded) {
    if (!fIncrementalDecodingState.has_value()) {
        return kInvalidParameters;
    }

    Result result = this->incrementalDecode(*fIncrementalDecodingState, rowsDecoded);

    if (result == kSuccess || result != kIncompleteInput) {
        fIncrementalDecodingState.reset();
    }

    return result;
}

SkCodec::Result SkJpegRustCodec::incrementalDecode(DecodingState& state, int* rowsDecodedPtr) {
    const size_t srcRowBytes = fReader->row_bytes();

    rust_jpeg::DecodingResult readResult = fReader->read_image_data();

    if (readResult != rust_jpeg::DecodingResult::Success &&
        readResult != rust_jpeg::DecodingResult::IncompleteInput) {
        return MapDecodingResult(readResult);
    }

    rust::Slice<const uint8_t> imageData;
    rust_jpeg::DecodedRowsInfo rowsInfo = fReader->get_next_rows(imageData);
    const uint32_t dstRowStart = rowsInfo.dst_row_start;
    const uint32_t rowCount = rowsInfo.row_count;

    if (rowCount > 0 && !imageData.empty()) {
        SkSpan<const uint8_t> srcImage(imageData.data(), imageData.size());

        for (uint32_t i = 0; i < rowCount; ++i) {
            SkSpan<const uint8_t> srcRow = srcImage.subspan(i * srcRowBytes, srcRowBytes);
            uint32_t dstY = dstRowStart + i;
            void* dstRow = state.fDst.data() + (dstY * state.fDstRowStride);
            this->swizzleRow(srcRow.data(), dstRow);
        }
        state.fTotalRowsDecoded += rowCount;
    }

    if (rowsDecodedPtr) {
        *rowsDecodedPtr = state.fTotalRowsDecoded;
    }

    return (readResult == rust_jpeg::DecodingResult::Success) ? kSuccess : kIncompleteInput;
}
