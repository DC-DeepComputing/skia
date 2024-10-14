/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "tools/Resources.h"

namespace skiagm {

namespace {
const SkScalar kTextSizes[] = {13, 17, 18, 30, 33.3};
const char kTestFontName[] = "fonts/ahem.ttf";

}  // namespace

class AhemBoundsFontationsGM : public GM {
public:
    AhemBoundsFontationsGM() : fTestName("AhemBounds") { this->setBGColor(SK_ColorWHITE); }

protected:
    void onOnceBeforeDraw() override {
        fTestTypeface =
                SkTypeface_Make_Fontations(GetResourceAsStream(kTestFontName), SkFontArguments());
    }

    SkString getName() const override { return SkString("fontations_ahembounds"); }

    SkISize getISize() override { return SkISize::Make(400, 200); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);

        if (!fTestTypeface) {
            *errorMsg = "Unable to initialize typeface.";
            return DrawResult::kSkip;
        }

        SkFont font(fTestTypeface);
        const uint16_t testText[] = {35};
        size_t testTextBytesize = 2;
        SkScalar x = 100;
        SkScalar y = 150;
        canvas->translate(x, y);


        for (SkScalar textSize : kTextSizes) {
            font.setSize(textSize);
            float spacing = font.getSpacing() * 1.5;


            canvas->translate(0, spacing);

            font.setEmbolden(true);

            canvas->drawSimpleText(
                    testText, testTextBytesize, SkTextEncoding::kGlyphID, 0, 0, font, paint);



            /* Draw origin marker as a green dot. */
            paint.setColor(SK_ColorGREEN);
            canvas->drawRect(SkRect::MakeXYWH(0, 0, 2, 2), paint);
            paint.setColor(SK_ColorBLACK);


            float newX = textSize * 1.2;

            font.setEmbolden(false);
            canvas->drawSimpleText(
                    testText, testTextBytesize, SkTextEncoding::kGlyphID, newX, 0, font, paint);

            SkRect bounds;
            SkPaint boundsPaint;
            boundsPaint.setStroke(true);
            boundsPaint.setStrokeWidth(0);
            boundsPaint.setColor(SK_ColorRED);
            font.measureText(testText, testTextBytesize, SkTextEncoding::kGlyphID, &bounds);
            canvas->drawRect(bounds, boundsPaint);

            /* Draw origin marker as a green dot. */
            paint.setColor(SK_ColorGREEN);
            canvas->drawRect(SkRect::MakeXYWH(newX, 0, 2, 2), paint);
            paint.setColor(SK_ColorBLACK);

            font.measureText(testText, testTextBytesize, SkTextEncoding::kGlyphID, &bounds);
            bounds.offset(newX, 0);
            canvas->drawRect(bounds, boundsPaint);


        }

        canvas->translate(100, 470);

        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;

    const SkString fTestName;
    sk_sp<SkTypeface> fTestTypeface;
};

DEF_GM(return new AhemBoundsFontationsGM();)

}  // namespace skiagm
