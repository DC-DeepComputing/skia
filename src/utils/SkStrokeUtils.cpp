/*
* Copyright 2021 Google LLC.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/utils/SkStrokeUtils.h"

SkPath SkStrokeUtils::StrokeAndFill(const SkPath& src, SkScalar width,
                                    SkPaint::Cap cap, SkPaint::Join join, SkScalar miter) {
    SkPaint paint;
    paint.setStroke(true);
    paint.setStrokeWidth(width);
    paint.setStrokeCap(cap);
    paint.setStrokeJoin(join);
    paint.setStrokeMiter(miter);

    SkPath dst;
    paint.getFillPath(src, &dst, nullptr);
    dst.addPath(src);
    return dst;
}
