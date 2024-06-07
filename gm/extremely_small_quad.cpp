/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

DEF_SIMPLE_GM(extremely_small_quad, canvas, 1024, 1024) {
    canvas->save();
    canvas->scale(10, 10);
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(5);
    SkPath path;
    path.moveTo(12, 24);
    path.quadTo(12, 23.9999981f, 11.999999f, 23.9999943f);
    canvas->drawPath(path, p);

    canvas->translate(20, 0);
    p.setAntiAlias(true);
    canvas->drawPath(path, p);
    canvas->restore();
}
