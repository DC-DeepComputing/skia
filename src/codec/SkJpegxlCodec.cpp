/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegxlCodec.h"

#include <limits>
#include <vector>

#include "include/core/SkData.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTo.h"
#include "jxl/decode.h"
#include "jxl/decode_cxx.h"
#include "src/codec/SkFrameHolder.h"
#include "src/codec/SkSampler.h"
#include "src/core/SkOpts.h"
#include "src/core/SkStreamPriv.h"

namespace {

class Frame : public SkFrame {
public:
    explicit Frame(int i) : INHERITED(i) {}
    SkEncodedInfo::Alpha onReportedAlpha() const override { return SkEncodedInfo::kOpaque_Alpha; }

private:
    using INHERITED = SkFrame;
};

constexpr int kNoLastFrameIndex = std::numeric_limits<int>::max() - 1;

}  // namespace

bool SkJpegxlCodec::IsJpegxl(const void* buffer, size_t bytesRead) {
    JxlSignature result = JxlSignatureCheck(reinterpret_cast<const uint8_t*>(buffer), bytesRead);
    return (result == JXL_SIG_CODESTREAM) || (result == JXL_SIG_CONTAINER);
}

class SkJpegxlCodecPriv : public SkFrameHolder {
public:
    SkJpegxlCodecPriv() : fDecoder(JxlDecoderMake(/* memory_manager= */ nullptr)) {}
    JxlDecoderPtr fDecoder;  // unique_ptr with custom destructor
    JxlBasicInfo fInfo;
    bool fSeenAllFrames = false;
    std::vector<Frame> fFrames;
    int fLastDecodedFrame = kNoLastFrameIndex;
    void* fDst;
    size_t fPixelShift;
    size_t fRowBytes;
    SkColorType fDstColorType;

protected:
    const SkFrame* onGetFrame(int i) const override {
        SkASSERT(i >= 0 && static_cast<size_t>(i) < fFrames.size());
        return static_cast<const SkFrame*>(&fFrames[i]);
    }
};

SkJpegxlCodec::SkJpegxlCodec(std::unique_ptr<SkJpegxlCodecPriv> codec,
                             SkEncodedInfo&& info,
                             std::unique_ptr<SkStream> stream,
                             sk_sp<SkData> data)
        : INHERITED(std::move(info), skcms_PixelFormat_RGBA_16161616LE, std::move(stream))
        , fCodec(std::move(codec))
        , fData(std::move(data)) {}

std::unique_ptr<SkCodec> SkJpegxlCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                       Result* result) {
    *result = kInternalError;
    // Either wrap or copy stream data.
    sk_sp<SkData> data = nullptr;
    if (stream->getMemoryBase()) {
        data = SkData::MakeWithoutCopy(stream->getMemoryBase(), stream->getLength());
    } else {
        data = SkCopyStreamToData(stream.get());
        // Data is copied; stream could be released now.
        stream.reset(nullptr);
    }

    auto priv = std::make_unique<SkJpegxlCodecPriv>();
    JxlDecoder* dec = priv->fDecoder.get();

    // Only query metadata this time.
    auto status = JxlDecoderSubscribeEvents(dec, JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING);
    if (status != JXL_DEC_SUCCESS) {
        SK_ABORT("Can't get here");
        return nullptr;
    }

    status = JxlDecoderSetInput(dec, data->bytes(), data->size());
    if (status != JXL_DEC_SUCCESS) {
        SK_ABORT("Can't get here");
        return nullptr;
    }

    status = JxlDecoderProcessInput(dec);
    if (status == JXL_DEC_NEED_MORE_INPUT) {
        *result = kIncompleteInput;
        return nullptr;
    }
    if (status != JXL_DEC_BASIC_INFO) {
        *result = kInvalidInput;
        return nullptr;
    }
    JxlBasicInfo& info = priv->fInfo;
    status = JxlDecoderGetBasicInfo(dec, &info);
    if (status != JXL_DEC_SUCCESS) {
        SK_ABORT("Can't get here");
        return nullptr;
    }

    // Check that image dimensaions are not too large.
    if (!SkTFitsIn<int32_t>(info.xsize) || !SkTFitsIn<int32_t>(info.ysize)) {
        *result = kInvalidInput;
        return nullptr;
    }
    int32_t width = SkTo<int32_t>(info.xsize);
    int32_t height = SkTo<int32_t>(info.ysize);

    bool hasAlpha = (info.alpha_bits != 0);
    bool isGray = (info.num_color_channels == 1);
    SkEncodedInfo::Alpha alpha =
            hasAlpha ? SkEncodedInfo::kUnpremul_Alpha : SkEncodedInfo::kOpaque_Alpha;
    SkEncodedInfo::Color color;
    if (hasAlpha) {
        color = isGray ? SkEncodedInfo::kGrayAlpha_Color : SkEncodedInfo::kRGBA_Color;
    } else {
        color = isGray ? SkEncodedInfo::kGray_Color : SkEncodedInfo::kRGB_Color;
    }

    status = JxlDecoderProcessInput(dec);
    if (status != JXL_DEC_COLOR_ENCODING) {
        *result = kInvalidInput;
        return nullptr;
    }

    size_t iccSize = 0;
    // TODO(eustas): format field is currently ignored by decoder.
    status = JxlDecoderGetICCProfileSize(
        dec, /* format = */ nullptr, JXL_COLOR_PROFILE_TARGET_DATA, &iccSize);
    if (status != JXL_DEC_SUCCESS) {
        // Likely incompatible colorspace.
        iccSize = 0;
    }
    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = nullptr;
    if (iccSize) {
        auto icc = SkData::MakeUninitialized(iccSize);
        // TODO(eustas): format field is currently ignored by decoder.
        status = JxlDecoderGetColorAsICCProfile(dec,
                                                /* format = */ nullptr,
                                                JXL_COLOR_PROFILE_TARGET_DATA,
                                                reinterpret_cast<uint8_t*>(icc->writable_data()),
                                                iccSize);
        if (status != JXL_DEC_SUCCESS) {
            SK_ABORT("Can't get here");
            return nullptr;
        }
        profile = SkEncodedInfo::ICCProfile::Make(std::move(icc));
    }

    // TODO(eustas): consider kN32_SkColorType (== kBGRA_8888_SkColorType or kRGBA_8888_SkColorType)
    // TODO(eustas): what F16 endiannes is expected by SKIA?
    int bitsPerChannel = 16;

    *result = kSuccess;
    SkEncodedInfo encodedInfo =
            SkEncodedInfo::Make(width, height, color, alpha, bitsPerChannel, std::move(profile));

    return std::unique_ptr<SkCodec>(new SkJpegxlCodec(
            std::move(priv), std::move(encodedInfo), std::move(stream), std::move(data)));
}

SkCodec::Result SkJpegxlCodec::onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t rowBytes,
                                           const Options& options, int* rowsDecodedPtr) {
    // TODO(eustas): implement
    if (options.fSubset) {
        return kUnimplemented;
    }
    auto& codec = *fCodec.get();
    const int index = options.fFrameIndex;
    SkASSERT(0 == index || static_cast<size_t>(index) < codec.fFrames.size());
    auto dec = codec.fDecoder.get();
    JxlDecoderStatus status;

    if (codec.fLastDecodedFrame >= index) {
        codec.fLastDecodedFrame = kNoLastFrameIndex;
        JxlDecoderRewind(dec);
        status = JxlDecoderSubscribeEvents(dec, JXL_DEC_FRAME | JXL_DEC_FULL_IMAGE);
        if (status != JXL_DEC_SUCCESS) {
            SK_ABORT("Can't get here");
            return kInternalError;
        }
        status = JxlDecoderSetInput(dec, fData->bytes(), fData->size());
        if (status != JXL_DEC_SUCCESS) {
            SK_ABORT("Can't get here");
            return kInternalError;
        }
        codec.fLastDecodedFrame = -1;
    }

    int nextFrame = codec.fLastDecodedFrame + 1;
    if (nextFrame < index) {
        JxlDecoderSkipFrames(dec, index - nextFrame);
        codec.fLastDecodedFrame = index;
    }

    // Decode till the frame start.
    status = JxlDecoderProcessInput(dec);
    if (status != JXL_DEC_FRAME) {
        SK_ABORT("Can't get here");
        return kInternalError;
    }

    // At this moment we could setup the callback.
    codec.fDst = dst;
    codec.fRowBytes = rowBytes;

    // TODO(eustas): num_channels depend on alpha
    JxlPixelFormat format =
        {/* num_channels = */ 4, JXL_TYPE_UINT8, JXL_LITTLE_ENDIAN, /* align = */ 0};
    status = JxlDecoderSetImageOutCallback(dec, &format, SkJpegxlCodec::imageOutCallback, this);
    if (status != JXL_DEC_SUCCESS) {
        SK_ABORT("Can't get here");
        return kInternalError;
    }
    // Decode till the frame start.
    status = JxlDecoderProcessInput(dec);
    if (status != JXL_DEC_FULL_IMAGE) {
        SK_ABORT("Can't get here");
        return kInternalError;
    }
    *rowsDecodedPtr = dstInfo.height();

    return kSuccess;
}

bool SkJpegxlCodec::onRewind() {
    JxlDecoderRewind(fCodec->fDecoder.get());
    return true;
}

bool SkJpegxlCodec::conversionSupported(const SkImageInfo& dstInfo, bool srcIsOpaque,
                                        bool needsColorXform) {
    fCodec->fDstColorType = dstInfo.colorType();
    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            return true;  // memcpy
        case kBGRA_8888_SkColorType:
            return true;  // rgba->bgra

        // TODO(eustas): implement
        case kRGBA_F16_SkColorType:
            SkASSERT(needsColorXform);
            // We will need to multiply F16 values by 256.0?
            // We could do that with bit magic
            // (increase magnitute, perhaps just "or" a single bit...)
            return false;

        // TODO(eustas): implement
        case kRGB_565_SkColorType:
            return false;
        case kGray_8_SkColorType:
            return false;
        case kAlpha_8_SkColorType:
            return false;

        default:
            return false;
    }
    return true;
}

void SkJpegxlCodec::imageOutCallback(void* opaque, size_t x, size_t y,
                                     size_t num_pixels, const void* pixels) {
    SkJpegxlCodec* zis = reinterpret_cast<SkJpegxlCodec*>(opaque);
    auto& codec = *zis->fCodec.get();
    size_t offset = y * codec.fRowBytes + (x << codec.fPixelShift);
    void* dst = SkTAddOffset<void>(codec.fDst, offset);
    if (zis->colorXform()) {
        zis->applyColorXform(dst, pixels, num_pixels);
        return;
    }
    switch (zis->fCodec->fDstColorType) {
        case kRGBA_8888_SkColorType:
            memcpy(dst, pixels, 4 * num_pixels);
            return;
        case kBGRA_8888_SkColorType:
            SkOpts::RGBA_to_bgrA((uint32_t*) dst, (const uint32_t*)(pixels), num_pixels);
            return;
        default:
            SK_ABORT("Selected output format is not supported yet");
            return;
    }
}

bool SkJpegxlCodec::scanFrames() {
    auto decoder = JxlDecoderMake(/* memory_manager = */ nullptr);
    JxlDecoder* dec = decoder.get();
    auto frameHolder = fCodec.get();
    auto& frames = frameHolder->fFrames;
    auto info = fCodec->fInfo;
    frames.clear();

    auto status = JxlDecoderSubscribeEvents(dec, JXL_DEC_FRAME);
    if (status != JXL_DEC_SUCCESS) {
        SK_ABORT("Can't get here");
        return true;
    }

    status = JxlDecoderSetInput(dec, fData->bytes(), fData->size());
    if (status != JXL_DEC_SUCCESS) {
        SK_ABORT("Can't get here");
        return true;
    }

    while (true) {
        status = JxlDecoderProcessInput(dec);
        switch (status) {
            case JXL_DEC_FRAME: {
                size_t frameId = frames.size();
                JxlFrameHeader frameHeader;
                if (JxlDecoderGetFrameHeader(dec, &frameHeader) != JXL_DEC_SUCCESS) {
                    return true;
                }
                frames.emplace_back(static_cast<int>(frameId));
                auto& frame = frames.back();
                // TODO(eustas): for better consistency we need to track total duration and report
                //               frame duration as delta to previous frame.
                int duration = (1000 * frameHeader.duration * info.animation.tps_denominator) /
                               info.animation.tps_numerator;
                frame.setDuration(duration);
                frameHolder->setAlphaAndRequiredFrame(&frame);
                break;
            }
            case JXL_DEC_SUCCESS: {
                return true;
            }
            default: {
                return true;
            }
        }
    }
}

int SkJpegxlCodec::onGetFrameCount() {
    if (!fCodec->fInfo.have_animation) {
        return 1;
    }

    if (!fCodec->fSeenAllFrames) {
        fCodec->fSeenAllFrames = scanFrames();
    }

    return fCodec->fFrames.size();
}

bool SkJpegxlCodec::onGetFrameInfo(int index, FrameInfo* frameInfo) const {
    if (index < 0) {
        return false;
    }
    if (static_cast<size_t>(index) >= fCodec->fFrames.size()) {
        return false;
    }
    fCodec->fFrames[index].fillIn(frameInfo, true);
    return true;
}

int SkJpegxlCodec::onGetRepetitionCount() {
    JxlBasicInfo& info = fCodec->fInfo;
    if (!info.have_animation) {
        return 0;
    }

    if (info.animation.num_loops == 0) {
        return kRepetitionCountInfinite;
    }

    if (SkTFitsIn<int>(info.animation.num_loops)) {
        return info.animation.num_loops - 1;
    }

    // Largest "non-infinite" value.
    return std::numeric_limits<int>::max();
}

const SkFrameHolder* SkJpegxlCodec::getFrameHolder() const {
    return fCodec.get();
}

// TODO(eustas): implement
// SkCodec::Result SkJpegxlCodec::onStartScanlineDecode(
//     const SkImageInfo& /*dstInfo*/, const Options& /*options*/) { return kUnimplemented; }

// TODO(eustas): implement
// SkCodec::Result SkJpegxlCodec::onStartIncrementalDecode(
//     const SkImageInfo& /*dstInfo*/, void*, size_t, const Options&) { return kUnimplemented; }

// TODO(eustas): implement
// SkCodec::Result SkJpegxlCodec::onIncrementalDecode(int*) { return kUnimplemented; }

// TODO(eustas): implement
// bool SkJpegxlCodec::onSkipScanlines(int /*countLines*/) { return false; }

// TODO(eustas): implement
// int SkJpegxlCodec::onGetScanlines(
//     void* /*dst*/, int /*countLines*/, size_t /*rowBytes*/) { return 0; }

// TODO(eustas): implement
// SkSampler* SkJpegxlCodec::getSampler(bool /*createIfNecessary*/) { return nullptr; }
