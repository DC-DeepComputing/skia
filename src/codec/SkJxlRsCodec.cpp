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
#include "src/core/SkColorData.h"
#include "include/private/base/SkTemplates.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkFrameHolder.h"
#include "src/core/SkColorSpacePriv.h"
#include "modules/skcms/skcms.h"

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

    bool isFullyReceived() const { return fFullyReceived; }
    void setFullyReceived() { fFullyReceived = true; }

private:
    const SkEncodedInfo::Alpha fReportedAlpha;
    bool fFullyReceived = false;

    using INHERITED = SkFrame;
};

// ============================================================================
// SkJxlRsCodec::FrameHolder
// ============================================================================

SkJxlRsCodec::FrameHolder::FrameHolder(int width, int height)
    : SkFrameHolder() {}

SkJxlRsCodec::FrameHolder::~FrameHolder() = default;

void SkJxlRsCodec::FrameHolder::appendFrame(const JxlFrameInfo& info, bool hasAlpha) {
    const int frameId = fFrames.size();

    // Determine alpha based on the image's basic info.
    auto alpha = hasAlpha ? SkEncodedInfo::Alpha::kUnpremul_Alpha
                          : SkEncodedInfo::Alpha::kOpaque_Alpha;

    fFrames.emplace_back(frameId, alpha);
    JxlFrame& frame = fFrames.back();

    // Set frame duration.
    frame.setDuration(info.header.duration_ms);

    // JXL frames are full-frame replacements - each frame is independent.
    // Use kKeep for disposal (the frame remains) and kSrc for blend (full replacement).
    frame.setDisposalMethod(SkCodecAnimation::DisposalMethod::kKeep);
    frame.setBlend(SkCodecAnimation::Blend::kSrc);

    // JXL frames don't depend on previous frames - each is a complete image.
    // kNoFrame (-1) means this frame doesn't require any previous frame.
    frame.setRequiredFrame(SkCodec::kNoFrame);

    // Set frame as having alpha if the image has an alpha channel.
    frame.setHasAlpha(hasAlpha);
}

void SkJxlRsCodec::FrameHolder::markFrameAsFullyReceived(size_t index) {
    if (index < fFrames.size()) {
        fFrames[index].setFullyReceived();
    }
}

size_t SkJxlRsCodec::FrameHolder::size() const {
    return fFrames.size();
}

bool SkJxlRsCodec::FrameHolder::getFrameInfo(int index, FrameInfo* info) const {
    if (index < 0 || static_cast<size_t>(index) >= fFrames.size()) {
        return false;
    }
    const SkFrame* frame = &fFrames[index];
    if (frame && info) {
        bool isFullyReceived = static_cast<const JxlFrame*>(frame)->isFullyReceived();
        frame->fillIn(info, isFullyReceived);
    }
    return !!frame;
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
    if (!stream->hasLength()) {
        *result = kInvalidInput;
        return nullptr;
    }
    sk_sp<SkData> data = SkData::MakeFromStream(stream.get(), stream->getLength());
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

    // Get ICC profile if available.
    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = nullptr;
    rust::Slice<const uint8_t> iccData = decoder->get_icc_profile();
    if (iccData.size() > 0) {
        skcms_ICCProfile parsedProfile;
        if (skcms_Parse(iccData.data(), iccData.size(), &parsedProfile)) {
            profile = SkEncodedInfo::ICCProfile::Make(parsedProfile);
        }
    }

    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(
        basicInfo.width,
        basicInfo.height,
        color,
        alpha,
        bitsPerComponent,
        std::move(profile));

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
    if (index < 0) {
        return false;
    }

    // For non-animated images, we have a single frame that isn't in fFrameHolder
    if (!fBasicInfo.have_animation) {
        if (index > 0) {
            return false;
        }
        if (info) {
            info->fRequiredFrame = SkCodec::kNoFrame;
            info->fDuration = 0;
            info->fFullyReceived = true;
            info->fAlphaType = fBasicInfo.has_alpha ? kUnpremul_SkAlphaType : kOpaque_SkAlphaType;
            info->fHasAlphaWithinBounds = fBasicInfo.has_alpha;
            info->fDisposalMethod = SkCodecAnimation::DisposalMethod::kKeep;
            info->fBlend = SkCodecAnimation::Blend::kSrc;
            info->fFrameRect = SkIRect::MakeWH(fBasicInfo.width, fBasicInfo.height);
        }
        return true;
    }

    // For animated images, check the frame holder
    if (index >= static_cast<int>(fFrameHolder.size())) {
        return false;
    }

    return fFrameHolder.getFrameInfo(index, info);
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
            fFrameHolder.appendFrame(frameInfo, fBasicInfo.has_alpha);
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

    return decodeFrame(frameIndex, dst, rowBytes, dstInfo);
}

SkCodec::Result SkJxlRsCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                        void* dst,
                                                        size_t rowBytes,
                                                        const Options& options) {
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

    // Store state for incremental decode.
    fIncrementalDst = dst;
    fIncrementalRowBytes = rowBytes;
    fIncrementalFrameIndex = frameIndex;
    fIncrementalDecodeStarted = true;
    fIncrementalDstInfo = dstInfo;

    return kSuccess;
}

SkCodec::Result SkJxlRsCodec::onIncrementalDecode(int* rowsDecoded) {
    if (!fIncrementalDecodeStarted) {
        return kInvalidParameters;
    }

    // Perform the actual decode.
    Result result = decodeFrame(fIncrementalFrameIndex, fIncrementalDst, fIncrementalRowBytes, fIncrementalDstInfo);

    if (result == kSuccess) {
        fIncrementalDecodeStarted = false;
        if (rowsDecoded) {
            *rowsDecoded = fBasicInfo.height;
        }
    }

    return result;
}

SkCodec::Result SkJxlRsCodec::decodeFrame(int frameIndex, void* dst, size_t rowBytes, const SkImageInfo& dstInfo) {
    // Reset and feed data.
    fDecoder->reset();
    rust::Slice<const uint8_t> dataSlice(
        static_cast<const uint8_t*>(fData->data()),
        fData->size());
    fDecoder->set_input(dataSlice, true);

    // Determine pixel format based on bit depth.
    bool decodeToF16 = (dstInfo.colorType() == kRGBA_F16_SkColorType);
    if (decodeToF16) {
        fDecoder->set_pixel_format(rust_jxl::PixelFormat::RgbaF32);
    } else {
        fDecoder->set_pixel_format(rust_jxl::PixelFormat::Rgba8);
    }

    // Determine if we need premultiplied alpha.
    const bool premulAlpha = (dstInfo.alphaType() == kPremul_SkAlphaType);
    const bool isBGRA = (dstInfo.colorType() == kBGRA_8888_SkColorType);

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
            // For non-animated images, we may not get a Frame event, so currentFrame stays at -1
            // In this case, increment it now so it becomes 0 (matching frameIndex 0)
            if (currentFrame == -1) {
                currentFrame = 0;
            }
            if (currentFrame == frameIndex) {
                // This is the frame we want - decode it.
                const uint32_t width = fBasicInfo.width;
                const uint32_t height = fBasicInfo.height;

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

                            // Premultiply if requested.
                            if (premulAlpha) {
                                if (a == 0.0f) {
                                    r = g = b = 0.0f;
                                } else if (a < 1.0f) {
                                    r *= a;
                                    g *= a;
                                    b *= a;
                                }
                            }

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

                    // Copy to destination with proper format handling.
                    for (uint32_t y = 0; y < height; ++y) {
                        uint8_t* dstRow = static_cast<uint8_t*>(dst) + y * rowBytes;

                        for (uint32_t x = 0; x < width; ++x) {
                            size_t srcIdx = (y * width + x) * 4;
                            uint8_t r = rgbaBuffer[srcIdx + 0];
                            uint8_t g = rgbaBuffer[srcIdx + 1];
                            uint8_t b = rgbaBuffer[srcIdx + 2];
                            uint8_t a = rgbaBuffer[srcIdx + 3];

                            // Premultiply if requested.
                            if (premulAlpha) {
                                if (a == 0) {
                                    r = g = b = 0;
                                } else if (a != 255) {
                                    r = (r * a + 127) / 255;
                                    g = (g * a + 127) / 255;
                                    b = (b * a + 127) / 255;
                                }
                            }

                            // Pack in the correct byte order.
                            size_t dstIdx = x * 4;
                            if (isBGRA) {
                                dstRow[dstIdx + 0] = b;
                                dstRow[dstIdx + 1] = g;
                                dstRow[dstIdx + 2] = r;
                                dstRow[dstIdx + 3] = a;
                            } else {
                                dstRow[dstIdx + 0] = r;
                                dstRow[dstIdx + 1] = g;
                                dstRow[dstIdx + 2] = b;
                                dstRow[dstIdx + 3] = a;
                            }
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
