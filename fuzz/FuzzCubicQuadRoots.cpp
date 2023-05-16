/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkCubics.h"
#include "src/base/SkQuads.h"
#include "src/base/SkUtils.h"

#include <cmath>

#if 0
static void fuzz_quad_real_roots(double A, double B, double C) {
    double roots[2];
    const int numSolutions = SkQuads::RootsReal(A, B, C, roots);
    SkASSERT_RELEASE(numSolutions >= 0 && numSolutions <= 2);
    for (int i = 0; i < numSolutions; i++) {
        SkASSERT_RELEASE(std::isfinite(roots[i]));
        // You may be tempted to add assertions that plug the provided solutions into
        // the quadratic equation and verify that the result is zero. Be advised
        // that the fuzzer is very good at finding float values that result in
        // seemingly arbitrarily large errors, due to the imprecision of floating
        // point math. Unless the input range is sufficiently small, such an
        // effort seems fruitless.
    }
    if (numSolutions == 2) {
        // Roots should not be duplicated
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[1]));
    }
}
#else
static void fuzz_quad_real_roots(double root0, double root1, double scale) {
    constexpr double kMachineError = std::numeric_limits<double>::epsilon();

    // Produce the quadratic equation for scale * (x - root0) * (x - root1) == 0
    // (x - r0)(x-r1) = x*x - r0*x - r1*x + r1 * r2
    double A = scale;
    double B = scale * (-root0 - root1);
    double C = scale * root0 * root1;
    double roots[2];
    const int numSolutions = SkQuads::RootsReal(A, B, C, roots);

    if (root0 == root1) {
        SkASSERT_RELEASE(numSolutions == 1);

        // Check relative error
        SkASSERT_RELEASE(abs(root0 - roots[0]) / abs(root0) <= 2.0 * kMachineError);
    } else {
        SkASSERT_RELEASE(numSolutions == 2);

        auto [answerLow, answerHi] = std::minmax(root0, root1);
        auto [resultLow, resultHi] = std::minmax(roots[0], roots[1]);

        SkASSERT_RELEASE(abs(answerLow - resultLow) / abs(answerLow) <= 2.0 * kMachineError);
        SkASSERT_RELEASE(abs(answerHi - resultHi) / abs(answerHi) <= 2.0 * kMachineError);
    }
}
#endif
static void fuzz_cubic_real_roots(double A, double B, double C, double D) {
    double roots[3];
    const int numSolutions = SkCubics::RootsReal(A, B, C, D, roots);
    SkASSERT_RELEASE(numSolutions >= 0 && numSolutions <= 3);
    for (int i = 0; i < numSolutions; i++) {
        SkASSERT_RELEASE(std::isfinite(roots[i]));
    }
    // Roots should not be duplicated
    if (numSolutions >= 2) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[1]));
    }
    if (numSolutions == 3) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[1], roots[2]));
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[2]));
    }
}

static void fuzz_cubic_roots_valid_t(double A, double B, double C, double D) {
    double roots[3];
    const int numSolutions = SkCubics::RootsValidT(A, B, C, D, roots);
    SkASSERT_RELEASE(numSolutions >= 0 && numSolutions <= 3);
    for (int i = 0; i < numSolutions; i++) {
        SkASSERT_RELEASE(std::isfinite(roots[i]));
        SkASSERT_RELEASE(roots[i] >= 0.0);
        SkASSERT_RELEASE(roots[i] <= 1.0);
    }
    // Roots should not be duplicated
    if (numSolutions >= 2) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[1]));
    }
    if (numSolutions == 3) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[1], roots[2]));
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[2]));
    }
}

static void fuzz_cubic_roots_binary_search(double A, double B, double C, double D) {
    double roots[3];
    const int numSolutions = SkCubics::BinarySearchRootsValidT(A, B, C, D, roots);
    SkASSERT_RELEASE(numSolutions >= 0 && numSolutions <= 3);
    for (int i = 0; i < numSolutions; i++) {
        SkASSERT_RELEASE(std::isfinite(roots[i]));
        SkASSERT_RELEASE(roots[i] >= 0.0);
        SkASSERT_RELEASE(roots[i] <= 1.0);
        double actual = SkCubics::EvalAt(A, B, C, D, roots[i]);
        // The binary search algorithm *should* be accurate regardless of the inputs.
        SkASSERT_RELEASE(std::abs(actual) < 0.001);
    }
    // Roots should not be duplicated
    if (numSolutions >= 2) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[1]));
    }
    if (numSolutions == 3) {
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[1], roots[2]));
        SkASSERT_RELEASE(!sk_doubles_nearly_equal_ulps(roots[0], roots[2]));
    }
}

DEF_FUZZ(QuadRoots, fuzz) {
    double root0, root1, scale;
    fuzz->next(&root0);
    fuzz->next(&root1);
    fuzz->next(&scale);

    // Only allow finite parameters.
    if (0.0 * root0 * root1 * scale != 0) {
        return;
    }

    fuzz_quad_real_roots(root0, root1, scale);
}

DEF_FUZZ(CubicQuadRoots, fuzz) {
    double A, B, C, D;
    fuzz->next(&A);
    fuzz->next(&B);
    fuzz->next(&C);
    fuzz->next(&D);

    // Uncomment for easy test case creation
//    SkDebugf("A %16e (0x%lx) B %16e (0x%lx) C %16e (0x%lx) D %16e (0x%lx)\n",
//             A, sk_bit_cast<uint64_t>(A), B, sk_bit_cast<uint64_t>(B),
//             C, sk_bit_cast<uint64_t>(C), D, sk_bit_cast<uint64_t>(D));

    fuzz_quad_real_roots(A, B, C);

    fuzz_cubic_real_roots(A, B, C, D);

    fuzz_cubic_roots_valid_t(A, B, C, D);

    fuzz_cubic_roots_binary_search(A, B, C, D);
}
