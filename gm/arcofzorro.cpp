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
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFF808080);
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(250, 250);
    }

    void onDraw(SkCanvas* canvas) override {

        SkPaint skyBlue;
        skyBlue.setColor(0xFF87CEEB);
        skyBlue.setAntiAlias(true);

        canvas->drawCircle({150, 75}, 70, skyBlue);

        SkPaint strokePaint;
        strokePaint.setStrokeWidth(5);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        strokePaint.setAntiAlias(true);

        canvas->drawCircle({150, 75}, 70, strokePaint);

        SkPath path;
        path.addCircle(150, 75, 70);
        path.addCircle(100, 75, 60);

        SkPaint p2;
        p2.setColor({1.0, 1.0, 0.0, 0.5});
        p2.setBlendMode(SkBlendMode::kLighten);
        p2.setAntiAlias(true);

        canvas->drawPath(path, p2);
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
