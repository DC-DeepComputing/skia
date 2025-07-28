// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_equal_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& a, const SkPath& b) -> void {
                SkDebugf("%s one %c= two\n", prefix, a == b ? '=' : '!');
    };
    SkPathBuilder one;
    SkPathBuilder two;
    debugster("empty", one.snapshot(), two.snapshot());
    one.moveTo(0, 0);
    debugster("moveTo", one.snapshot(), two.snapshot());
    one.moveTo(0, 0);
    one.reset();
    debugster("reset", one.snapshot(), two.snapshot());
}
}  // END FIDDLE
