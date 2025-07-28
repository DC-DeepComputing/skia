// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_isEmpty, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
        SkDebugf("%s path is %s" "empty\n", prefix, path.isEmpty() ? "" : "not ");
    };
    SkPathBuilder path;
    debugster("initial", path.snapshot());
    path.moveTo(0, 0);
    debugster("after moveTo", path.snapshot());
    path.reset();
    debugster("after reset", path.snapshot());
    path.lineTo(0, 0);
    debugster("after lineTo", path.snapshot());
    path.reset();
    debugster("after reset", path.snapshot());
}
}  // END FIDDLE
