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
    SkPath symbol;
    symbol.moveTo(622.812f, 360.614f);
    symbol.cubicTo(617.008f, 362.4f,
                   603.937f, 363.27f,
                   600.595f, 362.84f);
    symbol.cubicTo(600.673f, 366.208f,
                   598.442f, 379.117f,
                   596.059f, 384.702f);
    symbol.cubicTo(601.862f, 382.916f,
                   614.934f, 382.046f,
                   618.276f, 382.476f);
    symbol.cubicTo(618.198f, 379.108f,
                   620.429f, 366.199f,
                   622.812f, 360.614f);
    symbol.close();

    SkPath rotatedRect;
    rotatedRect.moveTo(613.297f, 350.046f);
    rotatedRect.lineTo(631.765f, 370.557f);
    rotatedRect.lineTo(605.012f, 394.646f);
    rotatedRect.lineTo(586.544f, 374.135f);
    rotatedRect.lineTo(613.297f, 350.046f);
    rotatedRect.close();

    SkDebugf("clip rect 1\n");
    canvas->clipRect({586.f, 350.f, 632.f, 395.f}, true); // scissor'able (bbox of rotatedRect...)
    SkDebugf("clip rotated rect\n");
    canvas->clipPath(rotatedRect, true); // ugh

    SkDebugf("clip rect 2\n");
    canvas->clipRect({596.f, 360.f, 623.f, 385.f}, true); // scissor'able again (bbox of symbol...)
    SkDebugf("clip symbol\n");
    canvas->clipPath(symbol, true);

    canvas->concat(SkMatrix::MakeAll(0.802957f, -0.891774f, 610.888f,
                                     0.891774f, 0.802957f, 347.371f,
                                     0.f, 0.f, 1.f));

    SkPaint paint;
    paint.setColor4f(SkColors::kRed);
    paint.setAntiAlias(true);
    SkDebugf("draw rect\n");
    canvas->drawRect(SkRect::MakeWH(30.f, 30.f), paint);
}
