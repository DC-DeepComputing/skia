/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkSafeMath.h"
#include "tests/Test.h"

#include <cstddef>
#include <limits>

DEF_TEST(SafeMath, r) {
    size_t max = std::numeric_limits<size_t>::max();

    {
        size_t halfMax = max >> 1;
        size_t halfMaxPlus1 = halfMax + 1;
        SkSafeMath safe;
        REPORTER_ASSERT(r, safe.add(halfMax, halfMax) == 2 * halfMax);
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, safe.add(halfMax, halfMaxPlus1) == max);
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, safe.add(max, 1) == 0);
        REPORTER_ASSERT(r, !safe);
    }

    {
        SkSafeMath safe;
        (void) safe.add(max, max);
        REPORTER_ASSERT(r, !safe);
    }

    {
        size_t bits = (sizeof(size_t) * 8);
        size_t halfBits = bits / 2;
        size_t sqrtMax = max >> halfBits;
        size_t sqrtMaxPlus1 = sqrtMax + 1;
        SkSafeMath safe;
        REPORTER_ASSERT(r, safe.mul(sqrtMax, sqrtMax) == sqrtMax * sqrtMax);
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, safe.mul(sqrtMax, sqrtMaxPlus1) == sqrtMax << halfBits);
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, safe.mul(sqrtMaxPlus1, sqrtMaxPlus1) == 0);
        REPORTER_ASSERT(r, !safe);
    }

    {
        SkSafeMath safe;
        (void) safe.mul(max, max);
        REPORTER_ASSERT(r, !safe);
    }

    {
        int maxInt = std::numeric_limits<int>::max();
        int minInt = std::numeric_limits<int>::min();
        int sqrtMax = 46340; // floor(sqrt(std::numeric_limits<int>::max()))

        SkSafeMath safe;

        // Zero and one identity
        REPORTER_ASSERT(r, safe.mulInt(maxInt, 1) == maxInt);
        REPORTER_ASSERT(r, safe.ok());
        REPORTER_ASSERT(r, safe.mulInt(maxInt, 0) == 0);
        REPORTER_ASSERT(r, safe.ok());

        // Simple positive overflow
        (void)safe.mulInt(maxInt, 2);
        REPORTER_ASSERT(r, !safe.ok());

        // Positive overflow just over the limit
        safe = {};
        (void)safe.mulInt(sqrtMax + 1, sqrtMax + 1);
        REPORTER_ASSERT(r, !safe.ok());

        // Underflow with a negative multiplier
        safe = {};
        (void)safe.mulInt(maxInt, -2);
        REPORTER_ASSERT(r, !safe.ok());

        // Underflow with minInt
        safe = {};
        (void)safe.mulInt(minInt, 2);
        REPORTER_ASSERT(r, !safe.ok());

        // Overflow with two negative numbers
        safe = {};
        (void)safe.mulInt(minInt, minInt);
        REPORTER_ASSERT(r, !safe.ok());

        // Two's complement minInt * -1
        safe = {};
        (void)safe.mulInt(minInt, -1);
        REPORTER_ASSERT(r, !safe.ok());
    }
}
