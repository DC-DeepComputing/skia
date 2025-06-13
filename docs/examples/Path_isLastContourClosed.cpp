// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_isLastContourClosed, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
        SkDebugf("%s last contour is %s" "closed\n", prefix,
                 path.isLastContourClosed() ? "" : "not ");
    };
    SkPathBuilder path;
    debugster("initial", path.snapshot());
    path.close();
    debugster("after close", path.snapshot());
    path.lineTo(0, 0);
    debugster("after lineTo", path.snapshot());
    path.close();
    debugster("after close", path.snapshot());
}
}  // END FIDDLE
