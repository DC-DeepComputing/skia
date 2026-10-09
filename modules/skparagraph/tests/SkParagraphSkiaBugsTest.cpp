/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
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
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

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

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
// Skia b/318667611 & b/328418459: CoreText resolves the .AppleSystemUIFont cascade across
// Latin, CJK, and Arabic runs directly via CTLineGetGlyphRuns without falling back to LastResort
DEF_TEST(SkParagraph_Skia_b_318667611_AppleSystemUIFontCascade, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* cascadeText =
            "SF Pro    \xE7\xAE\x80\xE4\xBD\x93\xE4\xB8\xAD\xE6\x96\x87    "
            "\xD9\x85\xD8\xB1\xD8\xAD\xD8\xA8\xD8\xA7";
    ParagraphStyle style;
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontFamilies({SkString(".AppleSystemUIFont")});
    ts.setFontSize(24.0f);
    builder.pushStyle(ts);
    builder.addText(cascadeText);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(1000);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->isCoreTextShaped());
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, !impl->runs().empty());

    bool hasSystemFontRun = false;
    bool hasChineseRun = false;
    bool hasArabicRun = false;
    for (const auto& run : impl->runs()) {
        SkString ps;
        if (run.font().getTypeface()) {
            run.font().getTypeface()->getPostScriptName(&ps);
        }
        REPORTER_ASSERT(reporter, !ps.contains("LastResort"),
                        "Cascade run fell back to LastResort: %s", ps.c_str());
        if (ps.contains("SF") || ps.contains("AppleSystemUIFont")) {
            hasSystemFontRun = true;
        }
        if (ps.contains("PingFang")) {
            hasChineseRun = true;
        }
        if (ps.contains("Arabic")) {
            hasArabicRun = true;
        }
    }
    REPORTER_ASSERT(reporter, hasSystemFontRun);
    REPORTER_ASSERT(reporter, hasChineseRun);
    REPORTER_ASSERT(reporter, hasArabicRun);
}
#endif

// Skia b/40040287: hit-testing across astral-plane emoji (U+1F600 surrogate pair) must return
// UTF-16 code unit offsets rather than raw UTF-8 byte offsets on every shaping engine
DEF_TEST(SkParagraph_Skia_b_40040287_UTF16_AstralPlaneIndices, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const TextShapingEngine engines[] = {
        TextShapingEngine::kHarfBuzz,
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
        TextShapingEngine::kCoreText,
#endif
    };
    // "A😀B": 'A' is UTF-16 [0, 1), '😀' (U+1F600) is [1, 3), 'B' is [3, 4)
    const char* text = "A\xF0\x9F\x98\x80\x42";
    const size_t len = strlen(text);

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontSize(24.0f);
        builder.pushStyle(ts);
        builder.addText(text, len);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(500);
        auto paragraphImpl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, paragraphImpl->isCoreTextShaped() == coreText);

        auto boxesA = paragraph->getRectsForRange(
                0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
        auto boxesEmoji = paragraph->getRectsForRange(
                1, 3, RectHeightStyle::kTight, RectWidthStyle::kTight);
        auto boxesB = paragraph->getRectsForRange(
                3, 4, RectHeightStyle::kTight, RectWidthStyle::kTight);

        REPORTER_ASSERT(reporter, !boxesA.empty() && !boxesEmoji.empty() && !boxesB.empty());
        if (boxesA.empty() || boxesEmoji.empty() || boxesB.empty()) {
            continue;
        }

        PositionWithAffinity posA =
                paragraph->getGlyphPositionAtCoordinate(boxesA[0].rect.fLeft + 1.0f, 12.0f);
        PositionWithAffinity posEmoji =
                paragraph->getGlyphPositionAtCoordinate(boxesEmoji[0].rect.fLeft + 1.0f, 12.0f);
        PositionWithAffinity posB =
                paragraph->getGlyphPositionAtCoordinate(boxesB[0].rect.fLeft + 1.0f, 12.0f);
        PositionWithAffinity posEnd =
                paragraph->getGlyphPositionAtCoordinate(boxesB[0].rect.fRight + 10.0f, 12.0f);

        REPORTER_ASSERT(reporter, posA.position == 0, "engine %d: got %d",
                        static_cast<int>(engine), posA.position);
        REPORTER_ASSERT(reporter, posEmoji.position == 1, "engine %d: got %d",
                        static_cast<int>(engine), posEmoji.position);
        REPORTER_ASSERT(reporter, posB.position == 3, "engine %d: got %d",
                        static_cast<int>(engine), posB.position);
        REPORTER_ASSERT(reporter, posEnd.position == 4, "engine %d: got %d",
                        static_cast<int>(engine), posEnd.position);
    }
}

// Skia b/448789662 & b/519671848: mixed LTR/RTL runs with font fallback must keep global cluster
// indices bounded within each run's UTF-8 textRange on every shaping engine
DEF_TEST(SkParagraph_Skia_b_448789662_BiDi_MixedRunsClusterBounds, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const TextShapingEngine engines[] = {
        TextShapingEngine::kHarfBuzz,
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
        TextShapingEngine::kCoreText,
#endif
    };
    const char* text = "Hello \xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D World";
    const size_t len = strlen(text);

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontSize(20.0f);
        builder.pushStyle(ts);
        builder.addText(text, len);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(400);

        auto paragraphImpl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, paragraphImpl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

        const auto& runs = paragraphImpl->runs();
        REPORTER_ASSERT(reporter, runs.size() >= 3);

        bool foundRtlRun = false;
        bool foundLtrRun = false;
        for (const auto& run : runs) {
            if (!run.leftToRight()) {
                foundRtlRun = true;
                REPORTER_ASSERT(reporter, run.bidiLevel() % 2 == 1);
                REPORTER_ASSERT(reporter, run.getTextDirection() == TextDirection::kRtl);
                REPORTER_ASSERT(reporter,
                                run.textRange().start >= 6 && run.textRange().end <= 14);
            } else {
                foundLtrRun = true;
                REPORTER_ASSERT(reporter, run.bidiLevel() % 2 == 0);
                REPORTER_ASSERT(reporter, run.getTextDirection() == TextDirection::kLtr);
            }

            for (size_t c = 0; c < run.size(); ++c) {
                size_t cluster = run.globalClusterIndex(c);
                REPORTER_ASSERT(reporter,
                                cluster >= run.textRange().start && cluster < run.textRange().end,
                                "engine %d: cluster %zu outside [%zu, %zu)",
                                static_cast<int>(engine), cluster,
                                run.textRange().start, run.textRange().end);
            }
        }

        REPORTER_ASSERT(reporter, foundRtlRun);
        REPORTER_ASSERT(reporter, foundLtrRun);

        auto hebrewBoxes = paragraph->getRectsForRange(
                6, 10, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !hebrewBoxes.empty());
        if (!hebrewBoxes.empty()) {
            REPORTER_ASSERT(reporter,
                            hebrewBoxes[0].rect.width() > 0 && hebrewBoxes[0].rect.height() > 0);
        }
    }
}

// Skia b/458132646 & b/463954175: Japanese Hiragana with .AppleSystemUIFont cascades to a CJK
// typeface (Hiragino / PingFang) rather than Apple Color Emoji or LastResort
DEF_TEST(SkParagraph_Skia_b_458132646_JapaneseKana_NotEmoji, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const TextShapingEngine engines[] = {
        TextShapingEngine::kHarfBuzz,
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
        TextShapingEngine::kCoreText,
#endif
    };
    const char* text = "\xE3\x81\x93\xE3\x82\x93\xE3\x81\xAB\xE3\x81\xA1\xE3\x81\xAF";
    const size_t len = strlen(text);

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
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
        REPORTER_ASSERT(reporter, paragraphImpl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

        const auto& runs = paragraphImpl->runs();
        REPORTER_ASSERT(reporter, !runs.empty());

        bool hasJapaneseFont = false;
        for (const auto& run : runs) {
            if (auto tf = run.font().getTypeface()) {
                SkString name;
                tf->getFamilyName(&name);
                REPORTER_ASSERT(reporter, !name.contains("LastResort") && !name.contains("Emoji"),
                                "engine %d: unexpected fallback %s",
                                static_cast<int>(engine), name.c_str());
                if (name.contains("Hiragino") || name.contains("PingFang") ||
                    name.contains("CJK")) {
                    hasJapaneseFont = true;
                }
            }
        }
        REPORTER_ASSERT(reporter, hasJapaneseFont);

        auto boxes = paragraph->getRectsForRange(
                0, 5, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxes.empty());
        if (!boxes.empty()) {
            REPORTER_ASSERT(reporter, boxes[0].rect.width() > 0 && boxes[0].rect.height() > 0);
        }
    }
}

// Skia b/341104766 & skia:12390: baseline shift across styled spans travels in Run::fBaselineShift
// on every engine without baking a second vertical shift into glyph fY positions
DEF_TEST(SkParagraph_Skia_b_341104766_BaselineShiftCarrier, reporter) {
    auto fontCollection = make_test_font_collection();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const TextShapingEngine engines[] = {
        TextShapingEngine::kHarfBuzz,
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
        TextShapingEngine::kCoreText,
#endif
    };
    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);

        TextStyle normalStyle;
        normalStyle.setFontSize(20.0f);
        TextStyle subStyle;
        subStyle.setFontSize(12.0f);
        subStyle.setBaselineShift(-6.0f);

        builder.pushStyle(normalStyle);
        builder.addText("H", 1);
        builder.pop();
        builder.pushStyle(subStyle);
        builder.addText("2", 1);
        builder.pop();
        builder.pushStyle(normalStyle);
        builder.addText("O", 1);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(200);

        auto paragraphImpl = static_cast<ParagraphImpl*>(paragraph.get());
        // A CoreText shaping failure falls back to HarfBuzz, which would make this pass vacuous
        REPORTER_ASSERT(reporter, paragraphImpl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        const auto& runs = paragraphImpl->runs();
        REPORTER_ASSERT(reporter, runs.size() == 3, "engine %d: %zu run(s) for H2O",
                        static_cast<int>(engine), runs.size());
        if (runs.size() != 3) {
            continue;
        }

        // TextLine::buildTextBlob translates each run by its baselineShift, so a shaper that
        // also bakes the shift into glyph fY paints the span back onto the unshifted baseline
        REPORTER_ASSERT(reporter, runs[0].baselineShift() == 0.0f);
        REPORTER_ASSERT(reporter, runs[1].baselineShift() == -6.0f);
        REPORTER_ASSERT(reporter, runs[2].baselineShift() == 0.0f);
        for (const Run& run : runs) {
            for (const SkPoint& pos : run.positions()) {
                REPORTER_ASSERT(reporter, SkScalarNearlyZero(pos.fY),
                                "engine %d: run [%zu, %zu) glyph fY %g left the baseline",
                                static_cast<int>(engine), run.textRange().start,
                                run.textRange().end, pos.fY);
            }
        }
    }
}
