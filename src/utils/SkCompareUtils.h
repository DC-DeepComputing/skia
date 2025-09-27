/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skia_compare_utils_DEFINED
#define skia_compare_utils_DEFINED

#include "include/core/SkTypes.h"

#include <functional>

class SkPath;

namespace skia_compare {
    // Called if pixels differ. If true if the comparison should continue, false to stop.
    using A8CompareProc = std::function<bool(int x, int y, uint8_t pixela, uint8_t pixelb)>;

    // Compare two paths, drawn in antialiasing. If dimensions differ, returns false.
    //
    // If any pixels diff, they are passed to cmp(), and if that ever returns false,
    // this returns false. If cmp is never called, or it always returns true, then this
    // returns true.
    //
    bool A8ComparePaths(const SkPath& a, const SkPath& b, A8CompareProc cmp);
}

#endif
