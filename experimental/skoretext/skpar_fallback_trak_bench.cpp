/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// First kCoreText layout of one 17 pt .AppleSystemUIFont line, with and without letterSpacing,
// for Latin text and for text with CJK, emoji and Hebrew fallback runs. letterSpacing goes to
// CoreText as kCTTrackingAttributeName on top of each font's trak, in one typeset pass, so the
// letter-spaced cost should stay close to the unspaced one. ParagraphCache is off so every layout
// shapes. Prints the median of 3 runs, each the best of 5 x 2000 layouts. Build: see README.md
#include "include/core/SkFontMgr.h"
#include "include/core/SkString.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skunicode/include/SkUnicode_apple.h"

#include <algorithm>
#include <chrono>
#include <cstdio>

using namespace skia::textlayout;

int main() {
    auto fc = sk_make_sp<FontCollection>();
    fc->setDefaultFontManager(SkFontMgr_New_CoreText(nullptr));
    fc->getParagraphCache()->turnOn(false);
    auto unicode = SkUnicodes::Apple::Make();

    const char* latin = "The quick brown fox jumps over the lazy dog again and again";
    // 2 CJK ideographs, 1 emoji and 3 Hebrew letters: 3 fallback fonts under .SFNS
    const char* mixed =
            "The quick \xE6\xB0\xB4\xE6\xB0\xB4 brown fox \xF0\x9F\x98\x80 jumps "
            "\xD7\xA9\xD7\x9C\xD7\x95 over";
    struct Case {
        const char* name;
        const char* text;
        float letterSpacing;
    } cases[] = {
            {"latin ls0", latin, 0},
            {"latin ls1", latin, 1},
            {"cjk+emoji+hebrew ls0", mixed, 0},
            {"cjk+emoji+hebrew ls1", mixed, 1},
    };
    for (const Case& c : cases) {
        auto once = [&]() {
            ParagraphStyle ps;
            ps.setTextShapingEngine(TextShapingEngine::kCoreText);
            TextStyle ts;
            ts.setFontFamilies({SkString(".AppleSystemUIFont")});
            ts.setFontSize(17);
            ts.setLetterSpacing(c.letterSpacing);
            auto b = ParagraphBuilder::make(ps, fc, unicode);
            b->pushStyle(ts);
            b->addText(c.text);
            auto p = b->Build();
            p->layout(300);
            return p->getHeight();
        };
        // Warm-up so font and fallback caches do not land in the first sample
        for (int i = 0; i < 50; ++i) once();
        constexpr int kIters = 2000;
        double runs[3];
        for (double& best : runs) {
            best = 1e9;
            for (int rep = 0; rep < 5; ++rep) {
                auto t0 = std::chrono::steady_clock::now();
                for (int i = 0; i < kIters; ++i) once();
                auto t1 = std::chrono::steady_clock::now();
                best = std::min(
                        best, std::chrono::duration<double, std::micro>(t1 - t0).count() / kIters);
            }
        }
        std::sort(runs, runs + 3);
        printf("%-22s first layout %7.1f us  (runs %.1f %.1f %.1f)\n",
               c.name,
               runs[1],
               runs[0],
               runs[1],
               runs[2]);
    }
    return 0;
}
