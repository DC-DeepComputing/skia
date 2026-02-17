/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"

static constexpr size_t kScale = 4;

static constexpr int kMaskWidth = 80;
static constexpr int kMaskHeight = 80;

static void draw_scaled_path(SkCanvas* canvas, const SkPath& path, const SkPaint& pathPaint) {
    auto ii = SkImageInfo::MakeN32Premul(kMaskWidth, kMaskHeight);
    auto surface = SkSurfaces::Raster(ii);
    SkASSERT_RELEASE(surface);

    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
    surface->getCanvas()->drawPath(path, pathPaint);
    auto pathImg = surface->makeImageSnapshot();

    canvas->drawImage(pathImg, 0, 0);
}

static void draw_grid(SkCanvas* canvas) {
    SkPaint gridPaint;
    gridPaint.setColor(SK_ColorDKGRAY);
    gridPaint.setStyle(SkPaint::Style::kStroke_Style);
    gridPaint.setStrokeWidth(0);

    for (int y = 0; y <= (int)kMaskHeight; ++y) {
        canvas->drawLine(0, y, kMaskWidth, y, gridPaint);
    }
    for (int x = 0; x <= (int)kMaskWidth; ++x) {
        canvas->drawLine(x, 0, x, kMaskHeight, gridPaint);
    }
}

// We would like to test the ability to draw closed and open contours with
// capped hairlines. Capped hairlines have interesting behavior because they extend out
// by about half a pixel, which has led to unintended behavior before when closing
// contours with antialiasing on. The contours created here represent closed contours
// with all the simple path types, followed by the open version of the contours.
static SkPath make_path_for_contours_with_capped_hairlines() {
    SkPathBuilder pb;

    // ideally the contours here should have a "sharp" close line, with no effect from
    // the antialiasing
    pb.moveTo(5.5, 5.5).lineTo(5.5, 10.5).lineTo(10.5, 10.5).lineTo(10.5, 5.5).close();

    pb.moveTo(20, 5).quadTo(40, 15, 20, 20).close();

    pb.moveTo(50, 5).cubicTo(40, 15, 15, 40, 50, 20).close();

    pb.moveTo(65.5, 10.5).lineTo(65.5, 5.5).lineTo(70.5, 5.5).lineTo(70.5, 10.5).close();

    // ideally the contours here should have noticable capped ends, which are antialiased
    pb.moveTo(5.5, 45.5).lineTo(5.5, 50.5).lineTo(10.5, 50.5).lineTo(10.5, 45.5);

    pb.moveTo(20, 45).quadTo(40, 55, 20, 60);

    pb.moveTo(50, 45).cubicTo(40, 55, 15, 80, 50, 60);

    pb.moveTo(65.5, 50.5).lineTo(65.5, 45.5).lineTo(70.5, 45.5).lineTo(70.5, 50.5).close();

    return pb.detach();
}

DEF_SIMPLE_GM(closedcappedhairline, canvas, 100, 500) {
    const SkPaint::Cap caps[] = {SkPaint::kButt_Cap, SkPaint::kSquare_Cap, SkPaint::kRound_Cap};
    canvas->save();

    for (auto cap : caps) {
        canvas->save();
        SkPaint paint;

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeCap(cap);
        paint.setStrokeWidth(0);
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);

        canvas->drawPath(make_path_for_contours_with_capped_hairlines(), paint);
        canvas->restore();
        canvas->translate(0, 400);
    }

    canvas->restore();
    canvas->translate(100, 0);
    canvas->scale(kScale, kScale);

    canvas->save();

    for (auto cap : caps) {
        canvas->save();
        SkPaint paint;

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeCap(cap);
        paint.setStrokeWidth(0);
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);

        draw_scaled_path(canvas, make_path_for_contours_with_capped_hairlines(), paint);
        canvas->restore();
        draw_grid(canvas);
        canvas->translate(0, 100);
    }

    canvas->restore();
}
