/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <string.h>
#include <initializer_list>
#include <vector>

namespace skiagm {

class ColrV1SweepGM : public GM {
private:
    sk_sp<SkTypeface> fTypeface;
  SkString fTestName;
  std::vector<std::vector<uint16_t>> fRowGlyphs;

public:
    ColrV1SweepGM(std::string testName, std::vector<std::vector<uint16_t>> rowGlyphs)
            : fTestName(testName.c_str()), fRowGlyphs(rowGlyphs) {}

protected:

    void onOnceBeforeDraw() override {
        fTypeface = MakeResourceAsTypeface("fonts/more_samples-glyf_colr_1.ttf");
    }

    SkString onShortName() override {
        return SkStringPrintf("colrv1_%s_samples", fTestName.c_str());
    }

    SkISize onISize() override { return SkISize::Make(1400, 600); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        canvas->translate(20, 20);

        if (!fTypeface) {
          *errorMsg = "Did not recognize COLR v1 font format.";
          return DrawResult::kSkip;
        }

        SkFont font(fTypeface);
        font.setSize(145);

        SkFontMetrics metrics;
        SkScalar y = 0;
        for (auto& glyphs : fRowGlyphs) {
            font.getMetrics(&metrics);
            y += -metrics.fAscent;
            canvas->drawSimpleText(glyphs.data(),
                                   glyphs.size() * sizeof(uint16_t),
                                   SkTextEncoding::kGlyphID,
                                   10, y, font, paint);
            y += metrics.fDescent + metrics.fLeading;
        }
        return DrawResult::kOk;
    }

private:


    using INHERITED = GM;
};

DEF_GM(return new ColrV1SweepGM("sweep_narrow",
                                {{2, 58, 59, 60, 61, 62, 63, 64},
                                 {65, 66, 67, 68, 69, 70, 71, 72},
                                 {73, 74, 75, 76, 77, 78, 79, 80}}));
DEF_GM(return new ColrV1SweepGM(
        "sweep_wide", {{81, 82, 83, 84, 85}, {86, 87, 88, 89, 90}, {91, 92, 93, 94, 95}}));

}  // namespace skiagm
