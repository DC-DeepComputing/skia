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
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkGainmapShader.h"
#include "include/private/SkJpegGainmapEncoder.h"
#include "src/codec/SkJpegCodec.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/codec/SkJpegSourceMgr.h"
#include "src/codec/SkJpegXmp.h"
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
        kUnseekable,    // SkJpegUnseekableSourceMgr
        kSeekable,      // SkJpegBufferedSourceMgr
        kMemoryMapped,  // SkJpegMemorySourceMgr
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
            if (segments[i].marker == kJpegMarkerStartOfScan) {
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

static bool find_mp_params_segment(SkStream* stream,
                                   std::unique_ptr<SkJpegMultiPictureParameters>* outMpParams,
                                   SkJpegSegment* outMpParamsSegment) {
    auto sourceMgr = SkJpegSourceMgr::Make(stream);
    for (const auto& segment : sourceMgr->getAllSegments()) {
        if (segment.marker != kMpfMarker) {
            continue;
        }
        auto parameterData = sourceMgr->getSegmentParameters(segment);
        if (!parameterData) {
            continue;
        }
        *outMpParams = SkJpegMultiPictureParameters::Make(parameterData);
        if (*outMpParams) {
            *outMpParamsSegment = segment;
            return true;
        }
    }
    return false;
}

DEF_TEST(Codec_massTranscode, r) {
    const struct Rec {
        const char* base;
        const char* gainmap;
        float logratioMin;
        float logratioMax;
        float hdrMin;
        float hdrMax;
        float epsilonSdr;
        float epsilonHdr;
    } recs[] = {
  {"new/XXXX_20220124_142949_982_final", "new/XXXX_20220124_142949_982_final-gainmap", -0.5761, 4.7091, 1, 26.796875, 0.015625, 0.015625, },
  {"new/XXXX_20201124_215019_825_final", "new/XXXX_20201124_215019_825_final-gainmap", -1.1045, 4.6532, 1, 26.203125, 0.015625, 0.015625, },
  {"new/XXXX_20221005_211633_682_final", "new/XXXX_20221005_211633_682_final-gainmap", -0.1961, 4.5452, 1, 22.859375, 0.015625, 0.015625, },
  {"personal/fuoco",                  "personal/fuoco-g",                         0.0000, 1.4427, 1, -1, 0.015625, 0.015625, },
  {"personal/pescariello04",          "personal/pescariello04-g",                 0.0000, 1.4427, 1, -1, 0.015625, 0.015625, },
  {"personal/salerno02",              "personal/salerno02-g",                     0.0000, 1.4427, 1, -1, 0.015625, 0.015625, },
  {"personal/salerno01",              "personal/salerno01-g",                     0.0000, 1.4427, 1, -1, 0.015625, 0.015625, },
  {"personal/salerno00",              "personal/salerno00-g",                     0.0000, 1.4427, 1, -1, 0.015625, 0.015625, },
  {"personal/trattore0",              "personal/trattore0-g",                     0.0000, 1.4427, 1, -1, 0.015625, 0.015625, },
  {"personal/trattore1",              "personal/trattore1-g",                     0.0000, 1.4427, 1, -1, 0.015625, 0.015625, },
  {"personal/conigli",                "personal/conigli-g",                       0.0000, 1.4427, 1, -1, 0.015625, 0.015625, },
  {"export/XXXX_20220630_024551_651", "export/XXXX_20220630_024551_651-gainmap", -0.6004, 2.9713, 1, 8.265625, 0.015625, 0.015625, },
  {"export/XXXX_20220630_002404_950", "export/XXXX_20220630_002404_950-gainmap", -2.0920, 2.9859, 1, 7.96875, 0.015625, 0.015625, },
  {"export/XXXX_20220630_002137_756", "export/XXXX_20220630_002137_756-gainmap", -1.0789, 3.7703, 1, 14.09375, 0.015625, 0.015625, },
  {"export/XXXX_20220629_213204_938", "export/XXXX_20220629_213204_938-gainmap", -0.8783, 2.9065, 1, 7.63671875, 0.015625, 0.015625, },
  {"export/XXXX_20220629_191252_036", "export/XXXX_20220629_191252_036-gainmap", -0.8186, 3.3632, 1, 10.765625, 0.015625, 0.015625, },
  {"export/XXXX_20220629_171926_835", "export/XXXX_20220629_171926_835-gainmap", -0.6750, 3.5421, 1, 12.359375, 0.015625, 0.015625, },
  {"export/XXXX_20220625_052117_681", "export/XXXX_20220625_052117_681-gainmap", -0.4852, 2.7477, 1, 6.79296875, 0.015625, 0.015625, },
  {"export/XXXX_20220625_051341_845", "export/XXXX_20220625_051341_845-gainmap", -1.0269, 2.7744, 1, 7.22265625, 0.015625, 0.015625, },
  {"export/XXXX_20220625_022257_767", "export/XXXX_20220625_022257_767-gainmap", -2.1208, 3.2575, 1, 8.453125, 0.015625, 0.015625, },
  {"export/XXXX_20220615_110943_423", "export/XXXX_20220615_110943_423-gainmap", -1.3406, 3.8409, 1, 15.0703125, 0.015625, 0.015625, },
  {"export/XXXX_20220615_105423_160", "export/XXXX_20220615_105423_160-gainmap", -1.2008, 3.2805, 1, 9.890625, 0.015625, 0.015625, },
  {"export/XXXX_20220512_234713_367", "export/XXXX_20220512_234713_367-gainmap", -2.6119, 2.2894, 1, 3.974609375, 0.015625, 0.015625, },
  {"export/XXXX_20220314_130549_539", "export/XXXX_20220314_130549_539-gainmap", -0.8376, 3.4176, 1, 15.96875, 0.015625, 0.015625, },
  {"export/XXXX_20220309_204648_807", "export/XXXX_20220309_204648_807-gainmap", -1.2540, 2.9122, 1, 8.015625, 0.015625, 0.015625, },
  {"export/XXXX_20220309_204526_146", "export/XXXX_20220309_204526_146-gainmap", -1.1994, 2.2764, 1, 4.609375, 0.015625, 0.015625, },
  {"export/XXXX_20220308_224912_678", "export/XXXX_20220308_224912_678-gainmap", -1.8087, 3.2942, 1, 7.51171875, 0.015625, 0.015625, },
  {"export/XXXX_20220302_031303_284", "export/XXXX_20220302_031303_284-gainmap", -1.7954, 3.6480, 1, 10.5625, 0.015625, 0.015625, },
  {"export/XXXX_20220301_021039_607", "export/XXXX_20220301_021039_607-gainmap", -0.8739, 2.6521, 1, 6.88671875, 0.015625, 0.015625, },
  {"export/XXXX_20220223_214338_595", "export/XXXX_20220223_214338_595-gainmap", -0.4049, 2.6167, 1, 6.2109375, 0.015625, 0.015625, },
  {"export/XXXX_20220222_004410_905", "export/XXXX_20220222_004410_905-gainmap", -0.6448, 1.3670, 1, 2.626953125, 0.015625, 0.015625, },
  {"export/XXXX_20220222_000647_474", "export/XXXX_20220222_000647_474-gainmap", -0.7731, 2.4213, 1, 4.65625, 0.015625, 0.015625, },
  {"export/XXXX_20220221_201601_970", "export/XXXX_20220221_201601_970-gainmap", -2.0978, 3.2784, 1, 6.2421875, 0.015625, 0.015625, },
  {"export/XXXX_20220220_210402_167", "export/XXXX_20220220_210402_167-gainmap", -2.5298, 2.2189, 1, 3.46875, 0.015625, 0.015625, },
  {"export/XXXX_20220220_195603_814", "export/XXXX_20220220_195603_814-gainmap", -2.2051, 2.9563, 1, 6.578125, 0.015625, 0.015625, },
  {"export/XXXX_20220219_231124_267", "export/XXXX_20220219_231124_267-gainmap", -0.9345, 2.8468, 1, 7.54296875, 0.015625, 0.015625, },
  {"export/XXXX_20220204_183435_167", "export/XXXX_20220204_183435_167-gainmap", -0.7051, 2.7274, 1, 6.74609375, 0.015625, 0.015625, },
  {"export/XXXX_20220204_165427_219", "export/XXXX_20220204_165427_219-gainmap", -0.6678, 2.6069, 1, 6.2109375, 0.015625, 0.015625, },
  {"export/XXXX_20220204_140738_092", "export/XXXX_20220204_140738_092-gainmap", -0.8353, 3.4030, 1, 11.015625, 0.015625, 0.015625, },
  {"export/XXXX_20220204_004940_440", "export/XXXX_20220204_004940_440-gainmap", -0.6577, 3.1061, 1, 9.03125, 0.015625, 0.015625, },
  {"export/XXXX_20220203_191714_442", "export/XXXX_20220203_191714_442-gainmap", -0.7268, 2.6061, 1, 6.17578125, 0.015625, 0.015625, },
  {"export/XXXX_20220203_183756_582", "export/XXXX_20220203_183756_582-gainmap", -0.5215, 3.9608, 1, 16.234375, 0.015625, 0.015625, },
  {"export/XXXX_20220202_200628_697", "export/XXXX_20220202_200628_697-gainmap", -2.2002, 3.1369, 1, 7.640625, 0.015625, 0.015625, },
  {"export/XXXX_20220202_190616_477", "export/XXXX_20220202_190616_477-gainmap", -0.6087, 2.8879, 1, 8.2421875, 0.015625, 0.015625, },
  {"export/XXXX_20220128_203657_193", "export/XXXX_20220128_203657_193-gainmap", -0.6593, 2.8160, 1, 7.6484375, 0.015625, 0.015625, },
  {"export/XXXX_20220128_201013_864", "export/XXXX_20220128_201013_864-gainmap", -0.7794, 2.9000, 1, 7.87109375, 0.015625, 0.015625, },
  {"export/XXXX_20220120_054656_367", "export/XXXX_20220120_054656_367-gainmap", -2.5616, 3.3973, 1, 8.1171875, 0.015625, 0.015625, },
  {"export/XXXX_20220120_041934_556", "export/XXXX_20220120_041934_556-gainmap", -1.7306, 2.9674, 1, 7.515625, 0.015625, 0.015625, },
  {"export/5066_20160413_204559_095", "export/5066_20160413_204559_095-gainmap", -0.7878, 3.5823, 1, 13.2265625, 0.015625, 0.015625, },
  {"export/020E_20180503_190546_154", "export/020E_20180503_190546_154-gainmap", -0.8433, 2.7848, 1, 12.1015625, 0.015625, 0.015625, },
  {"export/0LBZ_20180902_145636_775", "export/0LBZ_20180902_145636_775-gainmap", -0.9237, 2.4004, 1, 6.78515625, 0.015625, 0.015625, },
  {"export/XXXX_20220128_161408_826", "export/XXXX_20220128_161408_826-gainmap", -0.6390, 4.0659, 1, 17.8125, 0.015625, 0.015625, },
  {"new/XXXX_20220510_215110_619_final", "new/XXXX_20220510_215110_619_final-gainmap", -0.5759, 5.2160, 1, 43.1875, 0.015625, 0.015625, },
  {"new/XXXX_20201108_192227_492_final", "new/XXXX_20201108_192227_492_final-gainmap", -0.7604, 4.5103, 1, 26.859375, 0.015625, 0.015625, },
  {"new/XXXX_20201014_193707_751_final", "new/XXXX_20201014_193707_751_final-gainmap", -0.5887, 4.7109, 1, 27.21875, 0.015625, 0.015625, },
  {"new/XXXX_20200707_120115_232_final", "new/XXXX_20200707_120115_232_final-gainmap", -0.7066, 4.6813, 1, 26.21875, 0.015625, 0.015625, },
  {"new/XXXX_20190814_143648_113_final", "new/XXXX_20190814_143648_113_final-gainmap", -0.9981, 4.6832, 1, 26.28125, 0.015625, 0.015625, },
  {"new/2769_20171211_144118_688_final", "new/2769_20171211_144118_688_final-gainmap", -0.5473, 4.6963, 1, 25.109375, 0.015625, 0.015625, },
  {"export/0FF7_20180813_212359_039", "export/0FF7_20180813_212359_039-gainmap", -0.9453, 2.7254, 1, 7.55078125, 0.015625, 0.015625, },
  {"XXXX_20220625_184938_987", "XXXX_20220625_184938_987-gainmap", -1.93, 3.0185, 1, 7.99609375, 0.015625, 0.015625, },
  {"XXXX_20220221_201550_252", "XXXX_20220221_201550_252-gainmap", -0.8177, 2.6247, 1, 6.00390625, 0.015625, 0.015625, },
  {"KMBB_20181031_192304_835", "KMBB_20181031_192304_835-gainmap", -0.3441, 3.0690, 1, 7.99609375, 0.015625, 0.015625, },
  {"final", "final-gainmap", -0.7581, 2.3769, 1, 5.25, 0.015625, 0.015625, }};

  for (const auto& rec :recs) {
      printf("%s\n", rec.base);
      const std::string prefix = "gainmap/";
      const std::string suffix = ".jpg";
      auto base = GetResourceAsData((prefix + rec.base + suffix).c_str());
      auto gainmap = GetResourceAsData((prefix + rec.gainmap + suffix).c_str());
      const std::string out = "resources/" + prefix + rec.base + "-compound" + suffix;
        REPORTER_ASSERT(r, base);
        REPORTER_ASSERT(r, gainmap);

{
    std::unique_ptr<SkCodec> gainmapCodec = SkCodec::MakeFromStream(SkMemoryStream::Make(gainmap));
    REPORTER_ASSERT(r, gainmapCodec);
    SkBitmap bm;
    bm.allocPixels(gainmapCodec->getInfo());
    SkBitmap gainmapBitmap;
    gainmapBitmap.allocPixels(gainmapCodec->getInfo());
    REPORTER_ASSERT(r,
                    SkCodec::kSuccess == gainmapCodec->getPixels(gainmapBitmap.info(),
                                                                 gainmapBitmap.getPixels(),
                                                                 gainmapBitmap.rowBytes()));

    SkBitmap monoBitmap;
    monoBitmap.allocPixels(gainmapCodec->getInfo().makeColorType(kR8_unorm_SkColorType));
    REPORTER_ASSERT(r, gainmapBitmap.readPixels(monoBitmap.pixmap(), 0, 0));
    

    SkDynamicMemoryWStream encodeStream;
    SkJpegEncoder::Options options;
    SkJpegEncoder::Encode(&encodeStream, monoBitmap.pixmap(), options);
    gainmap = encodeStream.detachAsData();
    REPORTER_ASSERT(r, gainmap);
    REPORTER_ASSERT(r, gainmap->size());

      const std::string gout = "resources/" + prefix + rec.gainmap + "-mono" + suffix;
        SkFILEWStream s(gout.c_str());
        s.write(gainmap->data(), gainmap->size());
}

        const float kLog2 = sk_float_log(2.f);
        SkGainmapInfo info;
        info.fGainmapRatioMin.fR = 
        info.fGainmapRatioMin.fG = 
        info.fGainmapRatioMin.fB = sk_float_exp(rec.logratioMin * kLog2);

        info.fGainmapRatioMax.fR = 
        info.fGainmapRatioMax.fG = 
        info.fGainmapRatioMax.fB = sk_float_exp(rec.logratioMax * kLog2);

        info.fEpsilonSdr.fR = 
        info.fEpsilonSdr.fG = 
        info.fEpsilonSdr.fB = rec.epsilonSdr;

        info.fEpsilonHdr.fR = 
        info.fEpsilonHdr.fG = 
        info.fEpsilonHdr.fB = rec.epsilonHdr;

        info.fDisplayRatioSdr = 1.f;
        info.fDisplayRatioHdr = info.fGainmapRatioMax.fR;

        SkFILEWStream encodeStream(out.c_str());

            SkJpegGainmapEncoder::Encode(&encodeStream,
info,
                                                        base.get(),
                                                        gainmap.get());

    }
}

DEF_TEST(Codec_multiPictureParams, r) {
    // Little-endian test.
    {
        const uint8_t bytes[] = {
                0x4d, 0x50, 0x46, 0x00, 0x49, 0x49, 0x2a, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03,
                0x00, 0x00, 0xb0, 0x07, 0x00, 0x04, 0x00, 0x00, 0x00, 0x30, 0x31, 0x30, 0x30,
                0x01, 0xb0, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
                0xb0, 0x07, 0x00, 0x20, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x20, 0xcf, 0x49, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xee, 0x28, 0x01, 0x00,
                0xf9, 0xb7, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00,
        };
        auto mpParams =
                SkJpegMultiPictureParameters::Make(SkData::MakeWithoutCopy(bytes, sizeof(bytes)));
        REPORTER_ASSERT(r, mpParams);
        REPORTER_ASSERT(r, mpParams->images.size() == 2);
        REPORTER_ASSERT(r, mpParams->images[0].dataOffset == 0);
        REPORTER_ASSERT(r, mpParams->images[0].size == 4837152);
        REPORTER_ASSERT(r, mpParams->images[1].dataOffset == 3979257);
        REPORTER_ASSERT(r, mpParams->images[1].size == 76014);
    }

    // Big-endian test.
    {
        const uint8_t bytes[] = {
                0x4d, 0x50, 0x46, 0x00, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08, 0x00,
                0x03, 0xb0, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x30, 0x31, 0x30, 0x30,
                0xb0, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xb0,
                0x02, 0x00, 0x07, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00,
                0x00, 0x00, 0x20, 0x03, 0x00, 0x00, 0x00, 0x56, 0xda, 0x2f, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xc6, 0x01,
                0x00, 0x55, 0x7c, 0x1f, 0x00, 0x00, 0x00, 0x00,
        };
        auto mpParams =
                SkJpegMultiPictureParameters::Make(SkData::MakeWithoutCopy(bytes, sizeof(bytes)));
        REPORTER_ASSERT(r, mpParams);
        REPORTER_ASSERT(r, mpParams->images.size() == 2);
        REPORTER_ASSERT(r, mpParams->images[0].dataOffset == 0);
        REPORTER_ASSERT(r, mpParams->images[0].size == 5691951);
        REPORTER_ASSERT(r, mpParams->images[1].dataOffset == 5602335);
        REPORTER_ASSERT(r, mpParams->images[1].size == 1361409);
    }

    // Three entry test.
    {
        const uint8_t bytes[] = {
                0x4d, 0x50, 0x46, 0x00, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08, 0x00,
                0x03, 0xb0, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x30, 0x31, 0x30, 0x30,
                0xb0, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0xb0,
                0x02, 0x00, 0x07, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x1f, 0x1c, 0xc2, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0xb0,
                0x00, 0x1f, 0x12, 0xec, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x96, 0x6b, 0x00, 0x22, 0x18, 0x9c, 0x00, 0x00, 0x00, 0x00,
        };
        auto mpParams =
                SkJpegMultiPictureParameters::Make(SkData::MakeWithoutCopy(bytes, sizeof(bytes)));
        REPORTER_ASSERT(r, mpParams);
        REPORTER_ASSERT(r, mpParams->images.size() == 3);
        REPORTER_ASSERT(r, mpParams->images[0].dataOffset == 0);
        REPORTER_ASSERT(r, mpParams->images[0].size == 2038978);
        REPORTER_ASSERT(r, mpParams->images[1].dataOffset == 2036460);
        REPORTER_ASSERT(r, mpParams->images[1].size == 198064);
        REPORTER_ASSERT(r, mpParams->images[2].dataOffset == 2234524);
        REPORTER_ASSERT(r, mpParams->images[2].size == 38507);
    }
}

DEF_TEST(Codec_jpegMultiPicture, r) {
    const char* path = "images/iphone_13_pro.jpeg";
    auto stream = GetResourceAsStream(path);
    REPORTER_ASSERT(r, stream);

    // Search and parse the MPF header.
    std::unique_ptr<SkJpegMultiPictureParameters> mpParams;
    SkJpegSegment mpParamsSegment;
    REPORTER_ASSERT(r, find_mp_params_segment(stream.get(), &mpParams, &mpParamsSegment));

    // Verify that we get the same parameters when we re-serialize and de-serialize them
    {
        auto mpParamsSerialized = mpParams->serialize();
        REPORTER_ASSERT(r, mpParamsSerialized);
        auto mpParamsRoundTripped = SkJpegMultiPictureParameters::Make(mpParamsSerialized);
        REPORTER_ASSERT(r, mpParamsRoundTripped);
        REPORTER_ASSERT(r, mpParamsRoundTripped->images.size() == mpParams->images.size());
        for (size_t i = 0; i < mpParamsRoundTripped->images.size(); ++i) {
            REPORTER_ASSERT(r, mpParamsRoundTripped->images[i].size == mpParams->images[i].size);
            REPORTER_ASSERT(
                    r,
                    mpParamsRoundTripped->images[i].dataOffset == mpParams->images[i].dataOffset);
        }
    }

    const struct Rec {
        const TestStream::Type streamType;
        const bool skipFirstImage;
        const size_t bufferSize;
    } recs[] = {
            {TestStream::Type::kMemoryMapped, false, 1024},
            {TestStream::Type::kMemoryMapped, true, 1024},
            {TestStream::Type::kSeekable, false, 1024},
            {TestStream::Type::kSeekable, true, 1024},
            {TestStream::Type::kSeekable, false, 7},
            {TestStream::Type::kSeekable, true, 13},
            {TestStream::Type::kSeekable, true, 1024 * 1024 * 16},
            {TestStream::Type::kUnseekable, false, 1024},
            {TestStream::Type::kUnseekable, true, 1024},
            {TestStream::Type::kUnseekable, false, 1},
            {TestStream::Type::kUnseekable, true, 1},
            {TestStream::Type::kUnseekable, false, 7},
            {TestStream::Type::kUnseekable, true, 13},
            {TestStream::Type::kUnseekable, false, 1024 * 1024 * 16},
            {TestStream::Type::kUnseekable, true, 1024 * 1024 * 16},
    };
    for (const auto& rec : recs) {
        stream->rewind();
        TestStream testStream(rec.streamType, stream.get());
        auto sourceMgr = SkJpegSourceMgr::Make(&testStream, rec.bufferSize);

        // Decode the images into bitmaps.
        size_t numberOfImages = mpParams->images.size();
        std::vector<SkBitmap> bitmaps(numberOfImages);
        for (size_t i = 0; i < numberOfImages; ++i) {
            if (i == 0) {
                REPORTER_ASSERT(r, mpParams->images[i].dataOffset == 0);
                continue;
            }
            if (i == 1 && rec.skipFirstImage) {
                continue;
            }
            auto imageData = sourceMgr->getSubsetData(
                    SkJpegMultiPictureParameters::GetAbsoluteOffset(mpParams->images[i].dataOffset,
                                                                    mpParamsSegment.offset),
                    mpParams->images[i].size);
            REPORTER_ASSERT(r, imageData);

            std::unique_ptr<SkCodec> codec =
                    SkCodec::MakeFromStream(SkMemoryStream::Make(imageData));
            REPORTER_ASSERT(r, codec);

            SkBitmap bm;
            bm.allocPixels(codec->getInfo());
            REPORTER_ASSERT(r,
                            SkCodec::kSuccess ==
                                    codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes()));
            bitmaps[i] = bm;
        }

        // Spot-check the image size and pixels.
        if (!rec.skipFirstImage) {
            REPORTER_ASSERT(r, bitmaps[1].dimensions() == SkISize::Make(1512, 2016));
            REPORTER_ASSERT(r, bitmaps[1].getColor(0, 0) == 0xFF3B3B3B);
            REPORTER_ASSERT(r, bitmaps[1].getColor(1511, 2015) == 0xFF101010);
        }
        REPORTER_ASSERT(r, bitmaps[2].dimensions() == SkISize::Make(576, 768));
        REPORTER_ASSERT(r, bitmaps[2].getColor(0, 0) == 0xFF010101);
        REPORTER_ASSERT(r, bitmaps[2].getColor(575, 767) == 0xFFB5B5B5);
    }
}

// Decode an image and its gainmap.
template <typename Reporter>
void decode_all(Reporter& r,
                std::unique_ptr<SkStream> stream,
                SkBitmap& baseBitmap,
                SkBitmap& gainmapBitmap,
                SkGainmapInfo& gainmapInfo) {
    // Decode the base bitmap.
    SkCodec::Result result = SkCodec::kSuccess;
    std::unique_ptr<SkCodec> baseCodec = SkJpegCodec::MakeFromStream(std::move(stream), &result);
    REPORTER_ASSERT(r, baseCodec);
    baseBitmap.allocPixels(baseCodec->getInfo());
    REPORTER_ASSERT(r,
                    SkCodec::kSuccess == baseCodec->getPixels(baseBitmap.info(),
                                                              baseBitmap.getPixels(),
                                                              baseBitmap.rowBytes()));
    std::unique_ptr<SkAndroidCodec> androidCodec =
            SkAndroidCodec::MakeFromCodec(std::move(baseCodec));
    REPORTER_ASSERT(r, androidCodec);

    // Extract the gainmap info and stream.
    std::unique_ptr<SkStream> gainmapStream;
    REPORTER_ASSERT(r, androidCodec->getAndroidGainmap(&gainmapInfo, &gainmapStream));
    REPORTER_ASSERT(r, gainmapStream);

    // Decode the gainmap bitmap.
    std::unique_ptr<SkCodec> gainmapCodec = SkCodec::MakeFromStream(std::move(gainmapStream));
    REPORTER_ASSERT(r, gainmapCodec);
    SkBitmap bm;
    bm.allocPixels(gainmapCodec->getInfo());
    gainmapBitmap.allocPixels(gainmapCodec->getInfo());
    REPORTER_ASSERT(r,
                    SkCodec::kSuccess == gainmapCodec->getPixels(gainmapBitmap.info(),
                                                                 gainmapBitmap.getPixels(),
                                                                 gainmapBitmap.rowBytes()));
}

// Render an applied gainmap.
SkBitmap render_gainmap(const SkImageInfo& renderInfo,
                        float renderHdrRatio,
                        const SkBitmap& baseBitmap,
                        const SkBitmap& gainmapBitmap,
                        const SkGainmapInfo& gainmapInfo,
                        int x,
                        int y) {
    SkRect baseRect = SkRect::MakeXYWH(x, y, renderInfo.width(), renderInfo.height());

    float scaleX = gainmapBitmap.width() / static_cast<float>(baseBitmap.width());
    float scaleY = gainmapBitmap.height() / static_cast<float>(baseBitmap.height());
    SkRect gainmapRect = SkRect::MakeXYWH(baseRect.x() * scaleX,
                                          baseRect.y() * scaleY,
                                          baseRect.width() * scaleX,
                                          baseRect.height() * scaleY);

    SkRect dstRect = SkRect::Make(renderInfo.dimensions());

    sk_sp<SkImage> baseImage = SkImage::MakeFromBitmap(baseBitmap);
    sk_sp<SkImage> gainmapImage = SkImage::MakeFromBitmap(gainmapBitmap);
    sk_sp<SkShader> shader = SkGainmapShader::Make(baseImage,
                                                   baseRect,
                                                   SkSamplingOptions(),
                                                   gainmapImage,
                                                   gainmapRect,
                                                   SkSamplingOptions(),
                                                   gainmapInfo,
                                                   dstRect,
                                                   renderHdrRatio,
                                                   renderInfo.refColorSpace());

    SkBitmap result;
    result.allocPixels(renderInfo);
    result.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(result);

    SkPaint paint;
    paint.setShader(shader);
    canvas.drawRect(dstRect, paint);

    return result;
}

DEF_TEST(AndroidCodec_xmpHdrgmAsFieldValue, r) {
    // Expose HDRM values as fields. Also place the HDRGM namespace in the rdf:RDF node.
    const char xmpData[] =
            "http://ns.adobe.com/xap/1.0/\0"
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 6.0.0\">\n"
            "   <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
            "            xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\">\n"
            "      <rdf:Description rdf:about=\"\">\n"
            "         <hdrgm:Version>1.0</hdrgm:Version>\n"
            "         <hdrgm:GainMapMax>3</hdrgm:GainMapMax>\n"
            "         <hdrgm:HDRCapacityMax>4</hdrgm:HDRCapacityMax>\n"
            "      </rdf:Description>\n"
            "   </rdf:RDF>\n"
            "</x:xmpmeta>\n";

    std::vector<sk_sp<SkData>> app1Params;
    app1Params.push_back(SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1));

    auto xmp = SkJpegXmp::Make(app1Params);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 8.f);
    REPORTER_ASSERT(r, info.fDisplayRatioHdr == 16.f);
}

DEF_TEST(AndroidCodec_xmpHdrgmAsDescriptionPropertyAttributes, r) {
    // Expose HDRGM values as attributes on an rdf:Description node.
    const char xmpData[] =
            "http://ns.adobe.com/xap/1.0/\0"
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 6.0.0\">\n"
            "   <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "      <rdf:Description rdf:about=\"\"\n"
            "            xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\"\n"
            "         hdrgm:Version=\"1.0\"\n"
            "         hdrgm:GainMapMax=\"3\"\n"
            "         hdrgm:HDRCapacityMax=\"4\"/>\n"
            "   </rdf:RDF>\n"
            "</x:xmpmeta>\n";

    std::vector<sk_sp<SkData>> app1Params;
    app1Params.push_back(SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1));

    auto xmp = SkJpegXmp::Make(app1Params);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 8.f);
    REPORTER_ASSERT(r, info.fDisplayRatioHdr == 16.f);
}

DEF_TEST(AndroidCodec_xmpContainerTypedNode, r) {
    // Container and Item using a node of type Container:Item.
    const char xmpData[] =
            "http://ns.adobe.com/xap/1.0/\0"
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 5.5.0\">\n"
            " <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "  <rdf:Description rdf:about=\"\"\n"
            "    xmlns:Container=\"http://ns.google.com/photos/1.0/container/\"\n"
            "    xmlns:Item=\"http://ns.google.com/photos/1.0/container/item/\">\n"
            "   <Container:Directory>\n"
            "    <rdf:Seq>\n"
            "     <rdf:li rdf:parseType=\"Resource\">\n"
            "      <Container:Item>\n"
            "       <Item:Mime>image/jpeg</Item:Mime>\n"
            "       <Item:Semantic>Primary</Item:Semantic>\n"
            "      </Container:Item>\n"
            "     </rdf:li>\n"
            "     <rdf:li rdf:parseType=\"Resource\">\n"
            "      <Container:Item\n"
            "         Item:Semantic=\"RecoveryMap\"\n"
            "         Item:Mime=\"image/jpeg\"\n"
            "         Item:Length=\"49035\"/>\n"
            "     </rdf:li>\n"
            "    </rdf:Seq>\n"
            "   </Container:Directory>\n"
            "  </rdf:Description>\n"
            " </rdf:RDF>\n"
            "</x:xmpmeta>\n";
    std::vector<sk_sp<SkData>> app1Params;
    app1Params.push_back(SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1));

    auto xmp = SkJpegXmp::Make(app1Params);
    REPORTER_ASSERT(r, xmp);

    size_t offset = 999;
    size_t size = 999;
    REPORTER_ASSERT(r, xmp->getContainerGainmapLocation(&offset, &size));
    REPORTER_ASSERT(r, size == 49035);
}

DEF_TEST(AndroidCodec_xmpContainerTypedNodeRdfEquivalent, r) {
    // Container and Item using rdf:value and rdf:type pairs.
    const char xmpData[] =
            "http://ns.adobe.com/xap/1.0/\0"
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 5.5.0\">\n"
            " <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "  <rdf:Description rdf:about=\"\"\n"
            "    xmlns:Container=\"http://ns.google.com/photos/1.0/container/\"\n"
            "    xmlns:Item=\"http://ns.google.com/photos/1.0/container/item/\">\n"
            "   <Container:Directory>\n"
            "    <rdf:Seq>\n"
            "     <rdf:li rdf:parseType=\"Resource\">\n"
            "      <rdf:value rdf:parseType=\"Resource\">\n"
            "       <Item:Mime>image/jpeg</Item:Mime>\n"
            "       <Item:Semantic>Primary</Item:Semantic>\n"
            "      </rdf:value>\n"
            "      <rdf:type rdf:resource=\"Item\"/>\n"
            "     </rdf:li>\n"
            "     <rdf:li rdf:parseType=\"Resource\">\n"
            "      <rdf:value rdf:parseType=\"Resource\">\n"
            "       <Item:Semantic>RecoveryMap</Item:Semantic>\n"
            "       <Item:Mime>image/jpeg</Item:Mime>\n"
            "       <Item:Length>49035</Item:Length>\n"
            "      </rdf:value>\n"
            "      <rdf:type rdf:resource=\"Item\"/>\n"
            "     </rdf:li>\n"
            "    </rdf:Seq>\n"
            "   </Container:Directory>\n"
            "  </rdf:Description>\n"
            " </rdf:RDF>\n"
            "</x:xmpmeta>\n";
    std::vector<sk_sp<SkData>> app1Params;
    app1Params.push_back(SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1));

    auto xmp = SkJpegXmp::Make(app1Params);
    REPORTER_ASSERT(r, xmp);

    size_t offset = 999;
    size_t size = 999;
    REPORTER_ASSERT(r, xmp->getContainerGainmapLocation(&offset, &size));
    REPORTER_ASSERT(r, size == 49035);
}

// Render a single pixel of an applied gainmap and return it.
SkColor4f render_gainmap_pixel(float renderHdrRatio,
                               const SkBitmap& baseBitmap,
                               const SkBitmap& gainmapBitmap,
                               const SkGainmapInfo& gainmapInfo,
                               int x,
                               int y) {
    SkImageInfo testPixelInfo = SkImageInfo::Make(
            1, 1, kRGBA_F16_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
    SkBitmap testPixelBitmap = render_gainmap(
            testPixelInfo, renderHdrRatio, baseBitmap, gainmapBitmap, gainmapInfo, x, y);
    return testPixelBitmap.getColor4f(0, 0);
}

static bool approx_eq(float x, float y, float epsilon) { return std::abs(x - y) < epsilon; }

static bool approx_eq_rgb(const SkColor4f& x, const SkColor4f& y, float epsilon) {
    return approx_eq(x.fR, y.fR, epsilon) && approx_eq(x.fG, y.fG, epsilon) &&
           approx_eq(x.fB, y.fB, epsilon);
}

DEF_TEST(AndroidCodec_jpegGainmapDecode, r) {
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

            SkBitmap baseBitmap;
            SkBitmap gainmapBitmap;
            SkGainmapInfo gainmapInfo;
            decode_all(r, std::move(testStream), baseBitmap, gainmapBitmap, gainmapInfo);

            // Spot-check the image size and pixels.
            REPORTER_ASSERT(r, gainmapBitmap.dimensions() == rec.dimensions);
            REPORTER_ASSERT(r, gainmapBitmap.getColor(0, 0) == rec.originColor);
            REPORTER_ASSERT(
                    r,
                    gainmapBitmap.getColor(rec.dimensions.fWidth - 1, rec.dimensions.fHeight - 1) ==
                            rec.farCornerColor);

            // Verify the gainmap rendering parameters.
            constexpr float kEpsilon = 1e-3f;
            REPORTER_ASSERT(r, approx_eq(gainmapInfo.fLogRatioMin.fR, rec.logRatioMin, kEpsilon));
            REPORTER_ASSERT(r, approx_eq(gainmapInfo.fLogRatioMin.fG, rec.logRatioMin, kEpsilon));
            REPORTER_ASSERT(r, approx_eq(gainmapInfo.fLogRatioMin.fB, rec.logRatioMin, kEpsilon));

            REPORTER_ASSERT(r, approx_eq(gainmapInfo.fLogRatioMax.fR, rec.logRatioMax, kEpsilon));
            REPORTER_ASSERT(r, approx_eq(gainmapInfo.fLogRatioMax.fG, rec.logRatioMax, kEpsilon));
            REPORTER_ASSERT(r, approx_eq(gainmapInfo.fLogRatioMax.fB, rec.logRatioMax, kEpsilon));

            REPORTER_ASSERT(r, approx_eq(gainmapInfo.fHdrRatioMin, rec.hdrRatioMin, kEpsilon));
            REPORTER_ASSERT(r, approx_eq(gainmapInfo.fHdrRatioMax, rec.hdrRatioMax, kEpsilon));

            REPORTER_ASSERT(r, gainmapInfo.fType == rec.type);
        }
    }
}

DEF_TEST(AndroidCodec_jpegNoGainmap, r) {
    // This test image has a large APP16 segment that will stress the various SkJpegSourceMgrs'
    // data skipping paths.
    const char* path = "images/icc-v2-gbr.jpg";

    TestStream::Type kStreamTypes[] = {
            TestStream::Type::kUnseekable,
            TestStream::Type::kSeekable,
            TestStream::Type::kMemoryMapped,
    };
    for (const auto& streamType : kStreamTypes) {
        bool useFileStream = streamType != TestStream::Type::kMemoryMapped;
        auto stream = GetResourceAsStream(path, useFileStream);
        REPORTER_ASSERT(r, stream);
        auto testStream = std::make_unique<TestStream>(streamType, stream.get());

        // Decode the base bitmap.
        SkCodec::Result result = SkCodec::kSuccess;
        std::unique_ptr<SkCodec> baseCodec =
                SkJpegCodec::MakeFromStream(std::move(testStream), &result);
        REPORTER_ASSERT(r, baseCodec);
        SkBitmap baseBitmap;
        baseBitmap.allocPixels(baseCodec->getInfo());
        REPORTER_ASSERT(r,
                        SkCodec::kSuccess == baseCodec->getPixels(baseBitmap.info(),
                                                                  baseBitmap.getPixels(),
                                                                  baseBitmap.rowBytes()));

        std::unique_ptr<SkAndroidCodec> androidCodec =
                SkAndroidCodec::MakeFromCodec(std::move(baseCodec));
        REPORTER_ASSERT(r, androidCodec);

        // Try to extract the gainmap info and stream. It should fail.
        SkGainmapInfo gainmapInfo;
        std::unique_ptr<SkStream> gainmapStream;
        REPORTER_ASSERT(r, !androidCodec->getAndroidGainmap(&gainmapInfo, &gainmapStream));
    }
}

#ifdef SK_ENCODE_JPEG
DEF_TEST(AndroidCodec_jpegGainmapTranscode, r) {
    const char* path = "images/iphone_13_pro.jpeg";
    SkBitmap baseBitmap[2];
    SkBitmap gainmapBitmap[2];
    SkGainmapInfo gainmapInfo[2];

    // Decode an MPF-based gainmap image.
    decode_all(r, GetResourceAsStream(path), baseBitmap[0], gainmapBitmap[0], gainmapInfo[0]);

    constexpr float kEpsilon = 1e-2f;
    for (size_t i = 0; i < 2; ++i) {
        SkDynamicMemoryWStream encodeStream;
        bool encodeResult = false;

        if (i == 0) {
            // Transcode to JpegR.
            encodeResult = SkJpegGainmapEncoder::EncodeJpegR(&encodeStream,
                                                             baseBitmap[0].pixmap(),
                                                             SkJpegEncoder::Options(),
                                                             gainmapBitmap[0].pixmap(),
                                                             SkJpegEncoder::Options(),
                                                             gainmapInfo[0]);
        } else {
            // Transcode to HDRGM.
            encodeResult = SkJpegGainmapEncoder::EncodeHDRGM(&encodeStream,
                                                             baseBitmap[0].pixmap(),
                                                             SkJpegEncoder::Options(),
                                                             gainmapBitmap[0].pixmap(),
                                                             SkJpegEncoder::Options(),
                                                             gainmapInfo[0]);
        }
        REPORTER_ASSERT(r, encodeResult);
        auto encodeData = encodeStream.detachAsData();

        // Decode the just-encoded image.
        auto decodeStream = std::make_unique<SkMemoryStream>(encodeData);
        decode_all(r, std::move(decodeStream), baseBitmap[1], gainmapBitmap[1], gainmapInfo[1]);

        // HDRGM will have the same rendering parameters.
        REPORTER_ASSERT(
                r,
                approx_eq_rgb(gainmapInfo[0].fLogRatioMin, gainmapInfo[1].fLogRatioMin, kEpsilon));
        REPORTER_ASSERT(
                r,
                approx_eq_rgb(gainmapInfo[0].fLogRatioMax, gainmapInfo[1].fLogRatioMax, kEpsilon));
        REPORTER_ASSERT(
                r,
                approx_eq_rgb(
                        gainmapInfo[0].fGainmapGamma, gainmapInfo[1].fGainmapGamma, kEpsilon));
        REPORTER_ASSERT(
                r,
                approx_eq(gainmapInfo[0].fEpsilonSdr.fR, gainmapInfo[1].fEpsilonSdr.fR, kEpsilon));
        REPORTER_ASSERT(
                r,
                approx_eq(gainmapInfo[0].fEpsilonHdr.fR, gainmapInfo[1].fEpsilonHdr.fR, kEpsilon));
        REPORTER_ASSERT(
                r, approx_eq(gainmapInfo[0].fHdrRatioMin, gainmapInfo[1].fHdrRatioMin, kEpsilon));
        REPORTER_ASSERT(
                r, approx_eq(gainmapInfo[0].fHdrRatioMax, gainmapInfo[1].fHdrRatioMax, kEpsilon));

#ifdef SK_ENABLE_SKSL
        // Render a few pixels and verify that they come out the same. Rendering requires SkSL.
        const struct Rec {
            int x;
            int y;
            float hdrRatio;
            SkColor4f expectedColor;
            SkColorType forcedColorType;
        } recs[] = {
                {1446, 1603, 1.05f, {0.984375f, 1.004883f, 1.008789f, 1.f}, kUnknown_SkColorType},
                {1446, 1603, 100.f, {1.147461f, 1.170898f, 1.174805f, 1.f}, kUnknown_SkColorType},
                {1446, 1603, 100.f, {1.147461f, 1.170898f, 1.174805f, 1.f}, kGray_8_SkColorType},
                {1446, 1603, 100.f, {1.147461f, 1.170898f, 1.174805f, 1.f}, kAlpha_8_SkColorType},
                {1446, 1603, 100.f, {1.147461f, 1.170898f, 1.174805f, 1.f}, kR8_unorm_SkColorType},
        };

        for (const auto& rec : recs) {
            SkBitmap gainmapBitmap0;
            SkASSERT(gainmapBitmap[0].colorType() == kGray_8_SkColorType);

            // Force various different single-channel formats, to ensure that they all work. Note
            // that when the color type is forced to kAlpha_8_SkColorType, the shader will always
            // read (0,0,0,1) if the alpha type is kOpaque_SkAlphaType.
            if (rec.forcedColorType == kUnknown_SkColorType) {
                gainmapBitmap0 = gainmapBitmap[0];
            } else {
                gainmapBitmap0.installPixels(gainmapBitmap[0]
                                                     .info()
                                                     .makeColorType(rec.forcedColorType)
                                                     .makeAlphaType(kPremul_SkAlphaType),
                                             gainmapBitmap[0].getPixels(),
                                             gainmapBitmap[0].rowBytes());
            }
            SkColor4f p0 = render_gainmap_pixel(
                    rec.hdrRatio, baseBitmap[0], gainmapBitmap0, gainmapInfo[0], rec.x, rec.y);
            SkColor4f p1 = render_gainmap_pixel(
                    rec.hdrRatio, baseBitmap[1], gainmapBitmap[1], gainmapInfo[1], rec.x, rec.y);

            REPORTER_ASSERT(r, approx_eq_rgb(p0, p1, kEpsilon));
        }
#endif  // SK_ENABLE_SKSL
    }
}
#endif  // SK_ENCODE_JPEG
