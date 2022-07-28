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

#include <string.h>
#include <initializer_list>

namespace skiagm {

namespace {
bool ColrV1VariationsEnabledForTest() { return true; }

const SkScalar kTextSizes[] = {12, 18, 30, 120};
const char testFontName[] = "fonts/test_glyphs-glyf_colr_1.ttf";
}

class ColrV1GM : public GM {
public:
    ColrV1GM(const char* testName,
             const uint32_t* codepoints,
             uint16_t numCodepoints,
             SkScalar skewX = 0.f,
             SkScalar rotateDeg = 0.f)
            : fTestName(testName)
            , fCodepoints(codepoints)
            , fNumCodepoints(numCodepoints)
            , fSkewX(skewX)
            , fRotateDeg(rotateDeg) {
        fPreviousFlagFunc =
                SkGraphics::SetVariableColrV1EnabledFunc(ColrV1VariationsEnabledForTest);
        fTypeface = MakeResourceAsTypeface(testFontName);
    }

    ~ColrV1GM() override { SkGraphics::SetVariableColrV1EnabledFunc(fPreviousFlagFunc); }

protected:
    void onOnceBeforeDraw() override {

        fVariationSliders = ToolUtils::VariationSliders(fTypeface.get());
    }

    SkString onShortName() override {
        SkASSERT(!fTestName.isEmpty());
        SkString gm_name = SkStringPrintf("colrv1_%s", fTestName.c_str());

        if (fSkewX) {
            gm_name.append(SkStringPrintf("_skew_%.2f", fSkewX));
        }

        if (fRotateDeg) {
            gm_name.append(SkStringPrintf("_rotate_%.2f", fRotateDeg));
        }

        return gm_name;
    }

    bool onGetControls(SkMetaData* controls) override {
        return fVariationSliders.writeControls(controls);
    }

    void onSetControls(const SkMetaData& controls) override {
        return fVariationSliders.readControls(controls);
    }

    SkISize onISize() override {
        SkFont measureFont(fTypeface);
        measureFont.setSize(kTextSizes[SK_ARRAY_COUNT(kTextSizes) - 1]);
        int xWidth = ceilf(measureFont.measureText(
                fCodepoints, fNumCodepoints * sizeof(uint32_t), SkTextEncoding::kUTF32));
        const int roundToMultiple = 200;
        xWidth = ((xWidth + roundToMultiple / 2) / roundToMultiple) * roundToMultiple;

        return SkISize::Make(xWidth, 600);
    }

    sk_sp<SkTypeface> makeVariedTypeface() {
        SkSpan<const SkFontArguments::VariationPosition::Coordinate> coords =
                fVariationSliders.getCoordinates();
        SkFontArguments::VariationPosition varPos = {coords.data(),
                                                     static_cast<int>(coords.size())};
        SkFontArguments args;
        args.setVariationDesignPosition(varPos);
        return fTypeface->makeClone(args);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;

        canvas->translate(200, 20);

        if (!fTypeface) {
          *errorMsg = "Did not recognize COLR v1 font format.";
          return DrawResult::kSkip;
        }

        canvas->rotate(fRotateDeg);
        canvas->skew(fSkewX, 0);

        SkFont font(makeVariedTypeface());

        SkFontMetrics metrics;
        SkScalar y = 0;
        std::vector<SkColor> paint_colors = {
                SK_ColorBLACK, SK_ColorGREEN, SK_ColorRED, SK_ColorBLUE};
        auto paint_color_iterator = paint_colors.begin();
        for (SkScalar textSize : kTextSizes) {
            font.setSize(textSize);
            font.getMetrics(&metrics);
            y += -metrics.fAscent;
            paint.setColor(*paint_color_iterator);
            canvas->drawSimpleText(fCodepoints,
                                   fNumCodepoints * sizeof(uint32_t),
                                   SkTextEncoding::kUTF32,
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

    SkString fTestName;
    sk_sp<SkTypeface> fTypeface;
    const uint32_t* fCodepoints;
    uint16_t fNumCodepoints;
    SkScalar fSkewX;
    SkScalar fRotateDeg;
    ToolUtils::VariationSliders fVariationSliders;
    SkGraphics::VariableColrV1EnabledFunc fPreviousFlagFunc;
};

// Generated using test_glyphs-glyf_colr_v1.py from https://github.com/googlefonts/color-fonts,
// Regenerate descriptions and paste the generated arrays here when updating the test font.
namespace ColrV1TestDefinitions {
const uint32_t gradient_stops_repeat[] = {0xf0500, 0xf0501, 0xf0502, 0xf0503};
const uint32_t sweep_varsweep[] = {0xf0400, 0xf0401, 0xf0402, 0xf0403, 0xf0404, 0xf0405,
                                   0xf0406, 0xf0407, 0xf0408, 0xf0409, 0xf040a, 0xf040b,
                                   0xf040c, 0xf040d, 0xf040e, 0xf040f, 0xf0410, 0xf0411,
                                   0xf0412, 0xf0413, 0xf0414, 0xf0415, 0xf0416, 0xf0417};
const uint32_t paint_scale[] = {0xf0600, 0xf0601, 0xf0602, 0xf0603, 0xf0604, 0xf0605};
const uint32_t extend_mode[] = {0xf0700, 0xf0701, 0xf0702, 0xf0703, 0xf0704, 0xf0705};
const uint32_t paint_rotate[] = {0xf0800, 0xf0801, 0xf0802, 0xf0803};
const uint32_t paint_skew[] = {0xf0900, 0xf0901, 0xf0902, 0xf0903, 0xf0904, 0xf0905};
const uint32_t paint_transform[] = {0xf0a00, 0xf0a01, 0xf0a02, 0xf0a03};
const uint32_t paint_translate[] = {0xf0b00, 0xf0b01, 0xf0b02, 0xf0b03, 0xf0b04, 0xf0b05, 0xf0b06};
const uint32_t composite_mode[] = {0xf0c00, 0xf0c01, 0xf0c02, 0xf0c03, 0xf0c04, 0xf0c05, 0xf0c06,
                                   0xf0c07, 0xf0c08, 0xf0c09, 0xf0c0a, 0xf0c0b, 0xf0c0c, 0xf0c0d,
                                   0xf0c0e, 0xf0c0f, 0xf0c10, 0xf0c11, 0xf0c12, 0xf0c13, 0xf0c14,
                                   0xf0c15, 0xf0c16, 0xf0c17, 0xf0c18, 0xf0c19, 0xf0c1a, 0xf0c1b};
const uint32_t foreground_color[] = {
        0xf0d00, 0xf0d01, 0xf0d02, 0xf0d03, 0xf0d04, 0xf0d05, 0xf0d06, 0xf0d07};
const uint32_t clipbox[] = {0xf0e00, 0xf0e01, 0xf0e02, 0xf0e03, 0xf0e04};
const uint32_t gradient_p2_skewed[] = {0xf0f00};
const uint32_t variable_alpha[] = {0xf1200};
};  // namespace ColrV1TestDefinitions


#define DEF_COLRV1_GM_SKEW_ROTATE(TEST_CATEGORY, SKEW, ROTATE)          \
        DEF_GM(return new ColrV1GM(#TEST_CATEGORY,                                       \
                                   ColrV1TestDefinitions::TEST_CATEGORY,                 \
                                   SK_ARRAY_COUNT(ColrV1TestDefinitions::TEST_CATEGORY), \
                                   SKEW,                                                 \
                                   ROTATE));

#define DEF_COLRV1_GM(TEST_CATEGORY) DEF_COLRV1_GM_SKEW_ROTATE(TEST_CATEGORY, 0.f, 0.f);


DEF_COLRV1_GM(clipbox);
DEF_COLRV1_GM(composite_mode);
DEF_COLRV1_GM_SKEW_ROTATE(composite_mode, -0.5f, 0.f);
DEF_COLRV1_GM_SKEW_ROTATE(composite_mode, -0.5f, 20.f);
DEF_COLRV1_GM_SKEW_ROTATE(composite_mode, 0.f, 20.f);
DEF_COLRV1_GM(extend_mode);
DEF_COLRV1_GM_SKEW_ROTATE(extend_mode, -0.5f, 0.f);
DEF_COLRV1_GM_SKEW_ROTATE(extend_mode, -0.5f, 20.f);
DEF_COLRV1_GM_SKEW_ROTATE(extend_mode, 0.f, 20.f);
DEF_COLRV1_GM(foreground_color);
DEF_COLRV1_GM(gradient_p2_skewed);
DEF_COLRV1_GM(gradient_stops_repeat);
DEF_COLRV1_GM_SKEW_ROTATE(gradient_stops_repeat, -0.5f, 0.f);
DEF_COLRV1_GM_SKEW_ROTATE(gradient_stops_repeat, -0.5f, 20.f);
DEF_COLRV1_GM_SKEW_ROTATE(gradient_stops_repeat, 0.f, 20.f);
DEF_COLRV1_GM(paint_rotate);
DEF_COLRV1_GM(paint_scale);
DEF_COLRV1_GM(paint_skew);
DEF_COLRV1_GM(paint_transform);
DEF_COLRV1_GM(paint_translate);
DEF_COLRV1_GM(sweep_varsweep);
DEF_COLRV1_GM(variable_alpha);

}  // namespace skiagm
