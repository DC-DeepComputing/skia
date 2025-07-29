// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(bugoftheday, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    SkPath path = SkPathBuilder()
        .moveTo(10, 10)
        .moveTo(75, 75)
        .lineTo(150, 75)
        .lineTo(150, 150)
        .lineTo(75, 150)
        .detach();
    canvas->drawPath(path, p);
}
}  // END FIDDLE
