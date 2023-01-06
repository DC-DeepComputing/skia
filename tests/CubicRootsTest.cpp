/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkFloatingPoint.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "tests/Test.h"

#include <cfloat>

static void testRootsReal(skiatest::Reporter* reporter, std::string name,
                          double A, double B, double C, double D,
                          SkSpan<const double> expectedRoots) {
    skiatest::ReporterContext subtest(reporter, name);
    // Validate test case
    REPORTER_ASSERT(reporter, expectedRoots.size() <= 3,
                    "Invalid test case, up to 3 roots allowed");

    for (size_t i = 0; i < expectedRoots.size(); i++) {
        double x = expectedRoots[i];
        // A*x^3 + B*x^2 + C*x + D should equal 0
        double y = A * x * x * x + B * x * x + C * x + D;
        REPORTER_ASSERT(reporter, sk_double_nearly_zero(y),
                    "Invalid test case %zu. %.16f != 0", i, y);
    }


    double roots[3];
    int rootCount = SkDCubic::RootsReal(A, B, C, D, roots);
    REPORTER_ASSERT(reporter, expectedRoots.size() == size_t(rootCount),
                    "Wrong number of roots returned %zu != %d", expectedRoots.size(), rootCount);
    for (int i = 0; i < rootCount; ++i) {
        REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(expectedRoots[i], roots[i]),
                        "%.16f != %.16f at index %d", expectedRoots[i], roots[i], i);
    }
}

DEF_TEST(CubicRootsReal, reporter) {
    testRootsReal(reporter, "first test",
                  1, 2, 3, 4,
                  {-1.650629191439388});
}
