/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/base/SkQuads.h"

#include "include/private/base/SkFloatingPoint.h"

#include <cmath>

// Solve 0 = M * x + B. If M is 0, there are no solutions, unless B is also 0,
// in which case there are infinite solutions, so we just return 1 of them.
static int solve_linear(const double M, const double B, double solution[2]) {
    if (sk_double_nearly_zero(M)) {
        solution[0] = 0;
        if (sk_double_nearly_zero(B)) {
            return 1;
        }
        return 0;
    }
    solution[0] = -B / M;
    if (!std::isfinite(solution[0])) {
        return 0;
    }
    return 1;
}

int SkQuads::RootsReal(const double A, const double B, const double C, double solution[2]) {
    if (sk_double_nearly_zero(A)) {
        return solve_linear(B, C, solution);
    }
    const double discriminant = B * B - (4 * A * C);
    if (discriminant < 0 || !std::isfinite(discriminant)) {
        return 0;
    }
    double sqrt_D = std::sqrt(discriminant);
    solution[0] = (-B - sqrt_D) / (2 * A);
    solution[1] = (-B + sqrt_D) / (2 * A);
    if (sk_double_nearly_zero(sqrt_D) ||
        sk_doubles_nearly_equal_ulps(solution[0], solution[1])) {
        return 1;
    }
    return 2;
}
