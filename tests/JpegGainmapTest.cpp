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

static void write_pixel(SkBitmap& bm,
                        int x,
                        int y,
                        SkColor4f color,
                        sk_sp<SkColorSpace> cs = SkColorSpace::MakeSRGB()) {
    SkImageInfo info = SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType, cs);
    const float pixel[] = {
            color.fR,
            color.fG,
            color.fB,
            color.fA,
    };
    SkPixmap src(info, pixel, info.minRowBytes());
    bm.writePixels(src, x, y);
}

static sk_sp<SkImage> make_1x1_image(sk_sp<SkColorSpace> cs,
                                     SkAlphaType alphaType,
                                     SkColor4f color,
                                     bool colorIsSRGB) {
    SkImageInfo info = SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, alphaType, cs);
    SkBitmap bm;
    if (!bm.tryAllocPixels(info)) {
        return nullptr;
    }
    bm.eraseColor(SK_ColorTRANSPARENT);
    write_pixel(bm, 0, 0, color, colorIsSRGB ? nullptr : cs);
    return SkImage::MakeFromBitmap(bm);
}

// Returns an unpremultiplied color in sRGB color space.
SkColor4f apply_1x1_gainmap(sk_sp<SkImage> sdrImage,
                            sk_sp<SkImage> gainmapImage,
                            const SkGainmapInfo& gainmapInfo,
                            float dstHdrRatio,
                            sk_sp<SkColorSpace> dstColorSpace = SkColorSpace::MakeSRGB(),
                            SkAlphaType dstAlphaType = kPremul_SkAlphaType) {
    SkImageInfo canvasInfo =
            SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, dstAlphaType, dstColorSpace);
    SkBitmap canvasBitmap;
    if (!canvasBitmap.tryAllocPixels(canvasInfo)) {
        return SkColors::kTransparent;
    }

    constexpr SkRect k1x1Rect = SkRect::MakeWH(1, 1);
    sk_sp<SkShader> shader = SkDrawImageWithGainmapShader(sdrImage,
                                                          k1x1Rect,
                                                          SkSamplingOptions(),
                                                          gainmapImage,
                                                          k1x1Rect,
                                                          SkSamplingOptions(),
                                                          gainmapInfo,
                                                          k1x1Rect,
                                                          dstColorSpace,
                                                          dstHdrRatio);

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

DEF_TEST(AndroidCodec_applyGainmapColorSpaces, r) {
    constexpr SkColor4f sdrColor = {1.f, 2.f, 3.f, 1.f};
    auto sdrImage = make_1x1_image(SkColorSpace::MakeSRGB()->makeColorSpin(),
                                   kPremul_SkAlphaType,
                                   sdrColor,
                                   /*colorIsSRGB=*/true);

    // Because the gainmap is applied in sdrImage's color space, this will gain red by exp(0.5),
    // green by exp(1), and blue by exp(0).
    constexpr SkColor4f gainColor = {1.f, 0.0f, 0.5f, 1.f};
    auto gainmapImage = make_1x1_image(
            SkColorSpace::MakeSRGBLinear(), kPremul_SkAlphaType, gainColor, /*colorIsSRGB=*/false);

    SkGainmapInfo gainmapInfo;
    const auto dstColorSpace =
            SkColorSpace::MakeRGB(SkNamedTransferFn::kHLG, SkNamedGamut::kRec2020);
    const SkColor4f color = apply_1x1_gainmap(
            sdrImage, gainmapImage, gainmapInfo, gainmapInfo.fHdrRatioMax, dstColorSpace);
    constexpr SkColor4f kExpectedColor = {1.1640f, 2.6881f, 3.0000f, 1.0000f};
    REPORTER_ASSERT(r, approx_equal(color, kExpectedColor));
}

DEF_TEST(AndroidCodec_applyGainmapAlphaType, r) {
    constexpr SkColor4f kSdrColor = {1.f, 0.5f, 0.2f, 0.5};
    constexpr SkColor4f kGainmapColor = {1.f, 1.f, 1.f, 1.f};
    constexpr SkColor4f kExpectedColor = {1.3441f, 0.6767f, 0.2762f, 0.5000f};

    const struct Rec {
        const SkAlphaType imageAlphaType;
        const SkAlphaType canvasAlphaType;
    } recs[] = {
            {kPremul_SkAlphaType, kPremul_SkAlphaType},
            {kPremul_SkAlphaType, kUnpremul_SkAlphaType},
            {kUnpremul_SkAlphaType, kPremul_SkAlphaType},
            {kUnpremul_SkAlphaType, kUnpremul_SkAlphaType},
    };

    for (const auto& rec : recs) {
        auto sdrImage = make_1x1_image(
                SkColorSpace::MakeSRGB(), rec.imageAlphaType, kSdrColor, /*colorIsSRGB=*/true);
        auto gainmapImage = make_1x1_image(SkColorSpace::MakeSRGB(),
                                           kOpaque_SkAlphaType,
                                           kGainmapColor,
                                           /*colorIsSRGB=*/false);

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
    SkColor4f sdrColors[4][2] = {
            {{1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.5f, 1.0f}},
            {{1.0f, 0.5f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.5f, 1.0f}},
            {{0.5f, 1.0f, 1.0f, 1.0f}, {0.5f, 1.0f, 0.5f, 1.0f}},
            {{0.5f, 0.5f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}},
    };

    // The top pixel indicates to gain only red, and the bottom pixel indicates to gain everything
    // except red.
    SkColor4f gainmapColors[2][1] = {
            {{1.0f, 0.0f, 0.0f, 1.f}},
            {{0.0f, 1.0f, 1.0f, 1.f}},
    };

    // Constants for the gainmap applied to 1.0 and 0.5.
    constexpr float k10G = 1.353256028586302f;   // This is linearToSRGB(2.0).
    constexpr float k05G = 0.6858361015012847f;  // This is linearToSRGB(srgbToLinear(0.5)*2.0)
    SkColor4f expectedColors[4][2] = {
            {{k10G, 1.0f, 1.0f, 1.0f}, {k10G, 1.0f, 0.5f, 1.0f}},
            {{k10G, 0.5f, 1.0f, 1.0f}, {k10G, 0.5f, 0.5f, 1.0f}},
            {{0.5f, k10G, k10G, 1.0f}, {0.5f, k10G, k05G, 1.0f}},
            {{0.5f, k05G, k10G, 1.0f}, {0.5f, k05G, k05G, 1.0f}},
    };

    sk_sp<SkImage> sdrImage;
    SkRect sdrImageRect;
    {
        SkImageInfo info = SkImageInfo::Make(
                2, 5, kRGBA_F32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
        SkBitmap bm;
        if (!bm.tryAllocPixels(info)) {
            ERRORF(r, "Failed to allocate SDR pixels");
            return;
        }
        bm.eraseColor(SK_ColorTRANSPARENT);
        for (int x = 0; x < 2; ++x) {
            for (int y = 0; y < 4; ++y) {
                write_pixel(bm, x, y + 1, sdrColors[y][x]);
            }
        }
        sdrImage = SkImage::MakeFromBitmap(bm);
        sdrImageRect = SkRect::MakeXYWH(0.f, 1.f, 2.f, 4.f);
    }

    sk_sp<SkImage> gainmapImage;
    SkRect gainmapImageRect;
    {
        SkImageInfo info = SkImageInfo::Make(2, 2, kRGBA_F32_SkColorType, kPremul_SkAlphaType);
        SkBitmap bm;
        if (!bm.tryAllocPixels(info)) {
            ERRORF(r, "Failed to allocate gainmap pixels");
            return;
        }
        bm.eraseColor(SK_ColorTRANSPARENT);
        write_pixel(bm, 1, 0, gainmapColors[0][0]);
        write_pixel(bm, 1, 1, gainmapColors[1][0]);
        gainmapImage = SkImage::MakeFromBitmap(bm);
        gainmapImageRect = SkRect::MakeXYWH(1.f, 0.f, 1.f, 2.f);
    }

    SkImageInfo canvasInfo = SkImageInfo::Make(
            4, 6, kRGBA_F32_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkBitmap canvasBitmap;
    SkRect canvasRect;
    {
        if (!canvasBitmap.tryAllocPixels(canvasInfo)) {
            ERRORF(r, "Failed to allocate canvas pixels");
            return;
        }
        canvasBitmap.eraseColor(SK_ColorTRANSPARENT);
        canvasRect = SkRect::MakeXYWH(1.f, 1.f, 2.f, 4.f);
    }

    SkGainmapInfo gainmapInfo;
    {
        sk_sp<SkShader> shader = SkDrawImageWithGainmapShader(sdrImage,
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

    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 2; ++x) {
            auto color = canvasBitmap.getColor4f(x + 1, y + 1);
            REPORTER_ASSERT(r, approx_equal(color, expectedColors[y][x]));
        }
    }
}
