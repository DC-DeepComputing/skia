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

#include "include/effects/SkImageFilters.h"

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

    SkISize getISize() override { return SkISize::Make(128, 128); }

    void onDraw(SkCanvas* canvas) override {

        SkRect rect = SkRect::MakeXYWH(10, 10, 100, 100);

        float sigma = 5.0f;

        sk_sp<SkImageFilter> blurIF = SkImageFilters::Blur(sigma, sigma, /* input= */ nullptr);

        SkPaint p;
        p.setImageFilter(std::move(blurIF));

        canvas->drawRect(rect, p);
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
