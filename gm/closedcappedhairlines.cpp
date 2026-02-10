/*
 * Copyright 2013 Google Inc.
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
#include "include/private/base/SkTArray.h"

using namespace skia_private;

namespace skiagm {

static void draw_closed_hairline_with_capped_hairlines_tests(SkCanvas* canvas, SkPaint paint) {
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
            SkPathBuilder().moveTo(40.5, 5.5).cubicTo(35, 25, 45, 20, 50.5, 5.5).close().detach(),
            paint);

    canvas->drawPath(
            SkPathBuilder().moveTo(60.5, 5.5).conicTo(65, 25, 70.5, 5.5, 50).close().detach(),
            paint);
}

DEF_SIMPLE_GM(squarehair, canvas, 240, 360) {
    const bool aliases[] = {false, true};
    const SkScalar widths[] = {0, 0.999f, 1, 1.001f};
    const SkPaint::Cap caps[] = {SkPaint::kButt_Cap, SkPaint::kSquare_Cap, SkPaint::kRound_Cap};
    // draw aliased on left side, and anti-aliased on right side
    for (auto alias : aliases) {
        canvas->save();
        for (auto width : widths) {
            for (auto cap : caps) {
                SkPaint paint;
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setStrokeCap(cap);
                paint.setStrokeWidth(width);
                paint.setAntiAlias(alias);
                paint.setColor(SK_ColorBLACK);
                draw_closed_hairline_with_capped_hairlines_tests(canvas, paint);
                canvas->translate(0, 30);
            }
        }
        canvas->restore();
        canvas->translate(120, 0);
    }
}

}  // namespace skiagm
