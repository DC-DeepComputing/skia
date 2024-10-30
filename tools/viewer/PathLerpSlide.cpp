/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cmath>
#include <utility>
#include <queue>
#include <deque>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkCubicMap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkSize.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkAssert.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "tools/viewer/Slide.h"

#include "imgui.h"

std::vector<SkScalar> getTValuesForSegment(std::deque<float>* tValuesToAdd, float t, float tNext) {
    std::vector<SkScalar> tVector;
    while (!tValuesToAdd->empty() && tValuesToAdd->front() > t && tValuesToAdd->front() < tNext) {
        SkScalar total_t = tValuesToAdd->front();
        tValuesToAdd->pop_front();
        SkScalar relative_t = (total_t - t) / (tNext - t);
        tVector.push_back(relative_t);
    }
    return tVector;
}

void addSegmentsFromTValues(const SkPoint cubic_pts[4], std::vector<SkScalar> t_values, SkPath* out) {
    size_t arr_size = t_values.size();
    int dst_size = (3*arr_size) + 4;
    SkPoint split_pts[dst_size];
    SkScalar tArray[arr_size];
    copy(t_values.begin(), t_values.end(), tArray);
    SkChopCubicAt(cubic_pts, split_pts, tArray, arr_size);

    for (size_t i = 0; i < arr_size + 1; i++) {
      out->cubicTo(split_pts[(i*3)+1], split_pts[(i*3)+2], split_pts[(i*3)+3]);
    }
}

bool createPathFromTValues(const SkPath& in, std::deque<float> tValuesToAdd, std::vector<float> tValues, SkPath* out) {
    SkPath::Iter iter(in, false);
    bool fBreak = false;
    // Only increment if we draw on the path
    size_t t_value_idx = 0;
    for (;;) {
      if (fBreak) break;
      bool needToSplit = false;
      SkPoint pts[4];
      SkPath::Verb verb = iter.next(pts);

      // Last tValue is always the end of the path (t=1).
      if (t_value_idx >= tValues.size() - 1) {
        break;
      }
      // t and tNext are the start and end of the current segment.
      float t = tValues[t_value_idx];
      float tNext = tValues[t_value_idx+1];

      if (!tValuesToAdd.empty() && tValuesToAdd.front() > t && tValuesToAdd.front() < tNext) {
        needToSplit = true;
      }

      switch (verb) {
        case SkPath::kMove_Verb:
        //only support one contour right now
          out->moveTo(pts[0]);
          break;
        case SkPath::kLine_Verb: {
            t_value_idx++;
            if (!needToSplit) {
              out->cubicTo(pts[1], pts[1], pts[1]);
            } else {
              std::vector<SkScalar> tVector = getTValuesForSegment(&tValuesToAdd, t, tNext);
              SkPoint pt1, pt2;
              pt1 = pts[0]*(1.0f / 3.0f) + pts[1]*(2.0f / 3.0f);
              pt2 = pts[0]*(2.0f / 3.0f) + pts[1]*(1.0f / 3.0f);
              const SkPoint cubic_pts[4] = {pts[0], pt1, pt2, pts[1]};
              addSegmentsFromTValues(cubic_pts, tVector, out);
            }
            break;
        }
        case SkPath::kQuad_Verb: {
            t_value_idx++;
            SkPoint pt1, pt2;
            pt1 = pts[0] + (pts[1]-pts[0])*(2.0f / 3.0f);
            pt2 = pts[2] + (pts[1]-pts[2])*(2.0f / 3.0f);
            if (!needToSplit) {
              out->cubicTo(pt1, pt2, pts[2]);
            } else {
              std::vector<SkScalar> tVector = getTValuesForSegment(&tValuesToAdd, t, tNext);
              const SkPoint cubic_pts[4] = {pts[0], pt1, pt2, pts[2]};
              addSegmentsFromTValues(cubic_pts, tVector, out);
            }
            break;
        }
        case SkPath::kConic_Verb:
          // Conic not yet supported
          return false;
        case SkPath::kCubic_Verb:
          t_value_idx++;
          if (!needToSplit) {
            out->cubicTo(pts[1], pts[2], pts[3]);
          } else {
              std::vector<SkScalar> tVector = getTValuesForSegment(&tValuesToAdd, t, tNext);
              addSegmentsFromTValues(pts, tVector, out);
          }
          break;
        case SkPath::kClose_Verb:
          // Close not yet supported
          out->close();
          break;
        case SkPath::kDone_Verb:
          fBreak = true;
      }
    }
    return true;
}

// Currently limited to only 1 contour
bool SkPath::general_interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const {
  int pointCount = fPathRef->countPoints();
  if (!pointCount || ending.fPathRef->countPoints() == 0) {
      return true;
  }
  if (this->isInterpolatable(ending)) {
    out->reset();
    out->addPath(*this);
    SkPathRef::Editor editor(&(out->fPathRef));
    fPathRef->interpolate(*ending.fPathRef, weight, out->fPathRef.get());
    return true;
  }

  // Convert all segments to cubics with the same amount of points along the path

  // New paths to store the transformed paths
  SkPath thisCubic;
  SkPath endingCubic;
  thisCubic.reset();
  endingCubic.reset();

  // Calculate the path lengths
  // Only does one contour so far, have to modify or make new pathmeasure code
  // to do multiple contours
  SkPathMeasure measure1(*this, false);
  SkPathMeasure measure2(ending, false);

  SkScalar length1 = measure1.getLength();
  SkScalar length2 = measure2.getLength();
  const SkContourMeasure* contour1 = measure1.currentMeasure();
  const SkContourMeasure* contour2 = measure2.currentMeasure();

  std::vector<SkScalar> distances1;
  size_t index = 0;
  std::optional<SkScalar> dist = contour1->getSegmentDistance(index);
  while (dist.has_value()) {
    distances1.push_back(dist.value());
    dist = contour1->getSegmentDistance(++index);
  }
  std::vector<SkScalar> distances2;
  index = 0;
  dist = contour2->getSegmentDistance(index);
  while (dist.has_value()) {
    distances2.push_back(dist.value());
    dist = contour2->getSegmentDistance(++index);
  }


  distances1.insert(distances1.begin(), 0.0f);
  distances2.insert(distances2.begin(), 0.0f);
  std::deque<float> tValuesToAdd1;
  std::deque<float> tValuesToAdd2;

  // Convert distances to t values
  std::vector<float> tValues1(distances1.size());
  std::vector<float> tValues2(distances2.size());
  for (size_t i = 0; i < distances1.size(); ++i) {
      tValues1[i] = distances1[i] / length1;
  }
  for (size_t i = 0; i < distances2.size(); ++i) {
      tValues2[i] = distances2[i] / length2;
  }

  // Find missing t values from each
  size_t i = 0, j = 0;
  while (i < tValues1.size() && j < tValues2.size()) {
      if (tValues1[i] < tValues2[j]) {
          tValuesToAdd2.push_back(tValues1[i]);
          i++;
      } else if (tValues1[i] > tValues2[j]) {
          tValuesToAdd1.push_back(tValues2[j]);
          j++;
      } else { // tValues1[i] == tValues2[j]
          i++;
          j++;
      }
  }
  // Add remaining t values
  while (i < tValues1.size()) {
      tValuesToAdd2.push_back(tValues1[i]);
      i++;
  }
  while (j < tValues2.size()) {
      tValuesToAdd1.push_back(tValues2[j]);
      j++;
  }

  createPathFromTValues(*this, tValuesToAdd1, tValues1, &thisCubic);
  createPathFromTValues(ending, tValuesToAdd2, tValues2, &endingCubic);

  // Check for another contour
  if (measure1.nextContour() || measure2.nextContour()) {
    return false;
  }

  out->reset();
  out->addPath(thisCubic);
  SkPathRef::Editor editor(&(out->fPathRef));
  fPathRef->interpolate(*endingCubic.fPathRef, weight, out->fPathRef.get());
  return true;
}


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
            "M0,0.5 L1,0.5", &p1));

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
        fPaths.first.general_interpolate(fPaths.second, fTimeMapper.computeYFromX(t), &fInterpolatedPath);

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
        // SkASSERT(p0.isInterpolatable(p1));

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


