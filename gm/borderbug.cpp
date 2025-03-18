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
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkBlurMask.h"

namespace skiagm {

// Border draw fails on Android/ARM with grdawn_vk.
class BorderBugGM : public GM {
public:
    BorderBugGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString getName() const override { return SkString("borderbug"); }

    SkISize getISize() override { return SkISize::Make(400, 400); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SkColorSetARGB(255, 255, 255, 0));
        paint.setStrokeWidth(20);
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::Style::kStroke_Style);
        canvas->drawRect(SkRect::MakeLTRB(10, 10, 230, 430), paint);
    }

private:
    using INHERITED = GM;
};

DEF_GM(return new BorderBugGM;)
}  // namespace skiagm
