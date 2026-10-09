/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPoint.h"
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

// Flutter 150824, 18915, 147708, 22572: .AppleSystemUIFont applies size-dependent optical sizing
// (opsz) and tracking (trak) when shaped via CoreText, whereas SkCTFontCreateExactCopy on the
// HarfBuzz path and non-system fonts (Helvetica) scale linearly with point size
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

    auto layoutWidth = [&](TextShapingEngine engine, const char* family, SkScalar fontSize) {
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
        return paragraph->getLongestLine();
    };

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;

        // Negative control: Helvetica has no opsz/trak table, so normalized width is size-invariant
        SkScalar helv11 = layoutWidth(engine, "Helvetica", 11.0f) / 11.0f;
        SkScalar helv48 = layoutWidth(engine, "Helvetica", 48.0f) / 48.0f;
        REPORTER_ASSERT(reporter, std::abs(helv11 - helv48) < 0.01f,
                        "engine %d: Helvetica normalized width drifted (%f vs %f)",
                        static_cast<int>(engine), helv11, helv48);

        SkScalar sf11 = layoutWidth(engine, ".AppleSystemUIFont", 11.0f) / 11.0f;
        SkScalar sf14 = layoutWidth(engine, ".AppleSystemUIFont", 14.0f);
        SkScalar sf17 = layoutWidth(engine, ".AppleSystemUIFont", 17.0f) / 17.0f;
        SkScalar sf28 = layoutWidth(engine, ".AppleSystemUIFont", 28.0f);
        SkScalar sf48 = layoutWidth(engine, ".AppleSystemUIFont", 48.0f) / 48.0f;
        SkScalar ratio28to14 = sf28 / sf14;

        if (coreText) {
            // CoreText evaluates SF Pro opsz + trak, tightening 17pt (#22572), 28pt vs 14pt
            // (#18915 & #147708), and 48pt vs 11pt (#150824)
            REPORTER_ASSERT(reporter, sf11 - sf17 > 0.3f,
                            "CoreText 17pt HIG body tracking should tighten vs 11pt (%f vs %f)",
                            sf11, sf17);
            REPORTER_ASSERT(reporter, ratio28to14 < 1.88f,
                            "CoreText 28pt/14pt display ratio should contract (< 1.88x; got %f)",
                            ratio28to14);
            REPORTER_ASSERT(reporter, sf11 - sf48 > 1.5f,
                            "CoreText .AppleSystemUIFont should tighten at 48pt (11pt=%f, 48pt=%f)",
                            sf11, sf48);
        } else {
            // SkCTFontCreateExactCopy pins optical size and disables CoreText tracking on HarfBuzz
            REPORTER_ASSERT(reporter, std::abs(sf11 - sf17) < 0.01f);
            REPORTER_ASSERT(reporter, std::abs(ratio28to14 - 2.0f) < 0.01f,
                            "HarfBuzz 28pt/14pt ratio expected 2.0x; got %f", ratio28to14);
            REPORTER_ASSERT(reporter, std::abs(sf11 - sf48) < 0.01f,
                            "HarfBuzz .AppleSystemUIFont should scale linearly (11pt=%f, 48pt=%f)",
                            sf11, sf48);
        }
    }
}

// Flutter 121680 & 108720: on keycap emoji sequences ("1️⃣2️⃣3️⃣"), CoreText collapses each keycap
// into 1 glyph (3 total, +48px under letterSpacing=16), whereas HarfBuzz's Apple Color Emoji morx
// chain leaves a phantom zero-advance glyph (id=3) for each U+FE0F selector (6 glyphs total,
// causing Run::addLetterSpacesEvenly to double-space every keycap by +32px, +96px total)
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
    const char* text =
            "1\xEF\xB8\x8F\xE2\x83\xA3"
            "2\xEF\xB8\x8F\xE2\x83\xA3"
            "3\xEF\xB8\x8F\xE2\x83\xA3";

    auto buildWithSpacing = [&](TextShapingEngine engine, SkScalar spacing) {
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString(".AppleSystemUIFont")});
        ts.setFontSize(28.0f);
        ts.setLetterSpacing(spacing);
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();
        auto p = builder.Build();
        p->layout(500.0f);
        return p;
    };

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        auto p0 = buildWithSpacing(engine, 0.0f);
        auto p16 = buildWithSpacing(engine, 16.0f);

        auto impl0 = static_cast<ParagraphImpl*>(p0.get());
        auto impl16 = static_cast<ParagraphImpl*>(p16.get());
        REPORTER_ASSERT(reporter, impl0->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, impl16->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, p0->unresolvedGlyphs() == 0 && p16->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl16->runs().size() == 1);
        if (impl0->runs().size() != 1 || impl16->runs().size() != 1) {
            continue;
        }

        const Run& r0 = impl0->runs()[0];
        const Run& r16 = impl16->runs()[0];
        SkString emojiFamily;
        r0.font().getTypeface()->getFamilyName(&emojiFamily);
        REPORTER_ASSERT(reporter, emojiFamily.contains("Emoji"),
                        "engine %d: expected Emoji family, got %s",
                        static_cast<int>(engine), emojiFamily.c_str());

        const auto& clusters = impl0->clusters();
        REPORTER_ASSERT(reporter, clusters.size() == 4);
        if (clusters.size() == 4) {
            REPORTER_ASSERT(reporter, clusters[0].textRange() == TextRange(0, 7));
            REPORTER_ASSERT(reporter, clusters[1].textRange() == TextRange(7, 14));
            REPORTER_ASSERT(reporter, clusters[2].textRange() == TextRange(14, 21));
        }

        SkScalar widthDelta = p16->getLongestLine() - p0->getLongestLine();
        if (coreText) {
            REPORTER_ASSERT(reporter, r0.size() == 3 && r16.size() == 3);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 48.0f, 0.1f),
                            "CoreText expected 3 keycaps * 16px = 48px delta; got %f", widthDelta);
        } else {
            REPORTER_ASSERT(reporter, r0.size() == 6 && r16.size() == 6);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 96.0f, 0.1f),
                            "HarfBuzz expected 6 glyphs (3 phantom FE0F) * 16px = 96px; got %f",
                            widthDelta);
        }
    }
}

// Flutter 137976 & 100342: on ZWJ emoji sequences containing U+FE0F variation selectors
// ("🏳️‍🌈❤️‍🔥👩‍💻"), CoreText emits 3 fused glyphs (+48px under letterSpacing=16), whereas
// HarfBuzz emits 5 glyphs (leaving 2 phantom id=3 glyphs for U+FE0F in 🏳️‍🌈 and ❤️‍🔥, inflating
// letterSpacing=16 width by +80px)
DEF_TEST(SkParagraph_Flutter_137976_100342_ZWJEmojiClusterBounds, reporter) {
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
    const char* zwjText =
            "\xF0\x9F\x8F\xB3\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x8C\x88"
            "\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x94\xA5"
            "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB";

    auto buildWithSpacing = [&](TextShapingEngine engine, SkScalar spacing) {
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString(".AppleSystemUIFont")});
        ts.setFontSize(28.0f);
        ts.setLetterSpacing(spacing);
        builder.pushStyle(ts);
        builder.addText(zwjText);
        builder.pop();
        auto p = builder.Build();
        p->layout(500.0f);
        return p;
    };

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        auto p0 = buildWithSpacing(engine, 0.0f);
        auto p16 = buildWithSpacing(engine, 16.0f);

        auto impl0 = static_cast<ParagraphImpl*>(p0.get());
        auto impl16 = static_cast<ParagraphImpl*>(p16.get());
        REPORTER_ASSERT(reporter, impl0->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, impl16->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, p0->unresolvedGlyphs() == 0 && p16->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl16->runs().size() == 1);
        if (impl0->runs().size() != 1 || impl16->runs().size() != 1) {
            continue;
        }

        const Run& r0 = impl0->runs()[0];
        const Run& r16 = impl16->runs()[0];
        SkScalar widthDelta = p16->getLongestLine() - p0->getLongestLine();
        if (coreText) {
            REPORTER_ASSERT(reporter, r0.size() == 3 && r16.size() == 3);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 48.0f, 0.1f),
                            "CoreText expected 3 ZWJ emoji * 16px = 48px delta; got %f",
                            widthDelta);
        } else {
            REPORTER_ASSERT(reporter, r0.size() == 5 && r16.size() == 5);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 80.0f, 0.1f),
                            "HarfBuzz expected 5 glyphs (2 phantom FE0F) * 16px = 80px; got %f",
                            widthDelta);
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

// Flutter 144528: on Devanagari "हिन्दी" (2 orthographic syllables, 4 glyphs: pre-base matra ि,
// base ह, conjunct न्द्, post-base matra ी), CoreText's kCTTrackingAttributeName spaces the 2
// syllables as intact units (+32px at letterSpacing=16), whereas HarfBuzz's
// Run::addLetterSpacesEvenly adds +16px between every glyph (+64px total, detaching both matras)
DEF_TEST(SkParagraph_Flutter_144528_IndicConjunctLetterSpacing, reporter) {
    if (!ToolUtils::TestFontMgr()->matchFamilyStyle("Kohinoor Devanagari", SkFontStyle())) {
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
    const char* hindi = "\xE0\xA4\xB9\xE0\xA4\xBF\xE0\xA4\xA8\xE0\xA5\x8D\xE0\xA4\xA6\xE0\xA5\x80";

    auto buildWithSpacing = [&](TextShapingEngine engine, SkScalar spacing) {
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString("Kohinoor Devanagari")});
        ts.setFontSize(24.0f);
        ts.setLetterSpacing(spacing);
        builder.pushStyle(ts);
        builder.addText(hindi);
        builder.pop();
        auto p = builder.Build();
        p->layout(500.0f);
        return p;
    };

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        auto p0 = buildWithSpacing(engine, 0.0f);
        auto p16 = buildWithSpacing(engine, 16.0f);

        auto impl0 = static_cast<ParagraphImpl*>(p0.get());
        auto impl16 = static_cast<ParagraphImpl*>(p16.get());
        REPORTER_ASSERT(reporter, impl0->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, impl16->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, p0->unresolvedGlyphs() == 0 && p16->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl16->runs().size() == 1);
        if (impl0->runs().size() != 1 || impl16->runs().size() != 1) {
            continue;
        }

        const Run& r0 = impl0->runs()[0];
        const Run& r16 = impl16->runs()[0];
        REPORTER_ASSERT(reporter, r0.size() == 4 && r16.size() == 4);
        if (r0.size() != 4 || r16.size() != 4) {
            continue;
        }

        SkScalar syl1Span0 = r0.positions()[1].fX - r0.positions()[0].fX;
        SkScalar syl1Span16 = r16.positions()[1].fX - r16.positions()[0].fX;
        SkScalar widthDelta = p16->getLongestLine() - p0->getLongestLine();

        if (coreText) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(syl1Span16, syl1Span0, 0.05f),
                            "CoreText matra detached inside syllable 1: ls0=%f, ls16=%f",
                            syl1Span0, syl1Span16);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 32.0f, 0.1f),
                            "CoreText expected 2 syllables * 16px = 32px delta; got %f",
                            widthDelta);
        } else {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(syl1Span16 - syl1Span0, 16.0f, 0.05f),
                            "HarfBuzz per-glyph spacing delta expected +16px; got %f",
                            syl1Span16 - syl1Span0);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 64.0f, 0.1f),
                            "HarfBuzz expected 4 glyphs * 16px = 64px delta; got %f",
                            widthDelta);
        }
    }
}
