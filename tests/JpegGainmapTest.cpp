/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkGainmapInfo.h"
#include "src/codec/SkJpegCodec.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/codec/SkJpegSourceMgr.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

namespace {

// A test stream to stress the different SkJpegSourceMgr sub-classes.
class TestStream : public SkStream {
public:
    enum class Type {
        kUnseekable,   // SkJpegUnseekableSourceMgr
        kSeekable,     // SkJpegBufferedSourceMgr
        kMemoryMapped, // SkJpegMemorySourceMgr
    };
    TestStream(Type type, SkStream* stream)
            : fStream(stream)
            , fSeekable(type != Type::kUnseekable)
            , fMemoryMapped(type == Type::kMemoryMapped) {}
    ~TestStream() override {}

    size_t read(void* buffer, size_t size) override { return fStream->read(buffer, size); }
    size_t peek(void* buffer, size_t size) const override { return fStream->peek(buffer, size); }
    bool isAtEnd() const override { return fStream->isAtEnd(); }
    bool rewind() override {
        if (!fSeekable) {
            return false;
        }
        return fStream->rewind();
    }
    bool hasPosition() const override {
        if (!fSeekable) {
            return false;
        }
        return fStream->hasPosition();
    }
    size_t getPosition() const override {
        if (!fSeekable) {
            return 0;
        }
        return fStream->hasPosition();
    }
    bool seek(size_t position) override {
        if (!fSeekable) {
            return 0;
        }
        return fStream->seek(position);
    }
    bool move(long offset) override {
        if (!fSeekable) {
            return 0;
        }
        return fStream->move(offset);
    }
    bool hasLength() const override {
        if (!fMemoryMapped) {
            return false;
        }
        return fStream->hasLength();
    }
    size_t getLength() const override {
        if (!fMemoryMapped) {
            return 0;
        }
        return fStream->getLength();
    }
    const void* getMemoryBase() override {
        if (!fMemoryMapped) {
            return nullptr;
        }
        return fStream->getMemoryBase();
    }

private:
    SkStream* const fStream;
    bool fSeekable = false;
    bool fMemoryMapped = false;
};

}  // namespace

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

        // Scan all the way to EndOfImage.
        auto sourceMgr = SkJpegSourceMgr::Make(stream.get());
        const auto& segments = sourceMgr->getAllSegments();

        // Verify we got the expected number of segments at EndOfImage
        REPORTER_ASSERT(r, rec.eoiSegmentCount == segments.size());

        // Verify we got the expected number of segments before StartOfScan
        for (size_t i = 0; i < segments.size(); ++i) {
            if (segments[i].marker == SkJpegSegmentScanner::kMarkerStartOfScan) {
                REPORTER_ASSERT(r, rec.sosSegmentCount == i + 1);
                break;
            }
        }

        // Verify the values for a randomly pre-selected segment index.
        const auto& segment = segments[rec.testSegmentIndex];
        REPORTER_ASSERT(r, rec.testSegmentMarker == segment.marker);
        REPORTER_ASSERT(r, rec.testSegmentOffset == segment.offset);
        REPORTER_ASSERT(r, rec.testSegmentParameterLength == segment.parameterLength);
    }
}

DEF_TEST(Codec_jpegMultiPicture, r) {
    const char* path = "images/iphone_13_pro.jpeg";
    auto stream = GetResourceAsStream(path);
    REPORTER_ASSERT(r, stream);

    // Search and parse the MPF header.
    std::unique_ptr<SkJpegMultiPictureParameters> mpParams;
    {
        auto sourceMgr = SkJpegSourceMgr::Make(stream.get());
        for (const auto& segment : sourceMgr->getAllSegments()) {
            static constexpr uint32_t kMpfMarker = 0xE2;
            static constexpr uint8_t kMpfSig[] = {'M', 'P', 'F', '\0'};
            if (segment.marker != kMpfMarker) {
                continue;
            }
            auto parameterData = sourceMgr->copyParameters(segment, kMpfSig, sizeof(kMpfSig));
            if (!parameterData) {
                continue;
            }
            mpParams = SkJpegParseMultiPicture(parameterData);
        }
    }
    REPORTER_ASSERT(r, mpParams);

    TestStream::Type kStreamTypes[] = {
            TestStream::Type::kUnseekable,
            TestStream::Type::kSeekable,
            TestStream::Type::kMemoryMapped,
    };
    bool kSkipFirstImageValues[] = {
            false,
            true,
    };
    for (const bool skipFirstImage : kSkipFirstImageValues) {
        SkJpegMultiPictureParameters testMpParams = *mpParams;
        if (skipFirstImage) {
            testMpParams.images[1].size = 0;
            testMpParams.images[1].dataOffset = 0;
        }
        for (const auto& streamType : kStreamTypes) {
            stream->rewind();
            TestStream testStream(streamType, stream.get());

            auto sourceMgr = SkJpegSourceMgr::Make(&testStream);

            // Extract the streams for the MultiPicture images.
            auto mpStreams = SkJpegExtractMultiPictureStreams(&testMpParams, sourceMgr.get());
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
                if (i == 1 && skipFirstImage) {
                    REPORTER_ASSERT(r, !imageStream);
                    continue;
                }
                REPORTER_ASSERT(r, imageStream);

                std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(imageStream));
                REPORTER_ASSERT(r, codec);

                SkBitmap bm;
                bm.allocPixels(codec->getInfo());
                REPORTER_ASSERT(r,
                                SkCodec::kSuccess ==
                                        codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes()));
                bitmaps[i] = bm;
            }

            // Spot-check the image size and pixels.
            if (!skipFirstImage) {
                REPORTER_ASSERT(r, bitmaps[1].dimensions() == SkISize::Make(1512, 2016));
                REPORTER_ASSERT(r, bitmaps[1].getColor(0, 0) == 0xFF3B3B3B);
                REPORTER_ASSERT(r, bitmaps[1].getColor(1511, 2015) == 0xFF101010);
            }
            REPORTER_ASSERT(r, bitmaps[2].dimensions() == SkISize::Make(576, 768));
            REPORTER_ASSERT(r, bitmaps[2].getColor(0, 0) == 0xFF010101);
            REPORTER_ASSERT(r, bitmaps[2].getColor(575, 767) == 0xFFB5B5B5);
        }
    }
}

DEF_TEST(AndroidCodec_jpegGainmap, r) {
    const struct Rec {
        const char* path;
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
             SkISize::Make(1512, 2016),
             0xFF3B3B3B,
             0xFF101010,
             0.f,
             1.f,
             1.f,
             2.71828f,
             SkGainmapInfo::Type::kMultiPicture},
            {"images/jpegr.jpg",
             SkISize::Make(1008, 756),
             0xFFCACACA,
             0xFFC8C8C8,
             -2.3669f,
             2.3669f,
             1.f,
             10.6643f,
             SkGainmapInfo::Type::kJpegR_HLG},
            {"images/hdrgm.jpg",
             SkISize::Make(188, 250),
             0xFFE9E9E9,
             0xFFAAAAAA,
             -2.209409f,
             2.209409f,
             1.f,
             9.110335f,
             SkGainmapInfo::Type::kHDRGM},
    };

    TestStream::Type kStreamTypes[] = {
            TestStream::Type::kUnseekable,
            TestStream::Type::kSeekable,
            TestStream::Type::kMemoryMapped,
    };
    for (const auto& streamType : kStreamTypes) {
        bool useFileStream = streamType != TestStream::Type::kMemoryMapped;
        for (const auto& rec : recs) {
            auto stream = GetResourceAsStream(rec.path, useFileStream);
            REPORTER_ASSERT(r, stream);
            auto testStream = std::make_unique<TestStream>(streamType, stream.get());

            SkCodec::Result result = SkCodec::kSuccess;
            std::unique_ptr<SkCodec> codec =
                    SkJpegCodec::MakeFromStream(std::move(testStream), &result);
            REPORTER_ASSERT(r, codec);

            std::unique_ptr<SkAndroidCodec> androidCodec =
                    SkAndroidCodec::MakeFromCodec(std::move(codec));
            REPORTER_ASSERT(r, androidCodec);

            SkGainmapInfo gainmapInfo;
            std::unique_ptr<SkStream> gainmapStream;
            REPORTER_ASSERT(r, androidCodec->getAndroidGainmap(&gainmapInfo, &gainmapStream));
            REPORTER_ASSERT(r, gainmapStream);

            std::unique_ptr<SkCodec> gainmapCodec =
                    SkCodec::MakeFromStream(std::move(gainmapStream));
            REPORTER_ASSERT(r, gainmapCodec);

            SkBitmap bm;
            bm.allocPixels(gainmapCodec->getInfo());
            REPORTER_ASSERT(r,
                            SkCodec::kSuccess == gainmapCodec->getPixels(
                                                         bm.info(), bm.getPixels(), bm.rowBytes()));

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
}
