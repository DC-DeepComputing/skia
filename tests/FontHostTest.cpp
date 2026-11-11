/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#if defined(SK_FONTMGR_FREETYPE_EMPTY_AVAILABLE)
#include "include/ports/SkFontMgr_empty.h"
#endif
#include "include/private/SkDebug.h"
#include "include/private/SkFixed.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkEndian.h"
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
#include <string_view>
#include <utility>
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

#if defined(SK_FONTMGR_FREETYPE_EMPTY_AVAILABLE)
namespace {

void append_u16(std::vector<uint8_t>& buf, uint16_t val) {
    buf.push_back(static_cast<uint8_t>(val >> 8));
    buf.push_back(static_cast<uint8_t>(val & 0xFF));
}

void append_s16(std::vector<uint8_t>& buf, int16_t val) {
    append_u16(buf, static_cast<uint16_t>(val));
}

void append_u32(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
    buf.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>(val & 0xFF));
}

void append_tag(std::vector<uint8_t>& buf, std::string_view tag) {
    for (char c : tag) {
        buf.push_back(static_cast<uint8_t>(c));
    }
}

struct SfntTableEntry {
    std::string_view tag;
    std::vector<uint8_t> data;
};

sk_sp<SkData> serialize_sfnt_tables(SkSpan<const SfntTableEntry> tables) {
    const uint16_t numTables = static_cast<uint16_t>(tables.size());
    uint16_t entrySelector = 0;
    uint16_t searchRange = 16;
    while (searchRange * 2 <= numTables * 16) {
        searchRange *= 2;
        ++entrySelector;
    }
    const uint16_t rangeShift = numTables * 16 - searchRange;

    std::vector<uint8_t> font;
    append_u32(font, 0x00010000);
    append_u16(font, numTables);
    append_u16(font, searchRange);
    append_u16(font, entrySelector);
    append_u16(font, rangeShift);

    uint32_t offset = 12 + numTables * 16;
    for (const auto& t : tables) {
        append_tag(font, t.tag);
        append_u32(font, 0);
        append_u32(font, offset);
        append_u32(font, static_cast<uint32_t>(t.data.size()));
        offset += (static_cast<uint32_t>(t.data.size()) + 3u) & ~3u;
    }
    for (const auto& t : tables) {
        font.insert(font.end(), t.data.begin(), t.data.end());
        while (font.size() % 4 != 0) {
            font.push_back(0);
        }
    }
    return SkData::MakeWithCopy(font.data(), font.size());
}

sk_sp<SkData> make_hinted_ttf(const std::vector<uint8_t>& instructions,
                              int16_t widthUnits,
                              uint16_t maxTwilightPoints) {
    std::vector<SfntTableEntry> tables;

    // 1. OS/2 (version 2, 96 bytes; non-zero sxHeight/sCapHeight avoids extra metric loads)
    {
        std::vector<uint8_t> os2;
        append_u16(os2, 2);
        append_s16(os2, widthUnits);
        append_u16(os2, 400);
        append_u16(os2, 5);
        append_u16(os2, 0);
        for (int i = 0; i < 11; ++i) {
            append_s16(os2, 0);
        }
        os2.insert(os2.end(), 10, 0);
        for (int i = 0; i < 4; ++i) {
            append_u32(os2, 0);
        }
        append_tag(os2, "TEST");
        append_u16(os2, 0x0040);
        append_u16(os2, 0x0041);
        append_u16(os2, 0x0043);
        append_s16(os2, 800);
        append_s16(os2, -200);
        append_s16(os2, 0);
        append_u16(os2, 800);
        append_u16(os2, 200);
        append_u32(os2, 0);
        append_u32(os2, 0);
        append_s16(os2, 500);
        append_s16(os2, 700);
        append_u16(os2, 0);
        append_u16(os2, 0x0020);
        append_u16(os2, 0);
        tables.push_back({"OS/2", std::move(os2)});
    }

    // 2. cmap (format 4: U+0041..U+0043 -> glyphs 1..3)
    {
        std::vector<uint8_t> cmap;
        append_u16(cmap, 0);
        append_u16(cmap, 1);
        append_u16(cmap, 3);
        append_u16(cmap, 1);
        append_u32(cmap, 12);
        append_u16(cmap, 4);
        append_u16(cmap, 32);
        append_u16(cmap, 0);
        append_u16(cmap, 4);
        append_u16(cmap, 4);
        append_u16(cmap, 1);
        append_u16(cmap, 0);
        append_u16(cmap, 0x0043);
        append_u16(cmap, 0xFFFF);
        append_u16(cmap, 0);
        append_u16(cmap, 0x0041);
        append_u16(cmap, 0xFFFF);
        append_s16(cmap, -0x0040);
        append_s16(cmap, 1);
        append_u16(cmap, 0);
        append_u16(cmap, 0);
        tables.push_back({"cmap", std::move(cmap)});
    }

    // 3. fpgm
    tables.push_back({"fpgm", {0x01}});

    // 4. glyf & 8. loca (built together, inserted in tag-sorted order)
    std::vector<uint8_t> glyf;
    std::vector<uint8_t> loca;
    {
        auto appendSimpleSquare = [&](const std::vector<uint8_t>& ins) {
            append_s16(glyf, 1);
            append_s16(glyf, 0);
            append_s16(glyf, 0);
            append_s16(glyf, widthUnits);
            append_s16(glyf, 640);
            append_u16(glyf, 3);
            append_u16(glyf, static_cast<uint16_t>(ins.size()));
            glyf.insert(glyf.end(), ins.begin(), ins.end());
            glyf.push_back(0x31);  // pt 0: (0, 0)
            glyf.push_back(0x21);  // pt 1: (+widthUnits, 0)
            glyf.push_back(0x11);  // pt 2: (0, +640)
            glyf.push_back(0x21);  // pt 3: (-widthUnits, 0)
            append_s16(glyf, widthUnits);
            append_s16(glyf, static_cast<int16_t>(-widthUnits));
            append_s16(glyf, 640);
            if (glyf.size() % 2 != 0) {
                glyf.push_back(0);
            }
        };

        append_u16(loca, 0);  // glyph 0 (empty)
        append_u16(loca, 0);  // glyph 1 (simple with instructions)
        appendSimpleSquare(instructions);

        append_u16(loca, static_cast<uint16_t>(glyf.size() / 2));  // glyph 2 (composite)
        append_s16(glyf, -1);
        append_s16(glyf, 0);
        append_s16(glyf, 0);
        append_s16(glyf, widthUnits);
        append_s16(glyf, 640);
        // ARG_1_AND_2_ARE_WORDS | ARGS_ARE_XY_VALUES | WE_HAVE_INSTRUCTIONS
        append_u16(glyf, 0x0103);
        append_u16(glyf, 3);  // component glyphIndex = 3 (unhinted square)
        append_s16(glyf, 0);
        append_s16(glyf, 0);
        append_u16(glyf, static_cast<uint16_t>(instructions.size()));
        glyf.insert(glyf.end(), instructions.begin(), instructions.end());
        if (glyf.size() % 2 != 0) {
            glyf.push_back(0);
        }

        append_u16(loca, static_cast<uint16_t>(glyf.size() / 2));  // glyph 3 (unhinted simple)
        appendSimpleSquare({});

        append_u16(loca, static_cast<uint16_t>(glyf.size() / 2));  // end
    }
    tables.push_back({"glyf", std::move(glyf)});

    // 5. head (54 bytes, unitsPerEm = 1024 so 640 units at 16 ppem = 10 px)
    {
        std::vector<uint8_t> head;
        append_u32(head, 0x00010000);
        append_u32(head, 0x00010000);
        append_u32(head, 0);
        append_u32(head, 0x5F0F3CF5);
        append_u16(head, 0x000B);
        append_u16(head, 1024);
        for (int i = 0; i < 4; ++i) {
            append_u32(head, 0);
        }
        append_s16(head, 0);
        append_s16(head, 0);
        append_s16(head, widthUnits);
        append_s16(head, 640);
        append_u16(head, 0);
        append_u16(head, 8);
        append_s16(head, 2);
        append_s16(head, 0);
        append_s16(head, 0);
        tables.push_back({"head", std::move(head)});
    }

    // 6. hhea (36 bytes)
    {
        std::vector<uint8_t> hhea;
        append_u32(hhea, 0x00010000);
        append_s16(hhea, 800);
        append_s16(hhea, -200);
        append_s16(hhea, 0);
        append_u16(hhea, static_cast<uint16_t>(widthUnits));
        append_s16(hhea, 0);
        append_s16(hhea, 0);
        append_s16(hhea, widthUnits);
        append_s16(hhea, 1);
        for (int i = 0; i < 6; ++i) {
            append_s16(hhea, 0);
        }
        append_u16(hhea, 4);
        tables.push_back({"hhea", std::move(hhea)});
    }

    // 7. hmtx (4 glyphs * 4 bytes)
    {
        std::vector<uint8_t> hmtx;
        for (int i = 0; i < 4; ++i) {
            append_u16(hmtx, static_cast<uint16_t>(widthUnits));
            append_s16(hmtx, 0);
        }
        tables.push_back({"hmtx", std::move(hmtx)});
    }

    // 8. loca
    tables.push_back({"loca", std::move(loca)});

    // 9. maxp (32 bytes, maxTwilightPoints = 0)
    {
        std::vector<uint8_t> maxp;
        append_u32(maxp, 0x00010000);
        append_u16(maxp, 4);                  // numGlyphs
        append_u16(maxp, 4);                  // maxPoints
        append_u16(maxp, 1);                  // maxContours
        append_u16(maxp, 4);                  // maxCompositePoints
        append_u16(maxp, 1);                  // maxCompositeContours
        append_u16(maxp, 2);                  // maxZones
        append_u16(maxp, maxTwilightPoints);  // maxTwilightPoints
        append_u16(maxp, 0);                  // maxStorage
        append_u16(maxp, 0);                  // maxFunctionDefs
        append_u16(maxp, 0);                  // maxInstructionDefs
        append_u16(maxp, 64);                 // maxStackElements
        append_u16(maxp, static_cast<uint16_t>(instructions.size()));
        append_u16(maxp, 1);  // maxComponentElements
        append_u16(maxp, 1);  // maxComponentDepth
        tables.push_back({"maxp", std::move(maxp)});
    }

    // 10. name (6 bytes)
    {
        std::vector<uint8_t> name;
        append_u16(name, 0);
        append_u16(name, 0);
        append_u16(name, 6);
        tables.push_back({"name", std::move(name)});
    }

    // 11. post (32 bytes)
    {
        std::vector<uint8_t> post;
        append_u32(post, 0x00030000);
        for (int i = 0; i < 7; ++i) {
            append_u32(post, 0);
        }
        tables.push_back({"post", std::move(post)});
    }

    // 12. prep
    tables.push_back({"prep", {0x00}});

    return serialize_sfnt_tables(tables);
}

// Builds a minimal TrueType font whose simple glyph 1 and composite glyph 2 (referencing
// unhinted glyph 3) arm phantom point pp2 (index 5) with FT_CURVE_TAG_TOUCH_Y via MDAP[0] on
// load 1 and execute `secondLoadBody` under INSTCTRL 4,3 on load 2+ if tags[5] persisted.
sk_sp<SkData> make_phantom_divergent_ttf(SkSpan<const uint8_t> secondLoadBody,
                                         int16_t widthUnits = 640) {
    std::vector<uint8_t> instructions = {
            0x00,              // SVTCA[y]
            0xB0, 0x05,        // PUSHB[0] 5 (phantom point pp2)
            0xB8, 0x7D, 0x00,  // PUSHW[0] 32000 (+500 px in 26.6)
            0x38,              // SHPIX (gated on tags[5] & FT_CURVE_TAG_TOUCH_Y in v40 mode)
            0xB0, 0x05,        // PUSHB[0] 5
            0x46,              // GC[0]
            0xB8, 0x3E, 0x80,  // PUSHW[0] 16000 (> 250 px?)
            0x52,              // GT
            0x58,              // IF
            0xB1, 0x04, 0x03,  // PUSHB[1] 4 3
            0x8E,              // INSTCTRL (waive backward_compatibility for this run)
    };
    instructions.insert(instructions.end(), secondLoadBody.begin(), secondLoadBody.end());
    constexpr std::array<uint8_t, 5> kEpilogue = {
            0x59,  // EIF
            0x00,  // SVTCA[y]
            0xB0,
            0x05,  // PUSHB[0] 5
            0x2E,  // MDAP[0] (sets FT_CURVE_TAG_TOUCH_Y on pp2)
    };
    instructions.insert(instructions.end(), kEpilogue.begin(), kEpilogue.end());
    return make_hinted_ttf(instructions, widthUnits, /*maxTwilightPoints=*/0);
}

// Builds a minimal TrueType font that increments persistent twilight point 0 (zone 0) by +64 on
// every load and executes `body` on loads >= `triggerLoad` (1-based), independent of phantom-point
// rewind behavior.
sk_sp<SkData> make_twilight_divergent_ttf(SkSpan<const uint8_t> body,
                                          uint8_t triggerLoad,
                                          int16_t widthUnits = 640) {
    const uint16_t threshold = static_cast<uint16_t>(triggerLoad) * 64u;
    std::vector<uint8_t> instructions = {
            0xB1,
            0x04,
            0x03,  // PUSHB[1] 4 3
            0x8E,  // INSTCTRL (disable backward_compatibility for this run)
            0x00,  // SVTCA[y]
            0xB0,
            0x00,  // PUSHB[0] 0
            0x13,  // SZP0 (twilight zone)
            0xB0,
            0x00,  // PUSHB[0] 0
            0x15,  // SZP2 (twilight zone)
            0xB1,
            0x00,
            0x40,  // PUSHB[1] 0 64
            0x38,  // SHPIX (twilight[0].y += 64)
            0xB0,
            0x00,  // PUSHB[0] 0
            0x46,  // GC[0]
            0xB8,
            static_cast<uint8_t>((threshold >> 8) & 0xFF),
            static_cast<uint8_t>(threshold & 0xFF),  // PUSHW[0] threshold
            0x53,                                    // GTEQ
            0x58,                                    // IF
            0xB0,
            0x01,  // PUSHB[0] 1
            0x16,  // SZPS (restore glyph zone 1)
    };
    instructions.insert(instructions.end(), body.begin(), body.end());
    instructions.push_back(0x59);  // EIF
    return make_hinted_ttf(instructions, widthUnits, /*maxTwilightPoints=*/1);
}

// Builds a minimal bitmap-only SFNT font (EBLC + EBDT, 16 ppem strike, 8x8 glyph 1) with the
// specified EBDT `bitDepth` (1 -> MONO, 2 -> GRAY2, 4 -> GRAY4, 8 -> GRAY).
sk_sp<SkData> make_ebdt_bitmap_font(uint8_t bitDepth) {
    std::vector<SfntTableEntry> tables;
    const uint32_t rowBytes = (8u * bitDepth + 7u) / 8u;
    const uint32_t imageBytes = rowBytes * 8u;
    const uint32_t glyphDataSize = 5u + imageBytes;

    // 1. EBDT
    {
        std::vector<uint8_t> ebdt;
        append_u32(ebdt, 0x00020000);  // version 2.0
        ebdt.push_back(8);             // height
        ebdt.push_back(8);             // width
        ebdt.push_back(0);             // horiBearingX
        ebdt.push_back(8);             // horiBearingY
        ebdt.push_back(8);             // horiAdvance
        ebdt.insert(ebdt.end(), imageBytes, 0xFF);
        tables.push_back({"EBDT", std::move(ebdt)});
    }

    // 2. EBLC
    {
        std::vector<uint8_t> eblc;
        append_u32(eblc, 0x00020000);  // version 2.0
        append_u32(eblc, 1);           // numSizes = 1

        // BitmapSize record (48 bytes)
        append_u32(eblc, 8 + 48);  // indexSubTableArrayOffset = 56
        append_u32(eblc, 8 + 16);  // indexTablesSize = 24
        append_u32(eblc, 1);       // numberOfIndexSubTables = 1
        append_u32(eblc, 0);       // colorRef
        // hori SbitLineMetrics (12 bytes)
        eblc.push_back(14);                        // ascender
        eblc.push_back(static_cast<uint8_t>(-2));  // descender
        eblc.push_back(8);                         // widthMax
        eblc.insert(eblc.end(), 9, 0);
        // vert SbitLineMetrics (12 bytes)
        eblc.push_back(14);
        eblc.push_back(static_cast<uint8_t>(-2));
        eblc.push_back(8);
        eblc.insert(eblc.end(), 9, 0);
        append_u16(eblc, 1);       // startGlyphIndex
        append_u16(eblc, 1);       // endGlyphIndex
        eblc.push_back(16);        // ppemX
        eblc.push_back(16);        // ppemY
        eblc.push_back(bitDepth);  // bitDepth
        eblc.push_back(0x01);      // flags (HORIZONTAL_METRICS)

        // IndexSubTableArray (8 bytes at offset 56)
        append_u16(eblc, 1);  // firstGlyphIndex
        append_u16(eblc, 1);  // lastGlyphIndex
        append_u32(eblc, 8);  // additionalOffsetToIndexSubtable

        // IndexSubTable1 (16 bytes at offset 64)
        append_u16(eblc, 1);              // indexFormat = 1 (4-byte offsets)
        append_u16(eblc, 1);              // imageFormat = 1 (small metrics, byte-aligned)
        append_u32(eblc, 4);              // imageDataOffset (after 4-byte EBDT header)
        append_u32(eblc, 0);              // offsetArray[0]
        append_u32(eblc, glyphDataSize);  // offsetArray[1]

        tables.push_back({"EBLC", std::move(eblc)});
    }

    // 3. OS/2 (version 2, 96 bytes)
    {
        std::vector<uint8_t> os2;
        append_u16(os2, 2);
        append_s16(os2, 512);
        append_u16(os2, 400);
        append_u16(os2, 5);
        append_u16(os2, 0);
        for (int i = 0; i < 11; ++i) {
            append_s16(os2, 0);
        }
        os2.insert(os2.end(), 10, 0);
        for (int i = 0; i < 4; ++i) {
            append_u32(os2, 0);
        }
        append_tag(os2, "TEST");
        append_u16(os2, 0x0040);
        append_u16(os2, 0x0041);
        append_u16(os2, 0x0041);
        append_s16(os2, 800);
        append_s16(os2, -200);
        append_s16(os2, 0);
        append_u16(os2, 800);
        append_u16(os2, 200);
        append_u32(os2, 0);
        append_u32(os2, 0);
        append_s16(os2, 500);
        append_s16(os2, 700);
        append_u16(os2, 0);
        append_u16(os2, 0x0020);
        append_u16(os2, 0);
        tables.push_back({"OS/2", std::move(os2)});
    }

    // 4. cmap (format 4: U+0041 -> glyph 1)
    {
        std::vector<uint8_t> cmap;
        append_u16(cmap, 0);
        append_u16(cmap, 1);
        append_u16(cmap, 3);
        append_u16(cmap, 1);
        append_u32(cmap, 12);
        append_u16(cmap, 4);
        append_u16(cmap, 32);
        append_u16(cmap, 0);
        append_u16(cmap, 4);
        append_u16(cmap, 4);
        append_u16(cmap, 1);
        append_u16(cmap, 0);
        append_u16(cmap, 0x0041);
        append_u16(cmap, 0xFFFF);
        append_u16(cmap, 0);
        append_u16(cmap, 0x0041);
        append_u16(cmap, 0xFFFF);
        append_s16(cmap, -0x0040);
        append_s16(cmap, 1);
        append_u16(cmap, 0);
        append_u16(cmap, 0);
        tables.push_back({"cmap", std::move(cmap)});
    }

    // 5. head (54 bytes)
    {
        std::vector<uint8_t> head;
        append_u32(head, 0x00010000);
        append_u32(head, 0x00010000);
        append_u32(head, 0);
        append_u32(head, 0x5F0F3CF5);
        append_u16(head, 0x000B);
        append_u16(head, 1024);
        for (int i = 0; i < 4; ++i) {
            append_u32(head, 0);
        }
        append_s16(head, 0);
        append_s16(head, 0);
        append_s16(head, 512);
        append_s16(head, 512);
        append_u16(head, 0);
        append_u16(head, 8);
        append_s16(head, 2);
        append_s16(head, 0);
        append_s16(head, 0);
        tables.push_back({"head", std::move(head)});
    }

    // 6. hhea (36 bytes)
    {
        std::vector<uint8_t> hhea;
        append_u32(hhea, 0x00010000);
        append_s16(hhea, 800);
        append_s16(hhea, -200);
        append_s16(hhea, 0);
        append_u16(hhea, 512);
        append_s16(hhea, 0);
        append_s16(hhea, 0);
        append_s16(hhea, 512);
        append_s16(hhea, 1);
        for (int i = 0; i < 6; ++i) {
            append_s16(hhea, 0);
        }
        append_u16(hhea, 2);
        tables.push_back({"hhea", std::move(hhea)});
    }

    // 7. hmtx (2 glyphs * 4 bytes)
    {
        std::vector<uint8_t> hmtx;
        for (int i = 0; i < 2; ++i) {
            append_u16(hmtx, 512);
            append_s16(hmtx, 0);
        }
        tables.push_back({"hmtx", std::move(hmtx)});
    }

    // 8. maxp (version 0.5, 6 bytes: numGlyphs = 2)
    {
        std::vector<uint8_t> maxp;
        append_u32(maxp, 0x00005000);
        append_u16(maxp, 2);
        tables.push_back({"maxp", std::move(maxp)});
    }

    // 9. name (6 bytes)
    {
        std::vector<uint8_t> name;
        append_u16(name, 0);
        append_u16(name, 0);
        append_u16(name, 6);
        tables.push_back({"name", std::move(name)});
    }

    // 10. post (32 bytes)
    {
        std::vector<uint8_t> post;
        append_u32(post, 0x00030000);
        for (int i = 0; i < 7; ++i) {
            append_u32(post, 0);
        }
        tables.push_back({"post", std::move(post)});
    }

    return serialize_sfnt_tables(tables);
}

bool glyph_image_has_nonzero_byte(const SkGlyph& glyph) {
    const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
    for (size_t i = 0; i < glyph.imageSize(); ++i) {
        if (bytes[i] != 0) {
            return true;
        }
    }
    return false;
}

}  // namespace

DEF_TEST(FontHost_FreeTypePhantomPointDivergence, reporter) {
    sk_sp<SkFontMgr> ftMgr = SkFontMgr_New_Custom_Empty();
    REPORTER_ASSERT(reporter, ftMgr);

    constexpr auto kShiftRight8200Px = std::to_array<uint8_t>({
            0x01,  // SVTCA[x]
            0xB0,
            0x02,  // PUSHB[0] 2
            0x17,  // SLOOP
            0xB1,
            0x01,
            0x02,  // PUSHB[1] 1 2
            0xB9,
            0x64,
            0x00,
            0x05,
            0x20,  // PUSHW[1] 25600 1312
            0x63,  // MUL (8200 * 64)
            0x38,  // SHPIX
    });

    // 1. b/574236375 PoC: second load shifts points 1 & 2 by +8200 px in X (imageTooLarge())
    // under a blur mask filter, for both simple glyph 1 and composite glyph 2, including SKP
    // round-trip deserialization through the FreeType typeface factory.
    {
        sk_sp<SkData> fontData = make_phantom_divergent_ttf(kShiftRight8200Px);
        for (SkGlyphID glyphID : {SkGlyphID{1}, SkGlyphID{2}}) {
            sk_sp<SkTypeface> tf = ftMgr->makeFromData(fontData);
            REPORTER_ASSERT(reporter, tf);

            SkPictureRecorder recorder;
            SkCanvas* recCanvas = recorder.beginRecording(64, 64);
            SkFont font(tf, 16.0f);
            font.setHinting(SkFontHinting::kNormal);
            font.setEdging(SkFont::Edging::kAntiAlias);
            SkPaint paint;
            paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0f));
            constexpr SkPoint kPos = SkPoint::Make(20.0f, 20.0f);
            recCanvas->drawGlyphs({&glyphID, 1}, {&kPos, 1}, {0.0f, 0.0f}, font, paint);
            sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
            sk_sp<SkData> skp = pic->serialize();
            REPORTER_ASSERT(reporter, skp);

            sk_sp<SkPicture> replayed = SkPicture::MakeFromData(skp.get());
            REPORTER_ASSERT(reporter, replayed);
            SkBitmap bm;
            REPORTER_ASSERT(reporter, bm.tryAllocN32Pixels(64, 64));
            SkCanvas canvas(bm);
            canvas.clear(SK_ColorWHITE);
            canvas.drawPicture(replayed);
        }
    }

    // 2. Horizontal and vertical LCD16 outline clipping when load 2 shifts left/up via stale
    // phantom-point tags (both disjoint and partially overlapping in subpixels).
    for (bool verticalLCD : {false, true}) {
        for (int16_t deltaFDot6 : {int16_t{-1280}, int16_t{-320}, int16_t{320}, int16_t{1280}}) {
            const uint8_t axisOp = verticalLCD ? 0x00 : 0x01;  // SVTCA[y] or SVTCA[x]
            const auto shiftAll4Points = std::to_array<uint8_t>({
                    axisOp,
                    0xB0,
                    0x04,  // PUSHB[0] 4
                    0x17,  // SLOOP
                    0xB3,
                    0x00,
                    0x01,
                    0x02,
                    0x03,  // PUSHB[3] 0 1 2 3
                    0xB8,
                    static_cast<uint8_t>((static_cast<uint16_t>(deltaFDot6) >> 8) & 0xFF),
                    static_cast<uint8_t>(static_cast<uint16_t>(deltaFDot6) & 0xFF),
                    0x38,  // SHPIX
            });
            sk_sp<SkData> fontData = make_phantom_divergent_ttf(shiftAll4Points);
            sk_sp<SkTypeface> tf = ftMgr->makeFromData(fontData);
            REPORTER_ASSERT(reporter, tf);

            SkFont font(tf, 16.0f);
            font.setHinting(SkFontHinting::kNormal);
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
            SkPaint paint;
            const SkPixelGeometry geom =
                    verticalLCD ? kRGB_V_SkPixelGeometry : kRGB_H_SkPixelGeometry;
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(font,
                                                             paint,
                                                             SkSurfaceProps(0, geom),
                                                             SkScalerContextFlags::kNone,
                                                             SkMatrix::I());
            std::unique_ptr<SkScalerContext> ctx = strikeSpec.createScalerContext();

            SkArenaAlloc alloc{512};
            SkGlyph glyph = ctx->makeGlyph(SkPackedGlyphID(SkGlyphID{1}), &alloc);
            REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kLCD16_Format);
            REPORTER_ASSERT(reporter, !glyph.isEmpty());
            REPORTER_ASSERT(reporter, glyph.setImage(&alloc, ctx.get()));

            // +/-20 px shifts move the 12 px LCD bitmap completely outside Load 1's bounds.
            // Only assert a blank mask if FreeType's phantom-point bug actually caused the
            // subsequent load to diverge (keeping the test green once FreeType fixes rewind).
            if (deltaFDot6 == -1280 || deltaFDot6 == 1280) {
                SkGlyph probe = ctx->makeGlyph(SkPackedGlyphID(SkGlyphID{1}), &alloc);
                if (probe.left() != glyph.left() || probe.top() != glyph.top()) {
                    REPORTER_ASSERT(reporter, !glyph_image_has_nonzero_byte(glyph));
                }
            }
        }
    }

    // 3. Subpixel outline translation (A8, BW, H-LCD, V-LCD) when load 2 shifts bbox near
    // INT32_MAX or INT32_MIN (fits in SkFDot6, but bbox + dx/xShift overflows 32-bit SkFDot6).
    {
        // +0x7fffffd8 in X: bbox.xMin = 0x7fffffd8, bbox.xMax = 0x7ffffff8 (with widthUnits = 32),
        // so subpixel dx = +48 pushes bbox.xMin + dx to 0x80000008.
        constexpr auto kShiftNearInt32Max = std::to_array<uint8_t>({
                0x01,                          // SVTCA[x]
                0xB0, 0x04,                    // PUSHB[0] 4
                0x17,                          // SLOOP
                0xB3, 0x00, 0x01, 0x02, 0x03,  // PUSHB[3] 0 1 2 3
                0xB8, 0x40, 0x00,              // PUSHW[0] 16384
                0x20,                          // DUP
                0x63,                          // MUL -> 0x00400000
                0xB8, 0x7F, 0xFF,              // PUSHW[0] 32767
                0x63,                          // MUL -> 0x7fff0000
                0xB8, 0x7F, 0xFF,              // PUSHW[0] 32767
                0x60,                          // ADD -> 0x7fff7fff
                0xB8, 0x7F, 0xD9,              // PUSHW[0] 32729
                0x60,                          // ADD -> 0x7fffffd8
                0x38,                          // SHPIX
        });
        // -0x7fffffe8 in Y: bbox.yMin = -0x7fffffe8, so dy = -48 (from subY = 0.75) underflows
        // 32-bit SkFDot6.
        constexpr auto kShiftNearInt32MinY = std::to_array<uint8_t>({
                0x00,                          // SVTCA[y]
                0xB0, 0x04,                    // PUSHB[0] 4
                0x17,                          // SLOOP
                0xB3, 0x00, 0x01, 0x02, 0x03,  // PUSHB[3] 0 1 2 3
                0xB8, 0x40, 0x00,              // PUSHW[0] 16384
                0x20,                          // DUP
                0x63,                          // MUL -> 0x00400000
                0xB8, 0x7F, 0xFF,              // PUSHW[0] 32767
                0x63,                          // MUL -> 0x7fff0000
                0xB8, 0x7F, 0xFF,              // PUSHW[0] 32767
                0x60,                          // ADD -> 0x7fff7fff
                0xB8, 0x7F, 0xE9,              // PUSHW[0] 32745
                0x60,                          // ADD -> 0x7fffffe8
                0x65,                          // NEG -> -0x7fffffe8
                0x38,                          // SHPIX
        });

        for (SkSpan<const uint8_t> prog : {SkSpan<const uint8_t>(kShiftNearInt32Max),
                                           SkSpan<const uint8_t>(kShiftNearInt32MinY)}) {
            for (SkFont::Edging edging : {SkFont::Edging::kAlias,
                                          SkFont::Edging::kAntiAlias,
                                          SkFont::Edging::kSubpixelAntiAlias}) {
                for (SkPixelGeometry geom : {kRGB_H_SkPixelGeometry, kRGB_V_SkPixelGeometry}) {
                    sk_sp<SkData> fontData =
                            make_twilight_divergent_ttf(prog, /*triggerLoad=*/2, /*widthUnits=*/32);
                    sk_sp<SkTypeface> tf = ftMgr->makeFromData(fontData);
                    REPORTER_ASSERT(reporter, tf);

                    SkFont font(tf, 16.0f);
                    font.setHinting(SkFontHinting::kNormal);
                    font.setEdging(edging);
                    font.setSubpixel(true);
                    SkPaint paint;
                    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(font,
                                                                     paint,
                                                                     SkSurfaceProps(0, geom),
                                                                     SkScalerContextFlags::kNone,
                                                                     SkMatrix::I());
                    std::unique_ptr<SkScalerContext> ctx = strikeSpec.createScalerContext();

                    SkArenaAlloc alloc{512};
                    SkGlyph glyph = ctx->makeGlyph(
                            SkPackedGlyphID(
                                    SkGlyphID{1}, SkFloatToFixed(0.75f), SkFloatToFixed(0.75f)),
                            &alloc);
                    REPORTER_ASSERT(reporter, !glyph.isEmpty());
                    REPORTER_ASSERT(reporter, glyph.setImage(&alloc, ctx.get()));
                    REPORTER_ASSERT(reporter, !glyph_image_has_nonzero_byte(glyph));
                }
            }
        }
    }

    // 4. Persistent twilight-zone (zone 0) divergence:
    //    (a) Disjoint +/-20 px shifts on Load 2 must leave BW, A8, H-LCD, and V-LCD masks cleared.
    //    (b) 3-load divergence under a blur mask filter (Load 1 origGlyph and Load 2 tmpGlyph stay
    //        10x10, while Load 3 inside generateImage() expands by +8200 px or shifts by -20 px).
    {
        for (SkFont::Edging edging : {SkFont::Edging::kAlias,
                                      SkFont::Edging::kAntiAlias,
                                      SkFont::Edging::kSubpixelAntiAlias}) {
            for (int16_t deltaFDot6 : {int16_t{-1280}, int16_t{1280}}) {
                const auto shiftX = std::to_array<uint8_t>({
                        0x01,  // SVTCA[x]
                        0xB0,
                        0x04,  // PUSHB[0] 4
                        0x17,  // SLOOP
                        0xB3,
                        0x00,
                        0x01,
                        0x02,
                        0x03,  // PUSHB[3] 0 1 2 3
                        0xB8,
                        static_cast<uint8_t>((static_cast<uint16_t>(deltaFDot6) >> 8) & 0xFF),
                        static_cast<uint8_t>(static_cast<uint16_t>(deltaFDot6) & 0xFF),
                        0x38,  // SHPIX
                });
                sk_sp<SkData> fontData = make_twilight_divergent_ttf(shiftX, /*triggerLoad=*/2);
                sk_sp<SkTypeface> tf = ftMgr->makeFromData(fontData);
                REPORTER_ASSERT(reporter, tf);

                SkFont font(tf, 16.0f);
                font.setHinting(SkFontHinting::kNormal);
                font.setEdging(edging);
                SkPaint paint;
                SkStrikeSpec strikeSpec =
                        SkStrikeSpec::MakeMask(font,
                                               paint,
                                               SkSurfaceProps(0, kRGB_H_SkPixelGeometry),
                                               SkScalerContextFlags::kNone,
                                               SkMatrix::I());
                std::unique_ptr<SkScalerContext> ctx = strikeSpec.createScalerContext();

                SkArenaAlloc alloc{512};
                SkGlyph glyph = ctx->makeGlyph(SkPackedGlyphID(SkGlyphID{1}), &alloc);
                REPORTER_ASSERT(reporter, !glyph.isEmpty());
                REPORTER_ASSERT(reporter, glyph.setImage(&alloc, ctx.get()));
                REPORTER_ASSERT(reporter, !glyph_image_has_nonzero_byte(glyph));
            }
        }

        // Load 3 divergence under a blur mask filter (bypasses getImage()'s Load 2 tmpGlyph check).
        for (SkFont::Edging edging :
             {SkFont::Edging::kAntiAlias, SkFont::Edging::kSubpixelAntiAlias}) {
            sk_sp<SkData> fontData =
                    make_twilight_divergent_ttf(kShiftRight8200Px, /*triggerLoad=*/3);
            sk_sp<SkTypeface> tf = ftMgr->makeFromData(fontData);
            REPORTER_ASSERT(reporter, tf);

            SkFont font(tf, 16.0f);
            font.setHinting(SkFontHinting::kNormal);
            font.setEdging(edging);
            SkPaint paint;
            paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0f));
            SkStrikeSpec strikeSpec =
                    SkStrikeSpec::MakeMask(font,
                                           paint,
                                           SkSurfaceProps(0, kRGB_H_SkPixelGeometry),
                                           SkScalerContextFlags::kNone,
                                           SkMatrix::I());
            std::unique_ptr<SkScalerContext> ctx = strikeSpec.createScalerContext();

            SkArenaAlloc alloc{1024};
            SkGlyph glyph = ctx->makeGlyph(SkPackedGlyphID(SkGlyphID{1}), &alloc);
            REPORTER_ASSERT(reporter, !glyph.isEmpty());
            REPORTER_ASSERT(reporter, glyph.setImage(&alloc, ctx.get()));
        }
    }

    // 5. Embedded EBDT bitmap strikes with unsupported pixel modes (2-bit GRAY2, 4-bit GRAY4) and
    // unsupported format pairs (8-bit GRAY with kBW_Format) at both identity and scaled sizes must
    // not assert and must zero-initialize the destination glyph mask.
    for (uint8_t bitDepth : {uint8_t{2}, uint8_t{4}, uint8_t{8}}) {
        sk_sp<SkData> fontData = make_ebdt_bitmap_font(bitDepth);
        sk_sp<SkTypeface> tf = ftMgr->makeFromData(fontData);
        REPORTER_ASSERT(reporter, tf);

        for (SkScalar textSize : {16.0f, 24.0f}) {
            for (SkFont::Edging edging : {SkFont::Edging::kAlias,
                                          SkFont::Edging::kAntiAlias,
                                          SkFont::Edging::kSubpixelAntiAlias}) {
                SkFont font(tf, textSize);
                font.setEmbeddedBitmaps(true);
                font.setEdging(edging);
                SkPaint paint;
                SkStrikeSpec strikeSpec =
                        SkStrikeSpec::MakeMask(font,
                                               paint,
                                               SkSurfaceProps(0, kRGB_H_SkPixelGeometry),
                                               SkScalerContextFlags::kNone,
                                               SkMatrix::I());
                std::unique_ptr<SkScalerContext> ctx = strikeSpec.createScalerContext();

                SkArenaAlloc alloc{512};
                // Poison an initial arena chunk so an uninitialized glyph image would retain non-0.
                void* poison = alloc.makeBytesAlignedTo(256, 4);
                memset(poison, 0xCC, 256);

                SkGlyph glyph = ctx->makeGlyph(SkPackedGlyphID(SkGlyphID{1}), &alloc);
                REPORTER_ASSERT(reporter, !glyph.isEmpty());
                REPORTER_ASSERT(reporter, glyph.setImage(&alloc, ctx.get()));

                const bool expectZero =
                        (bitDepth == 2 || bitDepth == 4) ||
                        (bitDepth == 8 && textSize == 16.0f && edging == SkFont::Edging::kAlias);
                if (expectZero) {
                    REPORTER_ASSERT(reporter, !glyph_image_has_nonzero_byte(glyph));
                } else {
                    REPORTER_ASSERT(reporter, glyph_image_has_nonzero_byte(glyph));
                }
            }
        }
    }
}
#endif  // defined(SK_FONTMGR_FREETYPE_EMPTY_AVAILABLE)

// need tests for SkStrSearch
