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
    SkRect rect = {100, 100, 300, 300};
    SkPaint paint;
    paint.setColor(0xffff0000);
    paint.setStroke(true);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1.5);

    SkRect layerBounds = rect;

    SkPath rectPath, layerBoundsPath;
    rectPath.addRect(rect);
    skpathutils::FillPathWithPaint(rectPath, paint, &layerBoundsPath);
    layerBounds = layerBoundsPath.getBounds();
    SkDebugf("path bounds = %f %f %f %f\n", layerBounds.fLeft, layerBounds.fTop, layerBounds.fRight, layerBounds.fBottom);

    auto dropshadow = SkImageFilters::DropShadowOnly(0,1, 15.0f, 15.0f, SK_ColorBLACK, nullptr);
    SkPaint layerPaint;
    layerPaint.setImageFilter(dropshadow);

    // if this is enabled, artifacts is lesser
    // layerBounds = layerPaint.computeFastBounds(layerBounds, &layerBounds);

    SkDebugf("drawRect with blur\n");
    canvas->saveLayer(layerBounds, &layerPaint);
    canvas->drawRect(rect, paint);
    canvas->restore();

    SkDebugf("drawRect without blur\n");
    canvas->saveLayer(layerBounds, nullptr);
    canvas->drawRect(rect, paint);
    canvas->restore();

    // canvas->translate(220.f, 0.f);
    // canvas->saveLayer(nullptr, &layerPaint);
    // canvas->drawRect(rect, paint);
    // canvas->restore();

    // canvas->translate(220.f, 0.f);
    // layerBounds = paint.computeFastBounds(rect, &layerBounds);
    // SkDebugf("fast bounds = %f %f %f %f\n", layerBounds.fLeft, layerBounds.fTop, layerBounds.fRight, layerBounds.fBottom);
    // canvas->saveLayer(layerBounds, &layerPaint);
    // canvas->drawRect(rect, paint);
    // canvas->restore();
}
