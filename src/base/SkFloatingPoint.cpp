/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFloatingPoint.h"

#include "include/private/base/SkAssert.h"

#include <algorithm>
#include <cmath>
#include <limits>

// The ulp that a floating point number has with an exponent of 0.
constexpr static double sk_double_epsilon = std::numeric_limits<float>::epsilon();
#if 1
bool sk_doubles_nearly_equal_ulps(double a, double b, uint8_t max_ulps_diff) {
    const double smallestExponent = std::numeric_limits<double>::min_exponent;
    const double smallestCohort = std::pow(2.0, smallestExponent);
    const double normalize = std::max(std::max(std::abs(a), std::abs(b)), smallestCohort);
    const double specialK = (std::numeric_limits<double>::epsilon() / 2.0) +
            (std::numeric_limits<double>::epsilon() / 8.0);
    const double errorFactor = max_ulps_diff * specialK;
    return a == b || std::abs(b - a) < normalize * errorFactor;
}
#else
static inline int64_t double_to_twos_complement_bits(double x) {
    // Convert a double to its bit pattern
    int64_t bits = 0;
    static_assert(sizeof(x) == sizeof(bits));
    std::memcpy(&bits, &x, sizeof(bits));
    // Convert a sign-bit int (i.e. double interpreted as int) into a 2s complement
    // int. This also converts -0 (0x8000000000000000) to 0. Doing this to a double allows
    // it to be compared using normal C operators (<, <=, etc.)
    if (bits < 0) {
        // Strip double sign bit.
        bits &= 0x7FFF'FFFF'FFFF'FFFF;
        bits = -bits;
    }
    return bits;
}

bool sk_doubles_nearly_equal_ulps(double a, double b, uint8_t max_ulps_diff) {
    // If both of these are zero (or very close), then using Units in the Last Place
    // will not be accurate, and we should use sk_double_nearly_zero instead.
    SkASSERT(!(fabs(a) < sk_double_epsilon && fabs(b) < sk_double_epsilon));
    // This algorithm does not work if both inputs are NaN.
    SkASSERT(!(std::isnan(a) && std::isnan(b)));
    // If both inputs are infinity (or actually equal), this catches it.
    if (a == b) {
        return true;
    }
    int64_t aBits = double_to_twos_complement_bits(a);
    int64_t bBits = double_to_twos_complement_bits(b);

    // Find the difference in Units in the Last Place (ULPs).
    // Read this line as:
    //   aBits - bBits < max_ulps_diff && bBits - aBits < max_ulps_diff;
    return aBits < bBits + max_ulps_diff && bBits < aBits + max_ulps_diff;
}
#endif

bool sk_double_nearly_zero(double a) {
    return a == 0 || fabs(a) < sk_double_epsilon;
}
