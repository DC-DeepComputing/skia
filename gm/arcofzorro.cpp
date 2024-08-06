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

#include "include/core/SkBlender.h"
#include "include/effects/SkGradientShader.h"


enum class ColorConstraint {
    kNone,
    kOpaque,
    kTransparent,
};

SkColor random_color(SkRandom* rand, ColorConstraint constraint) {
    uint32_t color = rand->nextU();

    switch (constraint) {
        case ColorConstraint::kNone:        return color;
        case ColorConstraint::kOpaque:      return 0xff000000 | color;
        case ColorConstraint::kTransparent: return 0x80000000 | color;
    }

    SkUNREACHABLE;
}


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
    SkString getName() const override { return SkString("arcofzorro"); }

    SkISize getISize() override { return SkISize::Make(200, 200); }

    void onDraw(SkCanvas* canvas) override {
        SkRandom rand;

        static constexpr int kMaxNumStops = 9;
        SkColor colors[kMaxNumStops] = {
                SK_ColorWHITE,
                SK_ColorRED,
                SK_ColorYELLOW,
                SK_ColorGREEN,
                SK_ColorCYAN,
                SK_ColorBLUE,
                SK_ColorBLACK,
                random_color(&rand, ColorConstraint::kOpaque),
                random_color(&rand, ColorConstraint::kOpaque)
        };

        static const float positions[kMaxNumStops] =
                { 0.0f, 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.75f, 0.875f, 1.0f };

        SkPaint paint;
        paint.setShader(SkGradientShader::MakeRadial(/* center= */ {100, 100},
                /* radius= */ 100,
                                                                   colors,
                                                                   positions,
                /* numStops= */ 7,
                                                                   SkTileMode::kClamp,
                /* flags= */ 0x0,
                /* localMatrix= */ nullptr));
        paint.setBlender(SkBlender::Mode(SkBlendMode::kSrcOver));

        canvas->drawRect({0, 0, 200, 200}, paint);
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
