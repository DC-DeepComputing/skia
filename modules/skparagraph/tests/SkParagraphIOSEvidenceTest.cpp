/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
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

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

using namespace skia::textlayout;

// The iOS bots run dm with --nocpu --nonativeFonts: a Ganesh test is the only kind they execute,
// and the system font manager is created directly because ToolUtils::TestFontMgr() is portable
#if defined(SK_GANESH) && defined(SK_PARAGRAPH_NATIVE_CORETEXT) && \
        !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ) && defined(SK_UNICODE_ICU_IMPLEMENTATION)

namespace {

constexpr TextShapingEngine kEngines[] = {TextShapingEngine::kDefault,
                                          TextShapingEngine::kCoreText};

const char* engine_name(TextShapingEngine engine) {
    return engine == TextShapingEngine::kCoreText ? "CoreText" : "HarfBuzz";
}

#if defined(SK_BUILD_FOR_IOS)
constexpr const char* kPlatform = "iOS";
#else
constexpr const char* kPlatform = "macOS";
#endif

// Mirrors the shipping pairing: Apple SkUnicode for CoreText where built, ICU otherwise
sk_sp<SkUnicode> make_unicode(TextShapingEngine engine) {
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
    if (engine == TextShapingEngine::kCoreText) {
        return SkUnicodes::Apple::Make();
    }
#endif
    return SkUnicodes::ICU::Make();
}

sk_sp<SkFontMgr> system_font_mgr() {
    static sk_sp<SkFontMgr> mgr = SkFontMgr_New_CoreText(nullptr);
    return mgr;
}

bool has_font_families(std::initializer_list<const char*> families) {
    for (const char* family : families) {
        if (!system_font_mgr()->matchFamilyStyle(family, SkFontStyle())) {
            SkDebugf("EVIDENCE-iOS %s: missing family %s, test skipped\n", kPlatform, family);
            return false;
        }
    }
    return true;
}

sk_sp<FontCollection> make_font_collection() {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(system_font_mgr());
    fontCollection->enableFontFallback();
    return fontCollection;
}

std::unique_ptr<ParagraphImpl> layout_spans(
        skiatest::Reporter* reporter,
        const sk_sp<FontCollection>& fontCollection,
        TextShapingEngine engine,
        std::initializer_list<std::pair<const TextStyle*, const char*>> spans) {
    ParagraphStyle style;
    style.setTextShapingEngine(engine);
    ParagraphBuilderImpl builder(style, fontCollection, make_unicode(engine));
    for (const auto& span : spans) {
        builder.pushStyle(*span.first);
        builder.addText(span.second);
        builder.pop();
    }
    std::unique_ptr<ParagraphImpl> paragraph(
            static_cast<ParagraphImpl*>(builder.Build().release()));
    paragraph->layout(4000);
    REPORTER_ASSERT(reporter,
                    paragraph->isCoreTextShaped() == (engine == TextShapingEngine::kCoreText));
    return paragraph;
}

// A null family leaves the family list empty, as a TextStyle without fontFamily does in Flutter
TextStyle make_style(const char* family, SkScalar fontSize) {
    TextStyle ts;
    if (family) {
        ts.setFontFamilies({SkString(family)});
    } else {
        ts.setFontFamilies({});
    }
    ts.setFontSize(fontSize);
    return ts;
}

std::vector<SkGlyphID> all_glyphs(ParagraphImpl* p) {
    std::vector<SkGlyphID> glyphs;
    for (const Run& run : p->runs()) {
        glyphs.insert(glyphs.end(), run.glyphs().begin(), run.glyphs().end());
    }
    return glyphs;
}

SkString describe_runs(ParagraphImpl* p) {
    SkString out;
    for (const Run& run : p->runs()) {
        SkString family;
        run.font().getTypeface()->getFamilyName(&family);
        out.appendf("%s:", family.c_str());
        for (size_t i = 0; i < run.size(); ++i) {
            out.appendf(" %d@%.1f,%.1f", run.glyphs()[i], run.positions()[i].fX,
                        run.positions()[i].fY);
        }
        out.append("; ");
    }
    return out;
}

}  // namespace

// Flutter 79363: TextStyle.locale ja-JP must give 直骨角 Japanese Han glyph forms on iOS, i.e. a
// Japanese face (Hiragino) rather than the zh-Hans face, whatever the requested family is
DEF_GANESH_TEST(SkParagraph_EvidenceIOS_79363_JapaneseLocaleHanForms, reporter, options,
                CtsEnforcement::kNever) {
    if (!has_font_families({"Hiragino Sans", "PingFang SC"})) {
        return;
    }
    auto fontCollection = make_font_collection();
    const char* text = "\xE7\x9B\xB4\xE9\xAA\xA8\xE8\xA7\x92";

    // Flutter iOS text reaches SkParagraph as the system UI font, the Cupertino alias, no family,
    // or an app font without CJK coverage that falls back by locale
    const char* families[] = {".AppleSystemUIFont", "CupertinoSystemText", nullptr,
                              "Helvetica"};
    for (const char* family : families) {
        const char* familyLabel = family ? family : "(empty)";
        for (TextShapingEngine engine : kEngines) {
            TextStyle ja = make_style(family, 24);
            ja.setLocale(SkString("ja-JP"));
            TextStyle zh = make_style(family, 24);
            zh.setLocale(SkString("zh-Hans"));
            auto pJa = layout_spans(reporter, fontCollection, engine, {{&ja, text}});
            auto pZh = layout_spans(reporter, fontCollection, engine, {{&zh, text}});
            SkString jaRuns = describe_runs(pJa.get());
            SkString zhRuns = describe_runs(pZh.get());
            SkDebugf("EVIDENCE-iOS %s 79363 %s family=%s: ja-JP -> %s| zh-Hans -> %s\n",
                     kPlatform, engine_name(engine), familyLabel, jaRuns.c_str(),
                     zhRuns.c_str());
            REPORTER_ASSERT(reporter, pJa->unresolvedGlyphs() == 0);
            REPORTER_ASSERT(reporter, jaRuns.contains("Hiragino"),
                            "%s %s family=%s: ja-JP resolved to %s instead of a Japanese face",
                            kPlatform, engine_name(engine), familyLabel, jaRuns.c_str());
            REPORTER_ASSERT(reporter, all_glyphs(pJa.get()) != all_glyphs(pZh.get()),
                            "%s %s family=%s: ja-JP and zh-Hans share glyphs (%s)", kPlatform,
                            engine_name(engine), familyLabel, jaRuns.c_str());
        }
    }
}

// Flutter 150744: FontFeature('sups') on the "orld" span of "World" must substitute superscript
// glyphs for exactly that span and leave 'W' untouched
DEF_GANESH_TEST(SkParagraph_EvidenceIOS_150744_SupsFeatureSpan, reporter, options,
                CtsEnforcement::kNever) {
    if (!has_font_families({".AppleSystemUIFont"})) {
        return;
    }
    auto fontCollection = make_font_collection();

    for (TextShapingEngine engine : kEngines) {
        TextStyle plain = make_style(".AppleSystemUIFont", 40);
        TextStyle sups = plain;
        sups.addFontFeature(SkString("sups"), 1);

        auto pPlain = layout_spans(reporter, fontCollection, engine, {{&plain, "World"}});
        auto pSpan = layout_spans(reporter, fontCollection, engine,
                                  {{&plain, "W"}, {&sups, "orld"}});
        auto pWhole = layout_spans(reporter, fontCollection, engine, {{&sups, "World"}});
        SkDebugf("EVIDENCE-iOS %s 150744 %s plain: %s width=%.2f\n", kPlatform,
                 engine_name(engine), describe_runs(pPlain.get()).c_str(),
                 pPlain->getLongestLine());
        SkDebugf("EVIDENCE-iOS %s 150744 %s W+sups(orld): %s width=%.2f\n", kPlatform,
                 engine_name(engine), describe_runs(pSpan.get()).c_str(),
                 pSpan->getLongestLine());
        SkDebugf("EVIDENCE-iOS %s 150744 %s sups(World): %s width=%.2f\n", kPlatform,
                 engine_name(engine), describe_runs(pWhole.get()).c_str(),
                 pWhole->getLongestLine());

        std::vector<SkGlyphID> gPlain = all_glyphs(pPlain.get());
        std::vector<SkGlyphID> gSpan = all_glyphs(pSpan.get());
        REPORTER_ASSERT(reporter, gPlain.size() == 5 && gSpan.size() == 5);
        if (gPlain.size() != 5 || gSpan.size() != 5) {
            continue;
        }
        REPORTER_ASSERT(reporter, gSpan[0] == gPlain[0], "%s %s: 'W' outside the span changed",
                        kPlatform, engine_name(engine));
        for (size_t i = 1; i < 5; ++i) {
            REPORTER_ASSERT(reporter, gSpan[i] != gPlain[i],
                            "%s %s: glyph %zu of the sups span was not substituted (%d)",
                            kPlatform, engine_name(engine), i, gSpan[i]);
        }
    }
}

#endif
