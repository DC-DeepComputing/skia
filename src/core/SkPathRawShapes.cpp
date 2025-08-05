/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkPathRawShapes.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkPathMakers.h"
#include "src/core/SkPathRawBuilder.h"

const SkPathFillType kDefFillType = SkPathFillType::kWinding;

const SkPathVerb gRectVerbs[] = {
    SkPathVerb::kMove,
    SkPathVerb::kLine,
    SkPathVerb::kLine,
    SkPathVerb::kLine,
    SkPathVerb::kClose
};

const uint8_t gRectSegMask = kLine_SkPathSegmentMask;

static void set_as_rect(SkPathRaw* raw, SkSpan<SkPoint> storage,
                        const SkRect& r, SkPathDirection dir, unsigned index) {
    SkASSERT(storage.size() >= 4);

    raw->fPoints = { storage.data(), 4 };
    raw->fVerbs = gRectVerbs;
    raw->fConics = {};
    raw->fBounds = r;
    raw->fFillType = kDefFillType;
    raw->fIsConvex = true;
    raw->fSegmentMask = gRectSegMask;

    SkPath_RectPointIterator iter(r, dir, index);

    storage[0] = iter.current();
    storage[1] = iter.next();
    storage[2] = iter.next();
    storage[3] = iter.next();
}

//////////////////

const SkPathVerb gOvalVerbs[] = {
    SkPathVerb::kMove,
    SkPathVerb::kConic,
    SkPathVerb::kConic,
    SkPathVerb::kConic,
    SkPathVerb::kConic,
    SkPathVerb::kClose
};

const uint8_t gOvalSegMask = kConic_SkPathSegmentMask;

const float gFourQuarterCircleConics[] = {
    SK_ScalarRoot2Over2,
    SK_ScalarRoot2Over2,
    SK_ScalarRoot2Over2,
    SK_ScalarRoot2Over2,
};

static void set_as_oval(SkPathRaw* raw, SkSpan<SkPoint> storage,
                        const SkRect& r, SkPathDirection dir, unsigned index) {
    SkASSERT(storage.size() >= 9);

    raw->fPoints = { storage.data(), 9 };
    raw->fVerbs = gOvalVerbs;
    raw->fConics = gFourQuarterCircleConics;
    raw->fBounds = r;
    raw->fFillType = kDefFillType;
    raw->fIsConvex = true;
    raw->fSegmentMask = gOvalSegMask;

    SkPath_OvalPointIterator ovalIter(r, dir, index);
    SkPath_RectPointIterator rectIter(r, dir, index + (dir == SkPathDirection::kCW ? 0 : 1));

    storage[0] = ovalIter.current();
    for (unsigned i = 0; i < 4; ++i) {
        storage[i*2 + 1] = rectIter.next();
        storage[i*2 + 2] = ovalIter.next();
    }
}

/////////////////////////////

const SkPathVerb gRRectVerbs_LineStart[] = {
    SkPathVerb::kMove,
    SkPathVerb::kLine, SkPathVerb::kConic,
    SkPathVerb::kLine, SkPathVerb::kConic,
    SkPathVerb::kLine, SkPathVerb::kConic,
    SkPathVerb::kLine, SkPathVerb::kConic,
    SkPathVerb::kClose
};

const SkPathVerb gRRectVerbs_ConicStart[] = {
    SkPathVerb::kMove,
    SkPathVerb::kConic, SkPathVerb::kLine,
    SkPathVerb::kConic, SkPathVerb::kLine,
    SkPathVerb::kConic, SkPathVerb::kLine,
    SkPathVerb::kConic, // we can skip the last line
    SkPathVerb::kClose
};

const uint8_t gRRectSegMask = kLine_SkPathSegmentMask | kConic_SkPathSegmentMask;

static void set_as_rrect(SkPathRaw* raw, SkSpan<SkPoint> storage,
                         const SkRRect& rrect, SkPathDirection dir, unsigned index) {
    // we start with a conic on odd indices when moving CW vs. even indices when moving CCW
    const bool startsWithConic = ((index & 1) == (dir == SkPathDirection::kCW));
    // if we start with a conic, we end with a line, which we can skip (relying on close())
    const size_t npoints = 13 - startsWithConic;
    const SkRect& bounds = rrect.getBounds();

    SkASSERT(storage.size() >= npoints);

    raw->fPoints = { storage.data(), npoints };
    if (startsWithConic) {
        raw->fVerbs = gRRectVerbs_ConicStart;
    } else {
        raw->fVerbs = gRRectVerbs_LineStart;
    }
    raw->fConics = gFourQuarterCircleConics;
    raw->fBounds = bounds;
    raw->fFillType = kDefFillType;
    raw->fIsConvex = true;
    raw->fSegmentMask = gRRectSegMask;

    SkPath_RRectPointIterator rrectIter(rrect, dir, index);
    // Corner iterator indices follow the collapsed radii model,
    // adjusted such that the start pt is "behind" the radii start pt.
    const unsigned rectStartIndex = index / 2 + (dir == SkPathDirection::kCW ? 0 : 1);
    SkPath_RectPointIterator rectIter(bounds, dir, rectStartIndex);

    storage[0] = rrectIter.current();
    if (startsWithConic) {
        for (unsigned i = 0; i < 3; ++i) {
            // conic points
            storage[i*3 + 1] = rectIter.next();
            storage[i*3 + 2] = rrectIter.next();
            // line point
            storage[i*3 + 3] = rrectIter.next();
        }
        // last conic points
        storage[10] = rectIter.next();
        storage[11] = rrectIter.next();
        // the final line is accomplished by close()
    } else {
        for (unsigned i = 0; i < 4; ++i) {
            // line point
            storage[i*3 + 1] = rrectIter.next();
            // conic points
            storage[i*3 + 2] = rectIter.next();
            storage[i*3 + 3] = rrectIter.next();
        }
    }
    // close
}

/////////////////////////////

SkPathRawShapes::Rect::Rect(const SkRect& r, SkPathDirection dir, unsigned index) {
    set_as_rect(this, fStorage, r, dir, index);
}

SkPathRawShapes::Oval::Oval(const SkRect& r, SkPathDirection dir, unsigned index) {
    set_as_oval(this, fStorage, r, dir, index);
}

SkPathRawShapes::RRect::RRect(const SkRRect& rrect, SkPathDirection dir, unsigned index) {
    const SkRect& bounds = rrect.getBounds();

    if (rrect.isRect() || rrect.isEmpty()) {
        // degenerate(rect) => radii points are collapsing
        set_as_rect(this, fStorage, bounds, dir, (index + 1) / 2);
    } else if (rrect.isOval()) {
        // degenerate(oval) => line points are collapsing
        set_as_oval(this, fStorage, bounds, dir, index / 2);
    } else {
        set_as_rrect(this, fStorage, rrect, dir, index);
    }
}

////////////

static std::optional<SkPoint> arc_is_lone_point(const SkRect& oval,
                                                float startAngle, float sweepAngle) {
    if (0 == sweepAngle && (0 == startAngle || 360 == startAngle)) {
        // Chrome uses this path to move into and out of ovals. If not
        // treated as a special case the moves can distort the oval's
        // bounding box (and break the circle special case).
        return {{oval.fRight, oval.centerY()}};
    } else if (0 == oval.width() && 0 == oval.height()) {
        // Chrome will sometimes create 0 radius round rects. Having degenerate
        // quad segments in the path prevents the path from being recognized as
        // a rect.
        // TODO: optimizing the case where only one of width or height is zero
        // should also be considered. This case, however, doesn't seem to be
        // as common as the single point case.
        return {{oval.fRight, oval.fTop}};
    }
    return {};
}

struct UnitVectors {
    SkVector        fStart, fStop;
    SkPathDirection fDir;
};

// Return the unit vectors pointing at the start/stop points for the given start/sweep angles
//
static UnitVectors angles_to_unit_vectors(float startAngle, float sweepAngle) {
    float startRad = SkDegreesToRadians(startAngle),
          stopRad  = SkDegreesToRadians(startAngle + sweepAngle);

    auto snap_radians_to_vector = [](float radians) {
        return SkVector{
            SkScalarCosSnapToZero(radians),
            SkScalarSinSnapToZero(radians),
        };
    };

    SkVector startV = snap_radians_to_vector(startRad),
             stopV  = snap_radians_to_vector(stopRad);

    /*  If the sweep angle is nearly (but less than) 360, then due to precision
        loss in radians-conversion and/or sin/cos, we may end up with coincident
        vectors, which will fool SkBuildQuadArc into doing nothing (bad) instead
        of drawing a nearly complete circle (good).
        e.g. canvas.drawArc(0, 359.99, ...)
        -vs- canvas.drawArc(0, 359.9, ...)
        We try to detect this edge case, and tweak the stop vector
     */
    if (startV == stopV) {
        float sw = std::abs(sweepAngle);
        if (sw < 360 && sw > 359) {
            // make a guess at a tiny angle (in radians) to tweak by
            float deltaRad = std::copysign(1.0f/512, sweepAngle);
            // not sure how much will be enough, so we use a loop
            do {
                stopRad -= deltaRad;
                stopV = snap_radians_to_vector(stopRad);
            } while (startV == stopV);
        }
    }

    return {
        startV,
        stopV,
        sweepAngle > 0 ? SkPathDirection::kCW : SkPathDirection::kCCW
    };
}

/**
 *  If this returns 0, then the caller should just line-to the singlePt, else it should
 *  ignore singlePt and append the specified number of conics.
 */
static int build_arc_conics(const SkRect& oval, const SkVector start, const SkVector stop,
                            SkPathDirection dir, SkConic conics[SkConic::kMaxConicsForArc],
                            SkPoint* singlePt) {
    SkMatrix    matrix;

    matrix.setScale(SkScalarHalf(oval.width()), SkScalarHalf(oval.height()));
    matrix.postTranslate(oval.centerX(), oval.centerY());

    SkRotationDirection rot = dir == SkPathDirection::kCW ? kCW_SkRotationDirection
                                                          : kCCW_SkRotationDirection;
    int count = SkConic::BuildUnitArc(start, stop, rot, &matrix, conics);
    if (0 == count) {
        *singlePt = matrix.mapPoint(stop);
    }
    return count;
}

static bool nearly_equal(SkPoint a, SkPoint b) {
    return SkScalarNearlyEqual(a.fX, b.fX) &&
           SkScalarNearlyEqual(a.fY, b.fY);
}

// Returns true if the result can be tagged as an "arc"
static bool add_arc(SkPathRawBuilder* bu, const SkRect& oval, float startAngle, float sweepAngle,
                    bool forceMoveTo, bool isArc, std::optional<SkPoint> lastPt) {
    if (oval.width() < 0 || oval.height() < 0) {
        return false; // empty
    }

    startAngle = std::fmodf(startAngle, 360.0f);

    if (auto lonePt = arc_is_lone_point(oval, startAngle, sweepAngle)) {
        forceMoveTo ? bu->moveTo(*lonePt) : bu->lineTo(*lonePt);
        return false;
    }

    auto [startV, stopV, dir] = angles_to_unit_vectors(startAngle, sweepAngle);

    // Adds a move-to to 'pt' if forceMoveTo is true. Otherwise a lineTo unless we're sufficiently
    // close to 'pt' currently. This prevents spurious lineTos when adding a series of contiguous
    // arcs from the same oval.
    auto addPt = [&](SkPoint pt) {
        if (forceMoveTo) {
            bu->moveTo(pt);
        } else if (!lastPt || !nearly_equal(*lastPt, pt)) {
            bu->lineTo(pt);
            isArc = false;
        }
    };

    // At this point, we know that the arc is not a lone point, but startV == stopV
    // indicates that the sweepAngle is too small such that angles_to_unit_vectors
    // cannot handle it.
    if (startV == stopV) {
        float endAngle = SkDegreesToRadians(startAngle + sweepAngle);
        float radiusX = oval.width() / 2;
        float radiusY = oval.height() / 2;
        // We do not use SkScalar[Sin|Cos]SnapToZero here. When sin(startAngle) is 0 and sweepAngle
        // is very small and radius is huge, the expected behavior here is to draw a line. But
        // calling SkScalarSinSnapToZero will make sin(endAngle) be 0 which will then draw a dot.
        addPt({
            oval.centerX() + radiusX * std::cos(endAngle),
            oval.centerY() + radiusY * std::sin(endAngle),
        });
        return false;
    }

    SkPoint singlePt;
    SkConic conics[SkConic::kMaxConicsForArc];
    int count = build_arc_conics(oval, startV, stopV, dir, conics, &singlePt);
    if (count) {
        addPt(conics[0].fPts[0]);
        for (int i = 0; i < count; ++i) {
            bu->conicTo(conics[i].fPts[1], conics[i].fPts[2], conics[i].fW);
        }
    } else {
        addPt(singlePt);
        isArc = false;
    }

    return isArc;
}

SkPathRawShapes::Arc::Arc(const SkRect& oval, float startAngle, float sweepAngle,
                          bool forceMoveTo, bool isArc, std::optional<SkPoint> lastPt) {
    SkPathRawBuilder bu(fPtStorage, fVbStorage, fCnStorage);

    fIsArc = add_arc(&bu, oval, startAngle, sweepAngle, forceMoveTo, isArc, lastPt);

    SkPathRaw* raw = this;
    *raw = bu.raw(SkPathFillType::kDefault, false);
}
