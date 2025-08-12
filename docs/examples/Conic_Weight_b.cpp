// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Conic_Weight_b, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* verbNames[] = { "move", "line", "quad", "conic", "cubic", "close", "done" };
    const int pointCount[]  = {     1 ,     2 ,     3 ,      3 ,      4 ,      1 ,     0  };
    SkPath path;
    path.arcTo(20, 0, 20, 20, 20);
    SkPath::Iter iter(path, false);
    while (auto rec = iter.next()) {
       SkDebugf("%s ", verbNames[(int)rec->fVerb]);
       for (int i = 0; i < pointCount[(int)rec->fVerb]; ++i) {
            SkDebugf("{%g, %g}, ", rec->fPoints[i].fX, rec->fPoints[i].fY);
       }
       if (SkPathVerb::kConic == rec->fVerb) {
           SkDebugf("weight = %g", rec->conicWeight());
       }
       SkDebugf("\n");
    }
}
}  // END FIDDLE
