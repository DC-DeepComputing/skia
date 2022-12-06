/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

DEF_SIMPLE_GM(crbug_1395402, canvas, 50, 50) {
    canvas->clear(SK_ColorWHITE);

    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeCap(SkPaint::kRound_Cap);
    p.setStrokeWidth(1);

    {
        SkPath path;
        path.moveTo(10.49f, 10.5f);
        path.lineTo(10.51f, 10.5f);

        canvas->drawPath(path, p);
    }

    canvas->translate(0, 20);

    {
        SkPath path;
        path.moveTo(10.49f, 10.5f);
        path.lineTo(10.51f, 10.5f);

        path.moveTo(20.f, 10.5f);
        path.lineTo(30.f, 10.5f);

        canvas->drawPath(path, p);
    }
}
