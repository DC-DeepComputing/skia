// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "bench/Benchmark.h"

#if !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
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

#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <utility>

using namespace skia::textlayout;

namespace {

constexpr SkScalar kDefaultWidth = 300;
constexpr size_t kUnlimitedLines = std::numeric_limits<size_t>::max();

// Returns true when c is an ASCII Latin letter
bool is_ascii_alpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

// Inserts UTF-8 U+00AD (\xC2\xAD) at the midpoint of ASCII words of 6 or more letters so
// TextWrapper breaks inside long words and TextLine::justify exercises Cluster::isSoftHyphen
// without enabling fRenderSoftHyphens (which would add HarfBuzz shaping to warm relayout)
std::string insert_soft_hyphens(const std::string& src) {
    std::string out;
    out.reserve(src.size() + src.size() / 6);
    size_t i = 0;
    while (i < src.size()) {
        if (!is_ascii_alpha(src[i])) {
            out.push_back(src[i]);
            ++i;
            continue;
        }
        size_t start = i;
        while (i < src.size() && is_ascii_alpha(src[i])) {
            ++i;
        }
        size_t len = i - start;
        if (len >= 6) {
            size_t mid = len / 2;
            out.append(src, start, mid);
            out.append("\xC2\xAD", 2);
            out.append(src, start + mid, len - mid);
        } else {
            out.append(src, start, len);
        }
    }
    return out;
}

// Loads resources/text/english.txt by explicit byte length without strlen, optionally inserts
// U+00AD soft hyphens, and repeats the unit until both copies and minBytes are satisfied; returns
// false when the resource file is absent
bool load_english_text(size_t minBytes, int copies, bool insertSoftHyphens, std::string* out) {
    sk_sp<SkData> data = GetResourceAsData("text/english.txt");
    if (!data) {
        return false;
    }
    SkASSERT_RELEASE(data->size() > 0);
    std::string unit(static_cast<const char*>(data->data()), data->size());
    while (!unit.empty() && (unit.back() == '\n' || unit.back() == ' ')) {
        unit.pop_back();
    }
    SkASSERT_RELEASE(!unit.empty());
    if (insertSoftHyphens) {
        unit = insert_soft_hyphens(unit);
    }
    std::string text = unit;
    for (int i = 1; i < copies || text.size() < minBytes; ++i) {
        text.push_back(' ');
        text.append(unit);
    }
    *out = std::move(text);
    return true;
}

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

// Builds a single-style UTF-8 paragraph using explicit byte length so SkData buffers never hit
// strlen
std::unique_ptr<Paragraph> build_single_style_paragraph(const std::string& text,
                                                        const ParagraphStyle& paraStyle,
                                                        const TextStyle& textStyle,
                                                        const sk_sp<FontCollection>& fonts,
                                                        const sk_sp<SkUnicode>& unicode) {
    auto builder = ParagraphBuilder::make(paraStyle, fonts, unicode);
    builder->pushStyle(textStyle);
    builder->addText(text.data(), text.size());
    builder->pop();
    return builder->Build();
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

// Base class for warm relayout benchmarks that measure markDirty + layout(fWidth) on a cached
// shaped paragraph
struct WarmRelayoutBase : public ParagraphBenchBase {
    explicit WarmRelayoutBase(SkScalar width = kDefaultWidth) : fWidth(width) {}

    // Measures warm relayout from the cached shaped paragraph without reshaping
    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            fParagraph->markDirty();
            fParagraph->layout(fWidth);
        }
    }

    SkScalar fWidth;
};

struct WarmRelayoutSpec {
    const char* fName;
    SkScalar fWidth = kDefaultWidth;
    size_t fMinLines = 1;
    size_t fMaxLines = kUnlimitedLines;
    TextAlign fAlign = TextAlign::kLeft;
    size_t fStyleMaxLines = kUnlimitedLines;
    bool fSoftHyphens = false;
    size_t fMinBytes = 0;
    int fCopies = 1;
    const char* fLiteral = nullptr;
};

// Configurable single-style warm relayout benchmark across prose sizes, widths, justify, and
// ellipsis modes
struct WarmRelayoutBench final : public WarmRelayoutBase {
    explicit WarmRelayoutBench(WarmRelayoutSpec spec)
            : WarmRelayoutBase(spec.fWidth), fSpec(spec) {}

    const char* onGetName() override { return fSpec.fName; }

    // Builds and lays out the paragraph once to populate ParagraphCache and assert workload bounds
    void onDelayedSetup() override {
        if (!this->initEnv(/*enableCache=*/true)) {
            return;
        }
        std::string text;
        if (fSpec.fLiteral) {
            text = fSpec.fLiteral;
        } else if (!load_english_text(fSpec.fMinBytes, fSpec.fCopies, fSpec.fSoftHyphens, &text)) {
            return;
        }
        TextStyle textStyle = make_text_style();
        ParagraphStyle paraStyle = make_paragraph_style(textStyle);
        paraStyle.setTextAlign(fSpec.fAlign);
        if (fSpec.fStyleMaxLines != kUnlimitedLines) {
            paraStyle.setMaxLines(fSpec.fStyleMaxLines);
            paraStyle.setEllipsis(std::u16string(u"\u2026"));
        }
        fParagraph = build_single_style_paragraph(text, paraStyle, textStyle, fFonts, fUnicode);
        fParagraph->layout(fWidth);
        SkASSERT_RELEASE(fParagraph->lineNumber() >= fSpec.fMinLines);
        SkASSERT_RELEASE(fParagraph->lineNumber() <= fSpec.fMaxLines);
        SkASSERT_RELEASE(fParagraph->unresolvedGlyphs() == 0);
        if (fSpec.fStyleMaxLines != kUnlimitedLines) {
            SkASSERT_RELEASE(fParagraph->didExceedMaxLines());
        }
    }

    WarmRelayoutSpec fSpec;
};

// Measures width-sweep relayout across 91 widths without markDirty so shaped runs stay resident on
// ParagraphImpl (RelayoutManyWidths_Latin: e4d7b83742 +8.8% no-LTO, daf82dad48 +14.4% LTO)
struct ParagraphLongResizeBench final : public ParagraphBenchBase {
    const char* onGetName() override { return "paragraph_long_resize"; }

    // Builds and lays out >=2 KB of English prose once so subsequent layouts only re-wrap lines
    void onDelayedSetup() override {
        if (!this->initEnv(/*enableCache=*/true)) {
            return;
        }
        std::string text;
        if (!load_english_text(
                    /*minBytes=*/2000, /*copies=*/1, /*insertSoftHyphens=*/false, &text)) {
            return;
        }
        TextStyle textStyle = make_text_style();
        fParagraph = build_single_style_paragraph(
                text, make_paragraph_style(textStyle), textStyle, fFonts, fUnicode);
        fParagraph->layout(kDefaultWidth);
        SkASSERT_RELEASE(fParagraph->lineNumber() >= 30);
        SkASSERT_RELEASE(fParagraph->unresolvedGlyphs() == 0);
    }

    // Sweeps widths 100..1000 step 10 without markDirty; fOldWidth=1000 at loop end forces
    // re-breaking at w=100 on the next iteration
    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            for (int w = 100; w <= 1000; w += 10) {
                fParagraph->layout(SkIntToScalar(w));
            }
        }
    }
};

// Measures warm relayout across 1000 distinct style blocks to exercise ParagraphImpl::findAllBlocks
// per line (StylesBigO / ManyStylesLayout: 8a256a9888 -13.4%, e0d79efbf1 +5.4%)
struct ParagraphManyStylesRelayoutBench final : public WarmRelayoutBase {
    const char* onGetName() override { return "paragraph_many_styles_relayout"; }

    // Alternates two colors across 1000 balanced pushStyle/pop blocks so adjacent blocks stay
    // distinct (a.fStyle != b.fStyle) while equalsByFonts stays true (1 shaped run)
    void onDelayedSetup() override {
        if (!this->initEnv(/*enableCache=*/true)) {
            return;
        }
        constexpr char kChunk[] = "vry shrt ";
        TextStyle styleA = make_text_style();
        TextStyle styleB = styleA;
        styleB.setColor(SK_ColorDKGRAY);
        auto builder = ParagraphBuilder::make(make_paragraph_style(styleA), fFonts, fUnicode);
        for (int i = 0; i < 1000; ++i) {
            builder->pushStyle(i % 2 == 0 ? styleA : styleB);
            builder->addText(kChunk, sizeof(kChunk) - 1);
            builder->pop();
        }
        fParagraph = builder->Build();
        fParagraph->layout(fWidth);
        SkASSERT_RELEASE(fParagraph->lineNumber() >= 100);
        SkASSERT_RELEASE(fParagraph->unresolvedGlyphs() == 0);
    }
};

// Pins ParagraphCache hit restore cost when non-space codepoints are unresolved (f2f4ac0e78
// +33.2%, ee27aa24cf -42.0%, f49060a27f -45.5%)
struct ParagraphCacheHitUnresolvedBench final : public WarmRelayoutBase {
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
        fParagraph->layout(fWidth);
        SkASSERT_RELEASE(fParagraph->lineNumber() > 1);
        SkASSERT_RELEASE(fParagraph->unresolvedGlyphs() == 819);
    }
};

}  // namespace

// Pins single-line warm relayout overhead (ShortLayout: e0d79efbf1 +5.3%, 8a256a9888 -4.9%)
DEF_BENCH(return new WarmRelayoutBench({
        .fName = "paragraph_short_relayout",
        .fMaxLines = 1,
        .fLiteral = "Hello World",
});)

// Pins multi-line TextWrapper::lookAhead relayout at width 300 (LongLayout: daf82dad48 +16.0%,
// f49060a27f -8.0%, 9c88ac5062 -6.3%)
DEF_BENCH(return new WarmRelayoutBench({
        .fName = "paragraph_long_relayout",
        .fMinLines = 20,
        .fCopies = 2,
});)

// Wider-line variant of LongLayout with longer cluster scans per line at width 1000 (daf82dad48)
DEF_BENCH(return new WarmRelayoutBench({
        .fName = "paragraph_long_relayout_w1000",
        .fWidth = 1000,
        .fMinLines = 5,
        .fCopies = 2,
});)

DEF_BENCH(return new ParagraphLongResizeBench;)

// Justified relayout with intra-word U+00AD soft hyphens (JustifyLayout: ce0b85b46d +19.3% no-LTO
// / +7.6% LTO, 3e2ee57355 +2.1%)
DEF_BENCH(return new WarmRelayoutBench({
        .fName = "paragraph_justify_relayout",
        .fMinLines = 20,
        .fAlign = TextAlign::kJustify,
        .fSoftHyphens = true,
        .fCopies = 2,
});)

// Three-line ellipsis truncation and reshaping relayout (EllipsisMaxLines3_Relayout: e0d79efbf1
// +15.7% LTO, d13c86b4b3 +11.0% no-LTO, e4d7b83742 +8.0% LTO)
DEF_BENCH(return new WarmRelayoutBench({
        .fName = "paragraph_ellipsis_relayout",
        .fMinLines = 3,
        .fMaxLines = 3,
        .fStyleMaxLines = 3,
        .fMinBytes = 4000,
});)

DEF_BENCH(return new ParagraphManyStylesRelayoutBench;)

// 16 KB prose relayout scaling check (TextBigO/16384: c2e5871a67 -7.7%, 3e2ee57355 -12.4%,
// daf82dad48 +8.0%)
DEF_BENCH(return new WarmRelayoutBench({
        .fName = "paragraph_large_relayout",
        .fMinLines = 200,
        .fMinBytes = 16384,
});)

DEF_BENCH(return new ParagraphCacheHitUnresolvedBench;)

#endif  // !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_BUILD_FOR_GOOGLE3)
