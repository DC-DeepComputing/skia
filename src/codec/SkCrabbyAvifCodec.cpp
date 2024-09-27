/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkCrabbyAvifCodec.h"

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkAvifDecoder.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkStreamPriv.h"

#include <cstdint>
#include <cstring>
#include <utility>

#include "avif/avif.h"
#include "avif/libavif_compat.h"

using namespace crabbyavif;

void AvifDecoderDeleter::operator()(avifDecoder* decoder) const {
    if (decoder != nullptr) {
        avifDecoderDestroy(decoder);
    }
}

bool SkCrabbyAvifCodec::IsAvif(const void* buffer, size_t bytesRead) {
    avifROData avifData = {static_cast<const uint8_t*>(buffer), bytesRead};
    bool isAvif = avifPeekCompatibleFileType(&avifData) == AVIF_TRUE;
    if (isAvif) return true;
    // Peeking sometimes fails if the ftyp box is too large. Check the signature
    // just to be sure.
    const char* bytes = static_cast<const char*>(buffer);
    isAvif = bytesRead >= 12 && !memcmp(&bytes[4], "ftyp", 4) &&
             (!memcmp(&bytes[8], "avif", 4) || !memcmp(&bytes[8], "avis", 4));
    return isAvif;
}

std::unique_ptr<SkCodec> SkCrabbyAvifCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                           Result* result) {
    SkASSERT(result);
    if (!stream) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }
    AvifDecoder avifDecoder(avifDecoderCreate());
    if (avifDecoder == nullptr) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }
    avifDecoder->ignoreXMP = AVIF_TRUE;
    avifDecoder->ignoreExif = AVIF_TRUE;
    avifDecoder->allowProgressive = AVIF_FALSE;
    avifDecoder->allowIncremental = AVIF_FALSE;
    avifDecoder->strictFlags = AVIF_STRICT_DISABLED;
    // TODO(vigneshv): Enable threading based on number of CPU cores available.
    avifDecoder->maxThreads = 1;

    // libavif needs a contiguous data buffer.
    sk_sp<SkData> data = nullptr;
    if (stream->getMemoryBase()) {
        // It is safe to make without copy because we'll hold onto the stream.
        data = SkData::MakeWithoutCopy(stream->getMemoryBase(), stream->getLength());
    } else {
        data = SkCopyStreamToData(stream.get());
        // If we are forced to copy the stream to a data, we can go ahead and
        // delete the stream.
        stream.reset(nullptr);
    }

    avifResult res = avifDecoderSetIOMemory(avifDecoder.get(), data->bytes(), data->size());
    if (res != AVIF_RESULT_OK) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    res = avifDecoderParse(avifDecoder.get());
    if (res != AVIF_RESULT_OK) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }

    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = nullptr;
    // TODO(vigneshv): Get ICC Profile from the avif decoder.

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    // TODO(vigneshv): Remove this #define and add a function to CrabbyAvif to
    // get the bits per component based on what it has decoded.
    // CrabbyAvif uses MediaCodec, which always sets bitsPerComponent to 8.
    const int bitsPerComponent = 8;
#else
    const int bitsPerComponent = avifDecoder->image->depth > 8 ? 16 : 8;
#endif  // defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    SkEncodedInfo::Color color;
    SkEncodedInfo::Alpha alpha;
    if (avifDecoder->alphaPresent) {
        color = SkEncodedInfo::kRGBA_Color;
        alpha = SkEncodedInfo::kUnpremul_Alpha;
    } else {
        color = SkEncodedInfo::kRGB_Color;
        alpha = SkEncodedInfo::kOpaque_Alpha;
    }
    SkEncodedInfo info = SkEncodedInfo::Make(avifDecoder->image->width,
                                             avifDecoder->image->height,
                                             color,
                                             alpha,
                                             bitsPerComponent,
                                             std::move(profile),
                                             avifDecoder->image->depth);
    bool animation = avifDecoder->imageCount > 1;
    *result = kSuccess;
    return std::unique_ptr<SkCodec>(new SkCrabbyAvifCodec(std::move(info),
                                                          std::move(stream),
                                                          std::move(data),
                                                          std::move(avifDecoder),
                                                          kDefault_SkEncodedOrigin,
                                                          animation));
}

SkCrabbyAvifCodec::SkCrabbyAvifCodec(SkEncodedInfo&& info,
                                     std::unique_ptr<SkStream> stream,
                                     sk_sp<SkData> data,
                                     AvifDecoder avifDecoder,
                                     SkEncodedOrigin origin,
                                     bool useAnimation)
        : INHERITED(std::move(info), skcms_PixelFormat_RGBA_8888, std::move(stream), origin)
        , fData(std::move(data))
        , fAvifDecoder(std::move(avifDecoder))
        , fUseAnimation(useAnimation) {}

int SkCrabbyAvifCodec::onGetFrameCount() {
    if (!fUseAnimation) {
        return 1;
    }

    if (fFrameHolder.size() == 0) {
        if (fAvifDecoder->imageCount <= 1) {
            fUseAnimation = false;
            return 1;
        }
        fFrameHolder.reserve(fAvifDecoder->imageCount);
        for (int i = 0; i < fAvifDecoder->imageCount; i++) {
            Frame* frame = fFrameHolder.appendNewFrame(fAvifDecoder->alphaPresent == AVIF_TRUE);
            frame->setXYWH(0, 0, fAvifDecoder->image->width, fAvifDecoder->image->height);
            frame->setDisposalMethod(SkCodecAnimation::DisposalMethod::kKeep);
            avifImageTiming timing;
            avifDecoderNthImageTiming(fAvifDecoder.get(), i, &timing);
            frame->setDuration(timing.duration * 1000);
            frame->setRequiredFrame(SkCodec::kNoFrame);
            frame->setHasAlpha(fAvifDecoder->alphaPresent == AVIF_TRUE);
        }
    }

    return fFrameHolder.size();
}

const SkFrame* SkCrabbyAvifCodec::FrameHolder::onGetFrame(int i) const {
    return static_cast<const SkFrame*>(this->frame(i));
}

SkCrabbyAvifCodec::Frame* SkCrabbyAvifCodec::FrameHolder::appendNewFrame(bool hasAlpha) {
    const int i = this->size();
    fFrames.emplace_back(i,
                         hasAlpha ? SkEncodedInfo::kUnpremul_Alpha : SkEncodedInfo::kOpaque_Alpha);
    return &fFrames[i];
}

const SkCrabbyAvifCodec::Frame* SkCrabbyAvifCodec::FrameHolder::frame(int i) const {
    SkASSERT(i >= 0 && i < this->size());
    return &fFrames[i];
}

bool SkCrabbyAvifCodec::onGetFrameInfo(int i, FrameInfo* frameInfo) const {
    if (i >= fFrameHolder.size()) {
        return false;
    }

    const Frame* frame = fFrameHolder.frame(i);
    if (!frame) {
        return false;
    }

    if (frameInfo) {
        frame->fillIn(frameInfo, true);
    }

    return true;
}

int SkCrabbyAvifCodec::onGetRepetitionCount() { return kRepetitionCountInfinite; }

bool SkCrabbyAvifCodec::conversionSupported(const SkImageInfo& dstInfo,
                                            bool srcIsOpaque,
                                            bool needsColorXform) {
    return dstInfo.colorType() == kRGBA_8888_SkColorType ||
           dstInfo.colorType() == kRGBA_1010102_SkColorType ||
           dstInfo.colorType() == kRGBA_F16_SkColorType;
}

SkCodec::Result SkCrabbyAvifCodec::onGetPixels(const SkImageInfo& dstInfo,
                                               void* dst,
                                               size_t dstRowBytes,
                                               const Options& options,
                                               int* rowsDecoded) {
    if (options.fSubset) {
        return kUnimplemented;
    }

    avifResult result = avifDecoderNthImage(fAvifDecoder.get(), options.fFrameIndex);
    if (result != AVIF_RESULT_OK) {
        return kInvalidInput;
    }

    if (this->dimensions() != dstInfo.dimensions()) {
        result = avifImageScale(
                fAvifDecoder->image, dstInfo.width(), dstInfo.height(), &fAvifDecoder->diag);
        if (result != AVIF_RESULT_OK) {
            return kInvalidInput;
        }
    }

    avifRGBImage rgbImage;
    avifRGBImageSetDefaults(&rgbImage, fAvifDecoder->image);

    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            rgbImage.depth = 8;
            break;
        case kRGBA_F16_SkColorType:
            rgbImage.depth = 16;
            rgbImage.isFloat = AVIF_TRUE;
            break;
        case kRGBA_1010102_SkColorType:
            rgbImage.depth = 10;
            rgbImage.format = AVIF_RGB_FORMAT_RGBA1010102;
            break;
        default:
            // TODO(vigneshv): Check if more color types need to be supported.
            // Currently android supports at least RGB565 and BGRA8888 which is
            // not supported here.
            return kUnimplemented;
    }

    rgbImage.pixels = static_cast<uint8_t*>(dst);
    rgbImage.rowBytes = dstRowBytes;
    rgbImage.chromaUpsampling = AVIF_CHROMA_UPSAMPLING_FASTEST;

    result = avifImageYUVToRGB(fAvifDecoder->image, &rgbImage);
    if (result != AVIF_RESULT_OK) {
        return kInvalidInput;
    }

    *rowsDecoded = fAvifDecoder->image->height;
    return kSuccess;
}

namespace SkAvifDecoder {
namespace CrabbyAvif {

bool IsAvif(const void* data, size_t len) { return SkCrabbyAvifCodec::IsAvif(data, len); }

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext) {
    SkCodec::Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }
    return SkCrabbyAvifCodec::MakeFromStream(std::move(stream), outResult);
}

std::unique_ptr<SkCodec> Decode(sk_sp<SkData> data,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext) {
    if (!data) {
        if (outResult) {
            *outResult = SkCodec::kInvalidInput;
        }
        return nullptr;
    }
    return Decode(SkMemoryStream::Make(std::move(data)), outResult, nullptr);
}

}  // namespace CrabbyAvif
}  // namespace SkAvifDecoder
