// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Contour.h"

#include "modules/bentleyottmann/include/Myers.h"

#include <vector>

namespace contour {
Contours Contours::Make(SkPath path) {
    SkPoint pts[4];
    SkPath::Iter iter(path, false);
    SkPath::Verb verb;
    Contours contours;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kConic_Verb: {
                SK_ABORT("Not implemented");
                break;
            }
            case SkPath::kMove_Verb:
                contours.closeContour();
                contours.addPointToCurrentContour(pts[0]);
                break;
            case SkPath::kLine_Verb: {
                contours.addPointToCurrentContour(pts[1]);
                break;
            }
            case SkPath::kQuad_Verb: {
                SK_ABORT("Not implemented");
                break;
            }
            case SkPath::kCubic_Verb: {
                SK_ABORT("Not implemented");
                break;
            }
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                contours.closeContour();
                break;
        }
    }

    return contours;
}

std::vector<myers::Segment> Contours::segments() const {
    return std::vector<myers::Segment>{};
}

SkIRect extendRect(SkIRect r, Point p) {
    int32_t left   = std::min(p.x, r.fLeft),
            top    = std::min(p.y, r.fTop),
            right  = std::max(p.x, r.fRight),
            bottom = std::max(p.y, r.fBottom);
    return {left, top, right, bottom};
}

void Contours::addPointToCurrentContour(SkPoint p) {
    Point point{SkScalarRoundToInt(p.x() * kScaleFactor), SkScalarRoundToInt(p.y() * kScaleFactor)};
    fPoints.push_back(point);
    fCurrentContour.bounds = extendRect(fCurrentContour.bounds, point);
    fCurrentContour.end = fPoints.size();
}

void Contours::closeContour() {
    if (!fContours.empty() && fContours.back().end == fCurrentContour.end) {
        // The current contour is empty. Don't record it.
        return;
    }
    fContours.push_back(fCurrentContour);
    fCurrentContour.bounds = SkIRect::MakeEmpty();
}
}  // namespace contour
