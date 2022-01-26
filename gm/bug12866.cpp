/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

static SkPath get_path() {
    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(SkBits2Float(0x45034ec4), SkBits2Float(0x42e7fb80));  // 2100.92f, 115.991f
    path.quadTo(SkBits2Float(0x4500f46c),
                SkBits2Float(0x43333300),
                SkBits2Float(0x4500f46c),
                SkBits2Float(0x431f0ec0));  // 2063.28f, 179.199f, 2063.28f, 159.058f
    path.quadTo(SkBits2Float(0x4500f46c),
                SkBits2Float(0x430ad7c0),
                SkBits2Float(0x45019462),
                SkBits2Float(0x42fed580));  // 2063.28f, 138.843f, 2073.27f, 127.417f
    path.quadTo(SkBits2Float(0x45023458),
                SkBits2Float(0x42e7fb80),
                SkBits2Float(0x45034ec4),
                SkBits2Float(0x42e7fb80));  // 2083.27f, 115.991f, 2100.92f, 115.991f
    path.close();
    return path;
}

// Reproduces skbug.com/12866, drawing differently (incorrectly) on CPU vs. GPU
DEF_SIMPLE_GM(bug12866, canvas, 64, 64) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(3);

    SkM44 m(
      -0.0000530114,      -1212.76, 640,  1005270,
            1212.76, -0.0000530114,	480, -1722010,
                  0,             0,  -1, -2425.52,
                  0,             0,   1,  2426.52
    );

    canvas->translate(-300, -300);
    canvas->concat(m);
    canvas->drawPath(get_path(), p);
}
