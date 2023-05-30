/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"

#include "include/core/SkPathBuilder.h"

namespace {

void forward(SkPathBuilder* builder, int curAngle, int length) {
    int foo = curAngle % 360;
    switch (curAngle) {

    }
    builder->rLineTo(0, 0);

}

void create_hilbert_path(SkPathBuilder* builder, int depth, int length,
                         int curAngle, int turnAmount) {
    if (depth == 0) {
        return;
    }

    curAngle += turnAmount;
    create_hilbert_path(builder, depth-1, length, curAngle, -turnAmount);
    forward(builder, curAngle, length);
    curAngle -= turnAmount;
    create_hilbert_path(builder, depth-1, length, curAngle, turnAmount);
    forward(builder, curAngle, length);
    create_hilbert_path(builder, depth-1, length, curAngle, turnAmount);
    curAngle -= turnAmount;
    forward(builder, curAngle, length);
    create_hilbert_path(builder, depth-1, length, curAngle, -turnAmount);
    curAngle += turnAmount;

    // left B forward right A forward A right forward B left
}

} // anonymous namespace

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
        return SkISize::Make(1024, 1024);
    }

    void onOnceBeforeDraw() override {
        SkBitmap bm;

        bm.allocN32Pixels(1024, 1024, true);

        SkCanvas canvas(bm);

        SkPathBuilder builder;

        builder.moveTo(0, 0);
        create_hilbert_path(&builder, 4, 1, 0, 90);

        SkPath path = builder.snapshot();

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(1.0f);

        canvas.drawPath(path, paint);

        fImg = bm.asImage();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawImage(fImg, 0, 0);
    }

private:
    sk_sp<SkImage> fImg;

};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
