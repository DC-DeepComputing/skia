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

// Skia b/318667611 & b/328418459: when .AppleSystemUIFont encounters unmapped supplementary
// Private Use Area codepoints (U+100000..U+100002) alongside Latin, CJK, and Arabic runs,
// HarfBuzz's matchFamilyStyleCharacter falls back to Apple's LastResort font, whereas CoreText
// suppresses LastResort via isLastResortFont() and preserves primary-font .notdef (glyph 0)
DEF_TEST(SkParagraph_Skia_b_318667611_AppleSystemUIFontCascade, reporter) {
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
    const char* cascadeText =
            "SF Pro  \xE7\xAE\x80\xE4\xBD\x93  \xD9\x85\xD8\xB1\xD8\xAD\xD8\xA8\xD8\xA7  "
            "\xF4\x80\x80\x80\xF4\x80\x80\x81\xF4\x80\x80\x82";

    for (TextShapingEngine engine : engines) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        ParagraphStyle style;
        style.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(style, fontCollection, unicode);
        TextStyle ts;
        ts.setFontFamilies({SkString(".AppleSystemUIFont")});
        ts.setFontSize(24.0f);
        builder.pushStyle(ts);
        builder.addText(cascadeText);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, !impl->runs().empty());

        bool hasSystemFontRun = false;
        bool hasChineseRun = false;
        bool hasArabicRun = false;
        bool hasLastResortRun = false;
        for (const auto& run : impl->runs()) {
            SkString ps;
            if (run.font().getTypeface()) {
                run.font().getTypeface()->getPostScriptName(&ps);
            }
            if (ps.contains("LastResort")) {
                hasLastResortRun = true;
            }
            if (ps.contains("SF") || ps.contains("AppleSystemUIFont")) {
                hasSystemFontRun = true;
            }
            if (ps.contains("PingFang")) {
                hasChineseRun = true;
            }
            if (ps.contains("Arabic") || ps.contains("Geeza")) {
                hasArabicRun = true;
            }
        }
        REPORTER_ASSERT(reporter, hasSystemFontRun);
        REPORTER_ASSERT(reporter, hasChineseRun);
        REPORTER_ASSERT(reporter, hasArabicRun);

        if (coreText) {
            REPORTER_ASSERT(reporter, !hasLastResortRun,
                            "CoreText must suppress LastResort on unmapped PUA codepoints");
            REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 3,
                            "CoreText expected 3 unresolved PUA .notdef glyphs; got %d",
                            paragraph->unresolvedGlyphs());
        } else {
            REPORTER_ASSERT(reporter, hasLastResortRun,
                            "HarfBuzz matchFamilyStyleCharacter expected LastResort fallback");
            REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        }
    }
}
