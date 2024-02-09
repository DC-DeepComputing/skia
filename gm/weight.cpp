/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"

DEF_SIMPLE_GM(weight, canvas, 256, 256) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);
    SkPath path;
    SkRect r{50, 50, 200, 200};
    SkRRect rr;
    const SkVector radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 50, 50 } };
    rr.setRectRadii(r, radii);
    path.addRRect(rr);
    //path.addRect(r);
    canvas->drawPath(path, p);
}
