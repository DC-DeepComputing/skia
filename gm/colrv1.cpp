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
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include "src/pdf/SkPDFFont.h"

#include <string.h>
#include <initializer_list>
#include <regex>

namespace skiagm {

namespace {
bool ColrV1VariationsEnabledForTest() { return true; }
}

class ColrV1GM : public GM {
public:
    ColrV1GM(const char* testName,
             const char* regularExpression,
             SkScalar skewX = 0.f,
             SkScalar rotateDeg = 0.f)
            : fTestName(testName)
            , fMatchingTestsExpression(regularExpression)
            , fSkewX(skewX)
            , fRotateDeg(rotateDeg) {
        fPreviousFlagFunc =
                SkGraphics::SetVariableColrV1EnabledFunc(ColrV1VariationsEnabledForTest);
    }

    ~ColrV1GM() override { SkGraphics::SetVariableColrV1EnabledFunc(fPreviousFlagFunc); }

protected:
    std::vector<uint16_t> matchingGlyphIds(SkTypeface* typeface,
                                           const char* regular_expression) {
        if (!typeface) {
            return {};
        }
        std::unique_ptr<SkString[]> glyphNames =
                std::make_unique<SkString[]>(typeface->countGlyphs());

        std::regex re(regular_expression);
        std::cmatch match;
        SkPDFFont::GetType1GlyphNames(*typeface, glyphNames.get());
        std::vector<uint16_t> matches;
        for (int i = 0; i < typeface->countGlyphs(); ++i) {
            if (std::regex_search(glyphNames[i].c_str(), match, re)) {
                matches.push_back(i);
            }
        }
        return matches;
    }

    struct EmojiFont {
        sk_sp<SkTypeface> fTypeface;
        std::vector<uint16_t> fGlyphs;
        size_t bytesize() { return fGlyphs.size() * sizeof(uint16_t); }
    } fTestFont;

    void onOnceBeforeDraw() override {

        const char testFontName[] = "fonts/test_glyphs-glyf_colr_1.ttf";

        fTestFont.fTypeface = MakeResourceAsTypeface(testFontName);
        fTestFont.fGlyphs =
                matchingGlyphIds(fTestFont.fTypeface.get(), fMatchingTestsExpression.c_str());

        fVariationSliders = ToolUtils::VariationSliders(fTestFont.fTypeface.get());
    }

    SkString onShortName() override {
        SkASSERT(!fTestName.isEmpty());
        SkString gm_name = SkStringPrintf("colrv1_%s", fTestName.c_str());

        if (fSkewX) {
            gm_name.append("_skew");
        }

        if (fRotateDeg) {
            gm_name.append("_rotate");
        }

        return gm_name;
    }

    bool onGetControls(SkMetaData* controls) override {
        return fVariationSliders.writeControls(controls);
    }

    void onSetControls(const SkMetaData& controls) override {
        return fVariationSliders.readControls(controls);
    }

    SkISize onISize() override { return SkISize::Make(1400, 600); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;

        canvas->translate(200, 20);

        if (!fTestFont.fTypeface) {
          *errorMsg = "Did not recognize COLR v1 font format.";
          return DrawResult::kSkip;
        }

        canvas->rotate(fRotateDeg);
        canvas->skew(fSkewX, 0);

        SkSpan<const SkFontArguments::VariationPosition::Coordinate> coords =
                fVariationSliders.getCoordinates();
        SkFontArguments::VariationPosition varPos = {coords.data(),
                                                     static_cast<int>(coords.size())};
        SkFontArguments args;
        args.setVariationDesignPosition(varPos);
        sk_sp<SkTypeface> axisAppliedTypeface = fTestFont.fTypeface->makeClone(args);
        SkFont font(axisAppliedTypeface);

        SkFontMetrics metrics;
        SkScalar y = 0;
        std::vector<SkColor> paint_colors = {
                SK_ColorBLACK, SK_ColorGREEN, SK_ColorRED, SK_ColorBLUE};
        auto paint_color_iterator = paint_colors.begin();
        for (SkScalar textSize : { 12, 18, 30, 120 }) {
            font.setSize(textSize);
            font.getMetrics(&metrics);
            y += -metrics.fAscent;
            paint.setColor(*paint_color_iterator);
            canvas->drawSimpleText(fTestFont.fGlyphs.data(),
                                   fTestFont.bytesize(),
                                   SkTextEncoding::kGlyphID,
                                   10, y, font, paint);
            y += metrics.fDescent + metrics.fLeading;
            paint_color_iterator++;
        }
        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;

    SkString fTestName;
    SkString fMatchingTestsExpression;
    SkScalar fSkewX;
    SkScalar fRotateDeg;
    ToolUtils::VariationSliders fVariationSliders;
    SkGraphics::VariableColrV1EnabledFunc fPreviousFlagFunc;
};

DEF_GM(return new ColrV1GM("linear_repeat_stops", "linear_repeat"));
DEF_GM(return new ColrV1GM("linear_repeat_stops", "linear_repeat", -0.5f, 0.f));
DEF_GM(return new ColrV1GM("linear_repeat_stops", "linear_repeat", 0.f, 20.f));
DEF_GM(return new ColrV1GM("linear_repeat_stops", "linear_repeat", -0.5f, 20.f));
DEF_GM(return new ColrV1GM("scaling", "scale_.*center_"));
DEF_GM(return new ColrV1GM("extend_mode", "_extend_mode_"));
DEF_GM(return new ColrV1GM("extend_mode", "_extend_mode_", -0.5f, 0.f));
DEF_GM(return new ColrV1GM("extend_mode", "_extend_mode_", 0.f, 20.f));
DEF_GM(return new ColrV1GM("extend_mode", "_extend_mode_", -0.5f, 20.f));
DEF_GM(return new ColrV1GM("rotate", "rotate_"));
DEF_GM(return new ColrV1GM("skew", "skew_"));
DEF_GM(return new ColrV1GM("transform", "transform_"));
DEF_GM(return new ColrV1GM("clip_box", "clip_box_"));
DEF_GM(return new ColrV1GM("clip_box", "clip_box_", -0.5f, 20.f));
DEF_GM(return new ColrV1GM("composite_modes", "composite_[A-Z_]+"));
DEF_GM(return new ColrV1GM("composite_modes", "composite_[A-Z_]+", -0.5f, 0.f));
DEF_GM(return new ColrV1GM("composite_modes", "composite_[A-Z_]+", 0.f, 20.f));
DEF_GM(return new ColrV1GM("composite_modes", "composite_[A-Z_]+", -0.5f, 20.f));
DEF_GM(return new ColrV1GM("foreground_color", "foreground_"));
DEF_GM(return new ColrV1GM("sweep_pad", "sweep_.*_pad"));
DEF_GM(return new ColrV1GM("sweep_reflect", "sweep_.*_reflect"));
DEF_GM(return new ColrV1GM("sweep_repeat", "sweep_.*_repeat"));

}  // namespace skiagm
