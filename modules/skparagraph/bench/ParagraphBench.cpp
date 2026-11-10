// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "bench/Benchmark.h"

#if !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)

#include "include/core/SkColor.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "modules/skunicode/include/SkUnicode_icu.h"
#include "tools/Resources.h"

#include <memory>
#include <string>

using namespace skia::textlayout;

namespace {

constexpr SkScalar kDefaultWidth = 300;

// Builds a Roboto TextStyle backed by resources/fonts/Roboto-Regular.ttf with opaque ink
TextStyle make_text_style(SkScalar fontSize = 16) {
    TextStyle style;
    style.setFontFamilies({SkString("Roboto")});
    style.setFontSize(fontSize);
    style.setColor(SK_ColorBLACK);
    return style;
}

// Builds a ParagraphStyle with hinting off so layout metrics do not depend on hinting settings
ParagraphStyle make_paragraph_style(const TextStyle& textStyle) {
    ParagraphStyle style;
    style.turnHintingOff();
    style.setTextStyle(textStyle);
    return style;
}

// Shared hermetic environment for SkParagraph CPU benchmarks using repository fonts and ICU
struct ParagraphBenchBase : public Benchmark {
    // Skips rendering configs and runs where icudtl.dat or font resources were unavailable in setup
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering && fParagraph != nullptr;
    }

    // Initializes ICU and Roboto-Regular.ttf from resources/fonts with explicit ParagraphCache
    // state; returns false when icudtl.dat or font resources are absent because nanobench runs
    // delayedSetup before isSuitableFor
    bool initEnv(bool enableCache) {
        fUnicode = SkUnicodes::ICU::Make();
        if (!fUnicode || !GetResourceAsData("fonts/Roboto-Regular.ttf")) {
            return false;
        }
        fFonts = sk_make_sp<TestFontCollection>(
                GetResourcePath("fonts").c_str(), /*testOnly=*/false, /*loadFonts=*/false);
        if (!fFonts->addFontFromFile("Roboto-Regular.ttf")) {
            return false;
        }
        SkASSERT_RELEASE(!fFonts->findTypefaces({SkString("Roboto")}, SkFontStyle()).empty());
        fFonts->getParagraphCache()->turnOn(enableCache);
        return true;
    }

    sk_sp<TestFontCollection> fFonts;
    sk_sp<SkUnicode> fUnicode;
    std::unique_ptr<Paragraph> fParagraph;
};

// Pins ParagraphCache hit restore cost when non-space codepoints are unresolved (f2f4ac0e78
// +33.2%, ee27aa24cf -42.0%, f49060a27f -45.5%)
struct ParagraphCacheHitUnresolvedBench final : public ParagraphBenchBase {
    const char* onGetName() override { return "paragraph_cache_hit_unresolved"; }

    // Populates ParagraphCache once with 205 Roboto spaces and 819 unresolved PUA code units
    void onDelayedSetup() override {
        if (!this->initEnv(/*enableCache=*/true)) {
            return;
        }
        std::u16string text;
        text.reserve(1024);
        for (char16_t i = 0; i < 1024; ++i) {
            text.push_back(i % 5 == 0 ? u' ' : static_cast<char16_t>(0xE000 + i));
        }
        TextStyle textStyle = make_text_style();
        auto builder = ParagraphBuilder::make(make_paragraph_style(textStyle), fFonts, fUnicode);
        builder->pushStyle(textStyle);
        builder->addText(text);
        builder->pop();
        fParagraph = builder->Build();
        fParagraph->layout(kDefaultWidth);
        SkASSERT_RELEASE(fParagraph->lineNumber() > 1);
        SkASSERT_RELEASE(fParagraph->unresolvedGlyphs() == 819);
    }

    // Measures warm relayout from the cached shaped paragraph without reshaping
    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            fParagraph->markDirty();
            fParagraph->layout(kDefaultWidth);
        }
    }
};

}  // namespace

DEF_BENCH(return new ParagraphCacheHitUnresolvedBench;)

#endif  // !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)
