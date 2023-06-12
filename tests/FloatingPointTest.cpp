/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkUtils.h"
#include "tests/Test.h"

#include <array>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

DEF_TEST(DoubleNearlyZero, reporter) {
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(0.));
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(-0.));
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(DBL_EPSILON));
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(-DBL_EPSILON));

    double nearly = 1. / 20000000000LL;
    REPORTER_ASSERT(reporter, nearly != 0);
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(nearly));
    REPORTER_ASSERT(reporter, sk_double_nearly_zero(-nearly));

    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(1));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-1));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(INFINITY));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(HUGE_VALF));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(HUGE_VAL));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(HUGE_VALL));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-INFINITY));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-HUGE_VALF));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-HUGE_VAL));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-HUGE_VALL));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(NAN));
    REPORTER_ASSERT(reporter, !sk_double_nearly_zero(-NAN));
}

DEF_TEST(DoubleNearlyEqualUlps, reporter) {
    // Our tolerance is looser than DBL_EPSILON
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(1., 1.));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(1., 1. - DBL_EPSILON));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(1., 1. + DBL_EPSILON));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(100.5, 100.5));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(100.5, 100.5 - DBL_EPSILON));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(100.5, 100.5 + DBL_EPSILON));

    // Our tolerance is tighter than FLT_EPSILON
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(1., 1. - FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(1., 1. + FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(100.5, 100.5 - FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(100.5, 100.5 + FLT_EPSILON));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(0, 0.1));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(FLT_EPSILON, 0));

    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(INFINITY, INFINITY));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(INFINITY, 10));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(10, INFINITY));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(NAN, INFINITY));

    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(INFINITY, -INFINITY));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(-INFINITY, INFINITY));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(-INFINITY, -INFINITY));

    // Test values upto the edge of infinity.
    const double biggest = std::numeric_limits<double>::max();
    auto almostBiggest = [&](int n) {
        double almostBiggest = biggest;
        for (int i = 0; i < n; ++i) {
            almostBiggest = std::nextafter(almostBiggest, -INFINITY);
        }
        return almostBiggest;
    };
    const double nextBiggest = almostBiggest(1);
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(biggest, nextBiggest));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(biggest, almostBiggest(16)));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(biggest, almostBiggest(17)));

    // One ulp less would be infinity.
    const uint64_t smallestNANPattern =
            0b0'11111111111'0000000000000000000000000000000000000000000000000001;
    double smallestNAN;
    memcpy(&smallestNAN, &smallestNANPattern, sizeof(double));
    SkASSERT(std::isnan(smallestNAN));
    SkASSERT(biggest != nextBiggest);

    // Sanity check.
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(smallestNAN, NAN));

    // Make sure to return false along the edge of infinity.
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(INFINITY, biggest));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(smallestNAN, biggest));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(smallestNAN, INFINITY));

    const double smallest = std::numeric_limits<double>::denorm_min();
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(NAN, NAN));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(smallest, 0));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(smallest, -smallest));
    REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(8*smallest, -8*smallest));
    REPORTER_ASSERT(reporter, !sk_doubles_nearly_equal_ulps(8*smallest, -9*smallest));
}

DEF_TEST(BitCastDoubleRoundTrip, reporter) {
    std::array<double, 5>  testCases = {0.0, 1.0, -13.0, 1.234567890123456, -543210.987654321};

    for (size_t i = 0; i < testCases.size(); i++) {
        double input = testCases[i];
        uint64_t bits = sk_bit_cast<uint64_t>(input);
        double output = sk_bit_cast<double>(bits);
        REPORTER_ASSERT(reporter, input == output, "%.16f is not exactly %.16f", input, output);
    }

    {
        uint64_t bits = sk_bit_cast<uint64_t>((double) NAN);
        double output = sk_bit_cast<double>(bits);
        REPORTER_ASSERT(reporter, std::isnan(output), "%.16f is not nan", output);
    }
    {
        uint64_t bits = sk_bit_cast<uint64_t>((double) INFINITY);
        double output = sk_bit_cast<double>(bits);
        REPORTER_ASSERT(reporter, !std::isfinite(output), "%.16f is not infinity", output);
    }
}

DEF_TEST(FMA, reporter) {
    // 0b0'01111111111'00'0000000000'0000000000'0000000010'0000000000'0000000000
    double over1 = 1+4.656612873e-10;

    // 0b0'01111111110'11'1111111111'1111111111'1111111100'0000000000'0000000000
    double under1 = 1-4.656612873e-10;

    // Precision loss
    //                         -------------- becomes 1; extra bits are rounded off.
    double x = std::fma(1, -1, over1 * under1);

    // Precision maintained
    //                  ------------- becomes 1 - 2^-62; extra bits are maintained
    double y = std::fma(over1, under1, -1);

    REPORTER_ASSERT(reporter, x == 0);
    REPORTER_ASSERT(reporter, y == -exp2(-62));
}

DEF_TEST(StaticCastDoubleToFloat, reporter) {
    const double one = 1.0;
    const double infinity = std::numeric_limits<double>::infinity();

    // Because the exponent 1.0f is 0, then machine epsilon is 1 ulp.
    const float floatUlp = std::numeric_limits<float>::epsilon();

    // A double that is right in between float 1 and float 1 + ulp.
    const double onePlusHalfUlp = one + floatUlp * 0.5;

    // Conversion should use round to even rounding it down to float 1. It's rounding down
    // because the binary form
    // 0b0'11111111110'0000'0000000000'0000000000'1000000000'0000000000'0000000000
    //                                          ---
    // The last digit in the float portion of the double is a 0 followed by the half ulp.
    REPORTER_ASSERT(reporter, 1.0f == static_cast<float>(onePlusHalfUlp));

    // One more double ulp should round to 1 + 1 float ulp showing the break over point.
    REPORTER_ASSERT(reporter, 1.0f + floatUlp ==
            static_cast<float>(std::nexttoward(onePlusHalfUlp, infinity)));

    // Create the number so that the round to even rounds up.
    // 0b0'11111111110'0000'0000000000'0000000001'0000000000'0000000000'0000000000
    const double onePlus1Ulp = one + floatUlp;

    // Create an odd floating number to force the round up.
    // 0b0'11111111110'0000'0000000000'0000000001'1000000000'0000000000'0000000000
    const double onePlus1andAHalfUlp = onePlus1Ulp + floatUlp * 0.5;

    // You can see this rounded up to 1 + 2Ulps instead of rounding down to 1 + 1Ulp.
    REPORTER_ASSERT(reporter, 1.0f + 2.0f * floatUlp == static_cast<float>(onePlus1andAHalfUlp));

    // Decrease the number by a double ulp, and it should round down demonstrating the break over
    // point.
    REPORTER_ASSERT(reporter, 1.0f + floatUlp ==
        static_cast<float>(std::nexttoward(onePlus1andAHalfUlp, -infinity)));
}
