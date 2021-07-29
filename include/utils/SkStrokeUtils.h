
/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrokeUtils_DEFINED
#define SkStrokeUtils_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

class SK_API SkStrokeUtils {
public:
    static SkPath StrokeAndFill(const SkPath&, SkScalar width,
                                SkPaint::Cap cap = SkPaint::kButt_Cap,
                                SkPaint::Join = SkPaint::kMiter_Join,
                                SkScalar miter = 4);

};

#endif
