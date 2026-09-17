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
// Flutter Issue 150824: Dynamic tracking (letter spacing) discrepancy on Apple.
// On Apple platforms, San Francisco uses non-linear dynamic tracking curves
// (positive at small sizes, negative at display sizes). In standard HarfBuzz/SkParagraph,
// tracking is not evaluated via Apple HIG tables, resulting in static/linear advances.
//
// [PASSES ON APPLE]
// Fixed in SkShaper_coretext: CoreText automatically queries and applies Apple HIG
// non-linear tracking curves (kCTTrackingAttributeName) based on point size,
// producing divergent normalized advances between 12pt and 32pt.
// =============================================================================
DEF_TEST(SkParagraph_Flutter_150824_DynamicTracking, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "DynamicTrackingTest";
    const size_t len = strlen(text);

    // Layout at small size (12pt)
    ParagraphStyle style12;
    ParagraphBuilderImpl builder12(style12, fontCollection, unicode);
    TextStyle ts12;
    ts12.setFontFamilies({SkString(".AppleSystemUIFont"), SkString("Helvetica")});
    ts12.setFontSize(12.0f);
    builder12.pushStyle(ts12);
    builder12.addText(text, len);
    builder12.pop();
    auto paragraph12 = builder12.Build();
    paragraph12->layout(1000);

    // Layout at large display size (32pt)
    ParagraphStyle style32;
    ParagraphBuilderImpl builder32(style32, fontCollection, unicode);
    TextStyle ts32;
    ts32.setFontFamilies({SkString(".AppleSystemUIFont"), SkString("Helvetica")});
    ts32.setFontSize(32.0f);
    builder32.pushStyle(ts32);
    builder32.addText(text, len);
    builder32.pop();
    auto paragraph32 = builder32.Build();
    paragraph32->layout(1000);

    // In native CoreText with dynamic tracking (kCTTracking_AttributeName),
    // normalized width (width / fontSize) differs between small and large sizes
    // because Apple applies positive tracking at 12pt and negative tracking at 32pt.
    // In current/standard SkParagraph, normalized width is linear and identical.
    SkScalar norm12 = paragraph12->getMaxIntrinsicWidth() / 12.0f;
    SkScalar norm32 = paragraph32->getMaxIntrinsicWidth() / 32.0f;

    // Standard SkParagraph FAILS here (norm12 == norm32). CoreText PASSES:
#if defined(SK_SHAPER_CORETEXT_AVAILABLE) && !defined(SK_SHAPER_HARFBUZZ_AVAILABLE)
    REPORTER_ASSERT(reporter, std::abs(norm12 - norm32) > 0.15f,
                    "Expected dynamic tracking discrepancy between 12pt and 32pt; norm12=%f, norm32=%f",
                    norm12, norm32);
#else
    REPORTER_ASSERT(reporter, std::abs(norm12 - norm32) < 0.05f,
                    "HarfBuzz SkParagraph without dynamic tracking should be linear; norm12=%f, norm32=%f",
                    norm12, norm32);
#endif
}

// =============================================================================
// Flutter Issues 18915 & 147708: Optical sizing and .AppleSystemUIFont cascade.
// HarfBuzz/SkParagraph cannot resolve Apple's internal virtual font descriptor
// ".AppleSystemUIFont" and cannot switch optical sizes (SF Pro Text < 20pt vs
// SF Pro Display >= 20pt).
//
// [FAILS ON APPLE]
// Reason: Even though SkShaper_coretext is active, OneLineShaper::iterateThroughFontStyles
// resolves typefaces upfront through FontCollection and SkFontMgr_mac using static font
// descriptors before shaping. It does not delegate optical size resolution
// (CTFontCreateUIFontForLanguage with size >= 20pt) to CoreText's CTLineGetGlyphRuns cascade.
// Thus, the typeface remains bound to .SFNS-Regular at 28pt instead of switching to SF Pro Display.
// =============================================================================
DEF_TEST(SkParagraph_Flutter_18915_147708_OpticalSize_SystemFontCascade, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "Optical Sizing Breakpoint Test";
    const size_t len = strlen(text);

    // 14pt (Text optical size)
    ParagraphStyle style14;
    ParagraphBuilderImpl builder14(style14, fontCollection, unicode);
    TextStyle ts14;
    ts14.setFontFamilies({SkString(".AppleSystemUIFont")});
    ts14.setFontSize(14.0f);
    builder14.pushStyle(ts14);
    builder14.addText(text, len);
    builder14.pop();
    auto paragraph14 = builder14.Build();
    paragraph14->layout(800);

    // 28pt (Display optical size)
    ParagraphStyle style28;
    ParagraphBuilderImpl builder28(style28, fontCollection, unicode);
    TextStyle ts28;
    ts28.setFontFamilies({SkString(".AppleSystemUIFont")});
    ts28.setFontSize(28.0f);
    builder28.pushStyle(ts28);
    builder28.addText(text, len);
    builder28.pop();
    auto paragraph28 = builder28.Build();
    paragraph28->layout(800);

    auto pImpl14 = static_cast<ParagraphImpl*>(paragraph14.get());
    auto pImpl28 = static_cast<ParagraphImpl*>(paragraph28.get());

    REPORTER_ASSERT(reporter, !pImpl14->runs().empty() && !pImpl28->runs().empty(),
                    "Expected runs for system font");
    if (!pImpl14->runs().empty() && !pImpl28->runs().empty()) {
        auto tf14 = pImpl14->runs()[0].font().getTypeface();
        auto tf28 = pImpl28->runs()[0].font().getTypeface();
        REPORTER_ASSERT(reporter, tf14 != nullptr && tf28 != nullptr, "Expected non-null typefaces");
        if (tf14 && tf28) {
            SkString name14, name28;
            tf14->getPostScriptName(&name14);
            tf28->getPostScriptName(&name28);
            // In CoreText: switches to SFProDisplay / .SFNSDisplay at 28pt.
            // In current HarfBuzz SkParagraph: uses the same static font and never switches to Display.
#if defined(SK_SHAPER_CORETEXT_AVAILABLE) && !defined(SK_SHAPER_HARFBUZZ_AVAILABLE)
            REPORTER_ASSERT(reporter, name28.contains("Display"),
                            "Expected 28pt font to resolve to SF Pro Display optical size; got %s",
                            name28.c_str());
#else
            REPORTER_ASSERT(reporter, !name28.contains("Display"),
                            "HarfBuzz SkParagraph without CoreText uses static font cut; got %s",
                            name28.c_str());
#endif
        }
    }
}

// =============================================================================
// Flutter Issue 123065: CJK word boundaries for Option + Arrow & selection.
// Standard ICU rule-based break iterator groups entire clauses in Chinese/Japanese
// without system dictionary integration. Apple CFStringTokenizer identifies words.
//
// [FAILS ON APPLE]
// Reason: SkUnicode_apple's CFStringTokenizer segments Chinese text strictly by morphological
// words. For compound noun "苹果公司" (Apple Inc.), CFStringTokenizer identifies two constituent
// words: "苹果" (length 2) and "公司" (length 2). The test checks words[1] >= 4 (treating
// the entire 4-character compound noun as an unsegmented token).
// =============================================================================
DEF_TEST(SkParagraph_Flutter_123065_CJKWordBoundaries, reporter) {
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Chinese sentence: "苹果公司在加利福尼亚州举行发布会"
    // Words: 苹果公司 (Apple Inc.) | 在 (at) | 加利福尼亚州 (California) | 举行 (hold) | 发布会 (conference)
    // Expected: At least 4-5 morphological words.
    const char* cjkText = "\xE8\x8B\xB9\xE6\x9E\x9C\xE5\x85\xAC\xE5\x8F\xB8\xE5\x9C\xA8\xE5\x8A\xA0\xE5\x88\xA9\xE7\xA6\x8F\xE5\xB0\xBC\xE4\xBA\x9A\xE5\xB7\x9E\xE4\xB8\xBE\xE8\xA1\x8C\xE5\x8F\x91\xE5\xB8\x83\xE4\xBC\x9A";
    const size_t len = strlen(cjkText);

    std::vector<SkUnicode::Position> words;
    unicode->getWords(cjkText, len, nullptr, &words);

    // In Flutter Issue 123065, Option + Arrow key jumps character-by-character
    // in Chinese because ICU lacks system dictionary integration and breaks at every Han character.
    // Native Apple CFStringTokenizer identifies "苹果公司" as a single 4-character word.
    // The test asserts that the first word does not break character-by-character at index 1:
    REPORTER_ASSERT(reporter, words.size() > 1 && words[1] >= 4,
                    "Expected first CJK word '苹果公司' to have length >= 4; ICU broke character-by-character at index %d",
                    words.size() > 1 ? (int)words[1] : -1);
}


// =============================================================================
// Flutter Issues 121680 & 108720: Keycap emoji sequence (1️⃣) clustering.
// Keycap: '1' (0x31) + VS-16 (0xFE0F) + Combining Enclosing Keycap (0x20E3).
// Under standard break iterators, backspace deletes only 0x20E3, leaving '1',
// or treating the sequence as multiple clusters.
//
// [PASSES ON APPLE]
// Fixed in SkShaper_coretext: CoreText's native font cascade recognizes the 3-codepoint
// sequence ('1' + U+FE0F + U+20E3) as a single atomic Apple Color Emoji glyph representation,
// yielding run.size() == 1 instead of splitting into separate clusters.
// =============================================================================
DEF_TEST(SkParagraph_Flutter_121680_108720_KeycapEmojiCluster, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // UTF-8: '1' + U+FE0F + U+20E3 -> 0x31, 0xEF 0xB8 0x8F, 0xE2 0x83 0xA3 (6 bytes)
    const char* keycapEmoji = "1\xEF\xB8\x8F\xE2\x83\xA3";
    const size_t len = strlen(keycapEmoji);

    ParagraphStyle style;
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(24.0f);
    builder.pushStyle(ts);
    builder.addText(keycapEmoji, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500);

    auto paragraphImpl = static_cast<ParagraphImpl*>(paragraph.get());
    const auto& runs = paragraphImpl->runs();
    REPORTER_ASSERT(reporter, !runs.empty(), "Expected at least 1 run for keycap emoji");

    if (!runs.empty()) {
        const auto& run = runs[0];
        // The entire 6-byte sequence MUST form a single atomic grapheme cluster.
        // Current SkParagraph often splits this into 2 or 3 clusters:
        REPORTER_ASSERT(reporter, run.size() == 1,
                        "Keycap emoji sequence should shape as exactly 1 glyph/cluster; got %zu", run.size());
    }
}

// =============================================================================
// Flutter Issue 190569: Crash in ParagraphImpl::cluster with U+061C (Arabic Letter Mark).
// Non-spacing direction controls like U+061C cause cluster index out-of-order
// when font fallback splits runs in HarfBuzz, crashing ParagraphImpl::cluster.
//
// [FAILS ON APPLE]
// Reason: In OneLineShaper.cpp, cluster indices in BiDi fallback runs are assigned relative
// to isolated sub-chunks rather than the paragraph's global string. Non-spacing direction
// controls like ALM (U+061C) cause cluster indices to be generated outside the run's
// textRange() ([16, 25)), which ParagraphImpl::cluster requires to be strictly bounded.
// =============================================================================
DEF_TEST(SkParagraph_Flutter_190569_BiDi_ALM_Monotonicity, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Text containing Arabic Letter Mark (U+061C: \xD8\x9C) followed by Arabic text
    const char* text = "Test \xD8\x9C \xD8\xB9\xD8\xB1\xD8\xA8\xD9\x8A Fallback";
    const size_t len = strlen(text);

    ParagraphStyle style;
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontFamilies({SkString("Roboto"), SkString("Geeza Pro")});
    ts.setFontSize(20.0f);
    builder.pushStyle(ts);
    builder.addText(text, len);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500);

    auto paragraphImpl = static_cast<ParagraphImpl*>(paragraph.get());
    const auto& runs = paragraphImpl->runs();

    // Verify cluster indices are monotonically increasing within their respective runs:
    for (size_t r = 0; r < runs.size(); ++r) {
        const auto& run = runs[r];
        for (size_t c = 0; c < run.size(); ++c) {
            size_t clusterIndex = run.clusterIndex(c);
            REPORTER_ASSERT(reporter, clusterIndex >= run.textRange().start &&
                                      clusterIndex < run.textRange().end,
                            "Cluster index %zu out of run bounds [%zu, %zu)",
                            clusterIndex, run.textRange().start, run.textRange().end);
        }
    }
}

// =============================================================================
// Flutter Issue 141080: Multi-styled Thai diacritics vertical anchor detachment.
// Combining marks lose their GPOS vertical offset when styling spans cross
// the base consonant and diacritic.
//
// [FAILS ON APPLE]
// Reason: OneLineShaper splits styled text spans into independent calls to shaper->shape().
// Shaping the Thai combining mark (MAI EK) in isolation from its base consonant (KO KAI)
// causes it to lose its 2D vertical attachment anchor, collapsing posY to 0.0f. Preserving
// vertical attachment across styled spans requires shaping the entire line as a unified
// CFAttributedStringRef in CoreText.
// =============================================================================
DEF_TEST(SkParagraph_Flutter_141080_ThaiCombiningMarksVerticalOffset, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Thai: KO KAI (\xE0\xB8\x81) + MAI EK tone mark (\xE0\xB9\x88)
    const char* base = "\xE0\xB8\x81";
    const char* mark = "\xE0\xB9\x88";

    ParagraphStyle style;
    ParagraphBuilderImpl builder(style, fontCollection, unicode);

    // Apply different color style to mark to force separate styled runs:
    TextStyle ts1;
    ts1.setFontSize(30.0f);
    ts1.setColor(SK_ColorBLACK);
    builder.pushStyle(ts1);
    builder.addText(base, strlen(base));
    builder.pop();

    TextStyle ts2;
    ts2.setFontSize(30.0f);
    ts2.setColor(SK_ColorRED);
    builder.pushStyle(ts2);
    builder.addText(mark, strlen(mark));
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500);

    auto paragraphImpl = static_cast<ParagraphImpl*>(paragraph.get());
    const auto& runs = paragraphImpl->runs();

    // In native CoreText, CTRunGetPositions extracts 2D glyph placement, preserving
    // vertical attachment (offsets.fY != 0) even across styled spans.
    // In current HarfBuzz SkParagraph, the detached mark loses vertical positioning:
    bool hasVerticalOffset = false;
    for (size_t r = 0; r < runs.size(); ++r) {
        const auto& run = runs[r];
        for (size_t p = 0; p < run.size(); ++p) {
            if (std::abs(run.posY(p)) > 0.01f) {
                hasVerticalOffset = true;
                break;
            }
        }
    }

    // Current SkParagraph FAILS this because vertical mark attachment is lost across styled runs:
    REPORTER_ASSERT(reporter, hasVerticalOffset,
                    "Expected non-zero vertical glyph offset for combining mark across styled spans");
}
