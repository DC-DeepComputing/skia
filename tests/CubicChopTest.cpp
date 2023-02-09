/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkBezierCurves.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "src/pathops/SkPathOpsPoint.h"
#include "src/pathops/SkPathOpsTypes.h"
#include "tests/Test.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>

// Grouping the test inputs into DoublePoints makes the test cases easier to read.
struct DoublePoint {
    double x;
    double y;
};

static bool nearly_equal(double expected, double actual) {
    if (sk_double_nearly_zero(expected)) {
        return sk_double_nearly_zero(actual);
    }
    return sk_doubles_nearly_equal_ulps(expected, actual, 64);
}

static void testChopCubicAtT(skiatest::Reporter* reporter, std::string name,
                             SkSpan<const DoublePoint> curveInputs, double t,
                             SkSpan<const DoublePoint> expectedOutputs) {
    skiatest::ReporterContext subtest(reporter, name);
    REPORTER_ASSERT(reporter, curveInputs.size() == 4,
                    "Invalid test case. Should have 4 input points.");
    REPORTER_ASSERT(reporter, t >= 0.0 && t <= 1.0,
                    "Invalid test case. t %f should be in [0, 1]", t);
    // Two cubic curves, sharing an input point
    REPORTER_ASSERT(reporter, expectedOutputs.size() == 7,
                    "Invalid test case. Should have 7 output points.");


    {
        skiatest::ReporterContext subsubtest(reporter, "Pathops Implementation");
        SkDCubic input;
        std::memcpy(&input.fPts[0], curveInputs.begin(), 8 * sizeof(double));
        SkDCubicPair output = input.chopAt(t);

        for (int i = 0; i < 7; ++i) {
            REPORTER_ASSERT(reporter,
                            nearly_equal(expectedOutputs[i].x, output.pts[i].fX) &&
                            nearly_equal(expectedOutputs[i].y, output.pts[i].fY),
                            "(%.16f, %.16f) != (%.16f, %.16f) at index %d",
                            expectedOutputs[i].x, expectedOutputs[i].y,
                            output.pts[i].fX, output.pts[i].fY, i);
        }
    }
    {
        skiatest::ReporterContext subsubtest(reporter, "SkBezier Implementation");
        double input[8];
        double output[14];
        std::memcpy(input, curveInputs.begin(), 8 * sizeof(double));
        SkBezierCubic::Subdivide(input, t, output);

        for (int i = 0; i < 7; ++i) {
            REPORTER_ASSERT(reporter,
                            nearly_equal(expectedOutputs[i].x, output[i*2]) &&
                            nearly_equal(expectedOutputs[i].y, output[i*2 + 1]),
                            "(%.16f, %.16f) != (%.16f, %.16f) at index %d",
                            expectedOutputs[i].x, expectedOutputs[i].y,
                            output[i*2], output[i*2 + 1], i);
        }
    }
}

DEF_TEST(ChopCubicAtT_NormalizedCubic, reporter) {
    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.0,
        {{  0.000000,  0.000000 }, {  0.000000,  0.000000 }, {  0.000000,  0.000000 },
         {  0.000000,  0.000000 },
         {  0.120000,  1.800000 }, {  0.920000, -1.650000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.1",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.1,
        {{  0.000000,  0.000000 }, {  0.012000,  0.180000 }, {  0.030800,  0.307500 },
         {  0.055000,  0.393850 },
         {  0.272800,  1.171000 }, {  0.928000, -1.385000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.2",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.2,
        {{  0.000000,  0.000000 }, {  0.024000,  0.360000 }, {  0.075200,  0.510000 },
         {  0.142400,  0.540800 },
         {  0.411200,  0.664000 }, {  0.936000, -1.120000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.3",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.3,
        {{  0.000000,  0.000000 }, {  0.036000,  0.540000 }, {  0.133200,  0.607500 },
         {  0.253800,  0.508950 },
         {  0.535200,  0.279000 }, {  0.944000, -0.855000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.4",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.4,
        {{  0.000000,  0.000000 }, {  0.048000,  0.720000 }, {  0.204800,  0.600000 },
         {  0.380800,  0.366400 },
         {  0.644800,  0.016000 }, {  0.952000, -0.590000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.5",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.5,
        {{  0.000000,  0.000000 }, {  0.060000,  0.900000 }, {  0.290000,  0.487500 },
         {  0.515000,  0.181250 },
         {  0.740000, -0.125000 }, {  0.960000, -0.325000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.6",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.6,
        {{  0.000000,  0.000000 }, {  0.072000,  1.080000 }, {  0.388800,  0.270000 },
         {  0.648000,  0.021600 },
         {  0.820800, -0.144000 }, {  0.968000, -0.060000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.7",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.7,
        {{  0.000000,  0.000000 }, {  0.084000,  1.260000 }, {  0.501200, -0.052500 },
         {  0.771400, -0.044450 },
         {  0.887200, -0.041000 }, {  0.976000,  0.205000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.8",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.8,
        {{  0.000000,  0.000000 }, {  0.096000,  1.440000 }, {  0.627200, -0.480000 },
         {  0.876800,  0.051200 },
         {  0.939200,  0.184000 }, {  0.984000,  0.470000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.9",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.9,
        {{  0.000000,  0.000000 }, {  0.108000,  1.620000 }, {  0.766800, -1.012500 },
         {  0.955800,  0.376650 },
         {  0.976800,  0.531000 }, {  0.992000,  0.735000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=1.0",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        1.0,
        {{  0.000000,  0.000000 }, {  0.120000,  1.800000 }, {  0.920000, -1.650000 },
         {  1.000000,  1.000000 },
         {  1.000000,  1.000000 }, {  1.000000,  1.000000 }, {  1.000000,  1.000000 }}
    );
}

DEF_TEST(ChopCubicAtT_ArbitraryCubic, reporter) {
    testChopCubicAtT(reporter, "Arbitrary Cubic, t=0.1234",
        {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
        0.1234,
        {{-10.00000000000000, -20.00000000000000 },
         { -9.62980000000000, -16.91500000000000 },
         { -8.98550392000000, -14.31728192000000 },
         { -8.16106580520000, -12.10537539118400 },
         { -2.30448192000000,   3.60740632000000 },
         { 12.64260000000000,  -0.14900000000000 },
         {  3.00000000000000,  13.00000000000000 }}
    );

    testChopCubicAtT(reporter, "Arbitrary Cubic, t=0.3456",
        {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
        0.3456,
        {{-10.00000000000000, -20.00000000000000 },
         { -8.96320000000000, -11.36000000000000 },
         { -5.77649152000000,  -6.54205952000000 },
         { -2.50378670080000,  -3.31715344793600 },
         {  3.69314048000000,   2.78926592000000 },
         { 10.19840000000000,   3.18400000000000 },
         {  3.00000000000000,  13.00000000000000 }}
    );

    testChopCubicAtT(reporter, "Arbitrary Cubic, t=0.8765",
        {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
        0.8765,
        {{-10.00000000000000, -20.00000000000000 },
         { -7.37050000000000,   1.91250000000000 },
         {  9.08754050000000,  -0.75907200000000 },
         {  5.70546664375000,   8.34743124475000 },
         {  5.22892800000000,   9.63054950000000 },
         {  4.35850000000000,  11.14750000000000 },
         {  3.00000000000000,  13.00000000000000 }}
    );
}


static void testBezierCurveHorizontalIntersectBinarySearch(skiatest::Reporter* reporter,
                std::string name, SkSpan<const DoublePoint> curveInputs, double xToIntersect,
                SkSpan<const double> expectedTValues,
                bool skipPathops = false) {
    skiatest::ReporterContext subtest(reporter, name);
    // Validate test case
    REPORTER_ASSERT(reporter, curveInputs.size() == 4,
                    "Invalid test case. Input curve should have 4 points");
    REPORTER_ASSERT(reporter, expectedTValues.size() <= 3,
                    "Invalid test case, up to 3 intersections possible");

    for (size_t i = 0; i < expectedTValues.size(); i++) {
        double t = expectedTValues[i];
        REPORTER_ASSERT(reporter, t >= 0.0 && t <= 1.0,
                    "Invalid test case t %zu. Should be between 0 and 1 inclusive.", i);

        auto [x, y] = SkBezierCubic::EvalAt(reinterpret_cast<const double*>(curveInputs.data()), t);
        // This is explicitly approximately_equal and not something more precise because
        // the binary search given by the pathops algorithm is not super precise.
        REPORTER_ASSERT(reporter, approximately_equal(xToIntersect, x),
                    "Invalid test case %zu, %1.16f != %1.16f", i, xToIntersect, x);

        if (i > 0) {
            REPORTER_ASSERT(reporter, expectedTValues[i-1] <= expectedTValues[i],
                    "Invalid test case t %zu. Sort intersections in ascending order", i);
        }
    }

    if (!skipPathops) {
        skiatest::ReporterContext subsubtest(reporter, "Pathops Implementation");
        // This implementation expects the coefficients to be (X, Y), (X, Y), (X, Y), (X, Y)
        // but it ignores the values at odd indices.
        SkDCubic inputCubic;
        for (int i = 0; i < 4; i++) {
            inputCubic.fPts[i].fX = curveInputs[i].x;
            inputCubic.fPts[i].fY = curveInputs[i].y;
        }
        double actualTValues[3] = {0, 0, 0};
        double extremeTs[6] = {0, 0, 0, 0, 0};
        int extrema = SkDCubic::FindExtrema(&inputCubic[0].fX, extremeTs);
        int rootCount = inputCubic.searchRoots(extremeTs, extrema, xToIntersect, SkDCubic::kXAxis,
                                               actualTValues);
        REPORTER_ASSERT(reporter, expectedTValues.size() == size_t(rootCount),
                        "Wrong number of roots returned %zu != %d", expectedTValues.size(),
                        rootCount);

        // We don't care which order the t values which are returned from the algorithm.
        // For determinism, we will sort them (and ensure the provided solutions are also sorted).
        std::sort(std::begin(actualTValues), std::begin(actualTValues) + rootCount);
        for (int i = 0; i < rootCount; i++) {
            if (sk_double_nearly_zero(expectedTValues[i])) {
                REPORTER_ASSERT(reporter, sk_double_nearly_zero(actualTValues[i]),
                                "0 != %.16f at index %d", actualTValues[i], i);
            } else {
                REPORTER_ASSERT(reporter,
                        sk_doubles_nearly_equal_ulps(expectedTValues[i], actualTValues[i], 64),
                        "%.16f != %.16f at index %d", expectedTValues[i], actualTValues[i], i);
            }
        }
    }
}

DEF_TEST(BezierCurveHorizontalIntersectBinarySearch, reporter) {
    // All answers are given with 16 significant digits (max for a double) or as an integer
    // when the answer is exact.
    // The Y values in these curves do not impact the test case, but make it easier to visualize
    // when plotting the curves to verify the solutions.
    testBezierCurveHorizontalIntersectBinarySearch(reporter,
           "straight curve crosses once @ 2.0",
           {{ 0, 0 }, { 0, 0 }, { 10, 10 }, { 10, 10 }},
           2.0,
           {0.2871407270431519});

    testBezierCurveHorizontalIntersectBinarySearch(reporter,
           "straight curve crosses once @ 6.0",
           {{ 0, 0 }, { 3, 3 }, { 6, 6 }, { 10, 10 }},
           6.0,
           {0.6378342509269714});


    testBezierCurveHorizontalIntersectBinarySearch(reporter,
           "out and back curve exactly touches @ 3.0",
           {{ 0, 0 }, { 4, 4 }, { 4, 4 }, { 0, 8 }},
           3.0,
           // The binary search algorithm (currently) gets close to, but not quite 0.5
           {0.4999389648437500,
            0.5000610351562500});

    testBezierCurveHorizontalIntersectBinarySearch(reporter,
           "out and back curve crosses twice @ 2.0",
           {{ 0, 0 }, { 4, 4 }, { 4, 4 }, { 0, 8 }},
           2.0,
           {0.2113248705863953,
            0.7886751294136047});

    testBezierCurveHorizontalIntersectBinarySearch(reporter,
           "out and back curve never crosses 4.0",
           {{ 0, 0 }, { 4, 4 }, { 4, 4 }, { 0, 8 }},
           4.0,
           {});


    testBezierCurveHorizontalIntersectBinarySearch(reporter,
            "left right left curve crosses three times @ 2.0",
           {{ 4, 0 }, { -3, 4 }, { 7, 4 }, { 0, 8 }},
           2.0,
           {  0.1361965624455005,
           // 0.1361965624455005397216403 according to Wolfram Alpha
              0.5,
              0.8638034375544995
           // 0.8638034375544994602783597 according to Wolfram Alpha
            },
            // PathOps version fails to find these roots (has not been investigated yet)
            true /* = skipPathops */);

    testBezierCurveHorizontalIntersectBinarySearch(reporter,
            "left right left curve crosses one time @ 1.0",
           {{ 4, 0 }, { -3, 4 }, { 7, 4 }, { 0, 8 }},
           1.0,
           {  0.9454060400510326,
           // 0.9454060415952252910201453 according to Wolfram Alpha
            });

    testBezierCurveHorizontalIntersectBinarySearch(reporter,
            "left right left curve crosses three times @ 2.5",
           {{ 4, 0 }, { -3, 4 }, { 7, 4 }, { 0, 8 }},
           2.5,
           {  0.0898667385750473,
           // 0.08986673805184604633583244 according to Wolfram Alpha
              0.6263520961813417,
           // 0.6263521357441907129444252 according to Wolfram Alpha
              0.7837811281217468,
           // 0.7837811262039632407197424 according to Wolfram Alpha
            });
}
