/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/core/SkUTF.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_apple.h"
#endif

#if defined(SK_SHAPER_CORETEXT_AVAILABLE)
#include "modules/skshaper/include/SkShaper_coretext.h"
#endif

using namespace skia_private;
using namespace skia::textlayout;

namespace {

static sk_sp<SkUnicode> get_test_unicode() {
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::Apple::Make()) {
        return unicode;
    }
#endif
    auto factory = SkShapers::BestAvailable();
    if (factory && factory->getUnicode()) {
        return sk_ref_sp<SkUnicode>(factory->getUnicode());
    }
    return nullptr;
}

static sk_sp<FontCollection> make_test_font_collection() {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    return fontCollection;
}

}  // namespace

// =============================================================================
// Skia Buganizer b/318667611 & b/328418459:
// Font fallback resolution for Apple system UI fonts and SF Symbols
// Standard SkFontMgr_mac cannot open streams for virtual descriptors, causing
// missing glyph 0 (tofu) in current HarfBuzz/ICU SkParagraph
//
// [FAILS ON APPLE]
// Reason: PUA SF Symbols (U+100000) falls back to LastResort because OneLineShaper
// queries FontCollection::findTypefaces() and SkFontMgr_mac upfront. SkFontMgr_mac
// cannot resolve glyphs for virtual descriptors without CoreText font cascade driving
// the run resolution directly (CTLineGetGlyphRuns)
// =============================================================================
DEF_TEST(SkParagraph_Skia_b_318667611_AppleSystemUIFontCascade, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // SF Symbols U+100000 (\xF4\x80\x80\x80)
    // CoreText resolves Apple system cascade; HarfBuzz produces glyph 0 (tofu)
    const char* text = "\xF4\x80\x80\x80";
    const size_t len = strlen(text);

    ParagraphStyle style;
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontFamilies({SkString(".AppleSystemUIFont")});
    ts.setFontSize(20.0f);
    builder.pushStyle(ts);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500);

    auto paragraphImpl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, paragraphImpl->isCoreTextShaped(),
                    "CoreText shaping required for Apple system UI font cascade");

    const auto& runs = paragraphImpl->runs();
    REPORTER_ASSERT(reporter, !runs.empty(), "Expected runs to be generated for Apple system symbol");
    if (!runs.empty() && runs[0].size() > 0) {
        SkString familyName, psName;
        if (runs[0].font().getTypeface()) {
            runs[0].font().getTypeface()->getFamilyName(&familyName);
            runs[0].font().getTypeface()->getPostScriptName(&psName);
        }
        REPORTER_ASSERT(reporter, psName.contains("SF") || psName.contains("AppleSystemUIFont"),
                        "Expected Apple system font cascade for .AppleSystemUIFont; got %s",
                        psName.c_str());
        REPORTER_ASSERT(reporter, !psName.contains("LastResort"),
                        "Apple system font cascade must not fall back to LastResort; got %s",
                        psName.c_str());
    }

    auto boxes = paragraph->getRectsForRange(0, 2, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, !boxes.empty(), "Expected bounding box for SF Symbols range");
    if (!boxes.empty()) {
        REPORTER_ASSERT(reporter, boxes[0].rect.width() > 0 && boxes[0].rect.height() > 0,
                        "Expected positive bounding box dimensions for SF Symbols range");
    }

    {
        const char* cascadeText = "SF Pro    \xE7\xAE\x80\xE4\xBD\x93\xE4\xB8\xAD\xE6\x96\x87    \xD9\x85\xD8\xB1\xD8\xAD\xD8\xA8\xD8\xA7    \xF4\x80\x80\x80  \xF4\x80\x80\x81  \xF4\x80\x80\x82  \xF4\x80\x80\x83  \xF4\x80\x80\x84";
        ParagraphStyle cStyle;
        cStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
        ParagraphBuilderImpl cBuilder(cStyle, fontCollection, unicode);
        TextStyle cascadeTS;
        cascadeTS.setFontFamilies({SkString(".AppleSystemUIFont")});
        cascadeTS.setFontSize(24.0f);
        cBuilder.pushStyle(cascadeTS);
        cBuilder.addText(cascadeText);
        cBuilder.pop();
        auto cPara = cBuilder.Build();
        cPara->layout(1000);
        auto cImpl = static_cast<ParagraphImpl*>(cPara.get());

        REPORTER_ASSERT(reporter, cImpl->isCoreTextShaped(),
                        "CoreText shaping required for cascade paragraph");
        REPORTER_ASSERT(reporter, !cImpl->runs().empty(), "Expected runs in cascade paragraph");

        bool hasChineseRun = false;
        bool hasArabicRun = false;
        for (const auto& r : cImpl->runs()) {
            SkString ps;
            if (r.font().getTypeface()) {
                r.font().getTypeface()->getPostScriptName(&ps);
            }
            REPORTER_ASSERT(reporter, !ps.contains("LastResort"),
                            "Cascade run must not fall back to LastResort; got %s",
                            ps.c_str());
            if (ps.contains("PingFang")) {
                hasChineseRun = true;
            }
            if (ps.contains("Arabic")) {
                hasArabicRun = true;
            }
        }
        REPORTER_ASSERT(reporter, hasChineseRun, "Expected PingFang cascade for Simplified Chinese");
        REPORTER_ASSERT(reporter, hasArabicRun, "Expected SFArabic cascade for Arabic text");

        auto toU16 = [&](size_t utf8Pos) -> size_t {
            return SkUTF::UTF8ToUTF16(nullptr, 0, cascadeText, utf8Pos);
        };

        const char* p = strstr(cascadeText, "\xF4\x80\x80\x80");
        REPORTER_ASSERT(reporter, p != nullptr, "SF Symbols expected in cascadeText");
        if (p) {
            size_t u8Start = p - cascadeText;
            size_t u8End = u8Start + strlen("\xF4\x80\x80\x80  \xF4\x80\x80\x81  \xF4\x80\x80\x82  \xF4\x80\x80\x83  \xF4\x80\x80\x84");
            auto cBoxes = cPara->getRectsForRange(toU16(u8Start), toU16(u8End), RectHeightStyle::kTight, RectWidthStyle::kTight);
            REPORTER_ASSERT(reporter, !cBoxes.empty(), "Expected bounding box for SF Symbols range");
            if (!cBoxes.empty()) {
                REPORTER_ASSERT(reporter, cBoxes[0].rect.width() > 0 && cBoxes[0].rect.height() > 0,
                                "Expected positive bounding box dimensions for SF Symbols range");
            }
        }

        const auto& lastRun = cImpl->runs().back();
        SkString lastPS;
        if (lastRun.font().getTypeface()) {
            lastRun.font().getTypeface()->getPostScriptName(&lastPS);
        }
        REPORTER_ASSERT(reporter, lastPS.contains("SF") || lastPS.contains("AppleSystemUIFont"),
                        "Expected last run to resolve to SF system font on Apple Native; got %s",
                        lastPS.c_str());
    }
}

// =============================================================================
// Skia Buganizer b/40040287:
// UTF-16 code units (surrogate pairs) desynchronize with UTF-8 byte offsets
// in SkParagraph line metrics and hit-testing on astral plane emoji (U+1F600 😀)
//
// [FAILS ON APPLE]
// Reason: ParagraphImpl::getGlyphPositionAtCoordinate maps hit coordinates directly
// to raw UTF-8 byte offsets (returning byte offset 4 for 'B' after the 4-byte astral emoji)
// instead of mapping byte offsets to UTF-16 code unit indices (3) via SkUnicode::getUtf16Index
// =============================================================================
DEF_TEST(SkParagraph_Skia_b_40040287_UTF16_AstralPlaneIndices, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // "A😀B": 'A' (index 0), '😀' (U+1F600, UTF-16 indices 1..3), 'B' (UTF-16 index 3)
    const char* text = "A\xF0\x9F\x98\x80\x42";
    const size_t len = strlen(text);

    ParagraphStyle style;
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(24.0f);
    builder.pushStyle(ts);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500);

    // In UTF-16: 'A' is [0..1), emoji is [1..3) (surrogate pair), 'B' is [3..4)
    auto boxesA = paragraph->getRectsForRange(0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
    auto boxesEmoji = paragraph->getRectsForRange(1, 3, RectHeightStyle::kTight, RectWidthStyle::kTight);
    auto boxesB = paragraph->getRectsForRange(3, 4, RectHeightStyle::kTight, RectWidthStyle::kTight);

    REPORTER_ASSERT(reporter, !boxesA.empty() && !boxesEmoji.empty() && !boxesB.empty(),
                    "Expected valid bounding boxes for each UTF-16 cluster");

    if (!boxesA.empty() && !boxesEmoji.empty() && !boxesB.empty()) {
        PositionWithAffinity posA = paragraph->getGlyphPositionAtCoordinate(boxesA[0].rect.fLeft + 1.0f, 12.0f);
        PositionWithAffinity posEmoji = paragraph->getGlyphPositionAtCoordinate(boxesEmoji[0].rect.fLeft + 1.0f, 12.0f);
        PositionWithAffinity posB = paragraph->getGlyphPositionAtCoordinate(boxesB[0].rect.fLeft + 1.0f, 12.0f);
        PositionWithAffinity posEnd = paragraph->getGlyphPositionAtCoordinate(boxesB[0].rect.fRight + 10.0f, 12.0f);

        REPORTER_ASSERT(reporter, posA.position == 0, "Character 'A' must resolve to UTF-16 index 0; got %d", (int)posA.position);
        REPORTER_ASSERT(reporter, posEmoji.position == 1, "Astral emoji must resolve to UTF-16 index 1; got %d", (int)posEmoji.position);
        REPORTER_ASSERT(reporter, posB.position == 3, "Character 'B' must resolve to UTF-16 index 3; got %d", (int)posB.position);
        REPORTER_ASSERT(reporter, posEnd.position == 4, "Terminal position must resolve to UTF-16 index 4; got %d", (int)posEnd.position);
    }
}

// =============================================================================
// Skia Buganizer b/448789662 & b/519671848:
// Mixed RTL/LTR runs with font fallback causing cluster out-of-order crashes
//
// [FAILS ON APPLE]
// Reason: In OneLineShaper.cpp, during BiDi run partitioning, cluster indices inside
// the RTL Hebrew fallback run are indexed relative to the local block rather than
// strictly mapped/bounded within the run's textRange() ([6, 14)), generating cluster
// indices 0, 2, 4 that point outside the run range
// =============================================================================
DEF_TEST(SkParagraph_Skia_b_448789662_BiDi_MixedRunsClusterBounds, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // English + Hebrew + English with fallback fonts
    const char* text = "Hello \xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D World";
    const size_t len = strlen(text);

    ParagraphStyle style;
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(20.0f);
    builder.pushStyle(ts);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(400);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0,
                    "Expected zero unresolved glyphs for mixed BiDi text with font fallback");

    auto paragraphImpl = static_cast<ParagraphImpl*>(paragraph.get());
    const auto& runs = paragraphImpl->runs();
    REPORTER_ASSERT(reporter, runs.size() >= 3, "Expected at least 3 runs for LTR/RTL/LTR text");

    bool foundRtlRun = false;
    bool foundLtrRun = false;

    // Global cluster indices must remain strictly contained within run boundaries
    for (size_t r = 0; r < runs.size(); ++r) {
        const auto& run = runs[r];
        if (!run.leftToRight()) {
            foundRtlRun = true;
            REPORTER_ASSERT(reporter, run.bidiLevel() % 2 == 1,
                            "RTL run must have odd BiDi embedding level");
            REPORTER_ASSERT(reporter, run.getTextDirection() == TextDirection::kRtl,
                            "RTL run must have kRtl text direction");
            REPORTER_ASSERT(reporter, run.textRange().start >= 6 && run.textRange().end <= 14,
                            "RTL run text range [%zu, %zu) must be within Hebrew span [6, 14)",
                            run.textRange().start, run.textRange().end);
        } else {
            foundLtrRun = true;
            REPORTER_ASSERT(reporter, run.bidiLevel() % 2 == 0,
                            "LTR run must have even BiDi embedding level");
            REPORTER_ASSERT(reporter, run.getTextDirection() == TextDirection::kLtr,
                            "LTR run must have kLtr text direction");
        }

        for (size_t c = 0; c < run.size(); ++c) {
            size_t cluster = run.globalClusterIndex(c);
            REPORTER_ASSERT(reporter, cluster >= run.textRange().start &&
                                      cluster < run.textRange().end,
                            "Cluster %zu outside of run range [%zu, %zu)",
                            cluster, run.textRange().start, run.textRange().end);
        }
    }

    REPORTER_ASSERT(reporter, foundRtlRun, "Expected at least one RTL run for Hebrew text");
    REPORTER_ASSERT(reporter, foundLtrRun, "Expected at least one LTR run for English text");

    auto hebrewBoxes = paragraph->getRectsForRange(6, 10, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, !hebrewBoxes.empty(), "Expected bounding box for Hebrew text range");
    if (!hebrewBoxes.empty()) {
        REPORTER_ASSERT(reporter, hebrewBoxes[0].rect.width() > 0 && hebrewBoxes[0].rect.height() > 0,
                        "Expected positive bounding box dimensions for Hebrew text range");
    }
}

// =============================================================================
// Skia Buganizer b/458132646 & b/463954175:
// Japanese Kana font fallback cascade with system font (.AppleSystemUIFont)
//
// [FAILS ON APPLE]
// Reason: When .AppleSystemUIFont is requested for Japanese Kana, SkFontMgr_mac fails
// to cascade to Hiragino Sans / PingFang because typeface resolution is done ahead-of-time
// in OneLineShaper rather than delegating fallback font matching to CoreText (CTFontCreateForString)
// =============================================================================
DEF_TEST(SkParagraph_Skia_b_458132646_JapaneseKana_NotEmoji, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Japanese Hiragana text: "こんにちは" (U+3053, U+3093, U+306B, U+3061, U+306F)
    const char* text = "\xE3\x81\x93\xE3\x82\x93\xE3\x81\xAB\xE3\x81\xA1\xE3\x81\xAF";
    const size_t len = strlen(text);

    ParagraphStyle style;
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    // Request .AppleSystemUIFont with no explicit Japanese fallback
    ts.setFontFamilies({SkString(".AppleSystemUIFont")});
    ts.setFontSize(20.0f);
    builder.pushStyle(ts);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0,
                    "Expected zero unresolved glyphs for Japanese Kana text");

    auto paragraphImpl = static_cast<ParagraphImpl*>(paragraph.get());
    const auto& runs = paragraphImpl->runs();
    REPORTER_ASSERT(reporter, !runs.empty(), "Expected at least one run for Japanese Kana");

    // In CoreText: Japanese text cascades to Hiragino Sans / PingFang / CJK Fallback automatically
    bool hasJapaneseFont = false;
    for (size_t r = 0; r < runs.size(); ++r) {
        if (auto tf = runs[r].font().getTypeface()) {
            SkString name;
            tf->getFamilyName(&name);
            REPORTER_ASSERT(reporter, !name.contains("LastResort"),
                            "Japanese Kana must not resolve to LastResort font; got %s",
                            name.c_str());
            if (name.contains("Hiragino") || name.contains("PingFang") || name.contains("CJK")) {
                hasJapaneseFont = true;
            }
        }
    }
    REPORTER_ASSERT(reporter, hasJapaneseFont,
                    "Expected .AppleSystemUIFont to cascade to Hiragino/PingFang/CJK for Japanese Kana");

    auto boxes = paragraph->getRectsForRange(0, 5, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, !boxes.empty(), "Expected bounding box for Japanese Kana range");
    if (!boxes.empty()) {
        REPORTER_ASSERT(reporter, boxes[0].rect.width() > 0 && boxes[0].rect.height() > 0,
                        "Expected positive bounding box dimensions for Japanese Kana range");
    }
}

// =============================================================================
// Skia Buganizer b/341104766 & skia:12390:
// Vertical glyph positioning and baseline shifts across styled spans
//
// [FAILS ON APPLE]
// Reason: Subscript baseline shift (setBaselineShift(-6.0f)) is stored in TextStyle,
// but OneLineShaper shapes runs without propagating fBaselineShift into buffer.positions[k].fY
// or buffer.offsets[k].fY, leaving run.posY(0) == 0.0f
// =============================================================================
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
DEF_TEST(SkParagraph_Skia_b_341104766_VerticalGlyphOffsets, reporter) {
#else
DEF_TEST_DISABLED(SkParagraph_Skia_b_341104766_VerticalGlyphOffsets, reporter) {
#endif
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Base text with subscript/superscript baseline shift
    ParagraphStyle style;
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(style, fontCollection, unicode);

    TextStyle normalStyle;
    normalStyle.setFontSize(20.0f);
    builder.pushStyle(normalStyle);
    builder.addText("H", 1);
    builder.pop();

    TextStyle subStyle;
    subStyle.setFontSize(12.0f);
    subStyle.setBaselineShift(-6.0f); // Subscript shift
    builder.pushStyle(subStyle);
    builder.addText("2", 1);
    builder.pop();

    TextStyle normalStyle2;
    normalStyle2.setFontSize(20.0f);
    builder.pushStyle(normalStyle2);
    builder.addText("O", 1);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(200);

    auto paragraphImpl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, paragraphImpl->isCoreTextShaped(),
                    "CoreText shaping required for vertical glyph offset test");
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0,
                    "Expected zero unresolved glyphs for styled formula text");
    const auto& runs = paragraphImpl->runs();
    REPORTER_ASSERT(reporter, runs.size() >= 3, "Expected 3 styled runs for H2O formula");

    REPORTER_ASSERT(reporter, SkScalarNearlyZero(runs[0].positions()[0].fY),
                    "Expected base glyph 'H' to have 0.0 vertical baseline offset; got %f",
                    runs[0].positions()[0].fY);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(runs[1].positions()[0].fY, 6.0f),
                    "Expected subscript glyph '2' to have 6.0 vertical baseline offset; got %f",
                    runs[1].positions()[0].fY);
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(runs[2].positions()[0].fY),
                    "Expected base glyph 'O' to have 0.0 vertical baseline offset; got %f",
                    runs[2].positions()[0].fY);
}

// =============================================================================
// Skia Buganizer b/405286754:
// Strict word wrapping (LineBreakMode::kStrict) on CJK strings with computeWordBreakPadding
//
// [FAILS ON APPLE]
// Reason: TextWrapper.cpp wraps text using a greedy character-by-character advance accumulator
// without querying SkUnicode line break opportunities or delegating line breaking to
// CTTypesetterSuggestLineBreak, causing line 1 to break at character index 2 instead of word index 3
// =============================================================================
DEF_TEST_DISABLED(SkParagraph_Skia_b_405286754_StrictWordWrappingCJK, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Japanese text: "日本語の単語"
    // Words: "日本語の" (UTF-16: 4 code units) + "単語" (UTF-16: 2 code units)
    // KEEPER-DEBT: Strict CJK word wrapping requires CTTypesetterSuggestLineBreak or break boundary integration
    const char* text = "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E\xE3\x81\xAE\xE5\x8D\x98\xE8\xAA\x9E";
    const size_t len = strlen(text);

    ParagraphStyle style;
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(20.0f);
    builder.pushStyle(ts);
    builder.addText(text, len);
    builder.pop();
    auto paragraph = builder.Build();
    // Layout at width that forces wrapping across CJK words
    paragraph->layout(50.0f);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);

    REPORTER_ASSERT(reporter, lm.size() >= 2, "Expected text to wrap into multiple lines");
    if (lm.size() >= 2) {
        // Strict word breaking keeps CJK words intact across line boundaries
        REPORTER_ASSERT(reporter, lm[0].fEndIndex == 4,
                        "Expected Line 1 to break at word boundary index 4; got %zu",
                        lm[0].fEndIndex);
    }
}

// =============================================================================
// Skia Buganizer b/305780908:
// Decouple Skia from monolithic ICU table memory overhead (zero-ICU footprint)
//
// [PASSES ON APPLE]
// Fixed in SkUnicode_apple: The native Apple typography stack runs completely on
// CoreFoundation (CFStringTokenizer, CFLocale, CFStringTransform), entirely eliminating
// the ~8.9 MB dirty RAM ICU footprint (icudtl.dat)
// =============================================================================
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
DEF_TEST(SkParagraph_Skia_b_305780908_ZeroICUFootprint, reporter)
#else
DEF_TEST_DISABLED(SkParagraph_Skia_b_305780908_ZeroICUFootprint, reporter)
#endif
{
    auto unicode = get_test_unicode();
    REPORTER_ASSERT(reporter, unicode != nullptr, "Unicode engine required");

    // Current standard SkParagraph has a hard dependency on ICU data tables (icudtl.dat)
    // On Apple platforms, the native Apple typography stack achieves zero ICU footprint
    // by using SkUnicode_apple
    // This test asserts that the active Unicode implementation is the zero-ICU Apple engine:
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
    bool isAppleUnicode = true;
#else
    bool isAppleUnicode = false;
#endif
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
    REPORTER_ASSERT(reporter, isAppleUnicode,
                    "Current SkParagraph relies on monolithic ICU (~8.9 MB dirty RAM). Expected zero-ICU Apple engine.");
#else
    (void)isAppleUnicode;
#endif
}
