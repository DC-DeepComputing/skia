// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_offset, 256, 60, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath pattern = SkPathBuilder()
        .moveTo(20, 20)
        .lineTo(20, 40)
        .lineTo(40, 20)
        .detach();
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 10; i++) {
        SkPath path = pattern.makeOffset(20 * i, 0);
        canvas->drawPath(path, paint);
    }
}
}  // END FIDDLE
