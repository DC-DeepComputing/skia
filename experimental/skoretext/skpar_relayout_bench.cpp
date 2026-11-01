/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Measures, on a SkoreText stack build, the cost of the first
// layout (text analysis + shaping + line breaking) and of a re-layout at a new width (line
// breaking only, shaping reused) for kCoreText and kDefault. ParagraphCache is off so every
// first layout really shapes
#include "include/core/SkFontMgr.h"
#include "include/core/SkString.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skunicode/include/SkUnicode_icu.h"
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_apple.h"
#endif

#include <mach/mach_time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

using namespace skia::textlayout;

static double now_ms() {
    static mach_timebase_info_data_t tb;
    if (tb.denom == 0) mach_timebase_info(&tb);
    return (double)mach_absolute_time() * tb.numer / tb.denom / 1e6;
}

static std::unique_ptr<Paragraph> build(const SkString& text, TextShapingEngine engine,
                                        sk_sp<FontCollection> fc, sk_sp<SkUnicode> unicode,
                                        const char* family, float letterSpacing) {
    ParagraphStyle ps;
    ps.setTextShapingEngine(engine);
    TextStyle ts;
    ts.setFontFamilies({SkString(family)});
    ts.setFontSize(15);
    ts.setLetterSpacing(letterSpacing);
    ts.setColor(SK_ColorBLACK);
    ps.setTextStyle(ts);
    auto b = ParagraphBuilder::make(ps, fc, unicode);
    b->pushStyle(ts);
    b->addText(text.c_str(), text.size());
    return b->Build();
}

int main(int argc, char** argv) {
    const char* family = argc > 1 ? argv[1] : "Helvetica";
    const float letterSpacing = argc > 2 ? (float)atof(argv[2]) : 0.f;
    const char* words = "The quick brown fox jumps over the lazy dog. Съешь же ещё этих мягких "
                        "французских булок. 敏捷的棕色狐狸 ";
    SkString text;
    for (int i = 0; i < 40; ++i) text.append(words);

    auto fc = sk_make_sp<FontCollection>();
    fc->setDefaultFontManager(SkFontMgr_New_CoreText(nullptr));
    fc->enableFontFallback();
    fc->getParagraphCache()->turnOn(false);
    // Each engine gets the SkUnicode its clients ship with, as SkParagraphFlutterBugsTest does
    auto icu = SkUnicodes::ICU::Make();
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
    auto apple = SkUnicodes::Apple::Make();
#else
    auto apple = icu;
#endif
    auto unicodeFor = [&](TextShapingEngine e) {
        return e == TextShapingEngine::kCoreText ? apple : icu;
    };

    // Profiling mode: shape kCoreText paragraphs for a fixed time so a profiler can attribute it
    if (argc > 3 && strcmp(argv[3], "loop") == 0) {
        double end = now_ms() + 8000;
        while (now_ms() < end) {
            build(text, TextShapingEngine::kCoreText, fc, apple, family, letterSpacing)
                    ->layout(300);
        }
        return 0;
    }

    const float widths[] = {120, 200, 320, 480, 800};
    const int kW = sizeof(widths) / sizeof(widths[0]);
    const int kIters = 200;

    struct Engine { const char* name; TextShapingEngine e; } engines[] = {
        {"kCoreText", TextShapingEngine::kCoreText}, {"kDefault ", TextShapingEngine::kDefault}};
    for (auto& eng : engines) {
        // Warm-up so font and fallback caches do not land in the first sample
        build(text, eng.e, fc, unicodeFor(eng.e), family, letterSpacing)->layout(300);

        double t0 = now_ms();
        for (int i = 0; i < kIters; ++i) {
            auto p = build(text, eng.e, fc, unicodeFor(eng.e), family, letterSpacing);
            p->layout(widths[i % kW]);
        }
        double firstMs = (now_ms() - t0) / kIters;

        auto p = build(text, eng.e, fc, unicodeFor(eng.e), family, letterSpacing);
        p->layout(1000);
        double lines = 0;
        t0 = now_ms();
        for (int i = 0; i < kIters; ++i) {
            // Consecutive widths always differ, so every call re-breaks the shaped runs
            p->layout(widths[i % kW]);
            lines += p->lineNumber();
        }
        double relayoutMs = (now_ms() - t0) / kIters;
        printf("%s %-10s text=%zu bytes  first layout %7.3f ms   re-layout %7.3f ms per width"
               "  (avg %.1f lines)\n", eng.name, family, text.size(), firstMs, relayoutMs,
               lines / kIters);
    }
    return 0;
}
