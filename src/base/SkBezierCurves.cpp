/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkBezierCurves.h"

#include "include/private/base/SkAssert.h"

static inline double interpolate(double A, double B, double t) {
    return A + (B - A) * t;
}

// Perform subdivision using De Casteljau's algorithm, that is, repeated linear
// interpolation between adjacent points.
void SkBezierCubic::subdivide(const double curve[8], double t,
                              double twoCurves[14]) {
    SkASSERT(0.0 <= t <= 1.0);
    // These macros are 1-indexed because the author thinks better that way.
    // We split the curve "in" into two curves "alpha" and "beta"
#define in_X(n) curve[2*(n-1)]
#define in_Y(n) curve[2*(n-1) + 1]
#define alpha_X(n) twoCurves[2*(n-1)]
#define alpha_Y(n) twoCurves[2*(n-1) + 1]
#define beta_X(n) twoCurves[2*(n-1) + 6]
#define beta_Y(n) twoCurves[2*(n-1) + 7]

    alpha_X(1) = in_X(1);
    alpha_Y(1) = in_Y(1);

    beta_X(4) = in_X(4);
    beta_Y(4) = in_Y(4);

    double x12 = interpolate(in_X(1), in_X(2), t);
    double y12 = interpolate(in_Y(1), in_Y(2), t);
    double x23 = interpolate(in_X(2), in_X(3), t);
    double y23 = interpolate(in_Y(2), in_Y(3), t);
    double x34 = interpolate(in_X(3), in_X(4), t);
    double y34 = interpolate(in_Y(3), in_Y(4), t);

    alpha_X(2) = x12;
    alpha_Y(2) = y12;

    beta_X(3) = x34;
    beta_Y(3) = y34;

    alpha_X(3) = interpolate(x12, x23, t);
    alpha_Y(3) = interpolate(y12, y23, t);

    beta_X(2) = interpolate(x23, x34, t);
    beta_Y(2) = interpolate(x23, x34, t);

    alpha_X(4) /*= beta_X(1) */ = interpolate(alpha_X(3), beta_X(2), t);
    alpha_Y(4) /*= beta_Y(1) */ = interpolate(alpha_Y(3), beta_Y(2), t);
#undef in_X
#undef in_Y
#undef alpha_X
#undef alpha_Y
#undef beta_X
#undef beta_Y
}
