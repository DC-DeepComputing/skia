// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(crbug_663246, 384, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    canvas->scale(0.5, 0.5);

    p.setStrokeWidth(195.23635741822847);
    SkPath path = SkPathBuilder()
        .moveTo(488, 182.10368918639688)
        .cubicTo(572, 182.10368918639688, 572, 97.61817870911423, 655, 97.61817870911423)
        .detach();
    p.setColor(SK_ColorGREEN);
    canvas->drawPath(path, p);

    p.setStrokeWidth(504.6189756548017);
    path = SkPathBuilder()
        .moveTo(20, 252.30948782740091)
        .cubicTo(31, 252.30948782740091, 31, 278.79312771879796, 41, 278.79312771879796)
        .detach();
    p.setColor(SK_ColorRED);
    canvas->drawPath(path, p);
}
}  // END FIDDLE
