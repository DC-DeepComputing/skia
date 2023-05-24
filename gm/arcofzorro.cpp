/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(150, 150);
    }

    void onDraw(SkCanvas* canvas) override {
        {
            SkRect clipRect = SkRect::MakeXYWH(10, 50, 100, 10);

            SkPaint clipP;
            clipP.setColor(SK_ColorBLACK);
            clipP.setStyle(SkPaint::kStroke_Style);
            clipP.setStrokeWidth(2.0f);

            canvas->drawRect(clipRect, clipP);

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

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
