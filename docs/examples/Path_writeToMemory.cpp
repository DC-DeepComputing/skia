// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_writeToMemory, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path = SkPathBuilder()
        .lineTo(6.f / 7, 2.f / 3)
        .detach();
    size_t size = path.writeToMemory(nullptr);
    SkTDArray<char> storage;
    storage.resize(size);
    path.writeToMemory(storage.begin());
    SkPath copy = SkPath::ReadFromMemory(storage.begin(), size).value();
    SkDebugf("path is " "%s" "equal to copy\n", path == copy ? "" : "not ");
}
}  // END FIDDLE
