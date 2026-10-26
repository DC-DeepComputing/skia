/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skunicode/include/SkUnicode.h"
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_apple.h"
#endif
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <vector>

using namespace skia::textlayout;

// Every contract below is asserted on the CoreText engine; without it there is nothing to test
#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)

namespace {

// A HarfBuzz-free build aborts on kDefault, so only the CoreText half of each contract runs
constexpr TextShapingEngine kEngines[] = {
#if !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)
        TextShapingEngine::kDefault,
#endif
        TextShapingEngine::kCoreText};

// Each engine gets the SkUnicode its clients ship with: Apple's for CoreText, ICU for HarfBuzz.
// A build lacking it yields null, which ParagraphBuilder::Build rejects instead of silently
// laying out with another implementation
sk_sp<SkUnicode> make_unicode(TextShapingEngine engine) {
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
    if (engine == TextShapingEngine::kCoreText) {
        return SkUnicodes::Apple::Make();
    }
#endif
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    return SkUnicodes::ICU::Make();
#else
    return nullptr;
#endif
}

sk_sp<FontCollection> make_test_font_collection() {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    return fontCollection;
}

// The bugs are about specific Apple faces; the portable font manager (--nonativeFonts) has
// none of them, so the tests skip instead of measuring substitutes
bool has_font_families(std::initializer_list<const char*> families) {
    for (const char* family : families) {
        if (!ToolUtils::TestFontMgr()->matchFamilyStyle(family, SkFontStyle())) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<Paragraph> layout_text(skiatest::Reporter* reporter,
                                       const sk_sp<FontCollection>& fontCollection,
                                       TextShapingEngine engine,
                                       const char* text,
                                       const char* family,
                                       SkScalar fontSize,
                                       SkScalar letterSpacing,
                                       SkScalar width) {
    ParagraphStyle style;
    style.setTextShapingEngine(engine);
    ParagraphBuilderImpl builder(style, fontCollection, make_unicode(engine));
    TextStyle ts;
    ts.setFontFamilies({SkString(family)});
    ts.setFontSize(fontSize);
    ts.setLetterSpacing(letterSpacing);
    builder.pushStyle(ts);
    builder.addText(text);
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(width);
    REPORTER_ASSERT(reporter,
                    static_cast<ParagraphImpl*>(paragraph.get())->isCoreTextShaped() ==
                            (engine == TextShapingEngine::kCoreText));
    return paragraph;
}

}  // namespace

// Flutter 150824, 18915, 22572: .AppleSystemUIFont applies size-dependent optical sizing
// (opsz) and tracking (trak) when shaped via CoreText, whereas SkCTFontCreateExactCopy on the
// HarfBuzz path and non-system fonts (Helvetica) scale linearly with point size
DEF_TEST(SkParagraph_Flutter_150824_18915_SystemFontOpticalSizeAndTracking, reporter) {
    if (!has_font_families({"Helvetica", ".AppleSystemUIFont"})) {
        return;
    }
    auto fontCollection = make_test_font_collection();
    const char* text = "DynamicTrackingTest Hello World";

    auto layoutWidth = [&](TextShapingEngine engine, const char* family, SkScalar fontSize) {
        auto paragraph =
                layout_text(reporter, fontCollection, engine, text, family, fontSize, 0, 2000);
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        return paragraph->getLongestLine();
    };

    for (TextShapingEngine engine : kEngines) {
        // Negative control: Helvetica has no opsz/trak table, so normalized width is size-invariant
        SkScalar helv11 = layoutWidth(engine, "Helvetica", 11.0f) / 11.0f;
        SkScalar helv48 = layoutWidth(engine, "Helvetica", 48.0f) / 48.0f;
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(helv11, helv48, 0.01f),
                        "engine %d: Helvetica normalized width drifted (%f vs %f)",
                        static_cast<int>(engine),
                        helv11,
                        helv48);
    }

    const TextShapingEngine ct = TextShapingEngine::kCoreText;
    SkScalar sf11 = layoutWidth(ct, ".AppleSystemUIFont", 11.0f) / 11.0f;
    SkScalar sf14 = layoutWidth(ct, ".AppleSystemUIFont", 14.0f);
    SkScalar sf17 = layoutWidth(ct, ".AppleSystemUIFont", 17.0f) / 17.0f;
    SkScalar sf28 = layoutWidth(ct, ".AppleSystemUIFont", 28.0f);
    SkScalar sf48 = layoutWidth(ct, ".AppleSystemUIFont", 48.0f) / 48.0f;
    SkScalar ratio28to14 = sf28 / sf14;

    // SF Pro opsz + trak tighten 17pt (#22572), 28pt vs 14pt (#18915), and 48pt vs
    // 11pt (#150824)
    REPORTER_ASSERT(reporter,
                    sf11 - sf17 > 0.3f,
                    "CoreText 17pt HIG body tracking should tighten vs 11pt (%f vs %f)",
                    sf11,
                    sf17);
    REPORTER_ASSERT(reporter,
                    ratio28to14 < 1.88f,
                    "CoreText 28pt/14pt display ratio should contract (< 1.88x; got %f)",
                    ratio28to14);
    REPORTER_ASSERT(reporter,
                    sf11 - sf48 > 1.5f,
                    "CoreText .AppleSystemUIFont should tighten at 48pt (11pt=%f, 48pt=%f)",
                    sf11,
                    sf48);

    // CupertinoTextThemeData's explicit HIG letterSpacing (-0.41 at 17pt, +0.36 at 28pt) replaces
    // .SFNS's built-in trak table via NSCTFontUnscaledTrackingAttribute=0 rather than stacking
    auto p17Cupertino = layout_text(
            reporter, fontCollection, ct, text, "CupertinoSystemText", 17.0f, -0.41f, 2000.0f);
    auto p28Cupertino = layout_text(
            reporter, fontCollection, ct, text, "CupertinoSystemDisplay", 28.0f, 0.36f, 2000.0f);
    SkScalar sf17Raw = sf17 * 17.0f;
    REPORTER_ASSERT(reporter,
                    SkScalarAbs(p17Cupertino->getLongestLine() - sf17Raw) < 2.0f,
                    "17pt explicit -0.41 letterSpacing double-tracked (%f vs auto-trak %f)",
                    p17Cupertino->getLongestLine(),
                    sf17Raw);
    REPORTER_ASSERT(reporter,
                    SkScalarAbs(p28Cupertino->getLongestLine() - sf28) < 2.0f,
                    "28pt explicit +0.36 letterSpacing double-tracked (%f vs auto-trak %f)",
                    p28Cupertino->getLongestLine(),
                    sf28);
}

// Flutter 36062: on keycap emoji sequences ("1️⃣2️⃣3️⃣"), CoreText collapses each keycap
// into 1 glyph (3 total, +48px under letterSpacing=16); HarfBuzz was measured to leave a phantom
// zero-advance glyph per U+FE0F selector, which Run::addLetterSpacesEvenly spaces as well
DEF_TEST(SkParagraph_Flutter_36062_KeycapEmojiCluster, reporter) {
    if (!has_font_families({".AppleSystemUIFont", "Apple Color Emoji"})) {
        return;
    }
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    const char* text =
            "1\xEF\xB8\x8F\xE2\x83\xA3"
            "2\xEF\xB8\x8F\xE2\x83\xA3"
            "3\xEF\xB8\x8F\xE2\x83\xA3";

    for (TextShapingEngine engine : kEngines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        auto p0 = layout_text(
                reporter, fontCollection, engine, text, ".AppleSystemUIFont", 28.0f, 0.0f, 500.0f);
        auto p16 = layout_text(
                reporter, fontCollection, engine, text, ".AppleSystemUIFont", 28.0f, 16.0f, 500.0f);

        auto impl0 = static_cast<ParagraphImpl*>(p0.get());
        auto impl16 = static_cast<ParagraphImpl*>(p16.get());
        REPORTER_ASSERT(reporter, p0->unresolvedGlyphs() == 0 && p16->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl16->runs().size() == 1);
        if (impl0->runs().size() != 1 || impl16->runs().size() != 1) {
            continue;
        }

        const Run& r0 = impl0->runs()[0];
        const Run& r16 = impl16->runs()[0];
        SkString emojiFamily;
        r0.font().getTypeface()->getFamilyName(&emojiFamily);
        REPORTER_ASSERT(reporter,
                        emojiFamily.contains("Emoji"),
                        "engine %d: expected Emoji family, got %s",
                        static_cast<int>(engine),
                        emojiFamily.c_str());

        const auto& clusters = impl0->clusters();
        REPORTER_ASSERT(reporter, clusters.size() == 4);
        if (clusters.size() == 4) {
            REPORTER_ASSERT(reporter, clusters[0].textRange() == TextRange(0, 7));
            REPORTER_ASSERT(reporter, clusters[1].textRange() == TextRange(7, 14));
            REPORTER_ASSERT(reporter, clusters[2].textRange() == TextRange(14, 21));
        }

        if (coreText) {
            SkScalar widthDelta = p16->getLongestLine() - p0->getLongestLine();
            REPORTER_ASSERT(reporter, r0.size() == 3 && r16.size() == 3);
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(widthDelta, 48.0f, 0.1f),
                            "CoreText expected 3 keycaps * 16px = 48px delta; got %f", widthDelta);
        }
    }
}

// No tracked issue: on ZWJ emoji sequences containing U+FE0F variation selectors
// ("🏳️‍🌈❤️‍🔥👩‍💻"), CoreText emits 3 fused glyphs (+48px under letterSpacing=16); HarfBuzz was
// measured to emit 5 glyphs (phantom U+FE0F glyphs in 🏳️‍🌈 and ❤️‍🔥) that all receive spacing
DEF_TEST(SkParagraph_ZWJEmojiVariationSelectorLetterSpacing, reporter) {
    if (!has_font_families({".AppleSystemUIFont", "Apple Color Emoji"})) {
        return;
    }
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    const char* zwjText =
            "\xF0\x9F\x8F\xB3\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x8C\x88"
            "\xE2\x9D\xA4\xEF\xB8\x8F\xE2\x80\x8D\xF0\x9F\x94\xA5"
            "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x92\xBB";

    for (TextShapingEngine engine : kEngines) {
        auto p0 = layout_text(reporter,
                              fontCollection,
                              engine,
                              zwjText,
                              ".AppleSystemUIFont",
                              28.0f,
                              0.0f,
                              500.0f);
        auto p16 = layout_text(reporter,
                               fontCollection,
                               engine,
                               zwjText,
                               ".AppleSystemUIFont",
                               28.0f,
                               16.0f,
                               500.0f);
        REPORTER_ASSERT(reporter, p0->unresolvedGlyphs() == 0 && p16->unresolvedGlyphs() == 0);

        auto impl0 = static_cast<ParagraphImpl*>(p0.get());
        auto impl16 = static_cast<ParagraphImpl*>(p16.get());
        REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl16->runs().size() == 1);
        if (impl0->runs().size() != 1 || impl16->runs().size() != 1 ||
            engine != TextShapingEngine::kCoreText) {
            continue;
        }
        SkScalar widthDelta = p16->getLongestLine() - p0->getLongestLine();
        REPORTER_ASSERT(reporter, impl0->runs()[0].size() == 3 && impl16->runs()[0].size() == 3);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(widthDelta, 48.0f, 0.1f),
                        "CoreText expected 3 ZWJ emoji * 16px = 48px delta; got %f",
                        widthDelta);
    }
}

// Flutter 118127: on decomposed "ax\u0301b" (which has no precomposed NFC form), CoreText's
// tracking keeps the combining acute anchored to 'x' under letterSpacing=10 and expands width by
// 3 clusters (30px); HarfBuzz was measured to shift the combining mark away from its base too
DEF_TEST(SkParagraph_Flutter_118127_CombiningDiacriticsLetterSpacing, reporter) {
    if (!has_font_families({"Helvetica"})) {
        return;
    }
    auto fontCollection = make_test_font_collection();
    const char* text = "ax\xCC\x81" "b";

    auto visualX = [](const Run& run, size_t idx) -> SkScalar {
        return run.positions()[idx].fX + run.offsets()[idx].fX;
    };

    for (TextShapingEngine engine : kEngines) {
        auto p0 = layout_text(
                reporter, fontCollection, engine, text, "Helvetica", 20.0f, 0.0f, 500.0f);
        auto p10 = layout_text(
                reporter, fontCollection, engine, text, "Helvetica", 20.0f, 10.0f, 500.0f);

        auto impl0 = static_cast<ParagraphImpl*>(p0.get());
        auto impl10 = static_cast<ParagraphImpl*>(p10.get());
        REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl10->runs().size() == 1);
        if (impl0->runs().size() != 1 || impl10->runs().size() != 1) {
            continue;
        }

        const Run& r0 = impl0->runs()[0];
        const Run& r10 = impl10->runs()[0];
        REPORTER_ASSERT(reporter, r0.size() == 4 && r10.size() == 4);
        if (r0.size() != 4 || r10.size() != 4 || engine != TextShapingEngine::kCoreText) {
            continue;
        }

        SkScalar markDelta0 = visualX(r0, 2) - visualX(r0, 1);
        SkScalar markDelta10 = visualX(r10, 2) - visualX(r10, 1);
        SkScalar widthDelta = p10->getLongestLine() - p0->getLongestLine();
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(markDelta10, markDelta0, 0.05f),
                        "CoreText mark detached under letterSpacing: ls0=%f, ls10=%f",
                        markDelta0,
                        markDelta10);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(widthDelta, 30.0f, 0.05f),
                        "CoreText widthDelta should be 3 clusters * 10px = 30px; got %f",
                        widthDelta);
    }
}

// Flutter 144528: on Devanagari "हिन्दी" (2 orthographic syllables, 4 glyphs: pre-base matra ि,
// base ह, conjunct न्द्, post-base matra ी), CoreText's tracking spaces the 2 syllables as intact
// units (+32px at letterSpacing=16); HarfBuzz was measured to space every glyph, detaching both
// matras
DEF_TEST(SkParagraph_Flutter_144528_IndicConjunctLetterSpacing, reporter) {
    if (!has_font_families({"Kohinoor Devanagari"})) {
        return;
    }
    auto fontCollection = make_test_font_collection();
    const char* hindi = "\xE0\xA4\xB9\xE0\xA4\xBF\xE0\xA4\xA8\xE0\xA5\x8D\xE0\xA4\xA6\xE0\xA5\x80";

    for (TextShapingEngine engine : kEngines) {
        auto p0 = layout_text(reporter,
                              fontCollection,
                              engine,
                              hindi,
                              "Kohinoor Devanagari",
                              24.0f,
                              0.0f,
                              500.0f);
        auto p16 = layout_text(reporter,
                               fontCollection,
                               engine,
                               hindi,
                               "Kohinoor Devanagari",
                               24.0f,
                               16.0f,
                               500.0f);

        auto impl0 = static_cast<ParagraphImpl*>(p0.get());
        auto impl16 = static_cast<ParagraphImpl*>(p16.get());
        REPORTER_ASSERT(reporter, p0->unresolvedGlyphs() == 0 && p16->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, impl0->runs().size() == 1 && impl16->runs().size() == 1);
        if (impl0->runs().size() != 1 || impl16->runs().size() != 1) {
            continue;
        }

        const Run& r0 = impl0->runs()[0];
        const Run& r16 = impl16->runs()[0];
        REPORTER_ASSERT(reporter, r0.size() == 4 && r16.size() == 4);
        if (r0.size() != 4 || r16.size() != 4 || engine != TextShapingEngine::kCoreText) {
            continue;
        }

        SkScalar syl1Span0 = r0.positions()[1].fX - r0.positions()[0].fX;
        SkScalar syl1Span16 = r16.positions()[1].fX - r16.positions()[0].fX;
        SkScalar widthDelta = p16->getLongestLine() - p0->getLongestLine();
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(syl1Span16, syl1Span0, 0.05f),
                        "CoreText matra detached inside syllable 1: ls0=%f, ls16=%f",
                        syl1Span0,
                        syl1Span16);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(widthDelta, 32.0f, 0.1f),
                        "CoreText expected 2 syllables * 16px = 32px delta; got %f",
                        widthDelta);
    }
}

// Flutter 147708: when CupertinoSystemText or CupertinoSystemDisplay is paired with a non-empty
// fontFamilyFallback list, CoreText resolves both aliases to .SFNS before consulting the fallback
// list and preserves the 28pt/14pt display optical-size contraction; HarfBuzz misses both aliases
// in matchFamilyStyle and falls through to the first fallback family (Helvetica)
DEF_TEST(SkParagraph_Flutter_147708_CupertinoAliasesBypassFallback, reporter) {
    if (!has_font_families({".AppleSystemUIFont", "Helvetica"})) {
        return;
    }
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    const char* text = "DynamicTrackingTest Hello World";

    auto layoutWithFallback = [&](TextShapingEngine engine, const char* alias, SkScalar fontSize) {
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, make_unicode(engine));
        TextStyle ts;
        ts.setFontFamilies({SkString(alias), SkString("Helvetica")});
        ts.setFontSize(fontSize);
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(2000.0f);
        REPORTER_ASSERT(reporter,
                        static_cast<ParagraphImpl*>(paragraph.get())->isCoreTextShaped() ==
                                (engine == TextShapingEngine::kCoreText));
        return paragraph;
    };

    for (const char* alias : {"CupertinoSystemText", "CupertinoSystemDisplay"}) {
        for (TextShapingEngine engine : kEngines) {
            auto p14 = layoutWithFallback(engine, alias, 14.0f);
            auto p28 = layoutWithFallback(engine, alias, 28.0f);
            auto impl28 = static_cast<ParagraphImpl*>(p28.get());
            REPORTER_ASSERT(reporter, impl28->runs().size() == 1);
            if (impl28->runs().size() != 1 || engine != TextShapingEngine::kCoreText) {
                continue;
            }
            SkString psName;
            impl28->runs()[0].font().getTypeface()->getPostScriptName(&psName);
            REPORTER_ASSERT(reporter,
                            psName.startsWith(".AppleSystemUIFont") || psName.startsWith(".SF"),
                            "CoreText %s fell through to %s",
                            alias,
                            psName.c_str());
            SkScalar ratio = p28->getLongestLine() / p14->getLongestLine();
            REPORTER_ASSERT(reporter,
                            ratio < 1.88f,
                            "CoreText %s 28pt/14pt ratio should contract (< 1.88x; got %f)",
                            alias,
                            ratio);
        }
    }
}

// Flutter 193812: .AppleSystemUIFontRounded, .AppleSystemUIFontSerif, and
// .AppleSystemUIFontMonospaced resolve to Apple's Rounded, NewYork, and SF Mono UI designs on
// CoreText (with fixed-pitch advances on Monospaced); HarfBuzz does not recognize the design
// suffixes and falls back to proportional Helvetica
DEF_TEST(SkParagraph_Flutter_193812_SystemDesigns, reporter) {
    if (!has_font_families({".AppleSystemUIFont", "Helvetica"})) {
        return;
    }
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    const char* text = "System UI Font Design Variants";

    const struct {
        const char* fFamily;
        const char* fExpectedPs;
    } designs[] = {
            {".AppleSystemUIFontRounded", "Rounded"},
            {".AppleSystemUIFontSerif", "NewYork"},
            {".AppleSystemUIFontMonospaced", "Monospaced"},
    };

    for (TextShapingEngine engine : kEngines) {
        for (const auto& d : designs) {
            auto p = layout_text(
                    reporter, fontCollection, engine, text, d.fFamily, 24.0f, 0.0f, 1000.0f);
            auto impl = static_cast<ParagraphImpl*>(p.get());
            REPORTER_ASSERT(reporter, impl->runs().size() == 1);
            if (impl->runs().size() != 1 || engine != TextShapingEngine::kCoreText) {
                continue;
            }
            SkString psName;
            impl->runs()[0].font().getTypeface()->getPostScriptName(&psName);
            REPORTER_ASSERT(reporter,
                            psName.contains(d.fExpectedPs),
                            "CoreText %s expected %s in PostScript name, got %s",
                            d.fFamily,
                            d.fExpectedPs,
                            psName.c_str());
        }

        auto pNarrow = layout_text(reporter,
                                   fontCollection,
                                   engine,
                                   "iiiiiiii",
                                   ".AppleSystemUIFontMonospaced",
                                   24.0f,
                                   0.0f,
                                   1000.0f);
        auto pWide = layout_text(reporter,
                                 fontCollection,
                                 engine,
                                 "WWWWWWWW",
                                 ".AppleSystemUIFontMonospaced",
                                 24.0f,
                                 0.0f,
                                 1000.0f);
        if (engine == TextShapingEngine::kCoreText) {
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(
                                    pNarrow->getLongestLine(), pWide->getLongestLine(), 0.05f),
                            "CoreText .AppleSystemUIFontMonospaced should have equal width for "
                            "'iiiiiiii' (%f) and 'WWWWWWWW' (%f)",
                            pNarrow->getLongestLine(),
                            pWide->getLongestLine());
        }
    }
}

// Flutter 109308 (partial, automatic optical size only): with no explicit opsz, CoreText sets the
// .SFNS opsz coordinate from the point size (17 at 12pt, 72 at 72pt) so glyph advances per em
// shrink at display sizes; HarfBuzz was measured to keep opsz 17 and identical advances per em at
// both sizes. Explicit axis values and the remaining variable font requests are out of scope
DEF_TEST(SkParagraph_Flutter_109308_AutoOpticalSize, reporter) {
    if (!has_font_families({".AppleSystemUIFont"})) {
        return;
    }
    auto fontCollection = make_test_font_collection();
    const char* text = "Variable fonts honor axes";
    const SkFourByteTag opszTag = SkSetFourByteTag('o', 'p', 's', 'z');
    const SkScalar sizes[2] = {12.0f, 72.0f};

    // Design coordinate of opsz on the face that renders the run; -1 when the face has no opsz
    auto opszOf = [&](const SkTypeface* typeface) -> SkScalar {
        int count = typeface->getVariationDesignPosition({});
        std::vector<SkFontArguments::VariationPosition::Coordinate> coords(std::max(count, 0));
        typeface->getVariationDesignPosition(coords);
        for (const auto& c : coords) {
            if (c.axis == opszTag) {
                return c.value;
            }
        }
        return -1;
    };

    for (TextShapingEngine engine : kEngines) {
        SkScalar advancePerEm[2] = {0, 0};
        SkScalar opsz[2] = {0, 0};
        bool singleRun = true;
        for (int i = 0; i < 2; ++i) {
            auto p = layout_text(reporter,
                                 fontCollection,
                                 engine,
                                 text,
                                 ".AppleSystemUIFont",
                                 sizes[i],
                                 0.0f,
                                 2000.0f);
            auto impl = static_cast<ParagraphImpl*>(p.get());
            REPORTER_ASSERT(reporter, p->unresolvedGlyphs() == 0 && impl->runs().size() == 1);
            if (impl->runs().size() != 1) {
                singleRun = false;
                break;
            }
            const Run& run = impl->runs()[0];
            // The font's own advances, so CoreText 'trak' does not count as optical sizing
            std::vector<SkScalar> widths(run.size());
            run.font().getWidths(run.glyphs(), widths);
            SkScalar sum = 0;
            for (SkScalar w : widths) {
                sum += w;
            }
            advancePerEm[i] = sum / sizes[i];
            opsz[i] = opszOf(run.font().getTypeface());
        }
        if (!singleRun || engine != TextShapingEngine::kCoreText) {
            continue;
        }
        REPORTER_ASSERT(reporter,
                        opsz[1] > opsz[0],
                        "CoreText opsz should follow the font size (12pt=%f, 72pt=%f)",
                        opsz[0],
                        opsz[1]);
        REPORTER_ASSERT(reporter,
                        advancePerEm[0] > advancePerEm[1] * 1.01f,
                        "CoreText advances per em should shrink at 72pt (12pt=%f, 72pt=%f)",
                        advancePerEm[0],
                        advancePerEm[1]);
    }
}

// Flutter 144099: Arabic text styled with CupertinoSystemText renders its Arabic runs in SF Arabic
// on CoreText, as native iOS text does; HarfBuzz was measured to miss the alias and fall back to
// Geeza Pro for Arabic and Helvetica for the space and colon
DEF_TEST(SkParagraph_Flutter_144099_CupertinoArabicSystemFont, reporter) {
    if (!has_font_families({".AppleSystemUIFont"})) {
        return;
    }
    auto fontCollection = make_test_font_collection();
    fontCollection->enableFontFallback();
    const char* text =
            "\xD9\x85\xD8\xB1\xD8\xAD\xD8\xA8\xD8\xA7 \xD9\xA1\xD9\xA1:\xD9\xA4\xD9\xA0";

    for (TextShapingEngine engine : kEngines) {
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, make_unicode(engine));
        TextStyle ts;
        ts.setFontFamilies({SkString("CupertinoSystemText")});
        ts.setFontSize(20.0f);
        ts.setLocale(SkString("ar"));
        builder.pushStyle(ts);
        builder.addText(text);
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter,
                        impl->isCoreTextShaped() == (engine == TextShapingEngine::kCoreText));
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0 && !impl->runs().empty());
        if (engine != TextShapingEngine::kCoreText) {
            continue;
        }
        for (const Run& run : impl->runs()) {
            // Space and ':' are common script and may stay in the Latin face; every Arabic letter
            // and Arabic-Indic digit here is a 0xD8/0xD9-led UTF-8 sequence
            const unsigned char lead = text[run.textRange().start];
            if (lead != 0xD8 && lead != 0xD9) {
                continue;
            }
            SkString family;
            run.font().getTypeface()->getFamilyName(&family);
            REPORTER_ASSERT(reporter,
                            family.contains("SF Arabic"),
                            "CoreText Arabic run [%zu, %zu) resolved to %s",
                            run.textRange().start,
                            run.textRange().end,
                            family.c_str());
        }
    }
}

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)
