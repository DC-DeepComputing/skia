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

static sk_sp<SkSurface> create_white_raster_surface() {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(70, 70);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    if (surface) {
        surface->getCanvas()->clear(SK_ColorWHITE);
    }
    return surface;
}

static SkPaint create_aa_hairline_paint() {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    return paint;
}

static void check_if_color_at_point_is_expected(skiatest::Reporter* reporter,
                                                const SkPixmap& pixmap,
                                                const std::string& testName,
                                                SkPoint pt,
                                                SkColor expected) {
    skiatest::ReporterContext subtest(reporter, testName);
    SkColor actual = pixmap.getColor(pt.x(), pt.y());

    REPORTER_ASSERT(reporter,
                    actual == expected,
                    "%s: actual color (%x) != expected color (%x) for pixel at (%f, %f)",
                    testName.c_str(),
                    actual,
                    expected,
                    pt.x(),
                    pt.y());
}

static void check_if_color_at_point_is_not_expected(skiatest::Reporter* reporter,
                                                    const SkPixmap& pixmap,
                                                    const std::string& testName,
                                                    SkPoint pt,
                                                    SkColor not_expected) {
    skiatest::ReporterContext subtest(reporter, testName);
    SkColor actual = pixmap.getColor(pt.x(), pt.y());

    REPORTER_ASSERT(reporter,
                    actual != not_expected,
                    "%s: actual color (%x) == not expected color (%x) for pixel at (%f, %f)",
                    testName.c_str(),
                    actual,
                    not_expected,
                    pt.x(),
                    pt.y());
}

DEF_TEST(DrawAntialiasHairline_SquareCappedNoPixelOffsetOpenContour_DoublePixelWideOutput,
         reporter) {
    sk_sp<SkSurface> surface = create_white_raster_surface();
    SkASSERT(surface);

    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint = create_aa_hairline_paint();
    paint.setStrokeCap(SkPaint::kSquare_Cap);

    SkPath path = SkPathBuilder().moveTo(5, 5).lineTo(5, 10).lineTo(10, 10).lineTo(10, 5).detach();

    canvas->drawPath(path, paint);

    SkPixmap pixmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pixmap));

    auto assert_color = [&](const std::string& name, SkPoint pt, SkColor expected) {
        check_if_color_at_point_is_expected(reporter, pixmap, name, pt, expected);
    };

    auto assert_not_color = [&](const std::string& name, SkPoint pt, SkColor expected) {
        check_if_color_at_point_is_not_expected(reporter, pixmap, name, pt, expected);
    };

    assert_not_color("above first point", {5.0, 4.0}, SK_ColorWHITE);
    assert_not_color("left of first point", {4.0, 5.0}, SK_ColorWHITE);
    assert_not_color("on first point", {5.0, 5.0}, SK_ColorWHITE);
    assert_color("right of first point", {6.0, 5.0}, SK_ColorWHITE);
    assert_not_color("below first point", {5.0, 6.0}, SK_ColorWHITE);

    assert_not_color("above second point", {5.0, 9.0}, SK_ColorWHITE);
    assert_color("left of second point", {4.0, 10.0}, SK_ColorWHITE);
    assert_not_color("on second point", {5.0, 10.0}, SK_ColorWHITE);
    assert_not_color("right of second point", {6.0, 10.0}, SK_ColorWHITE);
    assert_color("below second point", {5.0, 11.0}, SK_ColorWHITE);

    assert_not_color("above third point", {10.0, 9.0}, SK_ColorWHITE);
    assert_not_color("left of third point", {9.0, 10.0}, SK_ColorWHITE);
    assert_color("on third point", {10.0, 10.0}, SK_ColorWHITE);
    assert_color("right of third point", {11.0, 10.0}, SK_ColorWHITE);
    assert_color("below third point", {10.0, 11.0}, SK_ColorWHITE);

    assert_not_color("above final point", {10.0, 4.0}, SK_ColorWHITE);
    assert_not_color("left of final point", {9.0, 5.0}, SK_ColorWHITE);
    assert_not_color("on final point", {10.0, 5.0}, SK_ColorWHITE);
    assert_color("right of final point", {11.0, 5.0}, SK_ColorWHITE);
    assert_not_color("below final point", {10.0, 6.0}, SK_ColorWHITE);
}

DEF_TEST(DrawAntialiasHairline_SquareCappedHalfPixelOffsetOpenContour_SinglePixelWideOutput,
         reporter) {
    sk_sp<SkSurface> surface = create_white_raster_surface();
    SkASSERT(surface);

    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint = create_aa_hairline_paint();
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

    auto assert_color = [&](const std::string& name, SkPoint pt, SkColor expected) {
        check_if_color_at_point_is_expected(reporter, pixmap, name, pt, expected);
    };

    auto asset_not_color = [&](const std::string& name, SkPoint pt, SkColor expected) {
        check_if_color_at_point_is_not_expected(reporter, pixmap, name, pt, expected);
    };

    assert_color("above first point", {5.0, 4.0}, SK_ColorWHITE);
    assert_color("left of first point", {4.0, 5.0}, SK_ColorWHITE);
    assert_color("on first point", {5.0, 5.0}, SK_ColorBLACK);
    assert_color("right of first point", {6.0, 5.0}, SK_ColorWHITE);
    assert_color("below first point", {5.0, 6.0}, SK_ColorBLACK);

    assert_color("above second point", {5.0, 9.0}, SK_ColorBLACK);
    assert_color("left of second point", {4.0, 10.0}, SK_ColorWHITE);
    asset_not_color("on second point", {5.0, 10.0}, SK_ColorWHITE);
    assert_color("right of second point", {6.0, 10.0}, SK_ColorBLACK);
    assert_color("below second point", {5.0, 11.0}, SK_ColorWHITE);

    assert_color("above third point", {10.0, 9.0}, SK_ColorBLACK);
    assert_color("left of third point", {9.0, 10.0}, SK_ColorBLACK);
    asset_not_color("on third point", {10.0, 10.0}, SK_ColorWHITE);
    assert_color("right of third point", {11.0, 10.0}, SK_ColorWHITE);
    assert_color("below third point", {10.0, 11.0}, SK_ColorWHITE);

    assert_color("above final point", {10.0, 4.0}, SK_ColorWHITE);
    assert_color("left of final point", {9.0, 5.0}, SK_ColorWHITE);
    assert_color("on final point", {10.0, 5.0}, SK_ColorBLACK);
    assert_color("right of final point", {11.0, 5.0}, SK_ColorWHITE);
    assert_color("below final point", {10.0, 6.0}, SK_ColorBLACK);
}
