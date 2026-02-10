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

namespace skiagm {

static void draw_closed_hairline_with_capped_hairlines_tests(SkCanvas* canvas, SkPaint paint) {
    canvas->save();
    canvas->drawPath(SkPathBuilder()
                             .moveTo(5.5, 5.5)
                             .lineTo(5.5, 10.5)
                             .lineTo(10.5, 10.5)
                             .lineTo(10.5, 5.5)
                             .close()
                             .detach(),
                     paint);

    canvas->drawPath(SkPathBuilder().moveTo(20.5, 5.5).quadTo(25, 15, 30.5, 5.5).close().detach(),
                     paint);

    canvas->drawPath(
            SkPathBuilder().moveTo(40.5, 5.5).cubicTo(35, 25, 45, 20, 50.5, 5.5).close().detach(),
            paint);

    canvas->drawPath(
            SkPathBuilder().moveTo(60.5, 5.5).conicTo(65, 25, 70.5, 5.5, 50).close().detach(),
            paint);

    canvas->restore();
    canvas->translate(0, 40);

    canvas->drawPath(SkPathBuilder()
                             .moveTo(5.5, 5.5)
                             .lineTo(5.5, 10.5)
                             .lineTo(10.5, 10.5)
                             .lineTo(10.5, 5.5)
                             .detach(),
                     paint);

    canvas->drawPath(SkPathBuilder().moveTo(20.5, 5.5).quadTo(25, 15, 30.5, 5.5).close().detach(),
                     paint);

    canvas->drawPath(
            SkPathBuilder().moveTo(40.5, 5.5).cubicTo(35, 25, 45, 20, 50.5, 5.5).close().detach(),
            paint);

    canvas->drawPath(
            SkPathBuilder().moveTo(60.5, 5.5).conicTo(65, 25, 70.5, 5.5, 50).close().detach(),
            paint);
}

class ClosedCappedHairlinesGM : public GM {
protected:
    SkString getName() const override { return SkString("ClosedCappedHairlinesGM"); }

    SkISize getISize() override { return SkISize::Make(1250, 1250); }

    void drawScaledPath(SkCanvas* canvas, const SkPath& path, const SkPaint& pathPaint) {
        auto ii = SkImageInfo::MakeN32Premul(fWidth, fHeight);
        auto surface = SkSurfaces::Raster(ii);
        SkASSERT_RELEASE(surface);

        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        surface->getCanvas()->drawPath(path, pathPaint);
        auto pathImg = surface->makeImageSnapshot();

        canvas->drawImage(pathImg, 0, 0);
    }

    void drawGrid(SkCanvas* canvas) {
        const SkScalar widths[] = {0, 0.999f, 1, 1.001f};
        const SkPaint::Cap caps[] = {SkPaint::kButt_Cap, SkPaint::kSquare_Cap, SkPaint::kRound_Cap};

        for (auto width : widths) {
            canvas->save();
            for (auto cap : caps) {
                SkPaint paint;
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setStrokeCap(cap);
                paint.setStrokeWidth(width);
                paint.setColor(SK_ColorBLACK);
                draw_closed_hairline_with_capped_hairlines_tests(canvas, paint);
                canvas->translate(0, 30);
            }
            canvas->restore();
            canvas->translate(120, 0);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(fScale, fScale);
        canvas->clear(SK_ColorWHITE);

        this->drawGrid(canvas);
    }

private:
    size_t fScale;
    size_t fWidth;
    size_t fHeight;
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ClosedCappedHairlinesGM)

}  // namespace skiagm
