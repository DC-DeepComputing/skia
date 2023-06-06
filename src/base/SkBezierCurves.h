/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBezierCurves_DEFINED
#define SkBezierCurves_DEFINED

#include "include/private/base/SkSpan_impl.h"
#include <array>

/**
 * Utilities for dealing with cubic Bézier curves. These have a start XY
 * point, an end XY point, and two control XY points in between. They take
 * a parameter t which is between 0 and 1 (inclusive) which is used to
 * interpolate between the start and end points, via a route dictated by
 * the control points, and return a new XY point.
 *
 * We store a Bézier curve as an array of 8 floats or doubles, where
 * the even indices are the X coordinates, and the odd indices are the Y
 * coordinates.
 */
class SkBezierCubic {
public:

    /**
     * Evaluates the cubic Bézier curve for a given t. It returns an X and Y coordinate
     * following the formula, which does the interpolation mentioned above.
     *     X(t) = X_0*(1-t)^3 + 3*X_1*t(1-t)^2 + 3*X_2*t^2(1-t) + X_3*t^3
     *     Y(t) = Y_0*(1-t)^3 + 3*Y_1*t(1-t)^2 + 3*Y_2*t^2(1-t) + Y_3*t^3
     *
     * t is typically in the range [0, 1], but this function will not assert that,
     * as Bézier curves are well-defined for any real number input.
     */
    static std::array<double, 2> EvalAt(const double curve[8], double t);

    /**
     * Splits the provided Bézier curve at the location t, resulting in two
     * Bézier curves that share a point (the end point from curve 1
     * and the start point from curve 2 are the same).
     *
     * t must be in the interval [0, 1].
     *
     * The provided twoCurves array will be filled such that indices
     * 0-7 are the first curve (representing the interval [0, t]), and
     * indices 6-13 are the second curve (representing [t, 1]).
     */
    static void Subdivide(const double curve[8], double t,
                          double twoCurves[14]);

    /**
     * Converts the provided Bézier curve into the the equivalent cubic
     *    f(t) = A*t^3 + B*t^2 + C*t + D
     * where f(t) will represent Y coordinates over time if yValues is
     * true and the X coordinates if yValues is false.
     *
     * In effect, this turns the control points into an actual line, representing
     * the x or y values.
     */
    static std::array<double, 4> ConvertToPolynomial(const double curve[8], bool yValues);

    /**
     * Given
     *    A0*t^2 -2*B0*t + C0 = 0 and A1*t^2 - 2*B1*t + C1 = 0,
     *
     * Find the t where A0*t^2 - 2*B0*t + C = 0, then return A1*t^2 + - 2*B1*t + C1 where t is on
     * [0, 1].
     *
     * - toIntersect - is the value at which the quadratic is being intersected.
     * - intersectionsStorage - is the array to hold the return data pointed to in the span.
     *
     * Returns a span with the intersections of toIntersect, and the quadratic formed by A, B,
     * and C.
     *
     * If intersecting with a horizontal line, A0, B0, and C0 should correspond to the
     * Y parametric equation and A1, B1, and C1 should correspond to the X parametric equation.
     * If intersecting with a vertical line A0, B0, and C0 should correspond to the X parametric
     * equation and A1, B1, and C1 should correspond to the Y parametric equation.
     */
    static SkSpan<double> Intersect(
            double A0, double B0, double C0,
            double A1, double B1, double C1, double toIntersect, double intersectionsStorage[2]);
};

#endif
