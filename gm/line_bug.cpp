/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkScalar.h"
#include "include/private/SkFloatBits.h"
#include "include/private/SkPathRef.h"
#include "src/core/SkPathPriv.h"

#define W   800
#define H   800

DEF_SIMPLE_GM(line_bug, canvas, W, H) {
    SkPaint p;
    p.setColor(SK_ColorMAGENTA);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(1);

    // canvas->drawLine(100, 100, 100, -10000000000, p);
    SkPath path;
    path.moveTo(100,100);
    path.lineTo(100,-10000000000);
    canvas->drawPath(path, p);
}
