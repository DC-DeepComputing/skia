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

#include "include/effects/SkGradientShader.h"

static sk_sp<SkShader> make_shader(SkBlendMode mode) {
    SkPoint pts[2];
    SkColor colors[2];

    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(100), 0);
    colors[0] = SK_ColorRED;
    colors[1] = SK_ColorBLUE;
    auto shaderA = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);

    pts[0].set(0, 0);
    pts[1].set(0, SkIntToScalar(100));
    colors[0] = SK_ColorBLACK;
    colors[1] = SkColorSetARGB(0x80, 0, 0, 0);
    auto shaderB = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);

    return SkShaders::Blend(mode, std::move(shaderA), std::move(shaderB));
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

    SkISize getISize() override { return SkISize::Make(100, 100); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;

        p.setShader(make_shader(SkBlendMode::kSrcOver));
        canvas->drawRect(SkRect::MakeWH(100, 100), p);
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
