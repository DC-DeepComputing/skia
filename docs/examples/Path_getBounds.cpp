// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_getBounds, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
            const SkRect& bounds = path.getBounds();
            SkDebugf("%s bounds = %g, %g, %g, %g\n", prefix,
                     bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
    };
    SkPathBuilder path;
    debugster("empty", path.snapshot());
    path.addCircle(50, 45, 25);
    debugster("circle", path.snapshot());
    SkMatrix matrix;
    matrix.setRotate(45, 50, 45);
    path.transform(matrix);
    debugster("rotated circle", path.snapshot());
}
}  // END FIDDLE
