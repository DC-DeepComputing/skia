/*
 * Copyright 2026 Google Inc.
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
#include "tools/viewer/ZoomInSlide.h"

namespace skiagm {

static constexpr size_t kScale = 4;

static constexpr int kMaskWidth = 80;
static constexpr int kMaskHeight = 80;

void drawScaledPath(SkCanvas* canvas, const SkPath& path, const SkPaint& pathPaint) {
    auto ii = SkImageInfo::MakeN32Premul(kMaskWidth, kMaskHeight);
    auto surface = SkSurfaces::Raster(ii);
    SkASSERT_RELEASE(surface);

    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
    surface->getCanvas()->drawPath(path, pathPaint);
    auto pathImg = surface->makeImageSnapshot();

    canvas->drawImage(pathImg, 0, 0);
}

void drawGrid(SkCanvas* canvas) {
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

static SkPath draw_closed_hairline_with_capped_hairlines_tests() {
    SkPathBuilder pb;
    pb.moveTo(5.5, 5.5).lineTo(5.5, 10.5).lineTo(10.5, 10.5).lineTo(10.5, 5.5).close();

    pb.moveTo(20.5, 5.5).quadTo(25, 15, 30.5, 5.5).close();

    pb.moveTo(40.5, 5.5).cubicTo(35, 25, 45, 20, 50.5, 5.5).close();

    pb.moveTo(60.5, 5.5).conicTo(65, 25, 70.5, 5.5, 50).close();

    pb.moveTo(5.5, 40);

    pb.moveTo(5.5, 45.5).lineTo(5.5, 50.5).lineTo(10.5, 60.5).lineTo(10.5, 65.5);

    pb.moveTo(20.5, 45.5).quadTo(25, 15, 30.5, 45.5);

    pb.moveTo(40.5, 45.5).cubicTo(35, 25, 45, 20, 50.5, 45.5);

    pb.moveTo(60.5, 45.5).conicTo(65, 25, 70.5, 45.5, 50);

    return pb.detach();
}

DEF_SIMPLE_GM_CAN_FAIL(closecappedhairlinetest, canvas, errorMsg, 100, 500) {
    canvas->scale(kScale, kScale);
    canvas->clear(SK_ColorWHITE);
    const SkScalar widths[] = {0, 0.999f, 1, 1.001f};
    const SkPaint::Cap caps[] = {SkPaint::kButt_Cap, SkPaint::kSquare_Cap, SkPaint::kRound_Cap};
    canvas->save();
    for (auto width : widths) {
        canvas->save();
        int i = 0;
        for (auto cap : caps) {
            auto surface = canvas->makeSurface(canvas->imageInfo().makeWH(80, 80));
            if (!surface) {
                *errorMsg = "Could not make subsurface";
                return DrawResult::kSkip;
            }
            SkPaint paint;

            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeCap(cap);
            paint.setStrokeWidth(width);
            paint.setColor(SK_ColorBLACK);
            paint.setAntiAlias(true);

            drawScaledPath(canvas, draw_closed_hairline_with_capped_hairlines_tests(), paint);
        }
        i++;
        canvas->restore();
        drawGrid(canvas);
        canvas->translate(0, 100);
    }
    canvas->restore();

    return skiagm::DrawResult::kOk;
}

}  // namespace skiagm
