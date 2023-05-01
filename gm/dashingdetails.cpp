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

float sum(SkSpan<float> intervals) {
    float sum = 0.0f;
    for (auto i : intervals) {
        sum += i;
    }
    return sum;
}

//
void helper(SkCanvas* canvas, const SkPoint line[2], SkSpan<float> intervals,
            float strokeWidth, SkPaint::Cap cap) {
    SkAutoCanvasRestore acr(canvas, /*doSave=*/ true);

    canvas->translate(0.0f, strokeWidth/2.0f);

    SkPath path = SkPathBuilder().moveTo(line[0]).lineTo(line[1]).detach();

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setStrokeWidth(strokeWidth);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeCap(cap);
    paint.setAntiAlias(true);

    paint.setPathEffect(SkDashPathEffect::Make(intervals.data(), intervals.size(), 0.0f));
    canvas->drawPath(path, paint);

    SkPaint red;
    red.setColor(SK_ColorRED);

    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, line, red);
}

float draw1(SkCanvas* canvas) {
    constexpr float kStrokeWidth = 5;

    float intervals[2] = { kStrokeWidth, kStrokeWidth };
    //float len = sum(intervals);
    SkPoint pts[2] = { { kStrokeWidth, 0.0f }, { 4*kStrokeWidth, 0.0f }};

    helper(canvas, pts, intervals, kStrokeWidth, SkPaint::kButt_Cap);
    return kStrokeWidth;
}

float draw2(SkCanvas* canvas) {
    constexpr float kStrokeWidth = 5;

    float intervals[2] = { 0.0f, 10.0f };
    //float len = sum(intervals);
    SkPoint pts[2] = { { kStrokeWidth, 0.0f }, { 3*kStrokeWidth+0.001f, 0.0f }};

    helper(canvas, pts, intervals, kStrokeWidth, SkPaint::kSquare_Cap);
    return kStrokeWidth;
}

} // anonymous namespace


namespace skiagm {

// Test out the finicky details of dashing
class DashingDetailsGM : public GM {
public:
    static constexpr int kWidth = 32;
    static constexpr int kHeight = 32;

    DashingDetailsGM() { this->setBGColor(SK_ColorBLACK); }

protected:
    SkString onShortName() override { return SkString("dashingdetails"); }

    SkISize onISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(0.5f, 1.5f);
        float height = draw1(canvas);
        canvas->translate(0.0f, height+1.0f);
        height = draw2(canvas);
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new DashingDetailsGM;)

}  // namespace skiagm
