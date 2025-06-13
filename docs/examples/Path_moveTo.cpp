// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_moveTo, 140, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkRect rect = { 20, 20, 120, 80 };
    SkPath path = SkPathBuilder()
        .addRect(rect)
        .moveTo(rect.fLeft, rect.fTop)
        .lineTo(rect.fRight, rect.fBottom)
        .moveTo(rect.fLeft, rect.fBottom)
        .lineTo(rect.fRight, rect.fTop)
        .detach();
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
