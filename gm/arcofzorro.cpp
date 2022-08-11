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
#include "include/utils/SkRandom.h"

#include "include/core/SkBitmap.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(16, 16);
    }

    void onDraw(SkCanvas* canvas) override {
        SkRect r { 0, 0, 16, 16};

        SkPaint p1;
        p1.setBlendMode(SkBlendMode::kColorDodge);
        p1.setColor({1, 0, 0, 1});

        SkPaint p2;
        p2.setBlendMode(SkBlendMode::kColorBurn);
        p2.setColor({0, 0, 1, 1});

        canvas->drawRect(r, p1);
        canvas->drawRect(r, p2);

        SkBitmap bm;
        SkImageInfo info = SkImageInfo::Make({16, 16}, SkColorType::kRGBA_8888_SkColorType,
                                             SkAlphaType::kPremul_SkAlphaType);

        bm.allocPixels(info);
        canvas->readPixels(bm, 0, 0);

        SkColor4f color = bm.getColor4f(8, 8);
        SkDebugf("%.2f %.2f %.2f %.2f\n", color.fR, color.fG, color.fB, color.fA);
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
