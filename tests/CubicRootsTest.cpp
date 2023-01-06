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

void bubble_sort(double array[], int count) {
    for (int i = count - 1; i >= 0; i--) {
        for (int j = i; j >= 0; j--) {
            if (array[j] < array[j - 1]) {
                double tmp = array[j];
                array[j] = array[j - 1];
                array[j - 1] = tmp;
            }
        }
    }
}

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

        if (i > 0) {
            REPORTER_ASSERT(reporter, expectedRoots[i-1] <= expectedRoots[i],
                    "Invalid test case %zu. Roots should be sorted in ascending order", i);
        }
    }


    double roots[3] = {0, 0, 0};
    int rootCount = SkDCubic::RootsReal(A, B, C, D, roots);
    REPORTER_ASSERT(reporter, expectedRoots.size() == size_t(rootCount),
                    "Wrong number of roots returned %zu != %d", expectedRoots.size(), rootCount);

    // We don't care which order the roots are returned from the algorithm.
    // For determinism, we will sort them (and ensure the provided solutions are also).
    bubble_sort(roots, rootCount);
    for (int i = 0; i < rootCount; i++) {
        REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(expectedRoots[i], roots[i]),
                        "%.16f != %.16f at index %d", expectedRoots[i], roots[i], i);
    }
}

DEF_TEST(CubicRootsReal_ActualCubics, reporter) {
    // All answers are given with 16 significant digits (max for a double) or as an integer
    // when the answer is exact.
    testRootsReal(reporter, "one intersection 1x^3 + 2x^2 + 3x + 4",
                  1, 2, 3, 4,
                  {-1.650629191439388});
                 //-1.650629191439388218880801 from Wolfram Alpha

    // (3x-5)(6x-10)(x+4) = 18x^3 + 12x^2 - 190x + 200
    testRootsReal(reporter, "touches y axis 18x^3 + 12x^2 - 190x + 200",
                  18, 12, -190, 200,
                  {-4.,
                    1.666666666666667, // 5/3
                   });

    testRootsReal(reporter, "three roots 10x^3 - 20x^2 - 30x + 40",
                  10, -20, -30, 40,
                  {-1.561552812808830,
                 //-1.561552812808830274910705 from Wolfram Alpha
                    1.,
                    2.561552812808830,
                 // 2.561552812808830274910705 from Wolfram Alpha
                   });

    testRootsReal(reporter, "three roots -10x^3 + 200x^2 + 300x - 400",
              -10, 200, 300, -400,
              {-2.179884793243323,
             //-2.179884793243323422232630 from Wolfram Alpha
                0.8607083693981839,
             // 0.8607083693981838897123320 from Wolfram Alpha
               21.31917642384514,
             //21.31917642384513953252030 from Wolfram Alpha
               });
}
