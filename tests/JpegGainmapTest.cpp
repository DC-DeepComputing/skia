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
#include "include/core/SkImageEncoder.h"
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

skcms_TransferFunction trfn_apply_gain(const skcms_TransferFunction trfn, float gain) {
    float pow_gain_ginv = sk_float_pow(gain, 1/trfn.g);
    skcms_TransferFunction result;
    result.g = trfn.g;
    result.a = trfn.a * pow_gain_ginv;
    result.b = trfn.b * pow_gain_ginv;
    result.c = trfn.c * gain;
    result.d = trfn.d;
    result.e = trfn.e * gain;
    result.f = trfn.f * gain;
    return result;
}

// Create an sRGB color space with a gain applied to reach the specified headroom.
sk_sp<SkColorSpace> get_gained_srgb_color_space(float headroom) {
    skcms_TransferFunction trfn = trfn_apply_gain(SkNamedTransferFn::kSRGB, headroom);
    return SkColorSpace::MakeRGB(trfn, SkNamedGamut::kSRGB);
}

void save_bitmap(const SkBitmap& bitmap, const char* fname) {
    auto data = SkEncodeBitmap(bitmap, SkEncodedImageFormat::kJPEG, 80);
    FILE* fout = fopen(fname, "wb");
    fwrite(data->data(), data->size(), 1, fout);
    fclose(fout);
}

DEF_TEST(AndroidCodec_jpegGainmap, r) {
    const struct Rec {
        const char* path;
        const char* name;
        SkISize dimensions;
        SkColor originColor;
        SkColor farCornerColor;
        float logRatioMin;
        float logRatioMax;
        float hdrRatioMin;
        float hdrRatioMax;
        SkGainmapInfo::Type type;
    } recs[] = {
            {"images/iphone_13_pro.jpeg",
             "mpf",
             SkISize::Make(1512, 2016),
             0xFF3B3B3B,
             0xFF101010,
             0.f,
             1.f,
             1.f,
             2.71828f,
             SkGainmapInfo::Type::kMultiPicture},
            {"images/jpegr.jpg",
             "jpegr",
             SkISize::Make(1008, 756),
             0xFFCACACA,
             0xFFC8C8C8,
             -2.3669f,
             2.3669f,
             1.f,
             10.6643f,
             SkGainmapInfo::Type::kJpegR_HLG},
    };

    for (const auto& rec : recs) {
        auto stream = GetResourceAsStream(rec.path);
        REPORTER_ASSERT(r, stream);

        std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
        REPORTER_ASSERT(r, codec);

        sk_sp<SkImage> baseImage;
        {
            SkBitmap baseBitmap;
            baseBitmap.allocPixels(codec->getInfo());
            REPORTER_ASSERT(r,
                            SkCodec::kSuccess == codec->getPixels(baseBitmap.info(),
                                                                  baseBitmap.getPixels(),
                                                                  baseBitmap.rowBytes()));
            baseImage = SkImage::MakeFromBitmap(baseBitmap);

            // Dump the base image.
            char fname[256];
            sprintf(fname, "base-%s.jpg", rec.name);
            save_bitmap(baseBitmap, fname);
        }

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

        sk_sp<SkImage> gainmapImage;
        {
            SkBitmap gainmapBitmap;
            gainmapBitmap.allocPixels(gainmapCodec->getInfo());
            REPORTER_ASSERT(r,
                            SkCodec::kSuccess == gainmapCodec->getPixels(gainmapBitmap.info(),
                                                                         gainmapBitmap.getPixels(),
                                                                         gainmapBitmap.rowBytes()));
            gainmapImage = SkImage::MakeFromBitmap(gainmapBitmap);

            // Dump the gainmap image.
            char fname[256];
            sprintf(fname, "gainmap-%s.jpg", rec.name);
            save_bitmap(gainmapBitmap, fname);
        }

        // Draw the image with the specified HDR ratio.
        {
            const float displayHdrRatio = 6.f;

            // The canvas will be "extended range" 8888 -- a 2.2 gamma that can reach
            // displayHdrRatio.
            SkImageInfo canvasInfo = SkImageInfo::Make(baseImage->dimensions(),
                                                       kN32_SkColorType,
                                                       kPremul_SkAlphaType,
                                                       get_gained_srgb_color_space(displayHdrRatio));
            SkBitmap canvasBitmap;
            canvasBitmap.allocPixels(canvasInfo);
            canvasBitmap.eraseColor(SK_ColorTRANSPARENT);
            SkCanvas canvas(canvasBitmap);

            SkRect dstRect = SkRect::Make(canvasInfo.dimensions());

            // Draw the image with all the parameters.
            {
                sk_sp<SkShader> shader = SkGainmapShader::Make(baseImage,
                                                               SkRect::Make(baseImage->dimensions()),
                                                               SkSamplingOptions(),
                                                               gainmapImage,
                                                               SkRect::Make(gainmapImage->dimensions()),
                                                               SkSamplingOptions(),
                                                               gainmapInfo,
                                                               dstRect,
                                                               displayHdrRatio,
                                                               canvasInfo.refColorSpace());
                SkPaint paint;
                paint.setShader(shader);
                canvas.drawRect(dstRect, paint);
            }

            {
                SkPaint paint;

                paint.setColor4f({1.f, 1.f, 1.f, 1.f}, SkColorSpace::MakeSRGBLinear().get());
                dstRect = SkRect::MakeXYWH(dstRect.x(), dstRect.y(), 300, 300);
                canvas.drawRect(dstRect, paint);

                paint.setColor4f({2.f, 2.f, 2.f, 1.f}, SkColorSpace::MakeSRGBLinear().get());
                dstRect = SkRect::MakeXYWH(dstRect.x(), dstRect.y(), 150, 150);
                canvas.drawRect(dstRect, paint);

                paint.setColor4f({4.f, 4.f, 4.f, 1.f}, SkColorSpace::MakeSRGBLinear().get());
                dstRect = SkRect::MakeXYWH(dstRect.x(), dstRect.y(), 100, 100);
                canvas.drawRect(dstRect, paint);

                paint.setColor4f({8.f, 8.f, 8.f, 1.f}, SkColorSpace::MakeSRGBLinear().get());
                dstRect = SkRect::MakeXYWH(dstRect.x(), dstRect.y(), 50, 50);
                canvas.drawRect(dstRect, paint);
            }


            // And save it.
            char fname[256];
            sprintf(fname, "out-%s-%1.02f.jpg", rec.name, displayHdrRatio);
            save_bitmap(canvasBitmap, fname);

            // Save it with the gain removed.
            SkPixmap ungainedPixmap = canvasBitmap.pixmap();
            ungainedPixmap.setColorSpace(SkColorSpace::MakeSRGB());
            SkBitmap ungainedBitmap;
            ungainedBitmap.installPixels(ungainedPixmap);
            sprintf(fname, "out-%s-%1.02f-without-gain.jpg", rec.name, displayHdrRatio);
            save_bitmap(ungainedBitmap, fname);
        }
        continue;

        // Spot-check the image size and pixels.
        REPORTER_ASSERT(r, bm.dimensions() == rec.dimensions);
        REPORTER_ASSERT(r, bm.getColor(0, 0) == rec.originColor);
        REPORTER_ASSERT(r,
                        bm.getColor(rec.dimensions.fWidth - 1, rec.dimensions.fHeight - 1) ==
                                rec.farCornerColor);

        // Verify the gainmap rendering parameters.
        auto approxEq = [=](float x, float y) { return std::abs(x - y) < 1e-3f; };

        REPORTER_ASSERT(r, approxEq(gainmapInfo.fLogRatioMin.fR, rec.logRatioMin));
        REPORTER_ASSERT(r, approxEq(gainmapInfo.fLogRatioMin.fG, rec.logRatioMin));
        REPORTER_ASSERT(r, approxEq(gainmapInfo.fLogRatioMin.fB, rec.logRatioMin));

        REPORTER_ASSERT(r, approxEq(gainmapInfo.fLogRatioMax.fR, rec.logRatioMax));
        REPORTER_ASSERT(r, approxEq(gainmapInfo.fLogRatioMax.fG, rec.logRatioMax));
        REPORTER_ASSERT(r, approxEq(gainmapInfo.fLogRatioMax.fB, rec.logRatioMax));

        REPORTER_ASSERT(r, approxEq(gainmapInfo.fHdrRatioMin, rec.hdrRatioMin));
        REPORTER_ASSERT(r, approxEq(gainmapInfo.fHdrRatioMax, rec.hdrRatioMax));

        REPORTER_ASSERT(r, gainmapInfo.fType == rec.type);
    }
}
