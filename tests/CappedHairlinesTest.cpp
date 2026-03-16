/*
 * Copyright 2026 Google Inc.
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
DEF_TEST(HairlineClosedContourCapOffset_Draw, reporter) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(70, 70);
    auto surface = SkSurfaces::Raster(info);
    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
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
    if (surface->peekPixels(&pixmap)) {
        SkPoint firstPoint = {5.0, 5.0};
        SkColor firstPointColor = pixmap.getColor(firstPoint.x(), firstPoint.y());

        REPORTER_ASSERT(reporter,
                        SkColorGetR(firstPointColor) > 0 && SkColorGetR(firstPointColor) < 255);

        SkPoint firstPointOffset = {4.0, 5.0};
        SkColor firstPointOffsetColor = pixmap.getColor(firstPointOffset.x(), firstPointOffset.y());

        REPORTER_ASSERT(
                reporter,
                SkColorGetR(firstPointOffsetColor) > 0 && SkColorGetR(firstPointOffsetColor) < 255);

        SkPoint secondPoint = {5.0, 10.0};
        SkColor secondPointColor = pixmap.getColor(secondPoint.x(), secondPoint.y());

        REPORTER_ASSERT(reporter, secondPointColor == SK_ColorBLACK);

        // testing for anti aliasing

        printf("firstPoint at (%f, %f): %08X\n", firstPoint.x(), firstPoint.y(), firstPointColor);
        printf("firstPointOffset at (%f, %f): %08X\n",
               firstPointOffset.x(),
               firstPointOffset.y(),
               firstPointOffsetColor);

        printf("secondPoint at (%f, %f): %08X\n",
               secondPoint.x(),
               secondPoint.y(),
               secondPointColor);
    } else {
        REPORTER_ASSERT(reporter, false, "Could not read pixels");
    }
}
