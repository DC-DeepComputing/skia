/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
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

#include <cmath>
#include <vector>

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

// Flutter 150824 & 18915: .AppleSystemUIFont applies size-dependent optical sizing (opsz) and
// tracking (trak) when shaped via CoreText, whereas SkCTFontCreateExactCopy on the HarfBuzz path
// and non-system fonts (Helvetica) scale linearly with point size
DEF_TEST(SkParagraph_Flutter_150824_18915_SystemFontOpticalSizeAndTracking, reporter) {
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
    const char* text = "DynamicTrackingTest Hello World";

    auto layoutNormalized = [&](TextShapingEngine engine, const char* family, SkScalar fontSize) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString(family)});
        ts.setFontSize(fontSize);
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(2000.0f);
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        return paragraph->getLongestLine() / fontSize;
    };

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;

        // Negative control: Helvetica has no opsz/trak table, so normalized width is size-invariant
        SkScalar helv11 = layoutNormalized(engine, "Helvetica", 11.0f);
        SkScalar helv48 = layoutNormalized(engine, "Helvetica", 48.0f);
        REPORTER_ASSERT(reporter, std::abs(helv11 - helv48) < 0.01f,
                        "engine %d: Helvetica normalized width drifted (%f vs %f)",
                        static_cast<int>(engine), helv11, helv48);

        SkScalar sf11 = layoutNormalized(engine, ".AppleSystemUIFont", 11.0f);
        SkScalar sf48 = layoutNormalized(engine, ".AppleSystemUIFont", 48.0f);
        if (coreText) {
            // CoreText evaluates SF Pro opsz + trak, tightening display sizes relative to 11pt
            REPORTER_ASSERT(reporter, sf11 - sf48 > 1.5f,
                            "CoreText .AppleSystemUIFont should tighten at 48pt (11pt=%f, 48pt=%f)",
                            sf11, sf48);
        } else {
            // SkCTFontCreateExactCopy pins optical size and disables CoreText tracking on HarfBuzz
            REPORTER_ASSERT(reporter, std::abs(sf11 - sf48) < 0.01f,
                            "HarfBuzz .AppleSystemUIFont should scale linearly (11pt=%f, 48pt=%f)",
                            sf11, sf48);
        }
    }
}

// Flutter 121680 & 108720: keycap emoji sequence ('1' + U+FE0F + U+20E3) embedded between ASCII
// letters must isolate into its own Emoji fallback run and form a single [1, 8) grapheme cluster
DEF_TEST(SkParagraph_Flutter_121680_108720_KeycapEmojiCluster, reporter) {
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
    // "a1️⃣b": 'a' [0, 1), '1' + U+FE0F + U+20E3 [1, 8), 'b' [8, 9)
    const char* text = "a1\xEF\xB8\x8F\xE2\x83\xA3" "b";

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString(".AppleSystemUIFont")});
        ts.setFontSize(20.0f);
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(500.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

        const auto& runs = impl->runs();
        REPORTER_ASSERT(reporter, runs.size() == 3, "engine %d: got %zu runs",
                        static_cast<int>(engine), runs.size());
        if (runs.size() != 3) {
            continue;
        }

        REPORTER_ASSERT(reporter, runs[0].textRange() == TextRange(0, 1));
        REPORTER_ASSERT(reporter, runs[1].textRange() == TextRange(1, 8));
        REPORTER_ASSERT(reporter, runs[2].textRange() == TextRange(8, 9));

        SkString emojiFamily;
        runs[1].font().getTypeface()->getFamilyName(&emojiFamily);
        REPORTER_ASSERT(reporter, emojiFamily.contains("Emoji"),
                        "engine %d: expected Emoji family, got %s",
                        static_cast<int>(engine), emojiFamily.c_str());
        if (coreText) {
            REPORTER_ASSERT(reporter, runs[1].size() == 1);
        } else {
            // HarfBuzz emits the keycap ligature plus a zero-advance variation selector glyph
            // in the same cluster
            for (size_t i = 0; i < runs[1].size(); ++i) {
                REPORTER_ASSERT(reporter, runs[1].globalClusterIndex(i) == 1);
            }
        }

        const auto& clusters = impl->clusters();
        REPORTER_ASSERT(reporter, clusters.size() == 4);
        if (clusters.size() == 4) {
            REPORTER_ASSERT(reporter, clusters[0].textRange() == TextRange(0, 1));
            REPORTER_ASSERT(reporter, clusters[1].textRange() == TextRange(1, 8));
            REPORTER_ASSERT(reporter, clusters[2].textRange() == TextRange(8, 9));
            REPORTER_ASSERT(reporter, clusters[1].width() > 0.0f);
        }
    }
}

// Flutter 190569: Arabic Letter Mark (U+061C) before RTL fallback runs must keep every glyph's
// global cluster index bounded within its run textRange on both shaping engines
DEF_TEST(SkParagraph_Flutter_190569_BiDi_ALM_Monotonicity, reporter) {
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
    const char* text = "Test \xD8\x9C \xD8\xB9\xD8\xB1\xD8\xA8\xD9\x8A Fallback";

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString(".AppleSystemUIFont")});
        ts.setFontSize(20.0f);
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(500.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, !impl->runs().empty());

        for (const auto& run : impl->runs()) {
            for (size_t c = 0; c < run.size(); ++c) {
                size_t clusterIndex = run.globalClusterIndex(c);
                REPORTER_ASSERT(reporter,
                                clusterIndex >= run.textRange().start &&
                                        clusterIndex < run.textRange().end,
                                "engine %d: cluster %zu out of run bounds [%zu, %zu)",
                                static_cast<int>(engine), clusterIndex,
                                run.textRange().start, run.textRange().end);
            }
        }
    }
}

// Flutter 141080: Thai consonant + tone mark + vowel ("ก่า") split across three colored spans
// must preserve per-codepoint cluster ranges [0,3), [3,6), [6,9) and keep the tone mark's
// horizontal anchor over the base consonant identical to unstyled text on both engines
DEF_TEST(SkParagraph_Flutter_141080_ThaiCombiningMarksStyledClusters, reporter) {
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
    const char* koKai = "\xE0\xB8\x81";
    const char* maiEk = "\xE0\xB9\x88";
    const char* saraAa = "\xE0\xB8\xB2";
    const char* fullThai = "\xE0\xB8\x81\xE0\xB9\x88\xE0\xB8\xB2";

    auto verifyAndGetMarkOffset = [&](Paragraph* paragraph, bool coreText) -> SkScalar {
        auto impl = static_cast<ParagraphImpl*>(paragraph);
        REPORTER_ASSERT(reporter, impl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);

        const auto& clusters = impl->clusters();
        REPORTER_ASSERT(reporter, clusters.size() == 4);
        if (clusters.size() == 4) {
            REPORTER_ASSERT(reporter, clusters[0].textRange() == TextRange(0, 3));
            REPORTER_ASSERT(reporter, clusters[1].textRange() == TextRange(3, 6));
            REPORTER_ASSERT(reporter, clusters[2].textRange() == TextRange(6, 9));
        }

        REPORTER_ASSERT(reporter, !impl->runs().empty());
        if (impl->runs().empty() || impl->runs()[0].size() < 2) {
            REPORTER_ASSERT(reporter, false, "Expected base consonant and tone mark in run 0");
            return 0.0f;
        }
        const Run& run0 = impl->runs()[0];
        // Run::copyTo places glyph i at positions()[i].fX + offsets()[i].fX
        SkScalar baseVisualX = run0.positions()[0].fX + run0.offsets()[0].fX;
        SkScalar markVisualX = run0.positions()[1].fX + run0.offsets()[1].fX;
        SkScalar baseEndX = run0.positions()[2].fX;
        REPORTER_ASSERT(reporter, markVisualX > baseVisualX && markVisualX < baseEndX,
                        "Tone mark visual X (%f) must sit inside base consonant [%f, %f)",
                        markVisualX, baseVisualX, baseEndX);
        return markVisualX - baseVisualX;
    };

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;

        ParagraphStyle unstyledPS;
        unstyledPS.setTextShapingEngine(engine);
        ParagraphBuilderImpl unstyledBuilder(unstyledPS, fontCollection, unicode);
        TextStyle baseTS;
        baseTS.setFontFamilies({SkString(".AppleSystemUIFont")});
        baseTS.setFontSize(20.0f);
        baseTS.setColor(SK_ColorBLACK);
        unstyledBuilder.pushStyle(baseTS);
        unstyledBuilder.addText(fullThai);
        unstyledBuilder.pop();
        auto unstyledPara = unstyledBuilder.Build();
        unstyledPara->layout(500.0f);

        ParagraphStyle styledPS;
        styledPS.setTextShapingEngine(engine);
        ParagraphBuilderImpl styledBuilder(styledPS, fontCollection, unicode);
        TextStyle redTS = baseTS;
        redTS.setColor(SK_ColorRED);
        TextStyle blueTS = baseTS;
        blueTS.setColor(SK_ColorBLUE);
        styledBuilder.pushStyle(redTS);
        styledBuilder.addText(koKai);
        styledBuilder.pop();
        styledBuilder.pushStyle(blueTS);
        styledBuilder.addText(maiEk);
        styledBuilder.pop();
        styledBuilder.pushStyle(redTS);
        styledBuilder.addText(saraAa);
        styledBuilder.pop();
        auto styledPara = styledBuilder.Build();
        styledPara->layout(500.0f);

        SkScalar unstyledOffset = verifyAndGetMarkOffset(unstyledPara.get(), coreText);
        SkScalar styledOffset = verifyAndGetMarkOffset(styledPara.get(), coreText);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(styledOffset, unstyledOffset, 0.05f),
                        "engine %d: styled mark offset %f != unstyled %f",
                        static_cast<int>(engine), styledOffset, unstyledOffset);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(styledPara->getLongestLine(),
                                            unstyledPara->getLongestLine(), 0.05f));
    }
}

// Flutter 130492: requesting SkFontStyle::Italic() on .AppleSystemUIFont must resolve an italic
// typeface or apply synthetic oblique skew on both shaping engines
DEF_TEST(SkParagraph_Flutter_130492_SystemFontItalicSkew, reporter) {
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
        TextStyle italicStyle;
        italicStyle.setFontFamilies({SkString(".AppleSystemUIFont")});
        italicStyle.setFontSize(16.0f);
        italicStyle.setFontStyle(SkFontStyle::Italic());
        builder.pushStyle(italicStyle);
        builder.addText("System Italic Skew");
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(500.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, !impl->runs().empty());
        if (!impl->runs().empty()) {
            auto tf = impl->runs()[0].font().getTypeface();
            REPORTER_ASSERT(reporter, tf != nullptr);
            SkString psName;
            tf->getPostScriptName(&psName);
            bool isItalic = psName.contains("Italic") || psName.contains("Oblique") ||
                            tf->fontStyle().slant() != SkFontStyle::kUpright_Slant ||
                            impl->runs()[0].font().getSkewX() != 0.0f;
            REPORTER_ASSERT(reporter, isItalic, "engine %d: got %s",
                            static_cast<int>(engine), psName.c_str());
        }
    }
}

// Flutter 118127: on decomposed "ax\u0301b" (which has no precomposed NFC form), CoreText's
// kCTTrackingAttributeName keeps the combining acute anchored to 'x' under letterSpacing=10 and
// expands width by 3 clusters (30px), whereas HarfBuzz's Run::addLetterSpacesEvenly shifts every
// glyph including the combining mark (+10px mark drift, 40px width expansion)
DEF_TEST(SkParagraph_Flutter_118127_CombiningDiacriticsLetterSpacing, reporter) {
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
    const char* text = "ax\xCC\x81" "b";

    auto buildWithSpacing = [&](TextShapingEngine engine, SkScalar spacing) {
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString("Helvetica")});
        ts.setFontSize(20.0f);
        ts.setLetterSpacing(spacing);
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();
        auto p = builder.Build();
        p->layout(500.0f);
        return p;
    };

    auto visualX = [](const Run& run, size_t idx) -> SkScalar {
        return run.positions()[idx].fX + run.offsets()[idx].fX;
    };

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        auto p0 = buildWithSpacing(engine, 0.0f);
        auto p10 = buildWithSpacing(engine, 10.0f);

        auto impl0 = static_cast<ParagraphImpl*>(p0.get());
        auto impl10 = static_cast<ParagraphImpl*>(p10.get());
        REPORTER_ASSERT(reporter, impl0->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, impl10->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl10->runs().size() == 1);
        if (impl0->runs().size() != 1 || impl10->runs().size() != 1) {
            continue;
        }

        const Run& r0 = impl0->runs()[0];
        const Run& r10 = impl10->runs()[0];
        // Both engines emit 4 glyphs: 'a' (0), 'x' (1), U+0301 (2), 'b' (3)
        REPORTER_ASSERT(reporter, r0.size() == 4 && r10.size() == 4);
        if (r0.size() != 4 || r10.size() != 4) {
            continue;
        }

        SkScalar markDelta0 = visualX(r0, 2) - visualX(r0, 1);
        SkScalar markDelta10 = visualX(r10, 2) - visualX(r10, 1);
        SkScalar widthDelta = p10->getLongestLine() - p0->getLongestLine();

        if (coreText) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(markDelta10, markDelta0, 0.05f),
                            "CoreText mark detached under letterSpacing: ls0=%f, ls10=%f",
                            markDelta0, markDelta10);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 30.0f, 0.05f),
                            "CoreText widthDelta should be 3 clusters * 10px = 30px; got %f",
                            widthDelta);
        } else {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(markDelta10 - markDelta0, 10.0f, 0.05f),
                            "HarfBuzz per-glyph spacing delta expected +10px; got %f",
                            markDelta10 - markDelta0);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 40.0f, 0.05f),
                            "HarfBuzz widthDelta expected 4 glyphs * 10px = 40px; got %f",
                            widthDelta);
        }
    }
}

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
// Flutter 144528: Devanagari conjunct "क्ट्र" remains a single fused glyph under letterSpacing
DEF_TEST(SkParagraph_Flutter_144528_IndicConjunctLetterSpacing, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* conjunct = "\xE0\xA4\x95\xE0\xA5\x8D\xE0\xA4\x9F\xE0\xA5\x8D\xE0\xA4\xB0";

    auto buildWithSpacing = [&](SkScalar spacing) {
        ParagraphStyle style;
        style.setTextShapingEngine(TextShapingEngine::kCoreText);
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
    REPORTER_ASSERT(reporter, impl0->isCoreTextShaped() && impl4->isCoreTextShaped());
    REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl0->runs()[0].size() == 1);
    REPORTER_ASSERT(reporter, impl4->runs().size() == 1 && impl4->runs()[0].size() == 1);

    SkScalar widthDelta = p4->getMaxIntrinsicWidth() - p0->getMaxIntrinsicWidth();
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 4.0f, 0.05f),
                    "Expected 4.0f letter spacing delta; got %f", widthDelta);
}

// Flutter 137976 & 100342: ZWJ emoji sequence shapes into a single atomic glyph in CoreText
DEF_TEST(SkParagraph_Flutter_137976_100342_ZWJEmojiClusterBounds, reporter) {
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    auto unicode = get_test_unicode();
    if (!unicode) {
        return;
    }

    const char* zwjEmoji = "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB";

    ParagraphStyle style;
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(style, fontCollection, unicode);
    TextStyle ts;
    ts.setFontSize(24.0f);
    builder.pushStyle(ts);
    builder.addText(zwjEmoji);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->isCoreTextShaped());
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    if (impl->runs().size() == 1) {
        REPORTER_ASSERT(reporter, impl->runs()[0].size() == 1);
        SkString fam;
        impl->runs()[0].font().getTypeface()->getFamilyName(&fam);
        REPORTER_ASSERT(reporter, fam.contains("Emoji"), "Expected Emoji font; got %s",
                        fam.c_str());
    }
}
#endif

// Flutter 161201: Ideographic Variation Sequence (U+8FBB U+E0100) in Hiragino Sans must select a
// distinct single variation glyph from the base ideograph (U+8FBB) on both shaping engines
DEF_TEST(SkParagraph_Flutter_161201_IdeographicVariationSequences, reporter) {
    if (!ToolUtils::TestFontMgr()->matchFamilyStyle("Hiragino Sans", SkFontStyle())) {
        return;
    }
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
    const char* baseText = "\xE8\xBE\xBB";
    const char* ivsText = "\xE8\xBE\xBB\xF3\xA0\x84\x80";

    auto shapeSingleGlyph = [&](TextShapingEngine engine, const char* text) -> SkGlyphID {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString("Hiragino Sans")});
        ts.setFontSize(20.0f);
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(500.0f);
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, impl->runs().size() == 1);
        if (impl->runs().size() != 1) {
            return 0;
        }
        REPORTER_ASSERT(reporter, impl->runs()[0].size() == 1,
                        "engine %d: expected 1 glyph, got %zu",
                        static_cast<int>(engine), impl->runs()[0].size());
        return impl->runs()[0].size() == 1 ? impl->runs()[0].glyphs()[0] : 0;
    };

    for (TextShapingEngine engine : engines) {
        SkGlyphID baseGlyph = shapeSingleGlyph(engine, baseText);
        SkGlyphID ivsGlyph = shapeSingleGlyph(engine, ivsText);
        REPORTER_ASSERT(reporter, baseGlyph != 0 && ivsGlyph != 0);
        REPORTER_ASSERT(reporter, ivsGlyph != baseGlyph,
                        "engine %d: IVS U+E0100 did not select variation glyph (both=%u)",
                        static_cast<int>(engine), baseGlyph);
    }
}

// Flutter 193281: RTL line wrapping across normal and bold fallback spans must stay within maxWidth
DEF_TEST(SkParagraph_Flutter_193281_BiDi_RTLBoldFallbackWrapping, reporter) {
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

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextDirection(TextDirection::kRtl);
        style.setTextShapingEngine(engine);
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

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->isCoreTextShaped() == coreText);

        std::vector<LineMetrics> lm;
        paragraph->getLineMetrics(lm);
        REPORTER_ASSERT(reporter, lm.size() >= 3);
        for (size_t i = 0; i < lm.size(); ++i) {
            REPORTER_ASSERT(reporter, lm[i].fWidth > 0.0f && lm[i].fWidth <= maxWidth + 0.5f,
                            "engine %d: line %zu width (%f) exceeded maxWidth (%f)",
                            static_cast<int>(engine), i, lm[i].fWidth, maxWidth);
        }
    }
}

// Flutter 54400: inline placeholder between two RTL words must sit visually between the first
// (rightmost) and second (leftmost) RTL text runs on both shaping engines
DEF_TEST(SkParagraph_Flutter_54400_RTLWidgetSpanPlacement, reporter) {
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

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextDirection(TextDirection::kRtl);
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontSize(16.0f);
        builder.pushStyle(ts);
        builder.addText("שלום ");
        PlaceholderStyle placeholder(
                30.0f, 20.0f, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0.0f);
        builder.addPlaceholder(placeholder);
        builder.addText(" עולם");
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(300.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, impl->runs().size() >= 3);

        auto rects = paragraph->getRectsForPlaceholders();
        REPORTER_ASSERT(reporter, rects.size() == 1);

        auto boxes1 = paragraph->getRectsForRange(
                0, 5, RectHeightStyle::kTight, RectWidthStyle::kTight);
        auto boxes2 = paragraph->getRectsForRange(
                6, 11, RectHeightStyle::kTight, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxes1.empty() && !boxes2.empty());

        if (!rects.empty() && !boxes1.empty() && !boxes2.empty()) {
            const auto& phRect = rects[0].rect;
            REPORTER_ASSERT(reporter, boxes1[0].rect.fLeft >= phRect.fRight - 0.5f,
                            "engine %d: run1.left=%f, ph.right=%f",
                            static_cast<int>(engine), boxes1[0].rect.fLeft, phRect.fRight);
            REPORTER_ASSERT(reporter, phRect.fLeft >= boxes2[0].rect.fRight - 0.5f,
                            "engine %d: ph.left=%f, run2.right=%f",
                            static_cast<int>(engine), phRect.fLeft, boxes2[0].rect.fRight);
        }
    }
}
