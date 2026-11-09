/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkDebug.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkEndian.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontStream.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeSpec.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

using namespace skia_private;

//#define DUMP_TABLES
//#define DUMP_TTC_TABLES

#define kFontTableTag_head          SkSetFourByteTag('h', 'e', 'a', 'd')
#define kFontTableTag_hhea          SkSetFourByteTag('h', 'h', 'e', 'a')
#define kFontTableTag_maxp          SkSetFourByteTag('m', 'a', 'x', 'p')

struct TagSize {
    SkFontTableTag  fTag;
    size_t          fSize;
};

static constexpr auto gKnownTableSizes = std::to_array<TagSize>({
        TagSize{   kFontTableTag_head,         54 },
        TagSize{   kFontTableTag_hhea,         36 },
});

// Test that getUnitsPerEm() agrees with a direct lookup in the 'head' table
// (if that table is available).
static void test_unitsPerEm(skiatest::Reporter* reporter, const sk_sp<SkTypeface>& face) {
    int nativeUPEM = face->getUnitsPerEm();

    int tableUPEM = -1;
    size_t size = face->getTableSize(kFontTableTag_head);
    if (size) {
        // unitsPerEm is at offset 18 into the 'head' table.
        uint16_t rawUPEM;
        face->getTableData(kFontTableTag_head, 18, sizeof(rawUPEM), &rawUPEM);
        tableUPEM = SkEndian_SwapBE16(rawUPEM);
    }

    if (tableUPEM >= 0) {
        REPORTER_ASSERT(reporter, tableUPEM == nativeUPEM);
    }
}

// Test that countGlyphs() agrees with a direct lookup in the 'maxp' table
// (if that table is available).
static void test_countGlyphs(skiatest::Reporter* reporter, const sk_sp<SkTypeface>& face) {
    int nativeGlyphs = face->countGlyphs();

    int tableGlyphs = -1;
    size_t size = face->getTableSize(kFontTableTag_maxp);
    if (size) {
        // glyphs is at offset 4 into the 'maxp' table.
        uint16_t rawGlyphs;
        face->getTableData(kFontTableTag_maxp, 4, sizeof(rawGlyphs), &rawGlyphs);
        tableGlyphs = SkEndian_SwapBE16(rawGlyphs);
    }

    if (tableGlyphs >= 0) {
        REPORTER_ASSERT(reporter, tableGlyphs == nativeGlyphs);
    }
}

static void test_fontstream(skiatest::Reporter* reporter, SkStream* stream, int ttcIndex) {
    int n = SkFontStream::GetTableTags(stream, ttcIndex, {});
    AutoTArray<SkFontTableTag> array(n);

    int n2 = SkFontStream::GetTableTags(stream, ttcIndex, array);
    REPORTER_ASSERT(reporter, n == n2);

    for (int i = 0; i < n; ++i) {
#ifdef DUMP_TTC_TABLES
        SkString str;
        SkFontTableTag t = array[i];
        str.appendUnichar((t >> 24) & 0xFF);
        str.appendUnichar((t >> 16) & 0xFF);
        str.appendUnichar((t >>  8) & 0xFF);
        str.appendUnichar((t >>  0) & 0xFF);
        SkDebugf("[%d:%d] '%s'\n", ttcIndex, i, str.c_str());
#endif
        size_t size = SkFontStream::GetTableSize(stream, ttcIndex, array[i]);
        for (size_t j = 0; j < std::size(gKnownTableSizes); ++j) {
            if (gKnownTableSizes[j].fTag == array[i]) {
                REPORTER_ASSERT(reporter, gKnownTableSizes[j].fSize == size);
            }
        }
    }
}

static void test_fontstream(skiatest::Reporter* reporter) {
    std::unique_ptr<SkStreamAsset> stream(GetResourceAsStream("fonts/test.ttc"));
    if (!stream) {
        SkDebugf("Skipping FontHostTest::test_fontstream\n");
        return;
    }

    int count = SkFontStream::CountTTCEntries(stream.get());
#ifdef DUMP_TTC_TABLES
    SkDebugf("CountTTCEntries %d\n", count);
#endif
    for (int i = 0; i < count; ++i) {
        test_fontstream(reporter, stream.get(), i);
    }
}

// Exercise this rare cmap format (platform 3, encoding 0)
static void test_symbolfont(skiatest::Reporter* reporter) {
    auto tf = ToolUtils::CreateTypefaceFromResource("fonts/SpiderSymbol.ttf");
    if (tf) {
        SkUnichar c = 0xf021;
        SkGlyphID g = SkFont(tf).unicharToGlyph(c);
        REPORTER_ASSERT(reporter, g == 3);
    } else {
        // not all platforms support data fonts, so we just note that failure
        SkDebugf("Skipping FontHostTest::test_symbolfont\n");
    }
}

static void test_tables(skiatest::Reporter* reporter, const sk_sp<SkTypeface>& face) {
    int count = face->countTables();

    std::vector<SkFontTableTag> tags(count);
    int count2 = face->readTableTags(tags);
    REPORTER_ASSERT(reporter, count2 == count);

    for (int i = 0; i < count; ++i) {
        size_t size = face->getTableSize(tags[i]);
        REPORTER_ASSERT(reporter, size > 0);

#ifdef DUMP_TABLES
        char name[5];
        name[0] = (tags[i] >> 24) & 0xFF;
        name[1] = (tags[i] >> 16) & 0xFF;
        name[2] = (tags[i] >>  8) & 0xFF;
        name[3] = (tags[i] >>  0) & 0xFF;
        name[4] = 0;
        SkDebugf("%s %d\n", name, size);
#endif

        for (size_t j = 0; j < std::size(gKnownTableSizes); ++j) {
            if (gKnownTableSizes[j].fTag == tags[i]) {
                REPORTER_ASSERT(reporter, gKnownTableSizes[j].fSize == size);
            }
        }

        // do we get the same size from GetTableData and GetTableSize
        {
            SkAutoMalloc data(size);
            size_t size2 = face->getTableData(tags[i], 0, size, data.get());
            REPORTER_ASSERT(reporter, size2 == size);
            sk_sp<SkData> data2 = face->copyTableData(tags[i]);
            REPORTER_ASSERT(reporter, size == data2->size());
            REPORTER_ASSERT(reporter, !memcmp(data.get(), data2->data(), size));
        }
    }
}

static void test_tables(skiatest::Reporter* reporter) {
    static constexpr auto gNames = std::to_array<const char*>({
            nullptr,  // default font
            "Helvetica",
            "Arial",
            "Times",
            "Times New Roman",
            "Courier",
            "Courier New",
            "Terminal",
            "MS Sans Serif",
            "Hiragino Mincho ProN",
            "MS PGothic",
    });

    for (size_t i = 0; i < std::size(gNames); ++i) {
        sk_sp<SkTypeface> face(ToolUtils::CreateTestTypeface(gNames[i], SkFontStyle()));
        if (face) {
#ifdef DUMP_TABLES
            SkDebugf("%s\n", gNames[i]);
#endif
            test_tables(reporter, face);
            test_unitsPerEm(reporter, face);
            test_countGlyphs(reporter, face);
        }
    }
}

/*
 * Verifies that the advance values returned by various methods match.
 */
static void test_advances(skiatest::Reporter* reporter) {
    static constexpr auto faces = std::to_array<const char*>({
            nullptr,  // default font
            "Arial",
            "Times",
            "Times New Roman",
            "Helvetica",
            "Courier",
            "Courier New",
            "Verdana",
            "monospace",
    });

    struct Settings {
        SkFontHinting   hinting;
        bool            linear;
        bool            subpixel;
    };

    static constexpr auto settings = std::to_array<Settings>({
            Settings{ SkFontHinting::kNone,   false, false },
            Settings{ SkFontHinting::kNone,   true,  false },
            Settings{ SkFontHinting::kNone,   false, true  },
            Settings{ SkFontHinting::kSlight, false, false },
            Settings{ SkFontHinting::kSlight, true,  false },
            Settings{ SkFontHinting::kSlight, false, true  },
            Settings{ SkFontHinting::kNormal, false, false },
            Settings{ SkFontHinting::kNormal, true,  false },
            Settings{ SkFontHinting::kNormal, false, true  },
    });

    struct ScaleRec {
        SkScalar    fScaleX;
        SkScalar    fSkewX;
    };

    static constexpr auto gScaleRec = std::to_array<ScaleRec>({
            ScaleRec{ SK_Scalar1, 0 },
            ScaleRec{ SK_Scalar1/2, 0 },
            // these two exercise obliquing (skew)
            ScaleRec{ SK_Scalar1, -SK_Scalar1/4 },
            ScaleRec{ SK_Scalar1/2, -SK_Scalar1/4 },
    });

    SkFont font;
    char const * const txt = "long.text.with.lots.of.dots.";
    size_t textLen = strlen(txt);

    for (size_t i = 0; i < std::size(faces); i++) {
        font.setTypeface(ToolUtils::CreateTestTypeface(faces[i], SkFontStyle()));

        for (size_t j = 0; j  < std::size(settings); j++) {
            font.setHinting(settings[j].hinting);
            font.setLinearMetrics(settings[j].linear);
            font.setSubpixel(settings[j].subpixel);

            for (size_t k = 0; k < std::size(gScaleRec); ++k) {
                font.setScaleX(gScaleRec[k].fScaleX);
                font.setSkewX(gScaleRec[k].fSkewX);

                SkRect bounds;

                SkScalar width1 = font.measureText(txt, textLen, SkTextEncoding::kUTF8);

                // Requesting the bounds forces a generateMetrics call.
                SkScalar width2 = font.measureText(txt, textLen, SkTextEncoding::kUTF8, &bounds);

                REPORTER_ASSERT(reporter, width1 == width2);
            }
        }
    }
}

DEF_TEST(FontHost, reporter) {
    test_tables(reporter);
    test_fontstream(reporter);
    test_advances(reporter);
    test_symbolfont(reporter);
}

namespace {

// Appends a Type 1 charstring integer operand (-107..107) to cs
void t1_push_int(std::vector<uint8_t>* cs, int v) {
    SkASSERT(v >= -107 && v <= 107);
    cs->push_back(static_cast<uint8_t>(v + 139));
}

// Appends a 1-byte or 2-byte (12, subop) Type 1 charstring operator to cs
void t1_op(std::vector<uint8_t>* cs, uint8_t op, int subop = -1) {
    cs->push_back(op);
    if (subop >= 0) {
        cs->push_back(static_cast<uint8_t>(subop));
    }
}

// Encrypts plain with the Adobe Type 1 stream cipher (seed 55665 for eexec, 4330 for lenIV=4)
// after prepending 4 zero IV bytes
std::vector<uint8_t> t1_encrypt(SkSpan<const uint8_t> plain, uint16_t r) {
    std::vector<uint8_t> out;
    out.reserve(plain.size() + 4);
    for (int i = 0; i < 4; ++i) {
        uint8_t cipher = static_cast<uint8_t>(0 ^ (r >> 8));
        r = static_cast<uint16_t>((cipher + r) * 52845u + 22719u);
        out.push_back(cipher);
    }
    for (uint8_t b : plain) {
        uint8_t cipher = static_cast<uint8_t>(b ^ (r >> 8));
        r = static_cast<uint16_t>((cipher + r) * 52845u + 22719u);
        out.push_back(cipher);
    }
    return out;
}

// Appends a binary PFB segment (0x80, segType, 4-byte LE length, payload) to pfb
void append_pfb_segment(std::vector<uint8_t>* pfb, uint8_t segType, SkSpan<const uint8_t> payload) {
    pfb->push_back(0x80);
    pfb->push_back(segType);
    uint32_t len = static_cast<uint32_t>(payload.size());
    pfb->push_back(static_cast<uint8_t>(len & 0xFF));
    pfb->push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
    pfb->push_back(static_cast<uint8_t>((len >> 16) & 0xFF));
    pfb->push_back(static_cast<uint8_t>((len >> 24) & 0xFF));
    pfb->insert(pfb->end(), payload.begin(), payload.end());
}

// Builds a minimal binary Type 1 PFB font (100 upem) containing .notdef plus glyphs; when
// enableBuildChar is true, includes a 1-axis MM dictionary with /BuildCharArray [0]
sk_sp<SkData> build_type1_pfb(SkSpan<const std::vector<uint8_t>> glyphs, bool enableBuildChar) {
    std::string header =
            "%!PS-AdobeFont-1.0: TestT1 001.000\n"
            "12 dict begin\n"
            "/FontInfo 4 dict dup begin\n"
            "/FullName (TestT1) readonly def\n"
            "/FamilyName (TestT1) readonly def\n"
            "end readonly def\n"
            "/FontName /TestT1 def\n"
            "/Encoding StandardEncoding def\n"
            "/PaintType 0 def\n"
            "/FontType 1 def\n"
            "/FontMatrix [0.01 0 0 0.01 0 0] readonly def\n"
            "/FontBBox [-100 -100 1000 1000] readonly def\n";
    if (enableBuildChar) {
        header +=
                "/BlendAxisTypes [/Weight] def\n"
                "/BlendDesignPositions [[0] [1]] def\n"
                "/BlendDesignMap [[[0 0] [1000 1]]] def\n"
                "/WeightVector [1 0] def\n";
    }
    header += "currentdict end\ncurrentfile eexec\n";

    std::string priv =
            "dup /Private 8 dict dup begin\n"
            "/RD {string currentfile exch readstring pop} executeonly def\n"
            "/ND {noaccess def} executeonly def\n"
            "/NP {noaccess put} executeonly def\n"
            "/BlueValues [] def\n"
            "/MinFeature {16 16} def\n"
            "/password 5839 def\n"
            "/lenIV 4 def\n";
    if (enableBuildChar) {
        priv += "/BuildCharArray [0] def\n";
    }
    priv += "2 index /CharStrings " + std::to_string(glyphs.size() + 1) + " dict dup begin\n";

    std::vector<uint8_t> eexecPlain(priv.begin(), priv.end());
    std::vector<uint8_t> notdefPlain;
    t1_push_int(&notdefPlain, 0);
    t1_push_int(&notdefPlain, 100);
    t1_op(&notdefPlain, 13);  // hsbw
    t1_op(&notdefPlain, 14);  // endchar
    std::vector<uint8_t> notdefEnc = t1_encrypt(notdefPlain, 4330);
    std::string notdefHdr = "/.notdef " + std::to_string(notdefEnc.size()) + " RD ";
    eexecPlain.insert(eexecPlain.end(), notdefHdr.begin(), notdefHdr.end());
    eexecPlain.insert(eexecPlain.end(), notdefEnc.begin(), notdefEnc.end());
    const char kNd[] = " ND\n";
    eexecPlain.insert(eexecPlain.end(), kNd, kNd + sizeof(kNd) - 1);

    for (size_t i = 0; i < glyphs.size(); ++i) {
        std::vector<uint8_t> enc = t1_encrypt(glyphs[i], 4330);
        std::string gHdr = "/g" + std::to_string(i) + " " + std::to_string(enc.size()) + " RD ";
        eexecPlain.insert(eexecPlain.end(), gHdr.begin(), gHdr.end());
        eexecPlain.insert(eexecPlain.end(), enc.begin(), enc.end());
        eexecPlain.insert(eexecPlain.end(), kNd, kNd + sizeof(kNd) - 1);
    }

    const std::string privEnd =
            "end\nend\nreadonly put\nnoaccess put\ndup /FontName get exch definefont pop\n"
            "mark currentfile closefile\n";
    eexecPlain.insert(eexecPlain.end(), privEnd.begin(), privEnd.end());
    std::vector<uint8_t> eexecCipher = t1_encrypt(eexecPlain, 55665);

    std::string trailer(512, '0');
    trailer += "\ncleartomark\n";

    std::vector<uint8_t> pfb;
    append_pfb_segment(
            &pfb, 1, SkSpan(reinterpret_cast<const uint8_t*>(header.data()), header.size()));
    append_pfb_segment(&pfb, 2, eexecCipher);
    append_pfb_segment(
            &pfb, 1, SkSpan(reinterpret_cast<const uint8_t*>(trailer.data()), trailer.size()));
    pfb.push_back(0x80);
    pfb.push_back(0x03);
    return SkData::MakeWithCopy(pfb.data(), pfb.size());
}

// Deserializes pfb through the 'free' typeface factory when FreeType is compiled in
sk_sp<SkTypeface> make_freetype_type1_typeface(sk_sp<SkData> pfb) {
#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
    SkFontDescriptor desc;
    desc.setFactoryId(SkSetFourByteTag('f', 'r', 'e', 'e'));
    desc.setStream(SkMemoryStream::Make(std::move(pfb)));
    SkDynamicMemoryWStream wstream;
    if (!desc.serialize(&wstream)) {
        return nullptr;
    }
    std::unique_ptr<SkStreamAsset> rstream = wstream.detachAsStream();
    return SkTypeface::MakeDeserialize(rstream.get(), nullptr);
#else
    (void)pfb;
    return nullptr;
#endif
}

// Builds a single-master Type 1 charstring burning burnSteps PRNG outputs (OtherSubr 28) and
// branching via OtherSubr 27 between 0 and 35 units in X and Y before drawing a 10x10 square
std::vector<uint8_t> make_random_offset_glyph(int burnSteps) {
    std::vector<uint8_t> cs;
    t1_push_int(&cs, 0);
    t1_push_int(&cs, 100);
    t1_op(&cs, 13);  // hsbw
    for (int i = 0; i < burnSteps; ++i) {
        t1_push_int(&cs, 0);
        t1_push_int(&cs, 28);
        t1_op(&cs, 12, 16);  // callothersubr (random)
        t1_op(&cs, 12, 17);  // pop
    }
    for (int axis = 0; axis < 2; ++axis) {
        t1_push_int(&cs, 0);
        t1_push_int(&cs, 35);
        t1_push_int(&cs, 0);
        t1_push_int(&cs, 28);
        t1_op(&cs, 12, 16);  // callothersubr (random)
        t1_push_int(&cs, 1);
        t1_push_int(&cs, 2);
        t1_op(&cs, 12, 12);  // div (0.5)
        t1_push_int(&cs, 4);
        t1_push_int(&cs, 27);
        t1_op(&cs, 12, 16);  // callothersubr (ifelse)
        t1_op(&cs, 12, 17);  // pop
    }
    t1_op(&cs, 21);  // rmoveto(dx, dy)
    t1_push_int(&cs, 10);
    t1_push_int(&cs, 0);
    t1_op(&cs, 5);  // rlineto
    t1_push_int(&cs, 0);
    t1_push_int(&cs, 10);
    t1_op(&cs, 5);  // rlineto
    t1_push_int(&cs, -10);
    t1_push_int(&cs, 0);
    t1_op(&cs, 5);   // rlineto
    t1_op(&cs, 9);   // closepath
    t1_op(&cs, 14);  // endchar
    return cs;
}

// Builds a deterministic Type 1 charstring using BuildCharArray[0] (OtherSubr 25/20/24/27) so
// the first FT_Load_Glyph places a boxSize square at (x1, y1) and later loads at (x2, y2). The
// Type 1 interpreter in psaux runs each charstring twice per load (a hint map pass that skips
// outline operators, then the outline pass), so the counter is 2 on the first outline pass
std::vector<uint8_t> make_sequenced_offset_glyph(int x1, int y1, int x2, int y2, int boxSize) {
    std::vector<uint8_t> cs;
    t1_push_int(&cs, 0);
    t1_push_int(&cs, 100);
    t1_op(&cs, 13);  // hsbw
    // Increment BuildCharArray[0]: BuildCharArray[0] = BuildCharArray[0] + 1
    t1_push_int(&cs, 0);
    t1_push_int(&cs, 1);
    t1_push_int(&cs, 25);
    t1_op(&cs, 12, 16);  // get BuildCharArray[0]
    t1_op(&cs, 12, 17);
    t1_push_int(&cs, 1);
    t1_push_int(&cs, 2);
    t1_push_int(&cs, 20);
    t1_op(&cs, 12, 16);  // add 1
    t1_op(&cs, 12, 17);
    t1_push_int(&cs, 0);
    t1_push_int(&cs, 2);
    t1_push_int(&cs, 24);
    t1_op(&cs, 12, 16);  // put BuildCharArray[0]

    for (auto [firstVal, secondVal] : {std::pair{x1, x2}, std::pair{y1, y2}}) {
        t1_push_int(&cs, firstVal);
        t1_push_int(&cs, secondVal);
        t1_push_int(&cs, 0);
        t1_push_int(&cs, 1);
        t1_push_int(&cs, 25);
        t1_op(&cs, 12, 16);  // get BuildCharArray[0]
        t1_op(&cs, 12, 17);
        t1_push_int(&cs, 2);
        t1_push_int(&cs, 4);
        t1_push_int(&cs, 27);
        t1_op(&cs, 12, 16);  // (count <= 2) ? firstVal : secondVal
        t1_op(&cs, 12, 17);
    }
    t1_op(&cs, 21);  // rmoveto(dx, dy)
    t1_push_int(&cs, 0);
    t1_push_int(&cs, boxSize);
    t1_op(&cs, 5);
    t1_push_int(&cs, boxSize);
    t1_push_int(&cs, 0);
    t1_op(&cs, 5);
    t1_push_int(&cs, 0);
    t1_push_int(&cs, -boxSize);
    t1_op(&cs, 5);
    t1_op(&cs, 9);
    t1_op(&cs, 14);
    return cs;
}

}  // namespace

// Draws single-master Type 1 glyphs that branch on OtherSubr 28 (random, seeded from stack
// addresses that differ between makeGlyph and generateImage) under H and V LCD; there are no
// result asserts, a mismatch trips the copyFT2LCD16 SkASSERTs in debug builds
// (crbug.com/576811175)
DEF_TEST(FreeType_Type1StackSeedLCD16, reporter) {
    constexpr int kNumTrialGlyphs = 32;
    std::vector<std::vector<uint8_t>> glyphs;
    glyphs.reserve(kNumTrialGlyphs);
    for (int j = 0; j < kNumTrialGlyphs; ++j) {
        glyphs.push_back(make_random_offset_glyph(j));
    }
    sk_sp<SkTypeface> tf = make_freetype_type1_typeface(build_type1_pfb(glyphs, false));
    if (!tf || tf->countGlyphs() <= kNumTrialGlyphs) {
        return;
    }

    for (SkPixelGeometry geo : {kRGB_H_SkPixelGeometry, kRGB_V_SkPixelGeometry}) {
        SkBitmap bm;
        REPORTER_ASSERT(reporter, bm.tryAllocN32Pixels(256, 64));
        SkSurfaceProps props(0, geo);
        SkCanvas canvas(bm, props);
        canvas.clear(SK_ColorWHITE);

        SkFont font(tf, 32.0f);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setHinting(SkFontHinting::kNormal);

        SkTextBlobBuilder builder;
        const SkTextBlobBuilder::RunBuffer& run =
                builder.allocRunPosH(font, kNumTrialGlyphs, 32.0f);
        for (int j = 0; j < kNumTrialGlyphs; ++j) {
            run.glyphs[j] = static_cast<SkGlyphID>(j + 1);
            run.pos[j] = static_cast<SkScalar>(j * 6);
        }
        canvas.drawTextBlob(builder.make(), 0, 0, SkPaint());
    }
}

// Verifies that FreeType kLCD16_Format rasterization intersects and clips FT_Bitmap in whole
// pixels (ftHoriScale / ftVertScale): disjoint second-load bounds must leave the glyph mask
// zeroed rather than falsely intersecting via 3x subpixel width/rows, and partially overlapping
// shifts must step FT_Bitmap by 3 subpixel samples per pixel (crbug.com/576811175)
DEF_TEST(FreeType_LCD16DivergentGlyphBounds, reporter) {
    const std::vector<uint8_t> glyphs[] = {
            // Glyph 1: horizontal disjoint shift (Load 1 at x=[35,45], Load 2 at x=[0,10])
            make_sequenced_offset_glyph(35, 0, 0, 0, 10),
            // Glyph 2: vertical disjoint shift (Load 1 at y=[0,10], Load 2 at y=[35,45])
            make_sequenced_offset_glyph(0, 0, 0, 35, 10),
            // Glyph 3: horizontal ~1px shift (Load 1 at x=[4,34], Load 2 at x=[0,30])
            make_sequenced_offset_glyph(4, 0, 0, 0, 30),
            // Glyph 4: vertical ~1px shift (Load 1 at y=[0,30], Load 2 at y=[4,34])
            make_sequenced_offset_glyph(0, 0, 0, 4, 30),
            // Glyph 5: static reference for Load 2 of Glyph 3 (x=[0,30], y=[0,30])
            make_sequenced_offset_glyph(0, 0, 0, 0, 30),
            // Glyph 6: static reference for Load 2 of Glyph 4 (x=[0,30], y=[4,34])
            make_sequenced_offset_glyph(0, 4, 0, 4, 30),
    };
    sk_sp<SkData> pfb = build_type1_pfb(glyphs, true);

    struct Case {
        SkGlyphID fGlyphID;
        SkGlyphID fRefGlyphID;
        SkPixelGeometry fGeo;
    };
    constexpr Case kCases[] = {
            {1, 0, kRGB_H_SkPixelGeometry},
            {2, 0, kRGB_V_SkPixelGeometry},
            {3, 5, kRGB_H_SkPixelGeometry},
            {4, 6, kRGB_V_SkPixelGeometry},
    };

    for (const Case& c : kCases) {
        // BuildCharArray[0] persists on FT_Face across glyphs, so instantiate a fresh face per case
        sk_sp<SkTypeface> tf = make_freetype_type1_typeface(pfb);
        if (!tf || tf->countGlyphs() < 7) {
            return;
        }
        SkFont font(tf, 32.0f);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setHinting(SkFontHinting::kNone);

        SkStrikeSpec spec = SkStrikeSpec::MakeMask(font,
                                                   SkPaint(),
                                                   SkSurfaceProps(0, c.fGeo),
                                                   SkScalerContextFlags::kNone,
                                                   SkMatrix::I());
        std::unique_ptr<SkScalerContext> ctx = spec.createScalerContext();

        char storage[8192];
        memset(storage, 0x5A, sizeof(storage));
        SkArenaAlloc alloc(storage, sizeof(storage), 8192);
        SkGlyph glyph = ctx->makeGlyph(SkPackedGlyphID(c.fGlyphID), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kLCD16_Format);
        REPORTER_ASSERT(reporter, glyph.imageSize() > 0);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, ctx.get()));

        if (c.fRefGlyphID == 0) {
            const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
            bool anyNonZero = false;
            for (size_t i = 0; i < glyph.imageSize(); ++i) {
                anyNonZero |= (bytes[i] != 0);
            }
            REPORTER_ASSERT(reporter, !anyNonZero);
            continue;
        }

        SkGlyph ref = ctx->makeGlyph(SkPackedGlyphID(c.fRefGlyphID), &alloc);
        REPORTER_ASSERT(reporter, ref.setImage(&alloc, ctx.get()));
        // A different interpreter pass count would draw both loads in one place and make
        // the comparison below vacuous
        REPORTER_ASSERT(reporter, glyph.iRect() != ref.iRect());
        SkIRect overlap = glyph.iRect();
        REPORTER_ASSERT(reporter, overlap.intersect(ref.iRect()));

        bool allMatch = true;
        bool anyOverlapInk = false;
        for (int y = glyph.top(); y < glyph.top() + glyph.height(); ++y) {
            const auto* row =
                    reinterpret_cast<const uint16_t*>(static_cast<const uint8_t*>(glyph.image()) +
                                                      (y - glyph.top()) * glyph.rowBytes());
            for (int x = glyph.left(); x < glyph.left() + glyph.width(); ++x) {
                uint16_t expected = 0;
                if (ref.iRect().contains(x, y)) {
                    const auto* refRow = reinterpret_cast<const uint16_t*>(
                            static_cast<const uint8_t*>(ref.image()) +
                            (y - ref.top()) * ref.rowBytes());
                    expected = refRow[x - ref.left()];
                    anyOverlapInk |= (expected != 0);
                }
                allMatch &= (row[x - glyph.left()] == expected);
            }
        }
        REPORTER_ASSERT(reporter, anyOverlapInk);
        REPORTER_ASSERT(reporter, allMatch);
    }
}

// need tests for SkStrSearch
