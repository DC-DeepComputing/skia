/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"

// https://crbug.com/772953
DEF_SIMPLE_GM(circle_sizes, canvas, 128, 128) {
    SkPaint p;
    p.setAntiAlias(true);
    for (int i = 0; i < 16; ++i) {
        canvas->drawCircle({14.0f + 32.0f * (i % 4),
                            14.0f + 32.0f * (i / 4)}, i + 1.0f, p);
    }
}

using DrawCircleFn = void(*)(SkCanvas*, SkPoint, SkScalar, const SkPaint&);

void draw_circle(SkCanvas* canvas, SkPoint center, SkScalar radius, const SkPaint& paint) {
    canvas->drawCircle(center, radius, paint);
}

void draw_circle_as_arc(SkCanvas* canvas, SkPoint center, SkScalar radius, const SkPaint& paint) {
    SkRect oval = SkRect::MakeLTRB(center.x() - radius, center.y() - radius,
                                   center.x() + radius, center.y() + radius);
    canvas->drawArc(SkArc::Make(oval, 0, 360, SkArc::Type::kArc), paint);
}

static void draw_tiny_circle_grid(SkCanvas* canvas, DrawCircleFn draw) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(3, 3);
    auto surface = canvas->makeSurface(info);
    if (!surface) {
        surface = SkSurfaces::Raster(info);
    }

    constexpr SkScalar radius = 0.8f;
    SkPaint paint;
    paint.setAntiAlias(true);

    canvas->translate(4, 4);

    for (int y = 0; y <= 16; ++y) {
        canvas->save();
        for (int x = 0; x <= 16; ++x) {
            float cx = 1.0f + (x / 16.0f);
            float cy = 1.0f + (y / 16.0f);
            surface->getCanvas()->clear(SK_ColorWHITE);
            draw(surface->getCanvas(), { cx, cy }, radius, paint);
            canvas->drawImageRect(
                    surface->makeImageSnapshot(), SkRect::MakeWH(24, 24), SkSamplingOptions{});
            canvas->translate(28, 0);
        }
        canvas->restore();
        canvas->translate(0, 28);
    }
}

DEF_SIMPLE_GM(tiny_circles, canvas, 480, 480) {
    draw_tiny_circle_grid(canvas, draw_circle);
}

DEF_SIMPLE_GM(tiny_circles_as_arcs, canvas, 480, 480) {
    draw_tiny_circle_grid(canvas, draw_circle_as_arc);
}
