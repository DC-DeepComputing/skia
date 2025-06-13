// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(pathmeasure, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path = SkPathBuilder()
        .moveTo(0, 0)
        .lineTo(1, 0)
        .moveTo(0, 1)
        .lineTo(1, 1)
        .detach();
    SkPathMeasure measure(path, false);
    SkPath result = measure.getSegment(.5, 1.5, true).value();
    result.dump();
}
}  // END FIDDLE
