// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Int96.h"

#include <cstdint>
#include <tuple>

namespace bentleyottmann {

Int96 Int96::Make(int32_t a) {
    return {a >= 0 ? 0 : -1, (uint32_t)a};
}

Int96 Int96::Make(int64_t a) {
    return {a >> 32, (uint32_t)(a & 0xFFFFFFFF)};
}

bool operator== (const Int96& a, const Int96& b) {
    return std::tie(a.hi, a.lo) == std::tie(b.hi, b.lo);
}

bool operator< (const Int96& a, const Int96& b) {
    return std::tie(a.hi, a.lo) < std::tie(b.hi, b.lo);
}

Int96 operator+ (const Int96& a, const Int96& b) {
    uint32_t lo = a.lo + b.lo;
    int64_t hi = a.hi + b.hi + (lo < a.lo);
    return {hi, lo};
}

Int96 multiply(int64_t a, int32_t b) {
    // Multiply the low 32-bits generating a 64-bit lower part.
    uint64_t loA = a & 0xFFFFFFFF;
    uint64_t loB = (uint32_t)b;
    uint64_t newLo = loA * loB;

    // Multiply the upper bits as signed as 32-bits resulting in the hi 64-bits of the Int96.
    // Remember that newLo is unsigned so will be zero filled by the shift.
    int64_t newHi = (a >> 32) * (int64_t)b;

    // Calculate the overflow into the upper 32-bits.
    int64_t lowOverflow = newLo >> 32;

    // Add overflow from the low multiply into hi.
    newHi += lowOverflow;

    // Compensate for the negative b in the calculation of newLo by subtracting out 2^32 * a.
    if (b < 0) {
        newHi -= loA;
    }

    return {newHi, (uint32_t)newLo};
}

Int96 multiply(int32_t a, int64_t b) {
    return multiply(b, a);
}

}  // namespace bentleyottmann
