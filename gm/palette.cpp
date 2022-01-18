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

        SkFontArguments cloneArguments;
        SkFontArguments::PaletteOverride paletteOverride;
        switch (fTestType) {
            case kPaletteSwitchDark: {
                paletteOverride.basePalette = 1;
                cloneArguments.setPaletteOverride(paletteOverride);
                fTypeface = fTypeface->makeClone(cloneArguments);
                break;
            }
            case kPaletteSwitchLight: {
                paletteOverride.basePalette = 2;
                cloneArguments.setPaletteOverride(paletteOverride);
                fTypeface = fTypeface->makeClone(cloneArguments);
                break;
            }
            case kPaletteOverrideOne: {
                SkFontArguments::PaletteOverride::ColorOverride colorOverrides[1] = {
                        {3, 0xff02dfe2},
                };
                paletteOverride.basePalette = 2;
                paletteOverride.colorOverrides = colorOverrides;
                paletteOverride.colorOverrideCount = 1;
                cloneArguments.setPaletteOverride(paletteOverride);
                fTypeface = fTypeface->makeClone(cloneArguments);
                break;
            }
            case kPaletteOverrideAll: {
                SkFontArguments::PaletteOverride::ColorOverride colorOverrides[12] = {
                        {0, 0xff310b55},
                        {1, 0xff510970},
                        {2, 0xff76078f},
                        {3, 0xff9606aa},
                        {4, 0xffb404c4},
                        {5, 0xffd802e2},
                        {6, 0xfffa00ff},
                        {7, 0xff888888},
                        {8, 0xff888888},
                        {9, 0xff888888},
                        {10, 0xff888888},
                        {11, 0xff888888}};
                paletteOverride.basePalette = 0;
                paletteOverride.colorOverrides = colorOverrides;
                paletteOverride.colorOverrideCount = 12;
                cloneArguments.setPaletteOverride(paletteOverride);
                fTypeface = fTypeface->makeClone(cloneArguments);
                break;
            }
            default:
                SkASSERT(false);
        }
    }

    SkString onShortName() override {
        SkString gm_name = SkStringPrintf("font_palette_%s", testTypeToString(fTestType).c_str());
        return gm_name;
    }

    SkISize onISize() override { return SkISize::Make(600, 400); }

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
        for (SkScalar textSize : {120, 160, 180}) {
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
// TODO: Support this not only in makeClone but also in makeFromFontArguments (?)

}  // namespace skiagm
