/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRRectPriv_DEFINED
#define SkRRectPriv_DEFINED

#include "include/core/SkRRect.h"
#include "src/utils/SkFloatUtils.h"

class SkRBuffer;
class SkWBuffer;

static bool are_radii_predicates_valid(SkScalar radiusFromMin,
                                       SkScalar radiusFromMax,
                                       SkScalar minCoord,
                                       SkScalar maxCoord) {
    if (minCoord > maxCoord || radiusFromMin < 0 || radiusFromMax < 0) {
        return false;
    }
    const SkScalar limit = maxCoord - minCoord;
    const SkScalar sum = radiusFromMin + radiusFromMax;
    // Check both lengths (sum <= limit) and absolute coordinates (min + r1 <= max - r2)
    // because floating-point rounding can cause one to pass while the other fails. Testing both
    // ensures the corner curves never overlap regardless of how algorithms compute edge spans.
    if (sum <= limit && minCoord + radiusFromMin <= maxCoord - radiusFromMax) {
        return true;
    }
    // Accept either floats that are within an absolute tolerance of each other (for small numbers)
    // or within a few ULPs (Units in the Last Place, i.e. the step between adjacent
    // representable floats, for large numbers) to be robust against floating-point imprecision
    // when translated. See https://skia-review.git.corp.google.com/c/skia/+/1279236.
    const SkFloatingPoint<float, 4> fpSum(sum);
    const SkFloatingPoint<float, 4> fpLimit(limit);
    return SkScalarNearlyEqual(sum, limit) || fpSum.AlmostEquals(fpLimit);
}

class SkRRectPriv {
public:
    static bool IsCircle(const SkRRect& rr) {
        return rr.isOval() && SkScalarNearlyEqual(rr.fRadii[0].fX, rr.fRadii[0].fY);
    }

    static SkVector GetSimpleRadii(const SkRRect& rr) {
        SkASSERT(!rr.isComplex());
        return rr.fRadii[0];
    }

    static bool IsSimpleCircular(const SkRRect& rr) {
        return rr.isSimple() && SkScalarNearlyEqual(rr.fRadii[0].fX, rr.fRadii[0].fY);
    }

    static bool AreRectAndRadiiValid(const SkRect& rect, const SkVector radii[4]) {
        if (!rect.isFinite() || !rect.isSorted()) {
            return false;
        }
        return are_radii_predicates_valid(radii[SkRRect::Corner::kUpperLeft_Corner].fX,
                                          radii[SkRRect::Corner::kUpperRight_Corner].fX,
                                          rect.fLeft,
                                          rect.fRight) &&
               are_radii_predicates_valid(radii[SkRRect::Corner::kLowerLeft_Corner].fX,
                                          radii[SkRRect::Corner::kLowerRight_Corner].fX,
                                          rect.fLeft,
                                          rect.fRight) &&
               are_radii_predicates_valid(radii[SkRRect::Corner::kUpperLeft_Corner].fY,
                                          radii[SkRRect::Corner::kLowerLeft_Corner].fY,
                                          rect.fTop,
                                          rect.fBottom) &&
               are_radii_predicates_valid(radii[SkRRect::Corner::kUpperRight_Corner].fY,
                                          radii[SkRRect::Corner::kLowerRight_Corner].fY,
                                          rect.fTop,
                                          rect.fBottom);
    }

    // Looser version of IsSimpleCircular, where the x & y values of the radii
    // only have to be nearly equal instead of strictly equal.
    static bool IsNearlySimpleCircular(const SkRRect& rr, float tolerance = SK_ScalarNearlyZero);

    static bool EqualRadii(const SkRRect& rr) {
        return rr.isRect() || SkRRectPriv::IsCircle(rr)  || SkRRectPriv::IsSimpleCircular(rr);
    }

    static const SkVector* GetRadiiArray(const SkRRect& rr) { return rr.fRadii; }

    static bool AllCornersCircular(const SkRRect& rr, float tolerance = SK_ScalarNearlyZero);

    // Prefer this over AllCornersCircular, which compares radii by absolute difference, which is
    // a less stable decision as scale changes.
    static bool AllCornersRelativelyCircular(const SkRRect& rr,
                                             float tolerance = SK_ScalarNearlyZero);

    // The same test used in AllCornersRelativelyCircular, but for provided radii.
    static bool IsRelativelyCircular(float rx, float ry, float tolerance = SK_ScalarNearlyZero);

    static bool ReadFromBuffer(SkRBuffer* buffer, SkRRect* rr);

    static void WriteToBuffer(const SkRRect& rr, SkWBuffer* buffer);

    // Compute an approximate largest inscribed bounding box of the rounded rect. For empty,
    // rect, oval, and simple types this will be the largest inscribed rectangle. Otherwise it may
    // not be the global maximum, but will be non-empty, touch at least one edge and be contained
    // in the round rect.
    static SkRect InnerBounds(const SkRRect& rr);

    // Attempt to compute the intersection of two round rects. The intersection is not necessarily
    // a round rect. This returns intersections only when the shape is representable as a new
    // round rect (or rect). Empty is returned if 'a' and 'b' do not intersect or if the
    // intersection is too complicated. This is conservative, it may not always detect that an
    // intersection could be represented as a round rect. However, when it does return a round rect
    // that intersection will be exact (i.e. it is NOT just a subset of the actual intersection).
    static SkRRect ConservativeIntersect(const SkRRect& a, const SkRRect& b);
};

#endif
