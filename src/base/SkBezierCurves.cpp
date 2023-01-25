/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkBezierCurves.h"

#include "include/private/base/SkAssert.h"

#include <cstddef>

static inline double interpolate(double A, double B, double t) {
    return A + (B - A) * t;
}

// Perform subdivision using De Casteljau's algorithm, that is, repeated linear
// interpolation between adjacent points.
void SkBezierCubic::Subdivide(const double curve[8], double t,
                              double twoCurves[14]) {
    SkASSERT(0.0 <= t && t <= 1.0);
    // These macros are 1-indexed because the author thinks better that way.
    // We split the curve "in" into two curves "alpha" and "beta"
    const auto in_X = [&curve](size_t n) { return curve[2*(n-1)]; };
    const auto in_Y = [&curve](size_t n) { return curve[2*(n-1) + 1]; };
    const auto alpha_X = [&twoCurves](size_t n) -> double& { return twoCurves[2*(n-1)]; };
    const auto alpha_Y = [&twoCurves](size_t n) -> double& { return twoCurves[2*(n-1) + 1]; };
    const auto beta_X = [&twoCurves](size_t n) -> double& { return twoCurves[2*(n-1) + 6]; };
    const auto beta_Y = [&twoCurves](size_t n) -> double& { return twoCurves[2*(n-1) + 7]; };

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
    beta_Y(2) = interpolate(y23, y34, t);

    alpha_X(4) /*= beta_X(1) */ = interpolate(alpha_X(3), beta_X(2), t);
    alpha_Y(4) /*= beta_Y(1) */ = interpolate(alpha_Y(3), beta_Y(2), t);
}

void SkBezierCubic::ConvertToPolynomial(const double curve[8], bool yValues,
                                        double* A, double* B, double* C, double* D) {
    const double* offset_curve = yValues ? curve + 1 : curve;
    const auto P = [&offset_curve](size_t n) { return offset_curve[2*(n-1)]; };
    // A cubic Bézier curve is interpolated as follows:
    //  c(t) = (1 - t)^3 P_1 + 3t(1 - t)^2 P_2 + 3t^2 (1 - t) P_3 + t^3 P_4
    //       = (-P_1 + 3P_2 + -3P_3 + P_4) t^3 + (3P_1 - 6P_2 + 3P_3) t^2 +
    //         (-3P_1 + 3P_2) t + P_1
    // Where P_N is the Nth point. The second step expands the polynomial and groups
    // by powers of t. The desired output is a cubic formula, so we just need to
    // combine the appropriate points to make the coefficients.
    *A = -P(1) + 3*P(2) - 3*P(3) + P(4);
    *B = 3*P(1) - 6*P(2) + 3*P(3);
    *C = -3*P(1) + 3*P(2);
    *D = P(1);
}

