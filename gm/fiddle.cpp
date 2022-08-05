/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

// Paste your fiddle.skia.org code over this stub.
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);

    SkPath path, result;

    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(200.f, 200.f);
    path.lineTo(164.f, 459.f); // s3
    path.quadTo(222.f, 493.f, 299.f, 493.f);
    path.quadTo(342.f, 493.f, 378.f, 485.f); // s4
    path.close();

    path.moveTo(415.698f, 499.f); // s17, s18
    path.lineTo(236.698f, 469.f); // s20
    path.lineTo(236.698f, 493.f); //
    path.lineTo(298.698f, 493.f); // s21
    // path.quadTo(337.698f, 493.f, 359.698f, 498.f);
    path.lineTo(337.698f, 493.f);
    path.close();

    bool ok = Simplify(path, &result);
    SkASSERT(ok);

    SkDebugf("original path\n");
    path.dump();
    SkDebugf("simplified to\n");
    result.dump();

    canvas->drawPath(path, paint);
}
