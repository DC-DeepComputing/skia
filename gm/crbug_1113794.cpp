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

DEF_SIMPLE_GM(skbug_12546, canvas, 390, 320) {
    int colorIndex = 0;
    const SkColor colors[] = { SK_ColorBLACK, SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    auto get_color = [&]() { return colors[colorIndex++ % 4]; };

    for (SkScalar delta : {0.0f, 0.01f}) {
        canvas->save();

        SkPath path;
        path.moveTo( 50, 50).rLineTo(delta, 0);
        path.moveTo(120, 50).rLineTo(delta, 0);

        for (auto s : {SkPaint::kSquare_Cap, SkPaint::kRound_Cap}) {
            auto draw = [&](SkPaint::Cap cap, SkColor color, sk_sp<SkPathEffect> pe) {
                SkPaint p;
                p.setStyle(SkPaint::kStroke_Style);
                p.setStrokeCap(cap);
                p.setStrokeWidth(50);
                p.setColor(color);
                p.setPathEffect(std::move(pe));
                canvas->drawPath(path, p);
            };

            auto color = get_color();
            draw(s, color, nullptr);

            // draw using a dash patheffect
            if (1) {
                canvas->save();
                canvas->translate(0, 64);

                const SkScalar intervals[] = {1000, 100};
                auto pe = SkDashPathEffect::Make(intervals, 2, 0);

                draw(s, SkColorSetA(color, 0x80), pe);

                canvas->restore();
            }
            canvas->translate(200, 0);
        }
        canvas->restore();
        canvas->translate(0, 150);
    }
}
