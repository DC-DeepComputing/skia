/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkSurface.h"
#include "tests/Test.h"

#include <cmath>

static inline sk_sp<SkSurface> CreateTestSurface() {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(70, 70);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    if (surface) {
        surface->getCanvas()->clear(SK_ColorWHITE);
    }
    return surface;
}

static inline SkPaint CreateTestPaint() {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    return paint;
}

void InspectPixelColorAtPoint(skiatest::Reporter* reporter,
                              const SkPixmap& pixmap,
                              const std::string& testName,
                              SkPoint pt,
                              SkColor expected) {
    skiatest::ReporterContext subtest(reporter, testName);
    int x = (int)pt.x();
    int y = (int)pt.y();
    SkColor actual = pixmap.getColor(x, y);
    REPORTER_ASSERT(reporter,
                    actual == expected,
                    "%s: actual color (%x) != expected color (%x) for pixel at (%f, %f)",
                    testName.c_str(),
                    actual,
                    expected,
                    pt.x(),
                    pt.y());
}

DEF_TEST(DrawAntialiasHairline_SquareCappedHalfPixelOffset_SinglePixelWideOpenContourOutput,
         reporter) {
    sk_sp<SkSurface> surface = CreateTestSurface();
    REPORTER_ASSERT(reporter, surface);
    if (!surface) return;
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint = CreateTestPaint();
    paint.setStrokeCap(SkPaint::kSquare_Cap);

    SkPath path = SkPathBuilder()
                          .moveTo(5, 5)
                          .lineTo(5, 10)
                          .lineTo(10, 10)
                          .lineTo(10, 5)
                          .detach()
                          .makeOffset(0.5, 0.5);

    canvas->drawPath(path, paint);

    SkPixmap pixmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pixmap));

    InspectPixelColorAtPoint(reporter, pixmap, "above first point", {5.0, 4.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "left of first point", {4.0, 5.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "on first point", {5.0, 5.0}, SK_ColorBLACK);
    InspectPixelColorAtPoint(reporter, pixmap, "right of first point", {6.0, 5.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "below first point", {5.0, 6.0}, SK_ColorBLACK);

    InspectPixelColorAtPoint(reporter, pixmap, "above second point", {5.0, 9.0}, SK_ColorBLACK);
    InspectPixelColorAtPoint(reporter, pixmap, "left of second point", {4.0, 10.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "on second point", {5.0, 10.0}, 0xFF404040);
    InspectPixelColorAtPoint(reporter, pixmap, "right of second point", {6.0, 10.0}, SK_ColorBLACK);
    InspectPixelColorAtPoint(reporter, pixmap, "below second point", {5.0, 11.0}, SK_ColorWHITE);

    InspectPixelColorAtPoint(reporter, pixmap, "above third point", {10.0, 9.0}, SK_ColorBLACK);
    InspectPixelColorAtPoint(reporter, pixmap, "left of third point", {9.0, 10.0}, SK_ColorBLACK);
    InspectPixelColorAtPoint(reporter, pixmap, "on third point", {10.0, 10.0}, 0xFF404040);
    InspectPixelColorAtPoint(reporter, pixmap, "right of third point", {11.0, 10.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "below third point", {10.0, 11.0}, SK_ColorWHITE);

    InspectPixelColorAtPoint(reporter, pixmap, "above final point", {10.0, 4.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "left of final point", {9.0, 5.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "on final point", {10.0, 5.0}, SK_ColorBLACK);
    InspectPixelColorAtPoint(reporter, pixmap, "right of final point", {11.0, 5.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "below final point", {10.0, 6.0}, SK_ColorBLACK);
}

DEF_TEST(DrawAntialiasHairline_SquareCappedNoPixelOffset_DoublePixelWideOpenContourOutput,
         reporter) {
    sk_sp<SkSurface> surface = CreateTestSurface();
    REPORTER_ASSERT(reporter, surface);
    if (!surface) return;
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint = CreateTestPaint();
    paint.setStrokeCap(SkPaint::kSquare_Cap);

    SkPath path = SkPathBuilder().moveTo(5, 5).lineTo(5, 10).lineTo(10, 10).lineTo(10, 5).detach();

    canvas->drawPath(path, paint);

    SkPixmap pixmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pixmap));

    InspectPixelColorAtPoint(reporter, pixmap, "above first point", {5.0, 4.0}, 0xffbfbfbf);
    InspectPixelColorAtPoint(reporter, pixmap, "left of first point", {4.0, 5.0}, 0xff808080);
    InspectPixelColorAtPoint(reporter, pixmap, "on first point", {5.0, 5.0}, 0xff7f7f7f);
    InspectPixelColorAtPoint(reporter, pixmap, "right of first point", {6.0, 5.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "below first point", {5.0, 6.0}, 0xff7f7f7f);

    InspectPixelColorAtPoint(reporter, pixmap, "above second point", {5.0, 9.0}, 0xff3f3f3f);
    InspectPixelColorAtPoint(reporter, pixmap, "left of second point", {4.0, 10.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "on second point", {5.0, 10.0}, 0xff7f7f7f);
    InspectPixelColorAtPoint(reporter, pixmap, "right of second point", {6.0, 10.0}, 0xff7f7f7f);
    InspectPixelColorAtPoint(reporter, pixmap, "below second point", {5.0, 11.0}, SK_ColorWHITE);

    InspectPixelColorAtPoint(reporter, pixmap, "above third point", {10.0, 9.0}, 0xff7f7f7f);
    InspectPixelColorAtPoint(reporter, pixmap, "left of third point", {9.0, 10.0}, 0xff7f7f7f);
    InspectPixelColorAtPoint(reporter, pixmap, "on third point", {10.0, 10.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "right of third point", {11.0, 10.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "below third point", {10.0, 11.0}, SK_ColorWHITE);

    InspectPixelColorAtPoint(reporter, pixmap, "above final point", {10.0, 4.0}, 0xffbfbfbf);
    InspectPixelColorAtPoint(reporter, pixmap, "left of final point", {9.0, 5.0}, 0xff808080);
    InspectPixelColorAtPoint(reporter, pixmap, "on final point", {10.0, 5.0}, 0xff7f7f7f);
    InspectPixelColorAtPoint(reporter, pixmap, "right of final point", {11.0, 5.0}, SK_ColorWHITE);
    InspectPixelColorAtPoint(reporter, pixmap, "below final point", {10.0, 6.0}, 0xff7f7f7f);
}
