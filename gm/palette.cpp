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

namespace skiagm {

class FontPaletteGM : public GM {
public:
    enum PaletteTestType {
        kPaletteSwitchLight,
        kPaletteSwitchDark,
        kPaletteOverrideOne,
        kPaletteOverrideAll
    };

    FontPaletteGM(PaletteTestType testType) : fTestType(testType) {}

protected:
    static SkString testTypeToString(PaletteTestType testType) {
        switch (testType) {
            case kPaletteSwitchLight:
                return SkString("light");
            case kPaletteSwitchDark:
                return SkString("dark");
            case kPaletteOverrideOne:
                return SkString("override_one");
            case kPaletteOverrideAll:
                return SkString("override_all");
        }
        SkASSERT(false); /* not reached */
        return SkString();
    }

    sk_sp<SkTypeface> fTypeface;
    std::vector<uint16_t> fGlyphs{{56, 57}};

    void onOnceBeforeDraw() override {
        fTypeface = MakeResourceAsTypeface("fonts/more_samples-glyf_colr_1.ttf");

        SkFontArguments::PaletteOverride::ColorOverride colorOverrides[10] = {{0, 0xFF001000},
                                                                              {1, 0xFF001800},
                                                                              {2, 0xFF002000},
                                                                              {3, 0xFF002800},
                                                                              {4, 0xFF003000},
                                                                              {5, 0xFF003800},
                                                                              {6, 0xFF004000},
                                                                              {7, 0xFF004800},
                                                                              {8, 0xFF005000},
                                                                              {9, 0xFF005800}};
        SkFontArguments::PaletteOverride paletteOverride;
        paletteOverride.basePalette = 0;
        paletteOverride.colorOverrides = colorOverrides;
        paletteOverride.colorOverrideCount = 10;

        SkFontArguments cloneArguments;
        cloneArguments.setPaletteOverride(paletteOverride);

        fTypeface = fTypeface->makeClone(cloneArguments);
    }

    SkString onShortName() override {
        SkString gm_name = SkStringPrintf("font_palette_%s", testTypeToString(fTestType).c_str());
        return gm_name;
    }

    SkISize onISize() override { return SkISize::Make(1400, 600); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;

        canvas->translate(200, 20);

        if (!fTypeface) {
            *errorMsg = "Did not recognize COLR v1 test font format.";
            return DrawResult::kSkip;
        }

        SkFont font(fTypeface);

        SkFontMetrics metrics;
        SkScalar y = 0;
        std::vector<SkColor> paint_colors = {
                SK_ColorBLACK, SK_ColorGREEN, SK_ColorRED, SK_ColorBLUE};
        auto paint_color_iterator = paint_colors.begin();
        for (SkScalar textSize : {12, 18, 30, 120}) {
            font.setSize(textSize);
            font.getMetrics(&metrics);
            y += -metrics.fAscent;
            paint.setColor(*paint_color_iterator);
            canvas->drawSimpleText(fGlyphs.data(),
                                   fGlyphs.size() * sizeof(uint16_t),
                                   SkTextEncoding::kGlyphID,
                                   10,
                                   y,
                                   font,
                                   paint);
            y += metrics.fDescent + metrics.fLeading;
            paint_color_iterator++;
        }
        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;
    PaletteTestType fTestType;
};

DEF_GM(return new FontPaletteGM(FontPaletteGM::kPaletteSwitchLight);)
DEF_GM(return new FontPaletteGM(FontPaletteGM::kPaletteSwitchDark);)
DEF_GM(return new FontPaletteGM(FontPaletteGM::kPaletteOverrideOne);)
DEF_GM(return new FontPaletteGM(FontPaletteGM::kPaletteOverrideAll);)

}  // namespace skiagm
