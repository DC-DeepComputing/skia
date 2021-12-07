/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"

#include "include/effects/SkGradientShader.h"

namespace skiagm {

// This is just for bootstrapping Graphite.
class GraphiteStartGM : public GM {
public:
    GraphiteStartGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString onShortName() override {
        return SkString("graphitestart");
    }

    SkISize onISize() override {
        return SkISize::Make(256, 256);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p1; //, p2, p3;

        p1.setColor(SK_ColorRED);
        p1.setAntiAlias(false);
//        p2.setColor(SK_ColorGREEN);
//        p3.setColor(SK_ColorBLUE);

#if 0
        SkPoint pts[2] = { {10, 10}, {100, 100} };
        const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };
        SkScalar pos[2] = { 0.0f, 1.0f };
        auto grad = SkGradientShader::MakeLinear(pts, colors, pos, 2, SkTileMode::kClamp);
        p1.setShader(grad);
#endif

        canvas->drawRect({10, 10, 100, 100}, p1);
//        canvas->drawRect({50, 50, 150, 150}, p2);
//        canvas->drawRect({100, 100, 200, 200}, p3);
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GraphiteStartGM;)

}  // namespace skiagm
