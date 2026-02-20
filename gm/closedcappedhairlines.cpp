/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "string"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

static SkPath make_path_for_hairline_line_contour(bool close);
static SkPath make_path_for_hairline_quad_contour(bool close);
static SkPath make_path_for_hairline_cubic_contour(bool close);

static constexpr size_t kScale = 4;

static constexpr int kMaskWidth = 50;
static constexpr int kMaskHeight = 70;
static constexpr int kRedBoxWH = 5;

static constexpr SkPaint::Cap caps[] = {
        SkPaint::kButt_Cap, SkPaint::kSquare_Cap, SkPaint::kRound_Cap};

// the paths to draw in order to set the hairline caps
// the parameters are
// * whether a path is open or closed (via. SkPathVerb::kClose)
// * whether the path is offset by a half pixel or not
// * the type of line used to draw the path
const SkPath linePathClosed = make_path_for_hairline_line_contour(true);
const SkPath quadPathClosed = make_path_for_hairline_quad_contour(true);
const SkPath cubicPathClosed = make_path_for_hairline_cubic_contour(true);
const SkPath offsetLinePathClosed = linePathClosed.makeOffset(0.5, 0.5);
const SkPath offsetQuadPathClosed = quadPathClosed.makeOffset(0.5, 0.5);
const SkPath offsetCubicPathClosed = cubicPathClosed.makeOffset(0.5, 0.5);
const SkPath linePathOpen = make_path_for_hairline_line_contour(false);
const SkPath quadPathOpen = make_path_for_hairline_quad_contour(false);
const SkPath cubicPathOpen = make_path_for_hairline_cubic_contour(false);
const SkPath offsetLinePathOpen = linePathOpen.makeOffset(0.5, 0.5);
const SkPath offsetQuadPathOpen = quadPathOpen.makeOffset(0.5, 0.5);
const SkPath offsetCubicPathOpen = cubicPathOpen.makeOffset(0.5, 0.5);

const SkPoint cappedPts[] = {
        linePathOpen.points().front(),
        linePathOpen.points().back(),
        quadPathOpen.points().front(),
        quadPathOpen.points().back(),
        cubicPathOpen.points().front(),
        cubicPathOpen.points().back(),
};
const SkPoint offsetCappedPts[] = {
        offsetLinePathOpen.points().front(),
        offsetLinePathOpen.points().back(),
        offsetQuadPathOpen.points().front(),
        offsetQuadPathOpen.points().back(),
        offsetCubicPathOpen.points().front(),
        offsetCubicPathOpen.points().back(),
};

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

static SkPath make_path_for_red_box_around_a_point(SkPoint pt) {
    SkPathBuilder pb;

    SkScalar left = pt.x() - (kRedBoxWH / 2);
    SkScalar top = pt.y() - (kRedBoxWH / 2);

    SkScalar right = pt.x() + (kRedBoxWH / 2);
    SkScalar bottom = pt.y() + (kRedBoxWH / 2);

    pb.addRect({left, top, right, bottom});
    return pb.detach();
};


static SkPath make_path_for_hairline_line_contour(bool close) {
    SkPathBuilder pb;

    pb.moveTo(5, 5).lineTo(5, 10).lineTo(10, 10).lineTo(10, 5);
    if (close) pb.close();

    return pb.detach();
}

static SkPath make_path_for_hairline_quad_contour(bool close) {
    SkPathBuilder pb;

    pb.moveTo(20, 5).quadTo(35, 10, 20, 15);
    if (close) pb.close();

    return pb.detach();
}

static SkPath make_path_for_hairline_cubic_contour(bool close) {
    SkPathBuilder pb;

    pb.moveTo(40, 5).cubicTo(35, 5, 35, 10, 40, 15);
    if (close) pb.close();

    return pb.detach();
}

static const char* cap_to_string(SkPaint::Cap cap) {
    switch (cap) {
        case SkPaint::kButt_Cap:
            return "Butt";
        case SkPaint::kRound_Cap:
            return "Round";
        case SkPaint::kSquare_Cap:
            return "Square";
    }
    return "Unknown Cap";
}

static void draw_help_text(SkCanvas* canvas, const SkString& textToDraw) {
    SkFont font = ToolUtils::DefaultPortableFont();
    font.setSize(12);
    SkPaint p_red;
    p_red.setColor(SK_ColorRED);
    canvas->drawSimpleText(
            textToDraw.c_str(), textToDraw.size(), SkTextEncoding::kUTF8, 0.0f, 32.0f, font, p_red);
}


// We would like to test the ability to draw closed and open contours with
// capped hairlines. Capped hairlines have interesting behavior because they extend out
// by about half a pixel, which has led to unintended behavior before when closing
// contours with antialiasing on. The contours created here represent closed contours
// with all the simple path types, followed by the open version of the contours.
// We also include versions on the pixel lines and offset of the pixel lines by a half
// pixel, as these have different behavior.
static void draw_hairline_paths(SkCanvas* canvas, SkPaint paint, SkPaint::Cap capStyle) {
    SkString message;

    canvas->save();
    canvas->save();
    canvas->drawPath(linePathClosed, paint);
    canvas->drawPath(quadPathClosed, paint);
    canvas->drawPath(cubicPathClosed, paint);
    message.printf("on pixel line, closed, %s", cap_to_string(capStyle));
    draw_help_text(canvas, message);
    canvas->restore();

    canvas->translate(0, 60);

    canvas->save();
    canvas->drawPath(offsetLinePathClosed, paint);
    canvas->drawPath(offsetQuadPathClosed, paint);
    canvas->drawPath(offsetCubicPathClosed, paint);
    message.printf("off pixel line, closed, %s", cap_to_string(capStyle));
    draw_help_text(canvas, message);
    canvas->restore();

    canvas->translate(0, 60);

    canvas->save();
    canvas->drawPath(linePathOpen, paint);
    canvas->drawPath(quadPathOpen, paint);
    canvas->drawPath(cubicPathOpen, paint);
    message.printf("on pixel line, open, %s", cap_to_string(capStyle));
    draw_help_text(canvas, message);
    canvas->restore();

    canvas->translate(0, 60);

    canvas->save();
    canvas->drawPath(offsetLinePathOpen, paint);
    canvas->drawPath(offsetQuadPathOpen, paint);
    canvas->drawPath(offsetCubicPathOpen, paint);
    message.printf("off pixel line, open, %s", cap_to_string(capStyle));
    draw_help_text(canvas, message);
    canvas->restore();
    canvas->restore();
}

static void draw_hairline_paths_zoomed_in(SkCanvas* canvas, SkPaint paint) {
    canvas->save();
    canvas->save();
    draw_scaled_path(canvas, linePathClosed, paint);
    draw_scaled_path(canvas, quadPathClosed, paint);
    draw_scaled_path(canvas, cubicPathClosed, paint);
    canvas->restore();

    canvas->translate(0, 15);

    canvas->save();
    draw_scaled_path(canvas, offsetLinePathClosed, paint);
    draw_scaled_path(canvas, offsetQuadPathClosed, paint);
    draw_scaled_path(canvas, offsetCubicPathClosed, paint);
    canvas->restore();

    canvas->translate(0, 15);

    canvas->save();
    draw_scaled_path(canvas, linePathOpen, paint);
    draw_scaled_path(canvas, quadPathOpen, paint);
    draw_scaled_path(canvas, cubicPathOpen, paint);
    canvas->restore();

    canvas->translate(0, 15);

    canvas->save();
    draw_scaled_path(canvas, offsetLinePathOpen, paint);
    draw_scaled_path(canvas, offsetQuadPathOpen, paint);
    draw_scaled_path(canvas, offsetCubicPathOpen, paint);
    canvas->restore();
    canvas->restore();
}

static void draw_red_boxes_over_capped_edges(SkCanvas* canvas, SkPaint paint) {
    canvas->save();
    canvas->translate(0, 30);

    for (SkPoint pt : cappedPts) {
        canvas->drawPath(make_path_for_red_box_around_a_point(pt), paint);
    }
    canvas->translate(0, 15);

    for (SkPoint pt : offsetCappedPts) {
        canvas->drawPath(make_path_for_red_box_around_a_point(pt), paint);
    }
    canvas->translate(0, 15);

    canvas->restore();
}

static void draw_closed_capped_hairline_with_cap_style(SkCanvas* canvas, SkPaint::Cap cap) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    paint.setStrokeCap(cap);

    draw_hairline_paths(canvas, paint, cap);

    canvas->translate(150, 0);

    canvas->scale(kScale, kScale);
    canvas->save();
    draw_hairline_paths_zoomed_in(canvas, paint);

    draw_grid(canvas);
    canvas->restore();
    paint.setColor(SK_ColorRED);
    draw_red_boxes_over_capped_edges(canvas, paint);
}

DEF_SIMPLE_GM(closedcappedhairlinebuttcap, canvas, 400, 300) {
    draw_closed_capped_hairline_with_cap_style(canvas, SkPaint::kButt_Cap);
}

DEF_SIMPLE_GM(closedcappedhairlinesquarecap, canvas, 400, 300) {
    draw_closed_capped_hairline_with_cap_style(canvas, SkPaint::kSquare_Cap);
}

DEF_SIMPLE_GM(closedcappedhairlineroundcap, canvas, 400, 300) {
    draw_closed_capped_hairline_with_cap_style(canvas, SkPaint::kRound_Cap);
}
