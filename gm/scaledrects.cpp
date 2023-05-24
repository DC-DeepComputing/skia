/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"

namespace skiagm {

// From crbug.com/1442854
class ScaledRectsGM : public GM {
public:
    ScaledRectsGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString onShortName() override {
        return SkString("scaledrects");
    }

    SkISize onISize() override {
        return SkISize::Make(128, 64);
    }

    void onDraw(SkCanvas* canvas) override {
        {
            SkRect clipRect = SkRect::MakeXYWH(10, 50, 100, 10);

            SkPaint clipP;
            clipP.setColor(SK_ColorBLACK);
            clipP.setStyle(SkPaint::kStroke_Style);
            clipP.setStrokeWidth(2.0f);

            canvas->clipRect(clipRect);
        }

        {
            SkPaint blue;
            blue.setColor(SK_ColorBLUE);

            SkMatrix m1 = SkMatrix::MakeAll(3, -0.5, 0,
                                            -0.5, -3, 0,
                                            0, 0, 1);
            canvas->setMatrix(m1);

            canvas->drawRect(SkRect::MakeXYWH(-1000, -1000, 2000, 2000), blue);
        }

        // The red rectangle should be equivalent and should
        // completely occlude the blue rectangle. The difference
        // is that a scale factor of 1000 was applied to the
        // transform and the scale of the fillRect was reduced by
        // 1000. This difference triggers a bug related to DMSAA
        // Where the transform somehow affects the clip even though
        // the clip was applied first.

        {
            SkPaint red;
            red.setColor(SK_ColorRED);

            SkMatrix m2 = SkMatrix::MakeAll(3000, -500, 0,
                                            -500, -3000, 0,
                                            0, 0, 1);
            canvas->setMatrix(m2);

            canvas->drawRect(SkRect::MakeXYWH(-1, -1, 2, 2), red);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ScaledRectsGM;)

}  // namespace skiagm
