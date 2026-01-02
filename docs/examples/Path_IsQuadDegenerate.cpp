// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_IsQuadDegenerate, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const SkPath& path, bool exact) -> void {
        auto points = path.points();
        SkDebugf("quad (%1.8g,%1.8g), (%1.8g,%1.8g), (%1.8g,%1.8g) is %s" "degenerate, %s\n",
            points[0].fX, points[0].fY, points[1].fX,
            points[1].fY, points[2].fX, points[2].fY,
            SkPath::IsQuadDegenerate(points[0], points[1], points[2], exact) ?
            "" : "not ", exact ? "exactly" : "nearly");
    };
    SkPath path = SkPathBuilder()
                  .moveTo({100, 100})
                  .quadTo({100.00001f, 100.00001f}, {100.00002f, 100.00002f})
                  .detach();
    SkPath offset = path.makeOffset(1000, 1000);
    for (bool exact : { false, true } ) {
        debugster(path, exact);
        debugster(offset, exact);
    }
}
}  // END FIDDLE
