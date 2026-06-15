/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GradientBitmap_DEFINED
#define skgpu_GradientBitmap_DEFINED

namespace skgpu {

// Gradient stop offsets / interval thresholds live in [0, 1] but lose too much
// precision when rounded to half floats directly. Encoding them as the
// (mantissa, exponent) pair returned by frexp preserves the value across an F16
// texture; the shader recovers it with ldexp(mantissa, exponent).
//
// On success, writes the mantissa and exponent and returns true. Returns false
// if the exponent does not round-trip through F16, in which case the caller must
// not use the texture encoding.
bool EncodeGradientStopToHalf(float offset, float* mantissa, float* exponent);

}  // namespace skgpu

#endif  // skgpu_GradientBitmap_DEFINED
