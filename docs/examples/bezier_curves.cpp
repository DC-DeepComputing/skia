// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(bezier_curves, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(8);
    paint.setColor(0xff4285F4);
    paint.setAntiAlias(true);
    paint.setStrokeCap(SkPaint::kRound_Cap);

    SkPath path = SkPathBuilder()
        .moveTo(10, 10)
        .quadTo(256, 64, 128, 128)
        .quadTo(10, 192, 250, 250)
        .detach();
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
