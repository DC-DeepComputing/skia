// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_readFromMemory, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path = SkPathBuilder()
        .lineTo(6.f / 7, 2.f / 3)
        .detach();
    size_t size = path.writeToMemory(nullptr);
    SkTDArray<char> storage;
    storage.resize(size);
    path.writeToMemory(storage.begin());
    size_t wrongSize = size - 4;
    size_t bytesRead;
    std::optional<SkPath> copy = SkPath::ReadFromMemory(storage.begin(), wrongSize, &bytesRead);
    SkDebugf("length = %zu; returned by readFromMemory = %zu\n", wrongSize, bytesRead);
    size_t largerSize = size + 4;
    copy = SkPath::ReadFromMemory(storage.begin(), largerSize, &bytesRead);
    SkDebugf("length = %zu; returned by readFromMemory = %zu\n", largerSize, bytesRead);
}
}  // END FIDDLE
