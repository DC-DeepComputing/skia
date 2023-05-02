/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/effects/SkDashPathEffect.h"

namespace {

//
void dash_helper(SkCanvas* canvas, const SkPoint line[2], SkSpan<float> intervals,
                float strokeWidth, SkPaint::Cap cap, float phase) {
    SkAutoCanvasRestore acr(canvas, /*doSave=*/ true);

    canvas->translate(0.0f, strokeWidth/2.0f);

    SkPath path = SkPathBuilder().moveTo(line[0]).lineTo(line[1]).detach();

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setStrokeWidth(strokeWidth);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeCap(cap);
    paint.setAntiAlias(true);

    paint.setPathEffect(SkDashPathEffect::Make(intervals.data(), intervals.size(), phase));
    canvas->drawPath(path, paint);

    SkPaint red;
    red.setColor(SK_ColorRED);
    red.setStrokeWidth(0.0f);
    red.setStroke(true);

    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, line, red);
}

void draw_cell(SkCanvas* canvas, float xOff, float yOff, bool halfPhase) {
    constexpr float kStrokeWidth = 5;

    float phase = halfPhase ? kStrokeWidth : 0.0f;

    SkAutoCanvasRestore acr(canvas, /*doSave=*/ true);

    canvas->translate(xOff, yOff);

    // Butt caps - 5-on/5-off
    {
        float intervals[2] = { kStrokeWidth, kStrokeWidth };
        //float len = sum(intervals);
        SkPoint pts[2] = { { kStrokeWidth, 0.0f }, { 4*kStrokeWidth, 0.0f }};

        dash_helper(canvas, pts, intervals, kStrokeWidth, SkPaint::kButt_Cap, phase);
    }

    canvas->translate(0, kStrokeWidth+2.0f);

    // Square caps - 0-on/10-off
    {
        float intervals[2] = { 0.0f, 2.0f * kStrokeWidth };
        //float len = sum(intervals);
        SkPoint pts[2] = { { kStrokeWidth, 0.0f }, { 3*kStrokeWidth+0.001f, 0.0f }};

        dash_helper(canvas, pts, intervals, kStrokeWidth, SkPaint::kSquare_Cap, phase);
    }
}

} // anonymous namespace

namespace skiagm {

// This creates blown-up versions of square and butt capped, 1on/1off dashed lines in the grid:
//     integer aligned      |   integer aligned w/ half phase
//    ----------------------|--------------------------------------
//     non-integer aligned  |   non-integer aligned w/ half phase
class DashingDetails1GM : public GM {
public:
    static constexpr int kWidth = 64;
    static constexpr int kHeight = 32;

    DashingDetails1GM() { this->setBGColor(SK_ColorBLACK); }

protected:
    SkString onShortName() override { return SkString("dashingdetails"); }

    SkISize onISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint lines;
        lines.setColor(SK_ColorWHITE);
        lines.setStrokeWidth(0.0f);
        lines.setStroke(true);

        canvas->drawLine(kWidth/2.0f, 0, kWidth/2.0f, kHeight, lines);
        canvas->drawLine(0.0f, kHeight/2.0f, kWidth, kHeight/2.0f, lines);

        draw_cell(canvas, 0.0f,            1.0f,             /* halfPhase= */ false);
        draw_cell(canvas, 0.5f,            kHeight/2 + 2.5f, /* halfPhase= */ false);
        draw_cell(canvas, kWidth/2,        1.0f,             /* halfPhase= */ true);
        draw_cell(canvas, kWidth/2 + 0.5f, kHeight/2 + 2.5f, /* halfPhase= */ true);
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new DashingDetails1GM;)

}  // namespace skiagm
