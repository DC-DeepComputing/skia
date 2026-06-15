/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GradientBitmap.h"

#include "include/private/SkAssert.h"
#include "src/core/SkHalf.h"

#include <cmath>

namespace skgpu {

bool EncodeGradientStopToHalf(float offset, float* mantissa, float* exponent) {
    int exp;
    float mant = std::frexp(offset, &exp);

    SkHalf halfE = SkFloatToHalf(static_cast<float>(exp));
    if (static_cast<int>(SkHalfToFloat(halfE)) != exp) {
        return false;
    }

#if defined(SK_DEBUG)
    SkHalf halfM = SkFloatToHalf(mant);
    float restored = std::ldexp(SkHalfToFloat(halfM), exp);
    SkASSERT(std::abs(restored - offset) < 0.001f);
#endif

    *mantissa = mant;
    *exponent = static_cast<float>(exp);
    return true;
}

}  // namespace skgpu
