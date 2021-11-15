/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/effects/SkDashPathEffect.h"

DEF_SIMPLE_GM(crbug_1113794, canvas, 600, 200) {
    SkPath path = SkPath::Line({50.f, 80.f}, {50.f, 20.f});

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(0.25f);
    paint.setStyle(SkPaint::kStroke_Style);

    static constexpr SkScalar kDash[2] = {10.f, 10.f};
    paint.setPathEffect(SkDashPathEffect::Make(kDash, 2, 0.f));

    SkMatrix viewBox = SkMatrix::RectToRect(SkRect::MakeWH(100, 100), SkRect::MakeWH(600, 200));
    canvas->concat(viewBox);

    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(skbug_12546, canvas, 400, 170) {
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);


    for (SkScalar delta : {1.01f, 0.0f}) {
        SkPath path;
        path.moveTo( 50, 50).rLineTo(delta, 0);
        path.moveTo(120, 50).rLineTo(delta, 0);

        for (auto s : {SkPaint::kButt_Cap, SkPaint::kSquare_Cap, SkPaint::kRound_Cap}) {
            p.setStrokeCap(s);

            auto draw = [&]() {
                p.setStrokeWidth(50);
                p.setColor(SK_ColorBLACK);
                canvas->drawPath(path, p);

                p.setStrokeWidth(0);
                p.setColor(SK_ColorRED);
                canvas->drawPath(path, p);
            };

            canvas->save();

            draw();

            SkScalar intervals[] = {1000, 100};
            p.setPathEffect(
              SkDashPathEffect::Make(intervals, 2, 0));

            canvas->translate(0, 64);

            draw();

            canvas->restore();
            canvas->translate(200, 0);
        }
    }
}
