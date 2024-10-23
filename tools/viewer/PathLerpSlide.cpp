/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cmath>
#include <utility>

#include "include/core/SkCanvas.h"
#include "include/core/SkCubicMap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkAssert.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkPathPriv.h"
#include "tools/viewer/Slide.h"

#include "imgui.h"

namespace {

class PathLerpSlide final : public Slide {
public:
    PathLerpSlide()
        : fTimeMapper({0.5f, 0}, {0.5f, 1}) {
        fName = "PathLerp";
    }

private:
    void load(SkScalar w, SkScalar h) override {
        fSize = {w, h};

        // Some hard-coded samples to start.
        // TODO: add more/selectable samples.
        SkPath p0, p1;
        SkAssertResult(SkParsePath::FromSVGString(
            "M0,0.5 Q0,0 0.5,0 Q1,0 1,0.5", &p0));
        SkAssertResult(SkParsePath::FromSVGString(
            "M0,0.5 Q0,1 0.5,1 Q1,1 1,0.5", &p1));

        this->updateAnimatingPaths(p0, p1);
    }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void draw(SkCanvas* canvas) override {
        SkPaint path_paint;
        path_paint.setColor(0xff424242);
        path_paint.setStyle(SkPaint::kStroke_Style);
        path_paint.setAntiAlias(true);
        path_paint.setStrokeCap(SkPaint::kRound_Cap);
        path_paint.setStrokeWidth(10 / fPathTransform.getScaleX());

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(fPathTransform);
        canvas->drawPath(fInterpolatedPath, path_paint);

        if (fShowVertices) {
            SkPaint vertex_paint, ctrl_paint;
            vertex_paint.setColor(0xffff0000);
            vertex_paint.setAntiAlias(true);
            ctrl_paint.setColor(0xff0000ff);
            ctrl_paint.setAntiAlias(true);

            const float vertex_radius = 10 / fPathTransform.getScaleX(),
                          ctrl_radius =  5 / fPathTransform.getScaleX();

            for (const auto [verb, pts, weights] : SkPathPriv::Iterate(fInterpolatedPath)) {
                switch (verb) {
                    case SkPathVerb::kMove: // pts: [ vertex ]
                        canvas->drawCircle(pts[0], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kLine: // pts: [ prev_vertex, vertex ]
                        canvas->drawCircle(pts[1], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kQuad: // pts: [ prev_vertex, ctrl, vertex ]
                        canvas->drawCircle(pts[1], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[2], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kCubic: // pts: [ prev_vertex, ctrl0, ctrl1, vertex ]
                        canvas->drawCircle(pts[1], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[2], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[3], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kConic: // pts: [ prev_vertex, ctrl, vertex ]
                        canvas->drawCircle(pts[1], ctrl_radius, ctrl_paint);
                        canvas->drawCircle(pts[2], vertex_radius, vertex_paint);
                        break;
                    case SkPathVerb::kClose: // pts: []
                        break;
                }
            }
        }
    }

    bool animate(double nanos) override {
        if (!fTimeBase) {
            fTimeBase = nanos;
        }

        // Oscillating between 0..1
        const float t =
                std::abs((std::fmod((nanos - fTimeBase)*0.000000001*fAnimationSpeed, 2) - 1));

        // Interpolate with easing
        fPaths.first.interpolate(fPaths.second, fTimeMapper.computeYFromX(t), &fInterpolatedPath);

        return true;
    }

    bool onChar(SkUnichar c) override {
        switch (c) {
            case 'v':
                fShowVertices = !fShowVertices;
                return true;
            default:
                return false;
        }
    }

    void updateAnimatingPaths(const SkPath& p0, const SkPath& p1) {
        // TODO: preprocess arbitrary paths with mismatched verbs/controls.
        SkASSERT(p0.isInterpolatable(p1));

        fPaths = {p0, p1};

        // Scale and center such that the path animation fills 90% of the view.
        SkRect bounds = p0.computeTightBounds();
        bounds.join(p1.computeTightBounds());

        const SkRect dst_rect = SkRect::MakeSize(fSize)
            .makeInset(fSize.width() * .05f, fSize.height() * .05f);
        fPathTransform = SkMatrix::MakeRectToRect(bounds, dst_rect, SkMatrix::kCenter_ScaleToFit);
    }

    SkSize                    fSize = {0,0};
    std::pair<SkPath, SkPath> fPaths;        // currently morphing paths
    SkPath                    fInterpolatedPath;
    SkMatrix                  fPathTransform = SkMatrix::I();

    float                     fAnimationSpeed = 1.f;
    double                    fTimeBase       = 0;
    const SkCubicMap          fTimeMapper;   // for animation easing

    bool                      fShowVertices   = false;
};

}  // namespace

DEF_SLIDE(return new PathLerpSlide();)
