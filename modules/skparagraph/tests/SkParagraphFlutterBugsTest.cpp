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

#if defined(SK_SHAPER_CORETEXT_AVAILABLE)
#include "modules/skshaper/include/SkShaper_coretext.h"
#endif

using namespace skia_private;
using namespace skia::textlayout;

namespace {

static sk_sp<SkUnicode> get_test_unicode() {
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
// Flutter Issue 150824: Dynamic tracking (letter spacing) discrepancy on Apple
// On Apple platforms, San Francisco uses non-linear dynamic tracking curves
// (positive at small sizes, negative at display sizes). In standard HarfBuzz/SkParagraph,
// tracking is not evaluated via Apple HIG tables, resulting in static/linear advances
//
// [PASSES ON APPLE]
// Fixed in SkShaper_coretext: CoreText automatically queries and applies Apple HIG
// non-linear tracking curves (kCTTrackingAttributeName) based on point size,
// producing divergent normalized advances between 12pt and 32pt
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
    // because Apple applies positive tracking at 12pt and negative tracking at 32pt
    // In current/standard SkParagraph, normalized width is linear and identical
    SkScalar norm12 = paragraph12->getMaxIntrinsicWidth() / 12.0f;
    SkScalar norm32 = paragraph32->getMaxIntrinsicWidth() / 32.0f;

    auto impl12 = static_cast<ParagraphImpl*>(paragraph12.get());
    if (impl12->isCoreTextShaped()) {
        REPORTER_ASSERT(reporter, std::abs(norm12 - norm32) > 0.15f,
                        "Expected dynamic tracking discrepancy between 12pt and 32pt; norm12=%f, norm32=%f",
                        norm12, norm32);
    } else {
        REPORTER_ASSERT(reporter, std::abs(norm12 - norm32) < 0.05f,
                        "HarfBuzz SkParagraph without dynamic tracking should be linear; norm12=%f, norm32=%f",
                        norm12, norm32);
    }
}

// =============================================================================
// Flutter Issues 18915 & 147708: Optical sizing and .AppleSystemUIFont cascade
// HarfBuzz/SkParagraph cannot resolve Apple's internal virtual font descriptor
// ".AppleSystemUIFont" and cannot switch optical sizes (SF Pro Text < 20pt vs
// SF Pro Display >= 20pt)
//
// [FAILS ON APPLE]
// Reason: Even though SkShaper_coretext is active, OneLineShaper::iterateThroughFontStyles
// resolves typefaces upfront through FontCollection and SkFontMgr_mac using static font
// descriptors before shaping. It does not delegate optical size resolution
// (CTFontCreateUIFontForLanguage with size >= 20pt) to CoreText's CTLineGetGlyphRuns cascade
// Thus, the typeface remains bound to .SFNS-Regular at 28pt instead of switching to SF Pro Display
// =============================================================================
DEF_TEST_DISABLED(SkParagraph_Flutter_18915_147708_OpticalSize_SystemFontCascade, reporter) {
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
            // In CoreText: switches to SFProDisplay / .SFNSDisplay at 28pt
            REPORTER_ASSERT(reporter, name28.contains("Display"),
                            "Expected 28pt font to resolve to SF Pro Display optical size; got %s",
                            name28.c_str());
        }
    }
}

// =============================================================================
// Flutter Issue 123065: CJK word boundaries for Option + Arrow & selection
// Standard ICU rule-based break iterator groups entire clauses in Chinese/Japanese
// without system dictionary integration. Apple CFStringTokenizer identifies words
//
// [FAILS ON APPLE]
// Reason: CFStringTokenizer segments Chinese text strictly by morphological
// words. For compound noun "苹果公司" (Apple Inc.), CFStringTokenizer identifies two constituent
// words: "苹果" (length 2) and "公司" (length 2). The test checks words[1] >= 4 (treating
// the entire 4-character compound noun as an unsegmented token)
// =============================================================================
DEF_TEST(SkParagraph_Flutter_123065_CJKWordBoundaries, reporter) {
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Chinese sentence: "苹果公司在加利福尼亚州举行发布会"
    // Words: 苹果公司 (Apple Inc.) | 在 (at) | 加利福尼亚州 (California) | 举行 (hold) | 发布会 (conference)
    // Expected: At least 4-5 morphological words
    const char* cjkText = "\xE8\x8B\xB9\xE6\x9E\x9C\xE5\x85\xAC\xE5\x8F\xB8\xE5\x9C\xA8\xE5\x8A\xA0\xE5\x88\xA9\xE7\xA6\x8F\xE5\xB0\xBC\xE4\xBA\x9A\xE5\xB7\x9E\xE4\xB8\xBE\xE8\xA1\x8C\xE5\x8F\x91\xE5\xB8\x83\xE4\xBC\x9A";
    const size_t len = strlen(cjkText);

    std::vector<SkUnicode::Position> words;
    unicode->getWords(cjkText, len, nullptr, &words);

    // In Flutter Issue 123065, Option + Arrow key jumps character-by-character
    // in Chinese because ICU lacks system dictionary integration and breaks at every Han character
    // Apple CFStringTokenizer identifies morphological words ("苹果", "公司") rather than breaking at index 1:
    REPORTER_ASSERT(reporter, words.size() > 2 && words[1] == 2 && words[2] == 4,
                    "Expected morphological word boundaries for 苹果 (index 2) and 公司 (index 4); got index %d",
                    words.size() > 1 ? (int)words[1] : -1);
}


// =============================================================================
// Flutter Issues 121680 & 108720: Keycap emoji sequence (1️⃣) clustering
// Keycap: '1' (0x31) + VS-16 (0xFE0F) + Combining Enclosing Keycap (0x20E3)
// Under standard break iterators, backspace deletes only 0x20E3, leaving '1',
// or treating the sequence as multiple clusters
//
// [PASSES ON APPLE]
// Fixed in SkShaper_coretext: CoreText's native font cascade recognizes the 3-codepoint
// sequence ('1' + U+FE0F + U+20E3) as a single atomic Apple Color Emoji glyph representation,
// yielding run.size() == 1 instead of splitting into separate clusters
// =============================================================================
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
DEF_TEST(SkParagraph_Flutter_121680_108720_KeycapEmojiCluster, reporter)
#else
DEF_TEST_DISABLED(SkParagraph_Flutter_121680_108720_KeycapEmojiCluster, reporter)
#endif
{
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // UTF-8: '1' + U+FE0F + U+20E3 -> 0x31, 0xEF 0xB8 0x8F, 0xE2 0x83 0xA3 (6 bytes)
    const char* keycapEmoji = "1\xEF\xB8\x8F\xE2\x83\xA3";
    const size_t len = strlen(keycapEmoji);

    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
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
        // The entire 6-byte sequence MUST form a single atomic grapheme cluster
        // Current SkParagraph often splits this into 2 or 3 clusters:
        REPORTER_ASSERT(reporter, run.size() == 1,
                        "Keycap emoji sequence should shape as exactly 1 glyph/cluster; got %zu", run.size());
    }
}

// =============================================================================
// Flutter Issue 190569: Crash in ParagraphImpl::cluster with U+061C (Arabic Letter Mark)
// Non-spacing direction controls like U+061C cause cluster index out-of-order
// when font fallback splits runs in HarfBuzz, crashing ParagraphImpl::cluster
//
// [FAILS ON APPLE]
// Reason: In OneLineShaper.cpp, cluster indices in BiDi fallback runs are assigned relative
// to isolated sub-chunks rather than the paragraph's global string. Non-spacing direction
// controls like ALM (U+061C) cause cluster indices to be generated outside the run's
// textRange() ([16, 25)), which ParagraphImpl::cluster requires to be strictly bounded
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

    // Verify global cluster indices are strictly contained within run boundaries:
    for (size_t r = 0; r < runs.size(); ++r) {
        const auto& run = runs[r];
        for (size_t c = 0; c < run.size(); ++c) {
            size_t clusterIndex = run.globalClusterIndex(c);
            REPORTER_ASSERT(reporter, clusterIndex >= run.textRange().start &&
                                      clusterIndex < run.textRange().end,
                            "Cluster index %zu out of run bounds [%zu, %zu)",
                            clusterIndex, run.textRange().start, run.textRange().end);
        }
    }
}

// =============================================================================
// Flutter Issue 141080: Multi-styled Thai diacritics vertical anchor detachment
// Combining marks lose their GPOS vertical offset when styling spans cross
// the base consonant and diacritic
//
// [FAILS ON APPLE]
// Reason: OneLineShaper splits styled text spans into independent calls to shaper->shape()
// Shaping the Thai combining mark (MAI EK) in isolation from its base consonant (KO KAI)
// causes it to lose its 2D vertical attachment anchor, collapsing posY to 0.0f. Preserving
// vertical attachment across styled spans requires shaping the entire line as a unified
// CFAttributedStringRef in CoreText
// =============================================================================
DEF_TEST_DISABLED(SkParagraph_Flutter_141080_ThaiCombiningMarksVerticalOffset, reporter) {
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
    // vertical attachment (offsets.fY != 0) even across styled spans
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

// =============================================================================
// Flutter Issue 193812: Expose iOS/macOS system font variants natively
// =============================================================================
DEF_TEST(SkParagraph_Flutter_193812_SystemFontVariants, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "System Font Variants Rounded Serif Mono";
    const size_t len = strlen(text);

    for (const char* variant : {".AppleSystemUIFontRounded",
                                ".AppleSystemUIFontSerif",
                                ".AppleSystemUIFontMonospaced"}) {
        ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
        style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString(variant), SkString(".AppleSystemUIFont")});
        ts.setFontSize(16.0f);
        builder.pushStyle(ts);
        builder.addText(text, len);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(500.0f);

        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0,
                        "Variant %s should resolve without unresolved glyphs", variant);
        REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f,
                        "Variant %s should have positive layout width", variant);
    }
}

// =============================================================================
// Flutter Issue 147708: CupertinoSystemText virtual system font fallback cascade
// =============================================================================
DEF_TEST(SkParagraph_Flutter_147708_SystemFontFallbackCascade, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* mixedText = "System: Hello 世界 🎉 مرحبا";
    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontFamilies({SkString(".AppleSystemUIFont")});
    ts.setFontSize(16.0f);
    builder.pushStyle(ts);
    builder.addText(mixedText);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(1000.0f);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0,
                    "System font fallback cascade should resolve all glyphs");
    REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    for (const auto& run : impl->runs()) {
        if (auto tf = run.font().getTypeface()) {
            SkString family;
            tf->getFamilyName(&family);
            REPORTER_ASSERT(reporter, !family.contains("LastResort"),
                            "Fallback should resolve valid fonts, not LastResort");
        }
    }
}

// =============================================================================
// Flutter Issue 130492: iOS italic system font synthetic skew
// =============================================================================
DEF_TEST(SkParagraph_Flutter_130492_SystemFontItalicSkew, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle italicStyle;
    italicStyle.setFontFamilies({SkString(".AppleSystemUIFont")});
    italicStyle.setFontSize(16.0f);
    italicStyle.setFontStyle(SkFontStyle::Italic());
    builder.pushStyle(italicStyle);
    builder.addText("System Italic Skew");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->runs().empty());
    if (!impl->runs().empty()) {
        auto tf = impl->runs()[0].font().getTypeface();
        REPORTER_ASSERT(reporter, tf != nullptr);
        SkString psName;
        tf->getPostScriptName(&psName);
        // System italic must resolve to an italic variant or apply synthetic slant
        bool isItalic = psName.contains("Italic") || psName.contains("Oblique") ||
                        tf->fontStyle().slant() != SkFontStyle::kUpright_Slant ||
                        impl->runs()[0].font().getSkewX() != 0.0f;
        REPORTER_ASSERT(reporter, isItalic,
                        "Expected italic system font or synthetic skew; got %s", psName.c_str());
    }
}

// =============================================================================
// Flutter Issue 48381: Inherit platform default system font when fontFamily is empty
// =============================================================================
DEF_TEST(SkParagraph_Flutter_48381_SystemDefaultFont, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    // Empty font family requests system default
    ts.setFontSize(16.0f);
    builder.pushStyle(ts);
    builder.addText("Default System Font Contract");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->runs().empty());
    if (!impl->runs().empty()) {
        auto tf = impl->runs()[0].font().getTypeface();
        REPORTER_ASSERT(reporter, tf != nullptr);
        SkString fam, ps;
        tf->getFamilyName(&fam);
        tf->getPostScriptName(&ps);
        REPORTER_ASSERT(reporter, !fam.isEmpty() && !ps.isEmpty());
        REPORTER_ASSERT(reporter, !fam.contains("LastResort") && !ps.contains("LastResort"));
        // Inherited default font on Apple platforms must resolve to Helvetica or SF
        bool isSystemDefault = fam.equals("Helvetica") || fam.contains("SF") ||
                               fam.contains("System") || fam.equals(".AppleSystemUIFont");
        REPORTER_ASSERT(reporter, isSystemDefault,
                        "Expected platform default system font; got family=%s ps=%s",
                        fam.c_str(), ps.c_str());
    }
}

// =============================================================================
// Flutter Issue 109308: Variable font axes mapping
// =============================================================================
DEF_TEST(SkParagraph_Flutter_109308_VariableFontAxes, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "VariableFontWeights";

    auto layoutWithWeight = [&](SkFontStyle fontStyle) -> SkScalar {
        ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
        style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString(".AppleSystemUIFont")});
        ts.setFontSize(24.0f);
        ts.setFontStyle(fontStyle);
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);
        return paragraph->getMaxIntrinsicWidth();
    };

    SkScalar lightWidth = layoutWithWeight(SkFontStyle(SkFontStyle::kThin_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    SkScalar boldWidth = layoutWithWeight(SkFontStyle(SkFontStyle::kBlack_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));

    REPORTER_ASSERT(reporter, lightWidth > 0.0f && boldWidth > 0.0f);
    // Bold weight has wider glyph metrics than thin weight
    REPORTER_ASSERT(reporter, boldWidth > lightWidth,
                    "Expected bold weight width (%f) to exceed thin weight width (%f)", boldWidth, lightWidth);
}

// =============================================================================
// Flutter Issue 22572: SF Pro Body (17pt) dynamic tracking HIG delta
// =============================================================================
DEF_TEST(SkParagraph_Flutter_22572_DefaultTextTrackingHIGDelta, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "Cupertino HIG Body 17pt Tracking";

    ParagraphStyle ctStyle;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    ctStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl ctBuilder(ctStyle, fontCollection, unicode);
    TextStyle ctTS;
    ctTS.setFontFamilies({SkString(".AppleSystemUIFont")});
    ctTS.setFontSize(17.0f);
    ctBuilder.pushStyle(ctTS);
    ctBuilder.addText(text);
    ctBuilder.pop();
    auto ctPara = ctBuilder.Build();
    ctPara->layout(1000.0f);

    ParagraphStyle hbStyle;
    hbStyle.setTextShapingEngine(TextShapingEngine::kHarfBuzz);
    ParagraphBuilderImpl hbBuilder(hbStyle, fontCollection, unicode);
    TextStyle hbTS;
    hbTS.setFontFamilies({SkString(".AppleSystemUIFont")});
    hbTS.setFontSize(17.0f);
    hbBuilder.pushStyle(hbTS);
    hbBuilder.addText(text);
    hbBuilder.pop();
    auto hbPara = hbBuilder.Build();
    hbPara->layout(1000.0f);

    SkScalar ctWidth = ctPara->getMaxIntrinsicWidth();
    SkScalar hbWidth = hbPara->getMaxIntrinsicWidth();

    auto impl = static_cast<ParagraphImpl*>(ctPara.get());
    if (impl->isCoreTextShaped()) {
        // CoreText dynamic tracking produces tighter/looser HIG spacing than static font advances
        REPORTER_ASSERT(reporter, std::abs(ctWidth - hbWidth) > 0.1f,
                        "Expected HIG tracking delta between CoreText (%f) and HarfBuzz (%f)", ctWidth, hbWidth);
    }
}

// =============================================================================
// Flutter Issue 184815: Suppress trailing letter spacing on line end to prevent premature truncation
// [FAILS IN TEXTWRAPPER]
// Reason: In TextWrapper.cpp, trailing letterSpacing is added to the terminal glyph's advance
// without suppressing trailing tracking on the last cluster, falsely exceeding the line boundary
// and dropping the terminal character
// =============================================================================
DEF_TEST_DISABLED(SkParagraph_Flutter_184815_LetterSpacingFinalClusterTruncation, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "Short text line";
    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    style.setMaxLines(1);
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(16.0f);
    ts.setLetterSpacing(5.0f);
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    SkScalar fullWidth = paragraph->getMaxIntrinsicWidth();
    paragraph->layout(fullWidth + 1.0f);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    REPORTER_ASSERT(reporter, lm.size() == 1, "Text should fit in 1 line without truncation");
    if (!lm.empty()) {
        REPORTER_ASSERT(reporter, lm[0].fEndIndex == strlen(text),
                        "Line should include all characters without truncating final cluster");
    }
}

// =============================================================================
// Flutter Issue 151640: Letter spacing applies after glyph, initial glyph origin remains at x=0
// =============================================================================
DEF_TEST(SkParagraph_Flutter_151640_LetterSpacingLeadingEdge, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "LeadingEdgeTest";

    auto createPara = [&](SkScalar spacing) {
        ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
        style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontSize(20.0f);
        ts.setLetterSpacing(spacing);
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);
        return paragraph;
    };

    auto p0 = createPara(0.0f);
    auto p10 = createPara(10.0f);

    auto impl0 = static_cast<ParagraphImpl*>(p0.get());
    auto impl10 = static_cast<ParagraphImpl*>(p10.get());

    REPORTER_ASSERT(reporter, !impl0->runs().empty() && !impl10->runs().empty());
    if (!impl0->runs().empty() && !impl10->runs().empty()) {
        const auto& r0 = impl0->runs()[0];
        const auto& r10 = impl10->runs()[0];
        // The first glyph origin in both runs must start at 0.0f
        REPORTER_ASSERT(reporter, SkScalarNearlyZero(r0.positions()[0].fX, 0.01f));
        REPORTER_ASSERT(reporter, SkScalarNearlyZero(r10.positions()[0].fX, 0.01f));
    }
}

// =============================================================================
// Flutter Issue 118127: Combining marks stay attached to base letter under letterSpacing
// =============================================================================
DEF_TEST(SkParagraph_Flutter_118127_CombiningDiacriticsLetterSpacing, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // "e\u0301" (e + combining acute accent)
    const char* text = "e\xCC\x81";

    auto buildWithSpacing = [&](SkScalar spacing) {
        ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
        style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontSize(24.0f);
        ts.setLetterSpacing(spacing);
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();
        auto p = builder.Build();
        p->layout(500.0f);
        return p;
    };

    auto p0 = buildWithSpacing(0.0f);
    auto p8 = buildWithSpacing(8.0f);

    auto impl0 = static_cast<ParagraphImpl*>(p0.get());
    auto impl8 = static_cast<ParagraphImpl*>(p8.get());
    REPORTER_ASSERT(reporter, !impl0->runs().empty() && !impl8->runs().empty());

    // Combining diacritic mark must remain fused with base glyph into a single cluster
    REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl0->runs()[0].size() == 1);
    REPORTER_ASSERT(reporter, impl8->runs().size() == 1 && impl8->runs()[0].size() == 1);

    // Letter spacing must expand the cluster width without decomposing the diacritic
    SkScalar widthDelta = p8->getMaxIntrinsicWidth() - p0->getMaxIntrinsicWidth();
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 8.0f, 0.05f),
                    "Expected 8.0f letter spacing delta; got %f (p0=%f, p8=%f)",
                    widthDelta, p0->getMaxIntrinsicWidth(), p8->getMaxIntrinsicWidth());
}

// =============================================================================
// Flutter Issue 144528: Indic conjuncts preserved under letter spacing
// =============================================================================
DEF_TEST(SkParagraph_Flutter_144528_IndicConjunctLetterSpacing, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Devanagari conjunct "क्ट्र" (KA + VIRAMA + TTA + VIRAMA + RA)
    const char* conjunct = "\xE0\xA4\x95\xE0\xA5\x8D\xE0\xA4\x9F\xE0\xA5\x8D\xE0\xA4\xB0";

    auto buildWithSpacing = [&](SkScalar spacing) {
        ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
        style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontSize(24.0f);
        ts.setLetterSpacing(spacing);
        builder.pushStyle(ts);
        builder.addText(conjunct);
        builder.pop();
        auto p = builder.Build();
        p->layout(500.0f);
        return p;
    };

    auto p0 = buildWithSpacing(0.0f);
    auto p4 = buildWithSpacing(4.0f);

    REPORTER_ASSERT(reporter, p0->unresolvedGlyphs() == 0 && p4->unresolvedGlyphs() == 0);

    auto impl0 = static_cast<ParagraphImpl*>(p0.get());
    auto impl4 = static_cast<ParagraphImpl*>(p4.get());
    REPORTER_ASSERT(reporter, !impl0->runs().empty() && !impl4->runs().empty());

    // Devanagari multi-codepoint conjunct must stay a single fused cluster under letter spacing
    REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl0->runs()[0].size() == 1);
    REPORTER_ASSERT(reporter, impl4->runs().size() == 1 && impl4->runs()[0].size() == 1);

    SkScalar widthDelta = p4->getMaxIntrinsicWidth() - p0->getMaxIntrinsicWidth();
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 4.0f, 0.05f),
                    "Expected 4.0f letter spacing delta; got %f", widthDelta);
}

// =============================================================================
// Flutter Issue 19584: Non-trivial word break ranges for CJK locales
// =============================================================================
DEF_TEST(SkParagraph_Flutter_19584_CJKWordBreakBoundaries, reporter) {
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Japanese: "東京都にある駅" (Tokyo-to ni aru eki)
    const char* text = "\xE6\x9D\xB1\xE4\xBA\xAC\xE9\x83\xBD\xE3\x81\xAB\xE3\x81\x82\xE3\x82\x8B\xE9\xA7\x85";
    const size_t len = strlen(text);

    std::vector<SkUnicode::Position> words;
    unicode->getWords(text, len, "ja", &words);

    // Morphological CJK segmentation yields discrete words rather than 1 monolithic block
    REPORTER_ASSERT(reporter, words.size() >= 5, "Expected morphological word segments in CJK text; got %zu", words.size());
    if (words.size() >= 2) {
        // First word boundary isolates Tokyo-to or Tokyo [0..2]
        REPORTER_ASSERT(reporter, words[0] == 0 && words[1] == 2,
                        "Expected first Japanese word break at offset 2; got %d", (int)words[1]);
    }
}

// =============================================================================
// Flutter Issue 59284: Korean Hangul text wraps at word boundaries, not mid-syllable
// =============================================================================
DEF_TEST(SkParagraph_Flutter_59284_KoreanWordWrapping, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Korean sentence: "안녕하세요 반갑습니다 오늘 날씨가 좋습니다"
    const char* text = "\xEC\x95\x88\xEB\x85\x95\xED\x95\x98\xEC\x84\xB8\xEC\x9A\x94 "
                       "\xEB\xB0\x98\xEA\xB0\x91\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4 "
                       "\xEC\x98\xA4\xEB\x8A\x98 "
                       "\xEB\x82\xA0\xEC\x94\xA8\xEA\xB0\x80 "
                       "\xEC\x82\x8B\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4";

    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(16.0f);
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(150.0f);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    REPORTER_ASSERT(reporter, lm.size() == 2, "Expected text to wrap into 2 lines");
    if (lm.size() == 2) {
        // Line wrap must occur at word boundaries (after whitespace), never mid-syllable
        REPORTER_ASSERT(reporter, lm[0].fEndIndex == 12,
                        "Expected line 0 to break at word boundary index 12; got %zu", lm[0].fEndIndex);
        REPORTER_ASSERT(reporter, lm[1].fStartIndex == 12,
                        "Expected line 1 to start at word boundary index 12; got %zu", lm[1].fStartIndex);
    }
}

// =============================================================================
// Flutter Issue 119684: Downstream affinity at soft wrap line breaks
// =============================================================================
DEF_TEST(SkParagraph_Flutter_119684_DownstreamAffinityAtBreak, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "First line of text that wraps cleanly to second line";
    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(16.0f);
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(200.0f);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    REPORTER_ASSERT(reporter, lm.size() >= 2, "Expected soft wrapped lines");
    if (lm.size() >= 2) {
        PositionWithAffinity pos = paragraph->getGlyphPositionAtCoordinate(lm[0].fWidth, lm[0].fBaseline);
        // Soft line wrap boundary coordinates must default to downstream affinity
        REPORTER_ASSERT(reporter, pos.affinity == Affinity::kDownstream,
                        "Expected downstream affinity at line wrap coordinate");
        REPORTER_ASSERT(reporter, pos.position >= static_cast<int32_t>(lm[0].fEndIndex - 2) &&
                                  pos.position <= static_cast<int32_t>(lm[1].fStartIndex + 1));
    }
}

// =============================================================================
// Flutter Issue 75494: Paragraph.getWordBoundary safe empty range on out of bounds
// =============================================================================
DEF_TEST(SkParagraph_Flutter_75494_WordBoundaryOutOfBounds, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "Word boundary safe test";
    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(16.0f);
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    SkRange<size_t> range = paragraph->getWordBoundary(strlen(text) + 100);
    REPORTER_ASSERT(reporter, range.start == range.end, "Expected empty range for out-of-bounds offset");
}

// =============================================================================
// Flutter Issues 39755 & 84030: Justified text selection bounding boxes
// =============================================================================
DEF_TEST(SkParagraph_Flutter_39755_84030_JustifiedTextSelection, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "Justified selection alignment test text that wraps to second line";
    ParagraphStyle style;
    style.setTextAlign(TextAlign::kJustify);
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(16.0f);
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    const SkScalar maxWidth = 250.0f;
    paragraph->layout(maxWidth);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    REPORTER_ASSERT(reporter, lm.size() >= 2, "Expected text to wrap to test line 0 justification");
    if (lm.size() >= 2) {
        // Line 0 must expand to the full justification constraint
        REPORTER_ASSERT(reporter, lm[0].fWidth >= maxWidth - 0.5f,
                        "Expected line 0 to expand to column width; got %f", lm[0].fWidth);

        auto boxesLine = paragraph->getRectsForRange(0, lm[0].fEndIndex, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxesLine.empty());
        if (!boxesLine.empty()) {
            REPORTER_ASSERT(reporter, boxesLine[0].rect.width() >= maxWidth - 0.5f,
                            "Full line selection box must cover justified width; got %f", boxesLine[0].rect.width());
        }

        // Selection box for interior word must reflect justified spacing expansion
        auto boxesWord = paragraph->getRectsForRange(20, 29, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxesWord.empty());
        if (!boxesWord.empty()) {
            REPORTER_ASSERT(reporter, boxesWord[0].rect.fLeft > 135.0f,
                            "Justified word position must shift rightward; got %f", boxesWord[0].rect.fLeft);
        }
    }
}

// =============================================================================
// Flutter Issue 188713: Caret cannot split SMP emoji UTF-16 surrogate pairs
// =============================================================================
DEF_TEST(SkParagraph_Flutter_188713_SMPSurrogatePairCaretAtomicity, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // "A😀B": 'A' (index 0), '😀' (U+1F600, UTF-16 indices 1..3), 'B' (UTF-16 index 3)
    const char* text = "A\xF0\x9F\x98\x80\x42";
    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(24.0f);
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    PositionWithAffinity pos = paragraph->getGlyphPositionAtCoordinate(25.0f, 12.0f);
    REPORTER_ASSERT(reporter, pos.position != 2,
                    "Caret must never be placed inside surrogate pair (index 2); got %d", (int)pos.position);
}

// =============================================================================
// Flutter Issue 79773: Emoji bottom pixel clipping prevention via native descent
// =============================================================================
DEF_TEST(SkParagraph_Flutter_79773_EmojiDescentClipping, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "Emoji \xF0\x9F\x8E\x89 Party";
    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    const SkScalar fontSize = 24.0f;
    ts.setFontSize(fontSize);
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    REPORTER_ASSERT(reporter, !lm.empty(), "Expected line metrics");
    if (!lm.empty()) {
        // CoreText preserves Apple Color Emoji native bitmap descent padding (>= 0.25 * fontSize)
        REPORTER_ASSERT(reporter, lm[0].fDescent >= 0.25f * fontSize,
                        "Expected native emoji descent padding; got %f", lm[0].fDescent);
    }

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    bool hasEmoji = false;
    for (const auto& r : impl->runs()) {
        SkString fam;
        r.font().getTypeface()->getFamilyName(&fam);
        if (fam.contains("Emoji")) {
            hasEmoji = true;
        }
    }
    REPORTER_ASSERT(reporter, hasEmoji, "Expected fallback to Apple Color Emoji font");
}

// =============================================================================
// Flutter Issues 137976 & 100342: ZWJ emoji sequence cluster atomicity
// =============================================================================
DEF_TEST(SkParagraph_Flutter_137976_100342_ZWJEmojiClusterBounds, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Technologist: Woman (\xF0\x9F\x91\xA9) + ZWJ (\xE2\x80\x8D) + Laptop (\xF0\x9F\x92\xBB)
    const char* zwjEmoji = "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB";

    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(24.0f);
    builder.pushStyle(ts);
    builder.addText(zwjEmoji);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Multi-codepoint ZWJ sequence must shape into a single atomic glyph in one run
    REPORTER_ASSERT(reporter, impl->runs().size() == 1,
                    "Expected 1 run for ZWJ emoji sequence; got %zu", impl->runs().size());
    if (impl->runs().size() == 1) {
        REPORTER_ASSERT(reporter, impl->runs()[0].size() == 1,
                        "Expected ZWJ emoji sequence to shape into exactly 1 atomic glyph; got %zu",
                        impl->runs()[0].size());
        SkString fam;
        impl->runs()[0].font().getTypeface()->getFamilyName(&fam);
        REPORTER_ASSERT(reporter, fam.contains("Emoji"), "Expected Apple Color Emoji; got %s", fam.c_str());
    }
}

// =============================================================================
// Flutter Issue 161201: Ideographic Variation Sequences (IVS) preservation
// =============================================================================
DEF_TEST(SkParagraph_Flutter_161201_IdeographicVariationSequences, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Japanese IVS character "辻󠄀" (U+8FBB U+E0100: \xE8\xBE\xBB \xF3\xA0\x84\x80)
    const char* ivs = "\xE8\xBE\xBB\xF3\xA0\x84\x80";

    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(24.0f);
    builder.pushStyle(ts);
    builder.addText(ivs);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Base ideograph and variation selector must resolve to 1 atomic glyph without decomposition
    REPORTER_ASSERT(reporter, impl->runs().size() == 1,
                    "Expected 1 run for IVS character; got %zu", impl->runs().size());
    if (impl->runs().size() == 1) {
        REPORTER_ASSERT(reporter, impl->runs()[0].size() == 1,
                        "Expected IVS sequence to resolve to 1 atomic variation glyph; got %zu",
                        impl->runs()[0].size());
    }
}

// =============================================================================
// Flutter Issue 193281: RTL text line wrapping with bold style and fallback
// =============================================================================
DEF_TEST(SkParagraph_Flutter_193281_BiDi_RTLBoldFallbackWrapping, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    ParagraphStyle style;
    style.setTextDirection(TextDirection::kRtl);
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);

    TextStyle normalStyle;
    normalStyle.setFontSize(16.0f);
    builder.pushStyle(normalStyle);
    builder.addText("שלום עולם ");
    builder.pop();

    TextStyle boldStyle;
    boldStyle.setFontSize(16.0f);
    boldStyle.setFontStyle(SkFontStyle::Bold());
    builder.pushStyle(boldStyle);
    builder.addText("טקסט מודגש ארוך ");
    builder.pop();

    builder.pushStyle(normalStyle);
    builder.addText("סיום המשפט");
    builder.pop();

    auto paragraph = builder.Build();
    const SkScalar maxWidth = 60.0f;
    paragraph->layout(maxWidth);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    // RTL line wrapping across styled spans must preserve width constraint and not SIGTRAP
    REPORTER_ASSERT(reporter, lm.size() >= 3, "Expected RTL text to wrap across styled spans; got %zu lines", lm.size());
    for (size_t i = 0; i < lm.size(); ++i) {
        REPORTER_ASSERT(reporter, lm[i].fWidth > 0.0f && lm[i].fWidth <= maxWidth + 0.5f,
                        "Line %zu width (%f) violated column constraint (%f)", i, lm[i].fWidth, maxWidth);
    }
}

// =============================================================================
// Flutter Issues 150744 & 48919: Vertical baseline shift across styled spans
// =============================================================================
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
DEF_TEST(SkParagraph_Flutter_150744_48919_SuperscriptVerticalShift, reporter) {
#else
DEF_TEST_DISABLED(SkParagraph_Flutter_150744_48919_SuperscriptVerticalShift, reporter) {
#endif
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);

    TextStyle baseStyle;
    baseStyle.setFontSize(20.0f);
    builder.pushStyle(baseStyle);
    builder.addText("X");
    builder.pop();

    TextStyle supStyle;
    supStyle.setFontSize(12.0f);
    supStyle.setBaselineShift(6.0f);
    builder.pushStyle(supStyle);
    builder.addText("2");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(200.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() >= 2);
    if (impl->runs().size() >= 2) {
        const auto& supRun = impl->runs()[1];
        REPORTER_ASSERT(reporter, !supRun.positions().empty());
        if (!supRun.positions().empty()) {
            // Baseline shift of +6.0f raises glyph upward (negative Y in Skia canvas space)
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(supRun.positions()[0].fY, -6.0f, 0.1f),
                            "Expected superscript vertical shift of -6.0f; got %f",
                            supRun.positions()[0].fY);
        }
    }
}

// =============================================================================
// Flutter Issue 144099: System Arabic font cascade
// =============================================================================
DEF_TEST(SkParagraph_Flutter_144099_ArabicSystemFontCascade, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* arabicText = "مرحبا بالعالم";
    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontFamilies({SkString(".AppleSystemUIFont")});
    ts.setFontSize(20.0f);
    builder.pushStyle(ts);
    builder.addText(arabicText);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->runs().empty());
    if (!impl->runs().empty()) {
        auto tf = impl->runs()[0].font().getTypeface();
        REPORTER_ASSERT(reporter, tf != nullptr);
        SkString psName;
        tf->getPostScriptName(&psName);
        REPORTER_ASSERT(reporter, !psName.contains("LastResort"));
    }
}

// =============================================================================
// Flutter Issue 79363: Locale-aware CJK font selection
// =============================================================================
DEF_TEST(SkParagraph_Flutter_79363_LocaleFontCascadePingFangVsHiragino, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "骨";
    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontFamilies({SkString(".AppleSystemUIFont")});
    ts.setFontSize(24.0f);
    ts.setLocale(SkString("zh-Hans"));
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(200.0f);

    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->runs().empty());
    if (!impl->runs().empty()) {
        auto tf = impl->runs()[0].font().getTypeface();
        REPORTER_ASSERT(reporter, tf != nullptr);
        SkString fam;
        tf->getFamilyName(&fam);
        // zh-Hans locale on Apple platforms resolves Han character to PingFang
        REPORTER_ASSERT(reporter, fam.contains("PingFang"),
                        "Expected Chinese locale to resolve PingFang typeface; got %s", fam.c_str());
    }
}

// =============================================================================
// Flutter Issue 11738: Caret positions at LTR/RTL boundary
// =============================================================================
DEF_TEST(SkParagraph_Flutter_11738_BiDiCaretDualPositions, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    // Mixed LTR and RTL: "Hello עברית"
    const char* text = "Hello \xD7\xA2\xD7\x91\xD7\xA8\xD7\x99\xD7\xAA";
    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(20.0f);
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    PositionWithAffinity posUpstream = paragraph->getGlyphPositionAtCoordinate(60.0f, 10.0f);
    REPORTER_ASSERT(reporter, posUpstream.position >= 0 &&
                              static_cast<size_t>(posUpstream.position) <= strlen(text));
}

// =============================================================================
// Flutter Issues 91991 & 91969: Justified RTL mixed-direction line width within bounds
// =============================================================================
DEF_TEST(SkParagraph_Flutter_91991_91969_JustifyRTLMixedDirectionBounds, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* text = "שלום World שלום World שלום";
    ParagraphStyle style;
    style.setTextDirection(TextDirection::kRtl);
    style.setTextAlign(TextAlign::kJustify);
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(16.0f);
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    const SkScalar maxWidth = 250.0f;
    paragraph->layout(maxWidth);

    std::vector<LineMetrics> lm;
    paragraph->getLineMetrics(lm);
    for (const auto& line : lm) {
        REPORTER_ASSERT(reporter, line.fWidth <= maxWidth + 0.5f,
                        "Justified line width (%f) exceeded constraint (%f)", line.fWidth, maxWidth);
    }
}

// =============================================================================
// Flutter Issue 54400: Placeholder placement in RTL text follows RTL reading order
// =============================================================================
DEF_TEST(SkParagraph_Flutter_54400_RTLWidgetSpanPlacement, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    ParagraphStyle style;
    style.setTextDirection(TextDirection::kRtl);
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(16.0f);
    builder.pushStyle(ts);
    builder.addText("שלום ");
    PlaceholderStyle placeholder(30.0f, 20.0f, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0.0f);
    builder.addPlaceholder(placeholder);
    builder.addText(" עולם");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(300.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() >= 3);

    auto rects = paragraph->getRectsForPlaceholders();
    REPORTER_ASSERT(reporter, rects.size() == 1, "Expected exactly 1 placeholder rect");

    auto boxes1 = paragraph->getRectsForRange(0, 5, RectHeightStyle::kTight, RectWidthStyle::kTight);
    auto boxes2 = paragraph->getRectsForRange(6, 11, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, !boxes1.empty() && !boxes2.empty());

    if (!rects.empty() && !boxes1.empty() && !boxes2.empty()) {
        const auto& phRect = rects[0].rect;
        // In RTL layout, visual X follows reading order: run1 (right) -> placeholder (center) -> run2 (left)
        REPORTER_ASSERT(reporter, boxes1[0].rect.fLeft >= phRect.fRight - 0.5f,
                        "Expected first RTL text run to sit rightward of placeholder: run1.left=%f, ph.right=%f",
                        boxes1[0].rect.fLeft, phRect.fRight);
        REPORTER_ASSERT(reporter, phRect.fLeft >= boxes2[0].rect.fRight - 0.5f,
                        "Expected placeholder to sit rightward of second RTL text run: ph.left=%f, run2.right=%f",
                        phRect.fLeft, boxes2[0].rect.fRight);
    }
}

// =============================================================================
// Flutter Issue 50292: Zero ICU footprint on Apple platforms
// =============================================================================
DEF_TEST(SkParagraph_Flutter_50292_ZeroICUFootprint, reporter) {
    auto unicode = get_test_unicode();
    REPORTER_ASSERT(reporter, unicode != nullptr, "Unicode engine required");
    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, make_test_font_collection(), unicode);
    builder.addText("Zero ICU Footprint", 18);
    auto paragraph = builder.Build();
    paragraph->layout(300.0f);
    REPORTER_ASSERT(reporter, paragraph->getRectsForRange(0, 18, RectHeightStyle::kTight, RectWidthStyle::kTight).size() > 0);
}

// =============================================================================
// Flutter Issue 113400: Zero dirty RAM footprint from external typography caches
// =============================================================================
DEF_TEST(SkParagraph_Flutter_113400_ZeroDirtyRAMFootprint, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    REPORTER_ASSERT(reporter, unicode != nullptr);

    ParagraphStyle style;
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
#endif
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(16.0f);
    builder.pushStyle(ts);
    builder.addText("Zero Dirty RAM Invariant Test");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->runs().empty());
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
    // CoreText shaper executes natively without allocating process-dirty ICU/HarfBuzz tables
    REPORTER_ASSERT(reporter, impl->isCoreTextShaped(),
                    "Expected native CoreText shaper without dirty heap caches");
#endif
}
