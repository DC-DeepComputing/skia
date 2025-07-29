// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_copy_const_SkPath, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path = SkPathBuilder().lineTo(20, 20).detach();
    SkPath path2(path);
#ifndef SK_HIDE_PATH_EDIT_METHODS
    path2.close();
    SkDebugf("path verbs: %d\n", path.countVerbs());
    SkDebugf("path2 verbs: %d\n", path2.countVerbs());
    path.reset();
#endif
    SkDebugf("after reset\n" "path verbs: %d\n", path.countVerbs());
    SkDebugf("path2 verbs: %d\n", path2.countVerbs());
}
}  // END FIDDLE
