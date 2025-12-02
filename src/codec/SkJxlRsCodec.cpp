/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJxlRsCodec.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkTemplates.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkFrameHolder.h"
#include "src/core/SkColorSpacePriv.h"

// Convert float32 to float16 (IEEE 754 binary16).
static uint16_t float_to_half(float f) {
    uint32_t bits;
    memcpy(&bits, &f, sizeof(float));

    uint16_t sign = (bits >> 16) & 0x8000;
    int32_t exponent = ((bits >> 23) & 0xff) - 127;
    uint32_t mantissa = bits & 0x007fffff;

    // Handle special cases.
    if (exponent == 128) {  // Infinity or NaN.
        return sign | 0x7c00 | (mantissa != 0 ? 0x0200 : 0);
    }
    if (exponent > 15) {  // Overflow to infinity.
        return sign | 0x7c00;
    }
    if (exponent < -14) {  // Too small for denormal, flush to zero.
        return sign;
    }
    if (exponent < -14 + 1) {  // Denormal.
        mantissa |= 0x00800000;  // Add implicit leading 1.
        uint32_t shift = -exponent - 14;
        return sign | (mantissa >> (23 - 10 + shift));
    }

    // Normal number.
    return sign | ((exponent + 15) << 10) | (mantissa >> 13);
}

// ============================================================================
// SkJxlRsCodec::FrameHolder::JxlFrame
// ============================================================================

class SkJxlRsCodec::FrameHolder::JxlFrame final : public SkFrame {
public:
    JxlFrame(int id, SkEncodedInfo::Alpha alpha)
        : INHERITED(id), fReportedAlpha(alpha) {}

    SkEncodedInfo::Alpha onReportedAlpha() const override { return fReportedAlpha; }

private:
    const SkEncodedInfo::Alpha fReportedAlpha;

    using INHERITED = SkFrame;
};

// ============================================================================
// SkJxlRsCodec::FrameHolder
// ============================================================================

SkJxlRsCodec::FrameHolder::FrameHolder(int width, int height)
    : INHERITED(width, height) {}

SkJxlRsCodec::FrameHolder::~FrameHolder() = default;

void SkJxlRsCodec::FrameHolder::appendFrame(const JxlFrameInfo& info) {
    const int frameId = fFrames.size();

    // Determine alpha based on basic info (stored in codec).
    // For now, assume alpha from basic info.
    auto alpha = SkEncodedInfo::Alpha::kOpaque_Alpha;

    fFrames.emplace_back(frameId, alpha);
    JxlFrame& frame = fFrames.back();

    // Set frame duration.
    frame.setDuration(info.header.duration_ms);

    // JXL doesn't have disposal or blend modes like PNG/GIF.
    // All frames are full-frame replacements.
    frame.setDisposalMethod(SkCodecAnimation::DisposalMethod::kRestorePrevious);
    frame.setBlend(SkCodecAnimation::Blend::kSrc);

    // Set frame as not fully received yet.
    frame.setHasAlpha(alpha != SkEncodedInfo::Alpha::kOpaque_Alpha);
}

void SkJxlRsCodec::FrameHolder::markFrameAsFullyReceived(size_t index) {
    if (index < fFrames.size()) {
        fFrames[index].setFullyReceived();
    }
}

const SkFrame* SkJxlRsCodec::FrameHolder::onGetFrame(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= fFrames.size()) {
        return nullptr;
    }
    return &fFrames[index];
}

// ============================================================================
// SkJxlRsCodec
// ============================================================================

SkJxlRsCodec::SkJxlRsCodec(SkEncodedInfo&& info,
                           std::unique_ptr<SkStream> stream,
                           sk_sp<const SkData> data,
                           rust_jxl::BasicInfo basicInfo)
    : INHERITED(std::move(info), skcms_PixelFormat_RGBA_8888, std::move(stream))
    , fDecoder(rust_jxl::new_decoder())
    , fData(std::move(data))
    , fBasicInfo(basicInfo)
    , fFrameHolder(basicInfo.width, basicInfo.height)
    , fIsHighBitDepth(basicInfo.bits_per_sample > 8) {
}

bool SkJxlRsCodec::IsJxl(const void* data, size_t length) {
    if (!data || length < 2) {
        return false;
    }

    rust::Slice<const uint8_t> slice(static_cast<const uint8_t*>(data), length);
    return rust_jxl::signature_check(slice);
}

std::unique_ptr<SkCodec> SkJxlRsCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                       Result* result) {
    // Read all data into memory. jxl-rs doesn't support streaming yet.
    sk_sp<SkData> data = SkCopyStreamToData(stream.get());
    if (!data || data->size() == 0) {
        *result = kInvalidInput;
        return nullptr;
    }

    // Check signature.
    if (!IsJxl(data->data(), data->size())) {
        *result = kInvalidInput;
        return nullptr;
    }

    // Create decoder and feed data.
    auto decoder = rust_jxl::new_decoder();
    rust::Slice<const uint8_t> dataSlice(
        static_cast<const uint8_t*>(data->data()),
        data->size());

    auto status = decoder->set_input(dataSlice, true);
    if (status != rust_jxl::DecodingStatus::Success) {
        *result = kInvalidInput;
        return nullptr;
    }

    // Process to get basic info.
    status = decoder->process();
    if (status != rust_jxl::DecodingStatus::BasicInfo) {
        *result = kInvalidInput;
        return nullptr;
    }

    auto basicInfo = decoder->get_basic_info();

    // Construct SkEncodedInfo.
    SkEncodedInfo::Color color = SkEncodedInfo::kRGBA_Color;
    SkEncodedInfo::Alpha alpha = basicInfo.has_alpha
                                      ? SkEncodedInfo::kUnpremul_Alpha
                                      : SkEncodedInfo::kOpaque_Alpha;
    uint8_t bitsPerComponent = std::min<uint32_t>(basicInfo.bits_per_sample, 16);

    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(
        basicInfo.width,
        basicInfo.height,
        color,
        alpha,
        bitsPerComponent);

    // Get ICC profile if available.
    auto iccData = decoder->get_icc_profile();
    sk_sp<SkColorSpace> colorSpace;
    if (!iccData.empty()) {
        sk_sp<SkData> iccSkData = SkData::MakeWithCopy(iccData.data(), iccData.size());
        colorSpace = SkColorSpace::Make(*skcms_ICCProfile_from_SkData(iccSkData.get()));
    }
    if (!colorSpace) {
        // Default to sRGB.
        colorSpace = SkColorSpace::MakeSRGB();
    }
    encodedInfo.setColorSpace(std::move(colorSpace));

    // Reset decoder for actual decoding.
    decoder->reset();

    *result = kSuccess;
    return std::unique_ptr<SkCodec>(new SkJxlRsCodec(
        std::move(encodedInfo),
        std::move(stream),
        std::move(data),
        basicInfo));
}

bool SkJxlRsCodec::onRewind() {
    fDecoder->reset();
    fNumDecodedFrames = 0;
    return true;
}

bool SkJxlRsCodec::conversionSupported(const SkImageInfo& dstInfo,
                                       bool srcIsOpaque,
                                       bool needsColorXform) {
    // Support RGBA_8888, RGBA_F16.
    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return true;
        case kRGBA_F16_SkColorType:
            return fIsHighBitDepth;
        default:
            return false;
    }
}

int SkJxlRsCodec::onGetFrameCount() {
    if (!fBasicInfo.have_animation) {
        return 1;
    }

    // Scan for all frames if we haven't yet.
    if (!fAllFramesScanned) {
        scanFrames();
    }

    return std::max<int>(1, fFrameHolder.size());
}

bool SkJxlRsCodec::onGetFrameInfo(int index, FrameInfo* info) const {
    if (index < 0 || index >= static_cast<int>(fFrameHolder.size())) {
        return false;
    }

    const SkFrame* frame = fFrameHolder.onGetFrame(index);
    if (!frame) {
        return false;
    }

    if (info) {
        frame->fillIn(info, true);
    }
    return true;
}

int SkJxlRsCodec::onGetRepetitionCount() {
    if (!fBasicInfo.have_animation) {
        return 0;  // Not animated.
    }

    if (fBasicInfo.animation_loop_count == 0) {
        return kRepetitionCountInfinite;
    }

    return fBasicInfo.animation_loop_count - 1;
}

SkCodec::IsAnimated SkJxlRsCodec::onIsAnimated() {
    return fBasicInfo.have_animation ? IsAnimated::kYes : IsAnimated::kNo;
}

const SkFrameHolder* SkJxlRsCodec::getFrameHolder() const {
    return &fFrameHolder;
}

bool SkJxlRsCodec::scanFrames() {
    if (fAllFramesScanned) {
        return true;
    }

    // Reset decoder and feed data.
    fDecoder->reset();
    rust::Slice<const uint8_t> dataSlice(
        static_cast<const uint8_t*>(fData->data()),
        fData->size());
    fDecoder->set_input(dataSlice, true);

    // Process until we discover all frames.
    for (;;) {
        auto status = fDecoder->process();

        if (status == rust_jxl::DecodingStatus::BasicInfo) {
            // Already have this.
            continue;
        }

        if (status == rust_jxl::DecodingStatus::Frame) {
            JxlFrameInfo frameInfo;
            frameInfo.header = fDecoder->get_frame_header();
            frameInfo.received = false;
            fFrameHolder.appendFrame(frameInfo);
            continue;
        }

        if (status == rust_jxl::DecodingStatus::FullImage) {
            // Skip actual decoding in scan mode.
            if (!fDecoder->has_more_frames()) {
                fAllFramesScanned = true;
                break;
            }
            continue;
        }

        if (status == rust_jxl::DecodingStatus::Success) {
            fAllFramesScanned = true;
            break;
        }

        // Error or need more input.
        break;
    }

    return fAllFramesScanned;
}

SkCodec::Result SkJxlRsCodec::onGetPixels(const SkImageInfo& dstInfo,
                                          void* dst,
                                          size_t rowBytes,
                                          const Options& options,
                                          int* rowsDecodedPtr) {
    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
    }

    if (!this->conversionSupported(dstInfo, !fBasicInfo.has_alpha, false)) {
        return kInvalidConversion;
    }

    int frameIndex = options.fFrameIndex;
    if (frameIndex < 0 || (fBasicInfo.have_animation && frameIndex >= onGetFrameCount())) {
        return kInvalidParameters;
    }

    return decodeFrame(frameIndex, dst, rowBytes);
}

SkCodec::Result SkJxlRsCodec::decodeFrame(int frameIndex, void* dst, size_t rowBytes) {
    // Reset and feed data.
    fDecoder->reset();
    rust::Slice<const uint8_t> dataSlice(
        static_cast<const uint8_t*>(fData->data()),
        fData->size());
    fDecoder->set_input(dataSlice, true);

    // Determine pixel format based on bit depth.
    bool decodeToF16 = fIsHighBitDepth;
    if (decodeToF16) {
        fDecoder->set_pixel_format(rust_jxl::PixelFormat::RgbaF32);
    } else {
        fDecoder->set_pixel_format(rust_jxl::PixelFormat::Rgba8);
    }

    // Process until we reach the desired frame.
    int currentFrame = -1;
    bool gotBasicInfo = false;

    for (;;) {
        auto status = fDecoder->process();

        if (status == rust_jxl::DecodingStatus::Error) {
            return kErrorInInput;
        }

        if (status == rust_jxl::DecodingStatus::NeedMoreInput) {
            return kIncompleteInput;
        }

        if (status == rust_jxl::DecodingStatus::BasicInfo) {
            gotBasicInfo = true;
            continue;
        }

        if (status == rust_jxl::DecodingStatus::Frame) {
            currentFrame++;
            continue;
        }

        if (status == rust_jxl::DecodingStatus::FullImage) {
            if (currentFrame == frameIndex) {
                // This is the frame we want - decode it.
                const uint32_t width = fBasicInfo.width;
                const uint32_t height = fBasicInfo.height;
                const bool hasAlpha = fBasicInfo.has_alpha;

                if (decodeToF16) {
                    // Decode to F32 then convert to F16.
                    size_t pixelCount = width * height;
                    std::vector<uint8_t> f32Buffer(pixelCount * 4 * sizeof(float));

                    rust::Slice<uint8_t> bufferSlice(f32Buffer.data(), f32Buffer.size());
                    status = fDecoder->get_pixels(bufferSlice);

                    if (status != rust_jxl::DecodingStatus::Success) {
                        return kErrorInInput;
                    }

                    // Convert F32 -> F16.
                    const float* srcPixels = reinterpret_cast<const float*>(f32Buffer.data());
                    for (uint32_t y = 0; y < height; ++y) {
                        uint64_t* dstRow = reinterpret_cast<uint64_t*>(
                            static_cast<uint8_t*>(dst) + y * rowBytes);

                        for (uint32_t x = 0; x < width; ++x) {
                            size_t srcIdx = (y * width + x) * 4;
                            float r = srcPixels[srcIdx + 0];
                            float g = srcPixels[srcIdx + 1];
                            float b = srcPixels[srcIdx + 2];
                            float a = srcPixels[srcIdx + 3];

                            // Pack as RGBA F16 (4 x uint16).
                            uint64_t pixel = (static_cast<uint64_t>(float_to_half(a)) << 48) |
                                           (static_cast<uint64_t>(float_to_half(b)) << 32) |
                                           (static_cast<uint64_t>(float_to_half(g)) << 16) |
                                           static_cast<uint64_t>(float_to_half(r));
                            dstRow[x] = pixel;
                        }
                    }
                } else {
                    // Decode to RGBA8.
                    size_t pixelCount = width * height;
                    std::vector<uint8_t> rgbaBuffer(pixelCount * 4);

                    rust::Slice<uint8_t> bufferSlice(rgbaBuffer.data(), rgbaBuffer.size());
                    status = fDecoder->get_pixels(bufferSlice);

                    if (status != rust_jxl::DecodingStatus::Success) {
                        return kErrorInInput;
                    }

                    // Copy to destination with swizzling for BGRA if needed.
                    for (uint32_t y = 0; y < height; ++y) {
                        uint32_t* dstRow = reinterpret_cast<uint32_t*>(
                            static_cast<uint8_t*>(dst) + y * rowBytes);

                        for (uint32_t x = 0; x < width; ++x) {
                            size_t srcIdx = (y * width + x) * 4;
                            uint8_t r = rgbaBuffer[srcIdx + 0];
                            uint8_t g = rgbaBuffer[srcIdx + 1];
                            uint8_t b = rgbaBuffer[srcIdx + 2];
                            uint8_t a = rgbaBuffer[srcIdx + 3];

                            // Pack as SkPMColor (ARGB or BGRA depending on platform).
                            dstRow[x] = SkPackARGB32(a, r, g, b);
                        }
                    }
                }

                fNumDecodedFrames = currentFrame + 1;
                return kSuccess;
            }

            // Not the frame we want - continue to next frame.
            if (!fDecoder->has_more_frames()) {
                return kInvalidParameters;  // Requested frame doesn't exist.
            }
            continue;
        }

        if (status == rust_jxl::DecodingStatus::Success) {
            // End of image without finding requested frame.
            return kInvalidParameters;
        }
    }

    return kErrorInInput;
}
