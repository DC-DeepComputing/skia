/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkGainmapShader.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

DEF_TEST(Codec_jpegSegmentScan, r) {
    const struct Rec {
        const char* path;
        size_t sosSegmentCount;
        size_t eoiSegmentCount;
        size_t testSegmentIndex;
        uint8_t testSegmentMarker;
        size_t testSegmentOffset;
        uint16_t testSegmentParameterLength;
    } recs[] = {
            {"images/wide_gamut_yellow_224_224_64.jpeg", 11, 15, 10, 0xda, 9768, 12},
            {"images/CMYK.jpg", 7, 8, 1, 0xee, 2, 14},
            {"images/b78329453.jpeg", 10, 23, 3, 0xe2, 154, 540},
            {"images/brickwork-texture.jpg", 8, 28, 12, 0xc4, 34183, 42},
            {"images/brickwork_normal-map.jpg", 8, 28, 27, 0xd9, 180612, 0},
            {"images/cmyk_yellow_224_224_32.jpg", 19, 23, 2, 0xed, 854, 2828},
            {"images/color_wheel.jpg", 10, 11, 2, 0xdb, 20, 67},
            {"images/cropped_mandrill.jpg", 10, 11, 4, 0xc0, 158, 17},
            {"images/dog.jpg", 10, 11, 5, 0xc4, 177, 28},
            {"images/ducky.jpg", 12, 13, 10, 0xc4, 3718, 181},
            {"images/exif-orientation-2-ur.jpg", 11, 12, 2, 0xe1, 20, 130},
            {"images/flutter_logo.jpg", 9, 27, 21, 0xda, 5731, 8},
            {"images/grayscale.jpg", 6, 16, 9, 0xda, 327, 8},
            {"images/icc-v2-gbr.jpg", 12, 25, 24, 0xd9, 43832, 0},
            {"images/mandrill_512_q075.jpg", 10, 11, 7, 0xc4, 393, 31},
            {"images/mandrill_cmyk.jpg", 19, 35, 16, 0xdd, 574336, 4},
            {"images/mandrill_h1v1.jpg", 10, 11, 1, 0xe0, 2, 16},
            {"images/mandrill_h2v1.jpg", 10, 11, 0, 0xd8, 0, 0},
            {"images/randPixels.jpg", 10, 11, 6, 0xc4, 200, 30},
            {"images/wide_gamut_yellow_224_224_64.jpeg", 11, 15, 10, 0xda, 9768, 12},
    };

    for (const auto& rec : recs) {
        auto stream = GetResourceAsStream(rec.path);
        if (!stream) {
            continue;
        }

        // Ensure that we get the expected number of segments for a scan that stops at StartOfScan.
        SkJpegSegmentScan::Options options;
        auto sosSegmentScan = SkJpegSegmentScan::Create(stream.get(), options);
        REPORTER_ASSERT(r, rec.sosSegmentCount == sosSegmentScan->segments().size());

        // Rewind and now go all the way to EndOfImage.
        stream->rewind();
        options.stopOnStartOfScan = false;
        auto eoiSegmentScan = SkJpegSegmentScan::Create(stream.get(), options);
        REPORTER_ASSERT(r, rec.eoiSegmentCount == eoiSegmentScan->segments().size());

        // Verify the values for a randomly pre-selected segment index.
        const auto& segment = eoiSegmentScan->segments()[rec.testSegmentIndex];
        REPORTER_ASSERT(r, rec.testSegmentMarker == segment.marker);
        REPORTER_ASSERT(r, rec.testSegmentOffset == segment.offset);
        REPORTER_ASSERT(r, rec.testSegmentParameterLength == segment.parameterLength);
    }
}

DEF_TEST(Codec_jpegMultiPicture, r) {
    const char* path = "images/iphone_13_pro.jpeg";
    auto stream = GetResourceAsStream(path);
    REPORTER_ASSERT(r, stream);

    auto segmentScan = SkJpegSegmentScan::Create(stream.get(), SkJpegSegmentScan::Options());
    REPORTER_ASSERT(r, segmentScan);

    // Extract the streams for the MultiPicture images.
    auto mpStreams = SkJpegExtractMultiPictureStreams(segmentScan.get());
    REPORTER_ASSERT(r, mpStreams);
    size_t numberOfImages = mpStreams->images.size();

    // Decode them into bitmaps.
    std::vector<SkBitmap> bitmaps(numberOfImages);
    for (size_t i = 0; i < numberOfImages; ++i) {
        auto imageStream = std::move(mpStreams->images[i].stream);
        if (i == 0) {
            REPORTER_ASSERT(r, !imageStream);
            continue;
        }
        REPORTER_ASSERT(r, imageStream);

        std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(imageStream));
        REPORTER_ASSERT(r, codec);

        SkBitmap bm;
        bm.allocPixels(codec->getInfo());
        REPORTER_ASSERT(
                r, SkCodec::kSuccess == codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes()));
        bitmaps[i] = bm;
    }

    // Spot-check the image size and pixels.
    REPORTER_ASSERT(r, bitmaps[1].dimensions() == SkISize::Make(1512, 2016));
    REPORTER_ASSERT(r, bitmaps[1].getColor(0, 0) == 0xFF3B3B3B);
    REPORTER_ASSERT(r, bitmaps[1].getColor(1511, 2015) == 0xFF101010);
    REPORTER_ASSERT(r, bitmaps[2].dimensions() == SkISize::Make(576, 768));
    REPORTER_ASSERT(r, bitmaps[2].getColor(0, 0) == 0xFF010101);
    REPORTER_ASSERT(r, bitmaps[2].getColor(575, 767) == 0xFFB5B5B5);
}

DEF_TEST(AndroidCodec_jpegGainmap, r) {
    const struct Rec {
        const char* path;
        SkISize dimensions;
        SkColor originColor;
        SkColor farCornerColor;
    } recs[] = {
            {"images/iphone_13_pro.jpeg", SkISize::Make(1512, 2016), 0xFF3B3B3B, 0xFF101010},
    };

    for (const auto& rec : recs) {
        auto stream = GetResourceAsStream(rec.path);
        REPORTER_ASSERT(r, stream);

        std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
        REPORTER_ASSERT(r, codec);

        std::unique_ptr<SkAndroidCodec> androidCodec =
                SkAndroidCodec::MakeFromCodec(std::move(codec));
        REPORTER_ASSERT(r, androidCodec);

        SkGainmapInfo gainmapInfo;
        std::unique_ptr<SkStream> gainmapStream;
        REPORTER_ASSERT(r, androidCodec->getAndroidGainmap(&gainmapInfo, &gainmapStream));
        REPORTER_ASSERT(r, gainmapStream);

        std::unique_ptr<SkCodec> gainmapCodec = SkCodec::MakeFromStream(std::move(gainmapStream));
        REPORTER_ASSERT(r, gainmapCodec);

        SkBitmap bm;
        bm.allocPixels(gainmapCodec->getInfo());
        REPORTER_ASSERT(r,
                        SkCodec::kSuccess ==
                                gainmapCodec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes()));

        // Spot-check the image size and pixels.
        REPORTER_ASSERT(r, bm.dimensions() == rec.dimensions);
        REPORTER_ASSERT(r, bm.getColor(0, 0) == rec.originColor);
        REPORTER_ASSERT(r,
                        bm.getColor(rec.dimensions.fWidth - 1, rec.dimensions.fHeight - 1) ==
                                rec.farCornerColor);
    }
}

static bool approx_equal(const SkColor4f& a, const SkColor4f& b) {
    constexpr float kEpsilon = 1e-3f;
    return std::abs(a.fR - b.fR) < kEpsilon && std::abs(a.fG - b.fG) < kEpsilon &&
           std::abs(a.fB - b.fB) < kEpsilon && std::abs(a.fA - b.fA) < kEpsilon;
}

static sk_sp<SkImage> make_1x1_image(sk_sp<SkColorSpace> cs,
                                     SkAlphaType alphaType,
                                     SkColor4f color) {
    SkImageInfo bmInfo = SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, alphaType, cs);
    SkBitmap bm;
    bm.allocPixels(bmInfo);
    SkImageInfo pmInfo = SkImageInfo::Make(
            1, 1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkPixmap pm(pmInfo, &color, pmInfo.minRowBytes());
    bm.writePixels(pm, 0, 0);
    return SkImage::MakeFromBitmap(bm);
}

// Returns an unpremultiplied color in sRGB color space.
SkColor4f apply_1x1_gainmap(sk_sp<SkImage> sdrImage,
                            sk_sp<SkImage> gainmapImage,
                            const SkGainmapInfo& gainmapInfo,
                            float dstHdrRatio,
                            sk_sp<SkColorSpace> dstColorSpace = SkColorSpace::MakeSRGB(),
                            SkAlphaType dstAlphaType = kPremul_SkAlphaType) {
    constexpr SkRect k1x1Rect = SkRect::MakeWH(1, 1);
    sk_sp<SkShader> shader = SkGainmapGetShader(sdrImage,
                                                k1x1Rect,
                                                SkSamplingOptions(),
                                                gainmapImage,
                                                k1x1Rect,
                                                SkSamplingOptions(),
                                                gainmapInfo,
                                                k1x1Rect,
                                                dstColorSpace,
                                                dstHdrRatio);

    SkImageInfo canvasInfo =
            SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, dstAlphaType, dstColorSpace);
    SkBitmap canvasBitmap;
    canvasBitmap.allocPixels(canvasInfo);

    SkPaint paint;
    paint.setShader(shader);

    SkCanvas canvas(canvasBitmap);
    canvas.drawRect(k1x1Rect, paint);

    SkImageInfo dstInfo = SkImageInfo::Make(
            1, 1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB());
    float dstPixel[4];
    canvasBitmap.readPixels(dstInfo, dstPixel, sizeof(dstPixel), 0, 0);
    return {dstPixel[0], dstPixel[1], dstPixel[2], dstPixel[3]};
}

DEF_TEST(AndroidCodec_applyGainmapAlphaTypeColorSpace, r) {
    constexpr SkColor4f kSdrColor = {1.f, 0.5f, 0.2f, 0.5};
    constexpr SkColor4f kGainmapColor = {1.f, 1.f, 1.f, 1.f};
    constexpr SkColor4f kExpectedColor = {1.3441f, 0.6767f, 0.2762f, 0.5000f};
    const auto kSRGB = SkColorSpace::MakeSRGB();

    const struct Rec {
        const SkAlphaType sdrAlphaType;
        const SkAlphaType canvasAlphaType;
        sk_sp<SkColorSpace> sdrColorSpace;
        sk_sp<SkColorSpace> gainmapColorSpace;
    } recs[] = {
            {kPremul_SkAlphaType, kPremul_SkAlphaType, kSRGB, kSRGB},
            {kPremul_SkAlphaType, kUnpremul_SkAlphaType, kSRGB, kSRGB},
            {kUnpremul_SkAlphaType, kPremul_SkAlphaType, kSRGB, kSRGB},
            {kUnpremul_SkAlphaType, kUnpremul_SkAlphaType, kSRGB, kSRGB},
            {kPremul_SkAlphaType,
             kPremul_SkAlphaType,
             SkColorSpace::MakeSRGB()->makeColorSpin(),
             SkColorSpace::MakeSRGBLinear()},
    };

    for (const auto& rec : recs) {
        auto sdrImage = make_1x1_image(rec.sdrColorSpace, rec.sdrAlphaType, kSdrColor);
        auto gainmapImage =
                make_1x1_image(rec.gainmapColorSpace, kOpaque_SkAlphaType, kGainmapColor);

        SkGainmapInfo gainmapInfo;
        const SkColor4f color = apply_1x1_gainmap(sdrImage,
                                                  gainmapImage,
                                                  gainmapInfo,
                                                  gainmapInfo.fHdrRatioMax,
                                                  SkColorSpace::MakeSRGB(),
                                                  rec.canvasAlphaType);
        REPORTER_ASSERT(r, approx_equal(color, kExpectedColor));
    }
}

DEF_TEST(AndroidCodec_applyGainmapImageRects, r) {
    SkColor4f sdrColors[5][2] = {
            {{-1.f, -1.f, -1.f, 1.0f}, {-1.f, -1.f, -1.f, 1.0f}},
            {{1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.5f, 1.0f}},
            {{1.0f, 0.5f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.5f, 1.0f}},
            {{0.5f, 1.0f, 1.0f, 1.0f}, {0.5f, 1.0f, 0.5f, 1.0f}},
            {{0.5f, 0.5f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}},
    };
    SkPixmap sdrPixmap(SkImageInfo::Make(2, 5, kRGBA_F32_SkColorType, kPremul_SkAlphaType),
                       sdrColors,
                       2 * sizeof(SkColor4f));
    auto sdrImage = SkImage::MakeFromRaster(sdrPixmap, nullptr, nullptr);
    const auto sdrImageRect = SkRect::MakeXYWH(0.f, 1.f, 2.f, 4.f);

    // The top pixel indicates to gain only red, and the bottom pixel indicates to gain everything
    // except red.
    SkColor4f gainmapColors[2][2] = {
            {{-1.f, -1.f, -1.f, 1.f}, {1.0f, 0.0f, 0.0f, 1.f}},
            {{-1.f, -1.f, -1.f, 1.f}, {0.0f, 1.0f, 1.0f, 1.f}},
    };
    SkPixmap gainmapPixmap(SkImageInfo::Make(2, 2, kRGBA_F32_SkColorType, kPremul_SkAlphaType),
                           gainmapColors,
                           2 * sizeof(SkColor4f));
    auto gainmapImage = SkImage::MakeFromRaster(gainmapPixmap, nullptr, nullptr);
    const auto gainmapImageRect = SkRect::MakeXYWH(1.f, 0.f, 1.f, 2.f);

    SkImageInfo canvasInfo = SkImageInfo::Make(
            4, 6, kRGBA_F32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkBitmap canvasBitmap;
    canvasBitmap.allocPixels(canvasInfo);
    canvasBitmap.eraseColor(SK_ColorTRANSPARENT);
    const auto canvasRect = SkRect::MakeXYWH(1.f, 1.f, 2.f, 4.f);

    SkGainmapInfo gainmapInfo;
    {
        sk_sp<SkShader> shader = SkGainmapGetShader(sdrImage,
                                                    sdrImageRect,
                                                    SkSamplingOptions(),
                                                    gainmapImage,
                                                    gainmapImageRect,
                                                    SkSamplingOptions(),
                                                    gainmapInfo,
                                                    canvasRect,
                                                    canvasInfo.refColorSpace(),
                                                    gainmapInfo.fHdrRatioMax);
        SkPaint paint;
        paint.setShader(shader);
        SkCanvas canvas(canvasBitmap);
        canvas.drawRect(canvasRect, paint);
    }

    // Compute and compare the expected colors.
    constexpr float k10G = 1.353256028586302f;   // This is linearToSRGB(2.0).
    constexpr float k05G = 0.6858361015012847f;  // This is linearToSRGB(srgbToLinear(0.5)*2.0)
    SkColor4f expectedColors[4][2] = {
            {{k10G, 1.0f, 1.0f, 1.0f}, {k10G, 1.0f, 0.5f, 1.0f}},
            {{k10G, 0.5f, 1.0f, 1.0f}, {k10G, 0.5f, 0.5f, 1.0f}},
            {{0.5f, k10G, k10G, 1.0f}, {0.5f, k10G, k05G, 1.0f}},
            {{0.5f, k05G, k10G, 1.0f}, {0.5f, k05G, k05G, 1.0f}},
    };
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 2; ++x) {
            auto color = canvasBitmap.getColor4f(x + 1, y + 1);
            REPORTER_ASSERT(r, approx_equal(color, expectedColors[y][x]));
        }
    }
}

DEF_TEST(AndroidCodec_applyGainmapGetImage, r) {
    SkColor4f sdrColors[4][2] = {
            {{1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.5f, 1.0f}},
            {{1.0f, 0.5f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.5f, 1.0f}},
            {{0.5f, 1.0f, 1.0f, 1.0f}, {0.5f, 1.0f, 0.5f, 1.0f}},
            {{0.5f, 0.5f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}},
    };
    SkPixmap sdrPixmap(SkImageInfo::Make(2, 4, kRGBA_F32_SkColorType, kOpaque_SkAlphaType),
                       sdrColors,
                       2 * sizeof(SkColor4f));
    auto sdrImage = SkImage::MakeFromRaster(sdrPixmap, nullptr, nullptr);

    // The top pixel indicates to gain only red, and the bottom pixel indicates to gain everything
    // except red.
    SkColor4f gainmapColors[2][1] = {
            {{1.0f, 0.0f, 0.0f, 1.f}},
            {{0.0f, 1.0f, 1.0f, 1.f}},
    };
    SkPixmap gainmapPixmap(SkImageInfo::Make(1, 2, kRGBA_F32_SkColorType, kOpaque_SkAlphaType),
                           gainmapColors,
                           1 * sizeof(SkColor4f));
    auto gainmapImage = SkImage::MakeFromRaster(gainmapPixmap, nullptr, nullptr);

    // Create the HDR representation of the image.
    SkGainmapInfo gainmapInfo;
    sk_sp<SkImage> hdrImage = SkGainmapGetHdrImage(
            nullptr, sdrImage, gainmapImage, gainmapInfo, gainmapInfo.fHdrRatioMax);

    // Draw the HDR image.
    SkImageInfo canvasInfo = SkImageInfo::Make(
            2, 4, kRGBA_F32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkBitmap canvasBitmap;
    canvasBitmap.allocPixels(canvasInfo);
    SkCanvas canvas(canvasBitmap);
    canvas.drawImage(hdrImage, 0, 0);

    // Compute and compare the expected colors.
    constexpr float k10G10 = 1.353256028586302f;   // linearToSRGB(2.0).
    constexpr float k10G75 = 1.255214549133273f;   // linearToSRGB(exp2(0.75))
    constexpr float k10G25 = 1.0789913696890638f;  // linearToSRGB(exp2(0.25))
    constexpr float k10G00 = 1.0f;
    constexpr float k05G10 = 0.6858361015012847f;  // linearToSRGB(srgbToLinear(0.5)*2.0)
    constexpr float k05G75 = 0.6342597804972736f;  // linearToSRGB(srgbToLinear(0.5)*exp2(0.75))
    constexpr float k05G25 = 0.5415546997287156f;  // linearToSRGB(srgbToLinear(0.5)*exp2(0.25))
    constexpr float k05G00 = 0.5f;
    SkColor4f expectedColors[4][2] = {
            {{k10G10, k10G00, k10G00, k10G00}, {k10G10, k10G00, k05G00, k10G00}},
            {{k10G75, k05G25, k10G25, k10G00}, {k10G75, k05G25, k05G25, k10G00}},
            {{k05G25, k10G75, k10G75, k10G00}, {k05G25, k10G75, k05G75, k10G00}},
            {{k05G00, k05G10, k10G10, k10G00}, {k05G00, k05G10, k05G10, k10G00}},
    };
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 2; ++x) {
            auto color = canvasBitmap.getColor4f(x, y);
            REPORTER_ASSERT(r, approx_equal(color, expectedColors[y][x]));
        }
    }
}
