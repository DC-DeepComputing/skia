/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkPngRustDecoder.h"

#include <functional>
#include <memory>
#include <optional>
#include <utility>

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkCodecAnimation.h"
#include "include/codec/SkPngChunkReader.h"
#if defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG)
#include "include/codec/SkPngDecoder.h"
#endif

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/SkGainmapInfo.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkSampler.h"
#include "tests/FakeStreams.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r,                                           \
                    actualResult == SkCodec::kSuccess,           \
                    "actualResult=\"%s\" != kSuccess",           \
                    SkCodec::ResultToString(actualResult))

namespace {
// This class wraps another SkStream. It does not own the underlying stream, so
// that the underlying stream can be reused starting from where the first
// client left off. This mimics Android's JavaInputStreamAdaptor.
// Replicated from tests/CodecExactReadTest.cpp.
class UnowningStream : public SkStream {
public:
    explicit UnowningStream(SkStream* stream) : fStream(stream) {}

    size_t read(void* buf, size_t bytes) override { return fStream->read(buf, bytes); }

    bool rewind() override { return fStream->rewind(); }

    bool isAtEnd() const override { return fStream->isAtEnd(); }

private:
    SkStream* fStream;  // Unowned.
};

}  // namespace

// Helper wrapping a call to `SkPngRustDecoder::Decode`.
std::unique_ptr<SkCodec> SkPngRustDecoderDecode(skiatest::Reporter* r, const char* path) {
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    return codec;
}

void AssertPixelColor(skiatest::Reporter* r,
                      const SkPixmap& pixmap,
                      int x,
                      int y,
                      SkColor expectedColor,
                      const char* description) {
    SkASSERT(r);
    SkASSERT(x >= 0);
    SkASSERT(y >= 0);
    SkASSERT(description);

    REPORTER_ASSERT(r, x < pixmap.width(), "x=%d >= width=%d", x, pixmap.width());
    REPORTER_ASSERT(r, y < pixmap.height(), "y=%d >= height=%d", y, pixmap.height());
    REPORTER_ASSERT(r,
                    kN32_SkColorType == pixmap.colorType(),
                    "kN32_SkColorType != pixmap.ColorType()=%d",
                    pixmap.colorType());

    SkColor actualColor = pixmap.getColor(x, y);
    REPORTER_ASSERT(r,
                    actualColor == expectedColor,
                    "actualColor=0x%08X != expectedColor==0x%08X at (%d,%d) (%s)",
                    actualColor,
                    expectedColor,
                    x,
                    y,
                    description);
}

void AssertGreenPixel(skiatest::Reporter* r,
                      const SkPixmap& pixmap,
                      int x,
                      int y,
                      const char* description = "Expecting a green pixel") {
    AssertPixelColor(r, pixmap, x, y, SkColorSetRGB(0x00, 0xFF, 0x00), description);
}

void AssertRedPixel(skiatest::Reporter* r,
                    const SkPixmap& pixmap,
                    int x,
                    int y,
                    const char* description = "Expecting a red pixel") {
    AssertPixelColor(r, pixmap, x, y, SkColorSetRGB(0xFF, 0x00, 0x00), description);
}

void AssertBluePixel(skiatest::Reporter* r,
                     const SkPixmap& pixmap,
                     int x,
                     int y,
                     const char* description = "Expecting a blue pixel") {
    AssertPixelColor(r, pixmap, x, y, SkColorSetRGB(0x00, 0x00, 0xFF), description);
}

void AssertSingleGreenFrame(skiatest::Reporter* r,
                            int expectedWidth,
                            int expectedHeight,
                            const char* resourcePath) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, resourcePath);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getFrameCount() == 1);
    REPORTER_ASSERT(r, codec->getRepetitionCount() == 0);

    SkCodec::FrameInfo info;
    REPORTER_ASSERT(r, codec->getFrameInfo(0, &info));
    REPORTER_ASSERT(r, info.fBlend == SkCodecAnimation::Blend::kSrc);
    REPORTER_ASSERT(r, info.fDisposalMethod == SkCodecAnimation::DisposalMethod::kKeep);
    REPORTER_ASSERT(r, info.fFrameRect == SkIRect::MakeWH(expectedWidth, expectedHeight));
    REPORTER_ASSERT(r, info.fRequiredFrame == SkCodec::kNoFrame);

    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r,
                    image->width() == expectedWidth,
                    "actualWidth=%d != expectedWidth=%d",
                    image->width(),
                    expectedWidth);
    REPORTER_ASSERT(r,
                    image->height() == expectedHeight,
                    "actualHeight=%d != expectedHeight=%d",
                    image->height(),
                    expectedHeight);

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));

    AssertGreenPixel(r, pixmap, 0, 0);
    AssertGreenPixel(r, pixmap, expectedWidth / 2, expectedHeight / 2);
}

static bool DecodeOneShot(skiatest::Reporter* r,
                          sk_sp<SkData> data,
                          SkBitmap* dstBitmap,
                          SkColorType colorType = kN32_SkColorType,
                          const SkIRect* subset = nullptr,
                          int frameIndex = 0) {
    auto codec = SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(data), nullptr);
    if (!codec) {
        return false;
    }

    SkISize dims = subset ? subset->size() : codec->getInfo().dimensions();
    SkImageInfo dstInfo = codec->getInfo().makeDimensions(dims).makeColorType(colorType);
    dstBitmap->allocPixels(dstInfo);

    for (int i = 0; i <= frameIndex; ++i) {
        SkCodec::Options options;
        options.fFrameIndex = i;
        if (i == frameIndex) {
            options.fSubset = subset;
        }
        if (i > 0) {
            SkCodec::FrameInfo info;
            if (codec->getFrameInfo(i, &info)) {
                options.fPriorFrame = info.fRequiredFrame;
            }
        }

        SkImageInfo decodeInfo = codec->getInfo().makeColorType(colorType);

        SkCodec::Result res = codec->startIncrementalDecode(
                decodeInfo, dstBitmap->getPixels(), dstBitmap->rowBytes(), &options);
        if (res != SkCodec::kSuccess) {
            ERRORF(r,
                   "startIncrementalDecode failed for frame %d: %s",
                   i,
                   SkCodec::ResultToString(res));
            return false;
        }

        int rowsDecoded = 0;
        res = codec->incrementalDecode(&rowsDecoded);
        if (res != SkCodec::kSuccess) {
            ERRORF(r, "incrementalDecode failed for frame %d: %s", i, SkCodec::ResultToString(res));
            return false;
        }
    }
    return true;
}

static SkImageInfo GetAndroidTargetInfo(SkAndroidCodec* codec,
                                        int sampleSize,
                                        SkColorType colorType,
                                        const SkIRect* subset) {
    SkISize desiredSize = codec->getSampledDimensions(sampleSize);
    if (subset) {
        int subsetWidth = SkCodecPriv::GetSampledDimension(subset->width(), sampleSize);
        int subsetHeight = SkCodecPriv::GetSampledDimension(subset->height(), sampleSize);
        desiredSize = SkISize::Make(subsetWidth, subsetHeight);
    }
    return codec->getInfo().makeDimensions(desiredSize).makeColorType(colorType);
}

static bool DecodeAndroidOneShot(skiatest::Reporter* r,
                                 sk_sp<SkData> data,
                                 SkBitmap* dstBitmap,
                                 int sampleSize,
                                 SkColorType colorType = kN32_SkColorType,
                                 const SkIRect* subset = nullptr) {
    auto codec = SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(data), nullptr);
    if (!codec) {
        ERRORF(r, "Failed to create Rust codec");
        return false;
    }
    std::unique_ptr<SkAndroidCodec> androidCodec = SkAndroidCodec::MakeFromCodec(std::move(codec));
    if (!androidCodec) {
        ERRORF(r, "Failed to create Android codec");
        return false;
    }

    SkImageInfo info = GetAndroidTargetInfo(androidCodec.get(), sampleSize, colorType, subset);
    dstBitmap->allocPixels(info);

    SkAndroidCodec::AndroidOptions androidOptions;
    androidOptions.fSampleSize = sampleSize;
    androidOptions.fSubset = subset;

    auto result = androidCodec->getAndroidPixels(
            dstBitmap->info(), dstBitmap->getPixels(), dstBitmap->rowBytes(), &androidOptions);
    if (result != SkCodec::kSuccess) {
        ERRORF(r, "getAndroidPixels failed: %s", SkCodec::ResultToString(result));
        return false;
    }
    return true;
}

static void CompareBitmaps(skiatest::Reporter* r, const SkBitmap& bm1, const SkBitmap& bm2) {
    const SkImageInfo& info = bm1.info();
    if (info != bm2.info()) {
        ERRORF(r, "Bitmaps have different image infos!");
        return;
    }
    const size_t rowBytes = info.minRowBytes();
    for (int i = 0; i < info.height(); i++) {
        if (0 != memcmp(bm1.getAddr(0, i), bm2.getAddr(0, i), rowBytes)) {
            ERRORF(r, "Bitmaps have different pixels, starting on line %i!", i);
            return;
        }
    }
}

static void VerifyProgressivePixels(skiatest::Reporter* r,
                                    const SkBitmap& bm,
                                    SkColor backgroundColor) {
    bool hasProgressivePixels = false;
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            if (bm.getColor(x, y) != backgroundColor) {
                hasProgressivePixels = true;
                break;
            }
        }
        if (hasProgressivePixels) {
            break;
        }
    }
    REPORTER_ASSERT(r, hasProgressivePixels, "Progressive rendering failed: no pixels written!");
}

static SkCodec::Result DecodeFrameToCompletion(SkCodec* codec,
                                               HaltingStream* stream,
                                               size_t fullLength,
                                               SkBitmap* bitmap,
                                               int frameIndex) {
    SkCodec::Options options;
    options.fFrameIndex = frameIndex;
    SkCodec::FrameInfo info;
    if (codec->getFrameInfo(frameIndex, &info)) {
        options.fPriorFrame = info.fRequiredFrame;
    }
    SkImageInfo dstInfo = codec->getInfo().makeColorType(bitmap->info().colorType());
    SkCodec::Result result = codec->startIncrementalDecode(
            dstInfo, bitmap->getPixels(), bitmap->rowBytes(), &options);
    if (result != SkCodec::kSuccess) {
        return result;
    }
    int rowsDecoded = -1;
    result = codec->incrementalDecode(&rowsDecoded);
    if (result == SkCodec::kIncompleteInput) {
        stream->addNewData(fullLength - stream->getLength());
        result = codec->incrementalDecode(&rowsDecoded);
    }
    return result;
}

static void CompareBitmapsWithPath(skiatest::Reporter* r,
                                   const SkBitmap& bm1,
                                   const SkBitmap& bm2,
                                   const char* path);

struct AndroidDecodeResult {
    SkBitmap fBitmap;
    SkCodec::Result fResult;
    int fRowsDecoded;
};

static AndroidDecodeResult DecodeAndroidPixelsHalting(skiatest::Reporter* r,
                                                      SkAndroidCodec* androidCodec,
                                                      int sampleSize,
                                                      const SkIRect* subset = nullptr) {
    SkAndroidCodec::AndroidOptions opts;
    opts.fSampleSize = sampleSize;
    opts.fSubset = subset;

    SkISize sampledDims = androidCodec->getSampledDimensions(sampleSize);
    SkImageInfo info =
            androidCodec->getInfo().makeDimensions(sampledDims).makeColorType(kN32_SkColorType);
    if (opts.fSubset) {
        int subsetWidth = SkCodecPriv::GetSampledDimension(opts.fSubset->width(), sampleSize);
        int subsetHeight = SkCodecPriv::GetSampledDimension(opts.fSubset->height(), sampleSize);
        info = info.makeWH(subsetWidth, subsetHeight);
    }

    SkBitmap bm;
    bm.allocPixels(info);
    bm.eraseColor(SK_ColorBLUE);

    auto result = androidCodec->getAndroidPixels(info, bm.getPixels(), bm.rowBytes(), &opts);

    // Detect rowsDecoded
    int rowsDecoded = 0;
    const size_t rowBytes = info.minRowBytes();
    std::vector<uint8_t> blueRow(rowBytes);
    SkBitmap blueBm;
    blueBm.allocPixels(SkImageInfo::Make(1, 1, info.colorType(), info.alphaType()));
    blueBm.eraseColor(SK_ColorBLUE);
    for (size_t b = 0; b < rowBytes; b += info.bytesPerPixel()) {
        memcpy(blueRow.data() + b, blueBm.getPixels(), info.bytesPerPixel());
    }
    for (int i = 0; i < info.height(); i++) {
        if (0 != memcmp(bm.getAddr(0, i), blueRow.data(), rowBytes)) {
            rowsDecoded = i + 1;
        }
    }

    return AndroidDecodeResult{bm, result, rowsDecoded};
}

static void AssertDecodeHalting(skiatest::Reporter* r,
                                const char* path,
                                const SkIRect* subset = nullptr,
                                SkColorType colorType = kN32_SkColorType,
                                size_t initialLimit = 0,
                                bool expectPartialHeight = true,
                                int frameIndex = 0) {
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return;
    }
    size_t fullLength = data->size();
    if (initialLimit == 0) {
        initialLimit = fullLength / 2;
    }

    struct HaltingResult {
        SkBitmap fBitmap;
        int fRowsDecodedAtHalt;
        SkCodec::Result fResultAtHalt;
        SkCodec::Result fResultAtFinal;
    };

    auto runDecode = [&](std::unique_ptr<SkCodec> codec,
                         HaltingStream* retainedStream) -> std::optional<HaltingResult> {
        if (!codec) return std::nullopt;

        SkISize dims = subset ? subset->size() : codec->getInfo().dimensions();
        SkImageInfo dstInfo = codec->getInfo().makeDimensions(dims).makeColorType(colorType);

        HaltingResult res;
        res.fBitmap.allocPixels(dstInfo);
        res.fBitmap.eraseColor(SK_ColorBLUE);

        // Decode prior frames to completion
        for (int i = 0; i < frameIndex; ++i) {
            SkCodec::Result resPrior = DecodeFrameToCompletion(
                    codec.get(), retainedStream, fullLength, &res.fBitmap, i);
            if (resPrior != SkCodec::kSuccess) {
                ERRORF(r, "Failed to decode prior frame %d", i);
                return std::nullopt;
            }
        }

        SkCodec::Options options;
        options.fSubset = subset;
        options.fFrameIndex = frameIndex;
        if (frameIndex > 0) {
            SkCodec::FrameInfo frameInfo;
            if (codec->getFrameInfo(frameIndex, &frameInfo)) {
                options.fPriorFrame = frameInfo.fRequiredFrame;
            }
        }

        SkImageInfo decodeInfo = codec->getInfo().makeColorType(colorType);
        SkCodec::Result startRes = codec->startIncrementalDecode(
                decodeInfo, res.fBitmap.getPixels(), res.fBitmap.rowBytes(), &options);
        if (startRes != SkCodec::kSuccess) {
            ERRORF(r, "startIncrementalDecode failed: %s", SkCodec::ResultToString(startRes));
            return std::nullopt;
        }

        res.fRowsDecodedAtHalt = 0;
        res.fResultAtHalt = codec->incrementalDecode(&res.fRowsDecodedAtHalt);

        // Verify progressive rendering (only if we decoded some rows)
        if (res.fRowsDecodedAtHalt > 0) {
            VerifyProgressivePixels(r, res.fBitmap, SK_ColorBLUE);
        }

        // Resume
        retainedStream->addNewData(fullLength - initialLimit);
        int rowsDecoded = 0;
        res.fResultAtFinal = codec->incrementalDecode(&rowsDecoded);

        return res;
    };

    // 1. Rust Decode
    auto streamRust = std::make_unique<HaltingStream>(data, initialLimit);
    HaltingStream* retainedStreamRust = streamRust.get();
    SkCodec::Result resultRust;
    auto codecRust = SkPngRustDecoder::Decode(std::move(streamRust), &resultRust);
    REPORTER_ASSERT(r, codecRust);
    if (!codecRust) return;

    auto rustRes = runDecode(std::move(codecRust), retainedStreamRust);
    REPORTER_ASSERT(r, rustRes.has_value());
    if (!rustRes) return;

    REPORTER_ASSERT(r, rustRes->fResultAtHalt == SkCodec::kIncompleteInput);
    REPORTER_ASSERT(r, rustRes->fResultAtFinal == SkCodec::kSuccess);

    if (expectPartialHeight) {
        REPORTER_ASSERT(r, rustRes->fRowsDecodedAtHalt < rustRes->fBitmap.height());
    } else {
        REPORTER_ASSERT(r, rustRes->fRowsDecodedAtHalt == rustRes->fBitmap.height());
    }

#if defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG)
    // 2. Libpng Decode
    bool compareWithLibpng = (frameIndex == 0);
    if (compareWithLibpng) {
        auto streamLibpng = std::make_unique<HaltingStream>(data, initialLimit);
        HaltingStream* retainedStreamLibpng = streamLibpng.get();
        SkCodec::Result resultLibpng;
        auto codecLibpng = SkPngDecoder::Decode(std::move(streamLibpng), &resultLibpng);
        REPORTER_ASSERT(r, codecLibpng);
        if (codecLibpng) {
            auto libpngRes = runDecode(std::move(codecLibpng), retainedStreamLibpng);
            REPORTER_ASSERT(r, libpngRes.has_value());
            if (libpngRes) {
                REPORTER_ASSERT(r, libpngRes->fResultAtHalt == SkCodec::kIncompleteInput);
                REPORTER_ASSERT(r, libpngRes->fResultAtFinal == SkCodec::kSuccess);

                // Compare at halt
                REPORTER_ASSERT(r,
                                rustRes->fRowsDecodedAtHalt == libpngRes->fRowsDecodedAtHalt,
                                "%s: rowsDecoded mismatch at halt: Rust %d != Libpng %d",
                                path,
                                rustRes->fRowsDecodedAtHalt,
                                libpngRes->fRowsDecodedAtHalt);

                // Compare final
                CompareBitmapsWithPath(r, libpngRes->fBitmap, rustRes->fBitmap, path);
            }
        }
    }
#endif

    // 3. Compare with one-shot
    SkBitmap bmReference;
    REPORTER_ASSERT(r, DecodeOneShot(r, data, &bmReference, colorType, subset, frameIndex));
    CompareBitmapsWithPath(r, bmReference, rustRes->fBitmap, path);
}

static void AssertAndroidDecodeHalting(
        skiatest::Reporter* r,
        const char* path,
        int sampleSize,
        std::function<SkIRect(const SkImageInfo&)> getSubset = nullptr,
        size_t initialLimit = 0) {
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return;
    }
    size_t fullLength = data->size();
    if (initialLimit == 0) {
        initialLimit = fullLength / 2;
    }

    struct AndroidHaltingResult {
        AndroidDecodeResult fHalt;
        AndroidDecodeResult fFinal;
    };

    auto runDecode = [&](std::unique_ptr<SkCodec> codec,
                         HaltingStream* retainedStream) -> std::optional<AndroidHaltingResult> {
        if (!codec) return std::nullopt;
        auto androidCodec = SkAndroidCodec::MakeFromCodec(std::move(codec));
        if (!androidCodec) return std::nullopt;

        SkIRect subset;
        const SkIRect* subsetPtr = nullptr;
        if (getSubset) {
            subset = getSubset(androidCodec->getInfo());
            subsetPtr = &subset;
        }

        AndroidHaltingResult res;
        res.fHalt = DecodeAndroidPixelsHalting(r, androidCodec.get(), sampleSize, subsetPtr);

        // Verify progressive rendering (only if we decoded some rows)
        if (res.fHalt.fRowsDecoded > 0) {
            VerifyProgressivePixels(r, res.fHalt.fBitmap, SK_ColorBLUE);
        }

        // Resume
        retainedStream->addNewData(fullLength - initialLimit);
        res.fFinal = DecodeAndroidPixelsHalting(r, androidCodec.get(), sampleSize, subsetPtr);

        return res;
    };

    // 1. Rust Decode
    auto streamRust = std::make_unique<HaltingStream>(data, initialLimit);
    HaltingStream* retainedStreamRust = streamRust.get();
    auto codecRust = SkPngRustDecoder::Decode(std::move(streamRust), nullptr);
    REPORTER_ASSERT(r, codecRust);
    if (!codecRust) return;

    auto rustRes = runDecode(std::move(codecRust), retainedStreamRust);
    REPORTER_ASSERT(r, rustRes.has_value());
    if (!rustRes) return;

    REPORTER_ASSERT(r, rustRes->fHalt.fResult == SkCodec::kIncompleteInput);
    REPORTER_ASSERT(r, rustRes->fFinal.fResult == SkCodec::kSuccess);

#if defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG)
    // 2. Libpng Decode
    auto streamLibpng = std::make_unique<HaltingStream>(data, initialLimit);
    HaltingStream* retainedStreamLibpng = streamLibpng.get();
    auto codecLibpng = SkPngDecoder::Decode(std::move(streamLibpng), nullptr);
    REPORTER_ASSERT(r, codecLibpng);
    if (codecLibpng) {
        auto libpngRes = runDecode(std::move(codecLibpng), retainedStreamLibpng);
        REPORTER_ASSERT(r, libpngRes.has_value());
        if (libpngRes) {
            REPORTER_ASSERT(r, libpngRes->fHalt.fResult == SkCodec::kIncompleteInput);
            REPORTER_ASSERT(r, libpngRes->fFinal.fResult == SkCodec::kSuccess);

            // Compare at halt
            REPORTER_ASSERT(r,
                            rustRes->fHalt.fRowsDecoded == libpngRes->fHalt.fRowsDecoded,
                            "%s: rowsDecoded mismatch at halt: Rust %d != Libpng %d",
                            path,
                            rustRes->fHalt.fRowsDecoded,
                            libpngRes->fHalt.fRowsDecoded);

            // Compare final
            CompareBitmapsWithPath(r, libpngRes->fFinal.fBitmap, rustRes->fFinal.fBitmap, path);
        }
    }
#endif

    // 3. Compare with one-shot
    SkBitmap bmReference;
    SkIRect subset;
    const SkIRect* subsetPtr = nullptr;
    if (getSubset) {
        auto tempCodec = SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(data), nullptr);
        if (tempCodec) {
            auto tempAndroid = SkAndroidCodec::MakeFromCodec(std::move(tempCodec));
            if (tempAndroid) {
                subset = getSubset(tempAndroid->getInfo());
                subsetPtr = &subset;
            }
        }
    }
    REPORTER_ASSERT(
            r,
            DecodeAndroidOneShot(r, data, &bmReference, sampleSize, kN32_SkColorType, subsetPtr));
    CompareBitmapsWithPath(r, bmReference, rustRes->fFinal.fBitmap, path);
}

static std::optional<SkBitmap> DecodeAndroidPixels(
        skiatest::Reporter* r,
        std::unique_ptr<SkCodec> codec,
        int sampleSize,
        std::function<SkIRect(const SkImageInfo&)> getSubset = nullptr) {
    if (!codec) {
        return std::nullopt;
    }

    auto androidCodec = SkAndroidCodec::MakeFromCodec(std::move(codec));
    REPORTER_ASSERT(r, androidCodec);
    if (!androidCodec) {
        return std::nullopt;
    }

    SkIRect subset;
    const SkIRect* subsetPtr = nullptr;
    if (getSubset) {
        subset = getSubset(androidCodec->getInfo());
        subsetPtr = &subset;
    }

    auto res = DecodeAndroidPixelsHalting(r, androidCodec.get(), sampleSize, subsetPtr);
    REPORTER_ASSERT(r, res.fResult == SkCodec::kSuccess);
    if (res.fResult != SkCodec::kSuccess) {
        return std::nullopt;
    }
    return res.fBitmap;
}

static void AssertAndroidDecodeSampling(
        skiatest::Reporter* r,
        const char* path,
        int sampleSize,
        std::function<SkIRect(const SkImageInfo&)> getSubset = nullptr) {
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return;
    }

    std::optional<SkBitmap> rustBm = DecodeAndroidPixels(
            r,
            SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(data), nullptr),
            sampleSize,
            getSubset);

#if defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG)
    std::optional<SkBitmap> libpngBm = DecodeAndroidPixels(
            r,
            SkPngDecoder::Decode(std::make_unique<SkMemoryStream>(data), nullptr),
            sampleSize,
            getSubset);

    if (!rustBm || !libpngBm) {
        return;
    }

    CompareBitmaps(r, *rustBm, *libpngBm);
#else
    REPORTER_ASSERT(r, rustBm.has_value());
#endif
}

sk_sp<SkImage> DecodeLastFrame(skiatest::Reporter* r, SkCodec* codec) {
    int frameCount = codec->getFrameCount();
    sk_sp<SkImage> image;
    SkCodec::Result result = SkCodec::kSuccess;
    for (int i = 0; i < frameCount; i++) {
        SkCodec::FrameInfo info;
        REPORTER_ASSERT(r, codec->getFrameInfo(i, &info));

        // This test method only supports `kKeep` disposal method.
        SkASSERT(info.fDisposalMethod == SkCodecAnimation::DisposalMethod::kKeep);

        if (!image) {
            std::tie(image, result) = codec->getImage();
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
            if (result != SkCodec::kSuccess) {
                return nullptr;
            }
        } else {
            SkPixmap pixmap;
            REPORTER_ASSERT(r, image->peekPixels(&pixmap));

            SkCodec::Options options;
            options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
            options.fSubset = nullptr;
            options.fFrameIndex = i;
            options.fPriorFrame = i - 1;

            result = codec->getPixels(pixmap, &options);
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
            if (result != SkCodec::kSuccess) {
                return nullptr;
            }
        }
    }

    return image;
}

sk_sp<SkImage> DecodeLastFrame(skiatest::Reporter* r, const char* resourcePath) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, resourcePath);
    if (!codec) {
        return nullptr;
    }

    return DecodeLastFrame(r, codec.get());
}

void AssertAnimationRepetitionCount(skiatest::Reporter* r,
                                    int expectedRepetitionCount,
                                    const char* resourcePath) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, resourcePath);
    if (!codec) {
        return;
    }

    int actualRepetitionCount = codec->getRepetitionCount();
    REPORTER_ASSERT(r,
                    actualRepetitionCount == expectedRepetitionCount,
                    "actualRepetitionCount=%d != expectedRepetitionCount=%d",
                    actualRepetitionCount,
                    expectedRepetitionCount);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#trivial-static-image
DEF_TEST(RustPngCodec_apng_basic_trivial_static_image, r) {
    AssertSingleGreenFrame(r, 128, 64, "images/apng-test-suite--basic--trivial-static-image.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#trivial-animated-image-one-frame-using-default-image
DEF_TEST(RustPngCodec_apng_basic_using_default_image, r) {
    AssertSingleGreenFrame(r, 128, 64, "images/apng-test-suite--basic--using-default-image.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#trivial-animated-image-one-frame-ignoring-default-image
//
// The input file contains the following PNG chunks: IHDR, acTL, IDAT, fcTL,
// fdAT, IEND.  Presence of acTL chunk + no fcTL chunk before IDAT means that
// the IDAT chunk is *not* part of the animation (i.e. APNG-aware decoders
// should ignore IDAT frame and start with fdAT frame).  This test will fail
// with non-APNG-aware decoders (e.g. with `SkPngCodec`), because the `IDAT`
// chunk represents a red image (the `fdAT` chunk represents a green image).
DEF_TEST(RustPngCodec_apng_basic_ignoring_default_image, r) {
    AssertSingleGreenFrame(r, 128, 64, "images/apng-test-suite--basic--ignoring-default-image.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-dispose-op-none-basic
//
// This test covers two aspects of `SkPngRustCodec` implementation:
//
// * Blink expects that `onGetFrameCount` returns the total frame count when
//   the complete image resource is available (i.e. a lower frame count should
//   only happen upon `SkCodec::kIncompleteInput`).  Before http://review.skia.org/911038
//   `SkPngRustCodec::onGetFrameCount` would not discover additional frames if
//   previous frames haven't been decoded yet.
// * Skia client (e.g. Blink; or here the testcase) is expected to handle
//   `SkCodecAnimation::DisposalMethod` and populate the target buffer (and
//   `SkCodec::Options::fPriorFrame`) with the expected pixels.  OTOH,
//   `SkPngRustCodec` needs to handle `SkCodecAnimation::Blend` - without this
//   the final frame in this test will contain red pixels.
DEF_TEST(RustPngCodec_apng_dispose_op_none_basic, r) {
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoderDecode(r, "images/apng-test-suite--dispose-ops--none-basic.png");
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getFrameCount() == 3);
    REPORTER_ASSERT(r, codec->getRepetitionCount() == 0);

    // We should have `FrameInfo` for all 3 frames.
    SkCodec::FrameInfo info[3];
    REPORTER_ASSERT(r, codec->getFrameInfo(0, &info[0]));
    REPORTER_ASSERT(r, codec->getFrameInfo(1, &info[1]));
    REPORTER_ASSERT(r, codec->getFrameInfo(2, &info[2]));

    // The codec should realize that the `SkStream` contains all the data of the
    // first 2 frames.  Currently `SkPngRustCodec::onGetFrameCount` stops after
    // parsing the final, 3rd `fcTL` chunk and therefore it can't tell if the
    // subsequent `fdAT` chunk has been fully received or not.
    REPORTER_ASSERT(r, info[0].fFullyReceived);
    REPORTER_ASSERT(r, info[1].fFullyReceived);
    REPORTER_ASSERT(r, !info[2].fFullyReceived);

    // Spot-check frame metadata.
    REPORTER_ASSERT(r, info[1].fAlphaType == kUnpremul_SkAlphaType);
    REPORTER_ASSERT(r, info[1].fBlend == SkCodecAnimation::Blend::kSrcOver);
    REPORTER_ASSERT(r, info[1].fDisposalMethod == SkCodecAnimation::DisposalMethod::kKeep);
    REPORTER_ASSERT(r, info[1].fDuration == 100, "dur = %d", info[1].fDuration);
    REPORTER_ASSERT(r, info[1].fFrameRect == SkIRect::MakeWH(128, 64));
    REPORTER_ASSERT(r, info[1].fHasAlphaWithinBounds);
    REPORTER_ASSERT(r, info[1].fRequiredFrame == 0);

    // Validate contents of the first frame.
    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 128, "width %d != 128", image->width());
    REPORTER_ASSERT(r, image->height() == 64, "height %d != 64", image->height());
    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertRedPixel(r, pixmap, 0, 0, "Frame #0 should be red");

    // Validate contents of the second frame.
    SkCodec::Options options;
    options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
    options.fSubset = nullptr;
    options.fFrameIndex = 1;  // We want to decode the second frame.
    options.fPriorFrame = 0;  // `pixmap` contains the first frame before `getPixels` call.
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(pixmap, &options));
    AssertGreenPixel(r, pixmap, 0, 0, "Frame #1 should be green");

    // Validate contents of the third frame.
    options.fFrameIndex = 2;  // We want to decode the second frame.
    options.fPriorFrame = 1;  // `pixmap` contains the second frame before `getPixels` call.
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(pixmap, &options));
    AssertGreenPixel(r, pixmap, 0, 0, "Frame #2 should be green");
}

// This test covers an incomplete input scenario:
//
// * Only half of 1st frame is available during `onGetFrameCount`.
//   In this situation `onGetFrameCount` may consume the whole input in a
//   (futile in this case) attempt to discover `fcTL` chunks for 2nd and 3rd
//   frame.  This will mean that the input stream is in the middle of the 1st
//   frame - no longer positioned correctly for decoding the 1st frame.
// * Full input is available when subsequently decoding 1st frame.
DEF_TEST(RustPngCodec_apng_dispose_op_none_basic_incomplete_input1, r) {
    const char* path = "images/apng-test-suite--dispose-ops--none-basic.png";
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return;
    }
    size_t fullLength = data->size();

    // Initially expose roughly middle of `IDAT` chunk (in this image `fcTL` is
    // present before the `IDAT` chunk and therefore the `IDAT` chunk is part of
    // the animated image).
    constexpr size_t kInitialBytes = 0xAD;
    auto streamForCodec = std::make_unique<HaltingStream>(std::move(data), kInitialBytes);
    HaltingStream* retainedStream = streamForCodec.get();

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkPngRustDecoder::Decode(std::move(streamForCodec), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    if (!codec) {
        return;
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocN32Pixels(codec->dimensions().width(), codec->dimensions().height())) {
        ERRORF(r, "Failed to allocate SkBitmap");
        return;
    }

    // Try to provoke the codec to consume the currently-available part of the
    // input stream.
    //
    // At this point only the metadata for the first frame is available.
    int frameCount = codec->getFrameCount();
    REPORTER_ASSERT(r, frameCount == 1);

    // Make the rest of the input available to the codec.
    retainedStream->addNewData(fullLength);

    // Try to decode the first frame and check its contents.
    sk_sp<SkImage> image;
    std::tie(image, result) = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 128, "width %d != 128", image->width());
    REPORTER_ASSERT(r, image->height() == 64, "height %d != 64", image->height());
    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertRedPixel(r, pixmap, 0, 0, "Frame #0 should be red");
}

// This test covers an incomplete input scenario:
//
// * Only half of 1st frame is available during the initial `incrementalDecode`.
// * Before retrying, `getFrameCount` is called.  This should *not* reposition
//   the stream while in the middle of an active incremental decode.
// * Then we retry `incrementalDecode`.
DEF_TEST(RustPngCodec_apng_dispose_op_none_basic_incomplete_input2, r) {
    const char* path = "images/apng-test-suite--dispose-ops--none-basic.png";
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return;
    }
    size_t fullLength = data->size();

    constexpr size_t kInitialBytes = 0x8D;  // Roughly middle of IDAT chunk.
    auto streamForCodec = std::make_unique<HaltingStream>(std::move(data), kInitialBytes);
    HaltingStream* retainedStream = streamForCodec.get();

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkPngRustDecoder::Decode(std::move(streamForCodec), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    if (!codec) {
        return;
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocN32Pixels(codec->dimensions().width(), codec->dimensions().height())) {
        ERRORF(r, "Failed to allocate SkBitmap");
        return;
    }
    const SkPixmap& pixmap = bitmap.pixmap();

    // Fill the `bitmap` with blue pixels to detect which pixels have been filled
    // by the codec during a partially-successful `incrementalDecode`.
    //
    // (The first frame has all red pixels.)
    bitmap.erase(SkColorSetRGB(0, 0, 0xFF), bitmap.bounds());
    AssertBluePixel(r, pixmap, 0, 0);
    AssertBluePixel(r, pixmap, 40, 29);
    AssertBluePixel(r, pixmap, 80, 35);
    AssertBluePixel(r, pixmap, 127, 63);

    // Decode partially-available, incomplete image.
    SkCodec::Options options;
    options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
    options.fSubset = nullptr;
    options.fFrameIndex = 0;
    options.fPriorFrame = SkCodec::kNoFrame;
    result = codec->startIncrementalDecode(bitmap.pixmap().info(),
                                           bitmap.pixmap().writable_addr(),
                                           bitmap.pixmap().rowBytes(),
                                           &options);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
    int rowsDecoded = -1;
    result = codec->incrementalDecode(&rowsDecoded);
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);
    REPORTER_ASSERT(r, rowsDecoded == 10, "actual rowsDecoded = %d", rowsDecoded);
    AssertRedPixel(r, pixmap, 0, 0);
    AssertBluePixel(r, pixmap, 40, 29);
    AssertBluePixel(r, pixmap, 80, 35);
    AssertBluePixel(r, pixmap, 127, 63);

    // Make the rest of the input available to the codec.
    retainedStream->addNewData(fullLength);

    // Try to provoke the codec to consume further into the input stream (doing
    // this would loose the position inside the currently active incremental
    // decode).
    //
    // At this point metadata of all the frames is available, but the codec
    // shouldn't read the other two `fcTL` chunks during an active incremental
    // decode.
    int frameCount = codec->getFrameCount();
    REPORTER_ASSERT(r, frameCount == 1);

    // Check that all the pixels of the first frame got decoded.
    rowsDecoded = -1;
    result = codec->incrementalDecode(&rowsDecoded);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
    REPORTER_ASSERT(r, rowsDecoded == -1);  // Not set when `kSuccess`.
    AssertRedPixel(r, pixmap, 0, 0);
    AssertRedPixel(r, pixmap, 40, 29);
    AssertRedPixel(r, pixmap, 80, 35);
    AssertRedPixel(r, pixmap, 127, 63);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-source-on-solid-colour
DEF_TEST(RustPngCodec_apng_blend_ops_source_on_solid, r) {
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoderDecode(r, "images/apng-test-suite--blend-ops--source-on-solid.png");
    if (!codec) {
        return;
    }
    sk_sp<SkImage> image = DecodeLastFrame(r, codec.get());
    if (!image) {
        return;
    }

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertGreenPixel(r, pixmap, 0, 0);

    SkCodec::FrameInfo info;
    REPORTER_ASSERT(r, codec->getFrameInfo(1, &info));
    REPORTER_ASSERT(r, info.fBlend == SkCodecAnimation::Blend::kSrc);
    REPORTER_ASSERT(r, info.fRequiredFrame == SkCodec::kNoFrame);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-source-on-nearly-transparent-colour
DEF_TEST(RustPngCodec_apng_blend_ops_source_on_nearly_transparent, r) {
    sk_sp<SkImage> image = DecodeLastFrame(
            r, "images/apng-test-suite--blend-ops--source-on-nearly-transparent.png");
    if (!image) {
        return;
    }

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertPixelColor(r,
                     pixmap,
                     0,
                     0,
                     SkColorSetARGB(0x02, 0x00, 0xFF, 0x00),
                     "Expecting a nearly transparent pixel");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-over-on-solid-and-transparent-colours
DEF_TEST(RustPngCodec_apng_blend_ops_over_on_solid_and_transparent, r) {
    sk_sp<SkImage> image = DecodeLastFrame(
            r, "images/apng-test-suite--blend-ops--over-on-solid-and-transparent.png");
    if (!image) {
        return;
    }

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertGreenPixel(r, pixmap, 0, 0);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-over-repeatedly-with-nearly-transparent-colours
DEF_TEST(RustPngCodec_apng_blend_ops_over_repeatedly, r) {
    sk_sp<SkImage> image =
            DecodeLastFrame(r, "images/apng-test-suite--blend-ops--over-repeatedly.png");
    if (!image) {
        return;
    }

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertGreenPixel(r, pixmap, 0, 0);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-dispose-op-none-in-region
DEF_TEST(RustPngCodec_apng_regions_dispose_op_none, r) {
    sk_sp<SkImage> image =
            DecodeLastFrame(r, "images/apng-test-suite--regions--dispose-op-none.png");
    if (!image) {
        return;
    }

    // Check all pixels.
    //
    // * The image (and the first frame) is 128x64
    // * The 2nd frame is 64x32 at offset (32,16)
    // * The 3rd frame is 1x1 at offset (0,0)
    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    for (int y = 0; y < pixmap.height(); y++) {
        for (int x = 0; x < pixmap.width(); x++) {
            AssertGreenPixel(r, pixmap, x, y);
        }
    }
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-plays-0
DEF_TEST(RustPngCodec_apng_num_plays_0, r) {
    AssertAnimationRepetitionCount(
            r, SkCodec::kRepetitionCountInfinite, "images/apng-test-suite--num-plays--0.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-plays-1
DEF_TEST(RustPngCodec_apng_num_plays_1, r) {
    AssertAnimationRepetitionCount(r, 0, "images/apng-test-suite--num-plays--1.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-plays-2
DEF_TEST(RustPngCodec_apng_num_plays_2, r) {
    AssertAnimationRepetitionCount(r, 1, "images/apng-test-suite--num-plays--2.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-frames-outside-valid-range
//
// In this test the `acTL` chunk sets `num_frames` to `2147483649u` (or `0x80000001u`):
//
// * AFAICT version 1.0 of the APNG spec only says that "0 is not a valid value"
// * The test suite webpage says that at one point the APNG spec said that
//   `num_frames` shall be "limited to the range 0 to (2^31)-1"
DEF_TEST(RustPngCodec_apng_invalid_num_frames_outside_valid_range, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(
            r, "images/apng-test-suite--invalid--num-frames-outside-valid-range.png");
    if (!codec) {
        return;
    }

    // Calling `codec->getFrameCount` exercises the code used to discover and
    // parse `fcTL` chunks.  With the initial implementation of
    // `SkPngRustCodec::getRawFrameCount` the call below would have failed
    // `SkASSERT(fFrameAtCurrentStreamPosition < this->getRawFrameCount())`
    // in `SkPngRustCodec::readToStartOfNextFrame`.
    //
    // Note that `SkPngRustCodec::onGetFrameCount` expectedly returns the number
    // of successfully parsed `fcTL` chunks (1 chunk in the test input) rather
    // than returning the raw `acTL.num_frames`.
    REPORTER_ASSERT(r, codec->getFrameCount() == 1);
}

DEF_TEST(RustPngCodec_png_swizzling_target_unimplemented, r) {
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoderDecode(r, "images/apng-test-suite--basic--ignoring-default-image.png");
    if (!codec) {
        return;
    }
    REPORTER_ASSERT(r, codec->getFrameCount() == 1);

    // Ask to decode into an esoteric `SkColorType`:
    //
    // * Unsupported by `SkSwizzler`.
    // * Supported by `SkCodec::conversionSupported`.
    SkImageInfo dstInfo = SkImageInfo::Make(
            codec->dimensions(), kBGRA_10101010_XR_SkColorType, kPremul_SkAlphaType);

    auto [image, result] = codec->getImage(dstInfo);
    REPORTER_ASSERT(r, result == SkCodec::kUnimplemented);
    REPORTER_ASSERT(r, !image);
}

DEF_TEST(RustPngCodec_png_was_encoded_with_16_bits_or_more_per_component, r) {
    struct Test {
        const char* fFilename;
        bool fEncodedWith16bits;
    };
    const std::array<Test, 4> kTests = {
            Test{"images/pngsuite/basn0g04.png", false},  // 4 bit (16 level) grayscale
            Test{"images/pngsuite/basn2c08.png", false},  // 3x8 bits rgb color
            Test{"images/pngsuite/basn2c16.png", true},   // 3x16 bits rgb color
            Test{"images/pngsuite/basn3p01.png", false}   // 1 bit (2 color) paletted
    };
    for (const auto& test : kTests) {
        std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, test.fFilename);
        if (codec) {
            REPORTER_ASSERT(r, codec->hasHighBitDepthEncodedData() == test.fEncodedWith16bits);
        }
    }
}

DEF_TEST(RustPngCodec_png_cicp, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, "images/cicp_pq.png");
    if (!codec) {
        return;
    }

    const skcms_ICCProfile* profile = codec->getICCProfile();
    REPORTER_ASSERT(r, profile);
    if (!profile) {
        return;
    }
    auto cs = SkColorSpace::Make(*profile);
    skcms_TransferFunction tf;
    cs->transferFn(&tf);

    REPORTER_ASSERT(r, skcms_TransferFunction_isPQish(&tf) ||
                       skcms_TransferFunction_isPQ(&tf));
}

DEF_TEST(RustPngCodec_green15x15, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, "images/green15x15.png");
    if (!codec) {
        return;
    }

    SkImageInfo dstInfo = codec->getInfo();
    dstInfo = dstInfo.makeColorSpace(SkColorSpace::MakeSRGB());
    auto [image, result] = codec->getImage(dstInfo);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    if (result != SkCodec::kSuccess) {
        return;
    }

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    const SkColor kExpectedColor = SkColorSetARGB(0xFF, 0x00, 0x80, 0x00);
    AssertPixelColor(r, pixmap, 0, 0, kExpectedColor, "Expecting a dark green pixel");
}

DEF_TEST(RustPngCodec_exif_orientation, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, "images/F-exif-chunk-early.png");
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getOrigin() == kRightTop_SkEncodedOrigin);
}

DEF_TEST(RustPngCodec_f16_trc_tables, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, "images/f16-trc-tables.png");
    REPORTER_ASSERT(r, codec);

    const SkImageInfo info = codec->getInfo();
    REPORTER_ASSERT(r, info.colorSpace());

    // Decoding to F16 without color space conversion.
    const SkImageInfo dstInfo = info.makeColorType(kRGBA_F16_SkColorType)
                                    .makeColorSpace(nullptr);
    // This should not crash.
    auto [image, result] = codec->getImage(dstInfo);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
}

DEF_TEST(RustPngCodec_crbug445556737, r) {
    sk_sp<SkImage> image = DecodeLastFrame(r, "images/crbug445556737.png");
    if (!image) {
        return;
    }

    // The main test verification is that there are no assertion failures nor
    // other crashes.  Cursory verification below is supplementary/secondary.
    REPORTER_ASSERT(r, image->height() == 5);
    REPORTER_ASSERT(r, image->width() == 5);
}

DEF_TEST(RustPngCodec_invalid_profile, r) {
    // This image has an gamma value of 0. For parity with Blink, we want to disregard
    // the ICC profile in this case and create the codec without it. This is different
    // than libpng SkPngCodec behavior, which will default to an SRGB icc profile.
    std::unique_ptr<SkCodec> codec =
        SkPngRustDecoderDecode(r, "images/png-zero-gamma-color-profile.png");
    REPORTER_ASSERT(r, codec);

    // There should be no ICC profile.
    REPORTER_ASSERT(r, !codec->getICCProfile());

    // This should not crash.
    auto [image, result] = codec->getImage(codec->getInfo());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
}

static bool bitmaps_equal(const SkBitmap& actual, const SkBitmap& expected) {
    for (int y = 0; y < actual.height(); ++y) {
        for (int x = 0; x < actual.width(); ++x) {
            SkColor c1 = actual.getColor(x, y);
            SkColor c2 = expected.getColor(x, y);
            SkPMColor actualPMColor = SkPreMultiplyColor(c1);
            SkPMColor expectedPMColor = SkPreMultiplyColor(c2);

            if (actualPMColor != expectedPMColor) {
                return false;
            }
        }
    }
    return true;
}

static void test_subset_decode(skiatest::Reporter* r, const char* resource) {
    skiatest::ReporterContext context(r, resource);
    std::unique_ptr<SkStream> stream(GetResourceAsStream(resource));
    REPORTER_ASSERT(r, stream);

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, codec);

    const SkImageInfo info = codec->getInfo();

    SkBitmap bm;
    bm.allocPixels(info);
    codec->getPixels(info, bm.getPixels(), bm.rowBytes());

    SkBitmap tiledBM;
    tiledBM.allocPixels(info);

    const int height = info.height();
    const int width = info.width();
    // Note that if numStripes does not evenly divide height there will be an extra
    // stripe.
    const int numStripes = 4;
    const int numVerticalStripes = 2;

    if (numStripes > height || numVerticalStripes > width) {
        // Image is too small.
        return;
    }

    const int stripeHeight = height / numStripes;
    const int stripeWidth = width / numVerticalStripes;

    // Iterate through the image twice. Once to decode odd stripes, and once for even.
    for (int oddEven = 1; oddEven >= 0; oddEven--) {
        for (int y = oddEven * stripeHeight; y < height; y += 2 * stripeHeight) {
            for (int xStripe = 0; xStripe < numVerticalStripes; xStripe++) {
                // Calculate all four bounds for the grid section
                const int top = y;
                const int bottom = std::min(y + stripeHeight, height);
                const int left = xStripe * stripeWidth;
                const int right = std::min((xStripe + 1) * stripeWidth, width);

                SkIRect subset = SkIRect::MakeLTRB(left, top, right, bottom);
                SkCodec::Options options;
                options.fSubset = &subset;

                // Decode each subset tile into a tightly allocated temporary bitmap
                // (sized exactly to the subset). This is a valid use case that
                // replicates how clients (like Android) perform subset decodes.
                // Doing so explicitly exercises the tight-allocation path where the
                // stride is exactly the subset row size (lacking full-image stride
                // padding), ensuring fDstRowBytes is calculated correctly.
                SkBitmap subsetBM;
                subsetBM.allocPixels(info.makeDimensions(subset.size()));

                REPORTER_ASSERT(
                        r,
                        SkCodec::kSuccess ==
                                codec->startIncrementalDecode(
                                        info, subsetBM.getPixels(), subsetBM.rowBytes(), &options));
                REPORTER_ASSERT(r, SkCodec::kSuccess == codec->incrementalDecode());

                // Copy the decoded subset into the full tiled bitmap
                REPORTER_ASSERT(r, tiledBM.writePixels(subsetBM.pixmap(), left, top));
            }
        }
    }

    REPORTER_ASSERT(r, bitmaps_equal(bm, tiledBM));
}

DEF_TEST(RustPngCodec_subset, r) {
    // Tests subsets by splitting the image into 8 or 10 different tiles and decoding
    // those each separately, then comparing to the full image decoded.
    test_subset_decode(r, "images/baby_tux.png");
    test_subset_decode(r, "images/plane_interlaced.png");
    test_subset_decode(r, "images/basi3p01.png");
}

DEF_TEST(RustPngCodec_interlaced_animated_blending, r) {
    std::unique_ptr<SkCodec> codec =
        SkPngRustDecoderDecode(r, "images/interlaced-multiframe-with-blending.png");
    REPORTER_ASSERT(r, codec);

    // Use incrementalDecode for each frame of this image. This should not crash.
    SkBitmap bm;
    SkImageInfo info = codec->getInfo();
    bm.allocPixels(info);
    REPORTER_ASSERT(r, codec->getFrameCount() == 4);
    for (int i = 0; i < codec->getFrameCount(); ++i) {
        SkCodec::Options options;
        options.fFrameIndex = i;
        options.fPriorFrame = i - 1;
        SkCodec::Result result;
        result = codec->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes(), &options);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        // We use std::ignore because incrementalDecode is expected to fail for
        // interlaced APNG frames > 0 due to an upstream bug in the rust-png crate.
        // See https://github.com/image-rs/image-png/issues/510.
        // This test only verifies that it does not crash.
        std::ignore = codec->incrementalDecode();
    }
}

DEF_TEST(RustPngCodec_sbit565_ihdr16bits, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, "images/basn2c16-sbit565.png");
    REPORTER_ASSERT(r, codec);

    SkBitmap bm;
    SkImageInfo info = codec->getInfo();
    bm.allocPixels(info);
    REPORTER_ASSERT(r, codec->getFrameCount() == 1);
    SkCodec::Result result;
    result = codec->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    result = codec->incrementalDecode();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
}

#ifdef SK_CODEC_USES_PNG_WITH_RUST_FOR_ANDROID
class MockChunkReader : public SkPngChunkReader {
public:
    bool readChunk(const char tag[], const void* data, size_t length) override {
        fChunks.push_back({tag, std::string((const char*)data, length)});
        return true;
    }

    std::vector<std::pair<std::string, std::string>> fChunks;
};

DEF_TEST(RustPngCodec_gainmapDecode, r) {
    auto stream = GetResourceAsStream("images/gainmap.png", false);
    REPORTER_ASSERT(r, stream);

    SkCodec::Result result = SkCodec::kSuccess;
    std::unique_ptr<SkCodec> baseCodec = SkPngRustDecoder::Decode(std::move(stream), &result);
    REPORTER_ASSERT(r, baseCodec);

    std::unique_ptr<SkAndroidCodec> androidCodec =
            SkAndroidCodec::MakeFromCodec(std::move(baseCodec));
    REPORTER_ASSERT(r, androidCodec);

    SkGainmapInfo gainmapInfo;
    std::unique_ptr<SkAndroidCodec> gainmapCodec;
    bool hasGainmap = androidCodec->getGainmapAndroidCodec(&gainmapInfo, &gainmapCodec);

    REPORTER_ASSERT(r, hasGainmap);
    REPORTER_ASSERT(r, gainmapCodec);

    // Decode the gainmap bitmap.
    SkBitmap gainmapBitmap;
    gainmapBitmap.allocPixels(gainmapCodec->getInfo());
    REPORTER_ASSERT(r,
                    SkCodec::kSuccess == gainmapCodec->getAndroidPixels(gainmapBitmap.info(),
                                                                        gainmapBitmap.getPixels(),
                                                                        gainmapBitmap.rowBytes()));

    // Spot-check the image size and pixels (dimensions should be 32x32 for gainmap.png)
    REPORTER_ASSERT(r, gainmapBitmap.dimensions() == SkISize::Make(32, 32));
    REPORTER_ASSERT(r, gainmapBitmap.getColor(0, 0) == 0xffffffff);
    REPORTER_ASSERT(r, gainmapBitmap.getColor(31, 31) == 0xff000000);

    // Verify some gainmap info values (matching recs in PngGainmapTest.cpp)
    REPORTER_ASSERT(r, gainmapInfo.fType == SkGainmapInfo::Type::kDefault);
    REPORTER_ASSERT(r, gainmapInfo.fBaseImageType == SkGainmapInfo::BaseImageType::kHDR);
    REPORTER_ASSERT(r, gainmapInfo.fDisplayRatioSdr == 2.f);
    REPORTER_ASSERT(r, gainmapInfo.fDisplayRatioHdr == 4.f);
}

DEF_TEST(RustPngCodec_gainmapRewind, r) {
    auto stream = GetResourceAsStream("images/gainmap.png", false);
    REPORTER_ASSERT(r, stream);

    MockChunkReader chunkReader;
    SkCodec::Result result = SkCodec::kSuccess;
    std::unique_ptr<SkCodec> baseCodec =
            SkPngRustDecoder::Decode(std::move(stream), &result, &chunkReader);
    REPORTER_ASSERT(r, baseCodec);
    REPORTER_ASSERT(r, chunkReader.fChunks.size() == 2);

    // Decode pixels the first time.
    SkImageInfo info = baseCodec->getInfo();
    SkBitmap bm;
    bm.allocPixels(info);
    result = baseCodec->getPixels(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    // Should still be 2 (no rewind was needed for first decode).
    REPORTER_ASSERT(r, chunkReader.fChunks.size() == 2);

    // Decode pixels again (forces rewind).
    result = baseCodec->getPixels(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    // Should be 4 now because rewind recreated the reader and processed chunks again.
    REPORTER_ASSERT(r, chunkReader.fChunks.size() == 4);
}

DEF_TEST(RustPngCodec_ninepatchPngChunkReader, r) {
    auto stream = GetResourceAsStream("images/ninepatch.png", false);
    REPORTER_ASSERT(r, stream);

    MockChunkReader chunkReader;

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec(
            SkPngRustDecoder::Decode(std::move(stream), &result, &chunkReader));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    // Now compare to the original.
    SkBitmap decodedBm;
    decodedBm.setInfo(codec->getInfo());
    decodedBm.allocPixels();
    result = codec->getPixels(codec->getInfo(), decodedBm.getPixels(), decodedBm.rowBytes());
    REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    REPORTER_ASSERT(r, decodedBm.getColor(0, 0) == SK_ColorBLUE);
    REPORTER_ASSERT(r, chunkReader.fChunks.size() == 3);
    REPORTER_ASSERT(r,
                    chunkReader.fChunks[0] ==
                            std::make_pair(std::string("npOl"), std::string("outline", 8)));
    REPORTER_ASSERT(r,
                    chunkReader.fChunks[1] ==
                            std::make_pair(std::string("npLb"), std::string("layoutBounds", 13)));
    REPORTER_ASSERT(r,
                    chunkReader.fChunks[2] ==
                            std::make_pair(std::string("npTc"), std::string("ninePatchData", 14)));
}

DEF_TEST(RustPngCodec_exactRead, r) {
    // Replicates the Codec_end test from CodecExactReadTest.cpp.
    // Verifies that with the limit reader enabled, we don't overshoot
    // and can decode subsequent images from the same stream.
    for (const char* path : {
                 "images/plane.png",
                 "images/yellow_rose.png",
                 "images/plane_interlaced.png",
         }) {
        sk_sp<SkData> data = GetResourceAsData(path);
        if (!data) {
            continue;
        }

        const int kNumImages = 2;
        const size_t size = data->size();
        sk_sp<SkData> multiData = SkData::MakeUninitialized(size * kNumImages);
        void* dst = multiData->writable_data();
        for (int i = 0; i < kNumImages; i++) {
            memcpy(SkTAddOffset<void>(dst, size * i), data->data(), size);
        }
        data.reset();

        SkMemoryStream stream(std::move(multiData));

        for (int i = 0; i < kNumImages; ++i) {
            SkCodec::Result result;
            std::unique_ptr<SkCodec> codec =
                    SkPngRustDecoder::Decode(std::make_unique<UnowningStream>(&stream), &result);
            if (!codec) {
                ERRORF(r, "Failed to create a codec from %s, iteration %i", path, i);
                continue;
            }

            auto info = codec->getInfo().makeColorType(kN32_SkColorType);
            SkBitmap bm;
            bm.allocPixels(info);

            result = codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes());
            if (result != SkCodec::kSuccess) {
                ERRORF(r, "Failed to getPixels from %s, iteration %i error %i", path, i, result);
                continue;
            }
        }
    }
}
#else
DEF_TEST(RustPngCodec_gainmapNoOp, r) {
    auto stream = GetResourceAsStream("images/gainmap.png", false);
    REPORTER_ASSERT(r, stream);

    SkCodec::Result result = SkCodec::kSuccess;
    std::unique_ptr<SkCodec> baseCodec = SkPngRustDecoder::Decode(std::move(stream), &result);
    REPORTER_ASSERT(r, baseCodec);

    std::unique_ptr<SkAndroidCodec> androidCodec =
            SkAndroidCodec::MakeFromCodec(std::move(baseCodec));
    REPORTER_ASSERT(r, androidCodec);

    SkGainmapInfo gainmapInfo;
    std::unique_ptr<SkAndroidCodec> gainmapCodec;
    bool hasGainmap = androidCodec->getGainmapAndroidCodec(&gainmapInfo, &gainmapCodec);

    // When the build flag is off, it should return false (no gainmap).
    REPORTER_ASSERT(r, !hasGainmap);
    REPORTER_ASSERT(r, !gainmapCodec);
}

DEF_TEST(RustPngCodec_exactRead_overshoot, r) {
    // Replicates the Codec_end test from CodecExactReadTest.cpp.
    // Verifies that without the limit reader, the decoder overshoots
    // and fails to decode the second image because the stream is misaligned.
    const char* path = "images/plane.png";
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        return;
    }

    const int kNumImages = 2;
    const size_t size = data->size();
    sk_sp<SkData> multiData = SkData::MakeUninitialized(size * kNumImages);
    void* dst = multiData->writable_data();
    for (int i = 0; i < kNumImages; i++) {
        memcpy(SkTAddOffset<void>(dst, size * i), data->data(), size);
    }
    data.reset();

    SkMemoryStream stream(std::move(multiData));

    for (int i = 0; i < kNumImages; ++i) {
        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec =
                SkPngRustDecoder::Decode(std::make_unique<UnowningStream>(&stream), &result);
        if (i == 0) {
            if (!codec) {
                ERRORF(r, "Failed to create a codec from %s, iteration %i", path, i);
                return;
            }
            auto info = codec->getInfo().makeColorType(kN32_SkColorType);
            SkBitmap bm;
            bm.allocPixels(info);
            result = codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes());
            if (result != SkCodec::kSuccess) {
                ERRORF(r, "Failed to getPixels from %s, iteration %i error %i", path, i, result);
                return;
            }
        } else {
            // We expect failure on the second iteration because the first decode overshot.
            REPORTER_ASSERT(r, !codec);
        }
    }
}
#endif


DEF_TEST(RustPngCodec_subsampling, r) {
    for (int sampleSize : {2, 3, 5, 8, 100, 1000}) {
        AssertAndroidDecodeSampling(r, "images/plane.png", sampleSize);
    }
}

DEF_TEST(RustPngCodec_subsampling_interlaced, r) {
    for (int sampleSize : {2, 3, 5, 8, 100, 1000}) {
        AssertAndroidDecodeSampling(r, "images/plane_interlaced.png", sampleSize);
    }
}

DEF_TEST(RustPngCodec_subsampling_subset, r) {
    for (int sampleSize : {2, 3, 5, 8, 100, 1000}) {
        AssertAndroidDecodeSampling(r, "images/plane.png", sampleSize, [](const SkImageInfo& info) {
            return SkIRect::MakeXYWH(info.width() / 2, 0, info.width() / 2, info.height());
        });
    }
}

DEF_TEST(RustPngCodec_subsampling_subset_interlaced, r) {
    for (int sampleSize : {2, 3, 5, 8, 100, 1000}) {
        AssertAndroidDecodeSampling(
                r, "images/plane_interlaced.png", sampleSize, [](const SkImageInfo& info) {
                    return SkIRect::MakeXYWH(0, 1, info.width(), info.height() - 1);
                });
    }
}

// --- Halting Tests without Subsampling ---

DEF_TEST(RustPngCodec_halting, r) { AssertDecodeHalting(r, "images/mandrill_128.png"); }

DEF_TEST(RustPngCodec_interlaced_halting, r) {
    AssertDecodeHalting(r,
                        "images/plane_interlaced.png",
                        nullptr,
                        kN32_SkColorType,
                        0,
                        /*expectPartialHeight=*/false);
}

DEF_TEST(RustPngCodec_interlaced_pass0_halting, r) {
    AssertDecodeHalting(r,
                        "images/plane_interlaced.png",
                        nullptr,
                        kN32_SkColorType,
                        300,
                        /*expectPartialHeight=*/true);
}

DEF_TEST(RustPngCodec_interlaced_1x1_halting, r) {
    AssertDecodeHalting(r,
                        "images/interlaced_1x1.png",
                        nullptr,
                        kN32_SkColorType,
                        45,
                        /*expectPartialHeight=*/true);
}

DEF_TEST(RustPngCodec_subset_halting, r) {
    SkIRect subset = SkIRect::MakeXYWH(32, 32, 64, 64);
    AssertDecodeHalting(r, "images/mandrill_128.png", &subset);
}

DEF_TEST(RustPngCodec_subset_interlaced_halting, r) {
    SkIRect subset = SkIRect::MakeXYWH(10, 10, 100, 50);
    AssertDecodeHalting(r,
                        "images/plane_interlaced.png",
                        &subset,
                        kN32_SkColorType,
                        0,
                        /*expectPartialHeight=*/false);
}

DEF_TEST(RustPngCodec_gray8_halting, r) {
    AssertDecodeHalting(r, "images/grayscale.png", nullptr, kGray_8_SkColorType);
}

DEF_TEST(RustPngCodec_rgb565_halting, r) {
    AssertDecodeHalting(r, "images/grayscale.png", nullptr, kRGB_565_SkColorType);
}

DEF_TEST(RustPngCodec_apng_halting, r) {
    AssertDecodeHalting(r,
                        "images/apng-test-suite--dispose-ops--none-basic.png",
                        nullptr,
                        kN32_SkColorType,
                        341,
                        /*expectPartialHeight=*/true,
                        1);
}

// --- Halting Tests with Subsampling ---

DEF_TEST(RustPngCodec_subsampling_halting, r) {
    for (int sampleSize : {2, 3, 5, 8, 100, 1000}) {
        AssertAndroidDecodeHalting(r, "images/mandrill_128.png", sampleSize);
    }
}

DEF_TEST(RustPngCodec_subsampling_interlaced_halting, r) {
    for (int sampleSize : {2, 3, 5, 8, 100, 1000}) {
        AssertAndroidDecodeHalting(r, "images/plane_interlaced.png", sampleSize);
    }
}

DEF_TEST(RustPngCodec_subsampling_interlaced_1x1_halting, r) {
    for (int sampleSize : {2, 3, 5, 8}) {
        AssertAndroidDecodeHalting(r, "images/interlaced_1x1.png", sampleSize, nullptr, 45);
    }
}

DEF_TEST(RustPngCodec_subsampling_subset_halting, r) {
    for (int sampleSize : {2, 3, 5, 8, 100, 1000}) {
        AssertAndroidDecodeHalting(
                r, "images/mandrill_128.png", sampleSize, [](const SkImageInfo& info) {
                    return SkIRect::MakeXYWH(info.width() / 2, 0, info.width() / 2, info.height());
                });
    }
}

DEF_TEST(RustPngCodec_subsampling_subset_interlaced_halting, r) {
    for (int sampleSize : {2, 3, 5, 8, 100, 1000}) {
        AssertAndroidDecodeHalting(
                r, "images/plane_interlaced.png", sampleSize, [](const SkImageInfo& info) {
                    return SkIRect::MakeXYWH(0, 1, info.width(), info.height() - 1);
                });
    }
}

static void CompareBitmapsWithPath(skiatest::Reporter* r,
                                   const SkBitmap& bm1,
                                   const SkBitmap& bm2,
                                   const char* path) {
    SkASSERT(bm1.info() == bm2.info());
    for (int i = 0; i < bm1.info().height(); i++) {
        if (0 != memcmp(bm1.getAddr(0, i), bm2.getAddr(0, i), bm1.info().minRowBytes())) {
            ERRORF(r,
                   "%s: Bitmaps have different pixels on success, starting on line %i!",
                   path,
                   i);
            break;
        }
    }
}
