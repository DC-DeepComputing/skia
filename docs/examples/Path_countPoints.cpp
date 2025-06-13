// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_countPoints, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
         SkDebugf("%s point count: %d\n", prefix, path.countPoints());
    };
    SkPathBuilder path;
    debugster("empty", path.snapshot());
    path.lineTo(0, 0);
    debugster("zero line", path.snapshot());
    path.reset();
    path.moveTo(10, 10);
    path.lineTo(20, 20);
    debugster("line", path.snapshot());
    path.moveTo(20, 20);
    debugster("second move", path.snapshot());
}
}  // END FIDDLE
