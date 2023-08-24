/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_BlurUtils_DEFINED
#define skgpu_BlurUtils_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkFloatingPoint.h"

#include <array>

// TODO(b/): Many of these utilities could be lifted even into src/core as part of the backend
// agnostic blur engine once that API exists.

class SkRuntimeEffect;
struct SkISize;
struct SkSize;

namespace skgpu {

// The kernel width of a Gaussian blur of the given pixel radius, for when all pixels are sampled.
constexpr int BlurKernelWidth(int radius) { return 2 * radius + 1; }

// The kernel width of a Gaussian blur of the given pixel radius, that relies on HW bilinear
// filtering to combine adjacent pixels.
constexpr int BlurLinearKernelWidth(int radius) { return radius + 1; }

// Any sigmas smaller than this are effectively an identity blur so can skip convolution at a higher
// level. The value was chosen because it corresponds roughly to a radius of 1/10px, and because
// 2*sigma^2 is slightly greater than SK_ScalarNearlyZero.
constexpr bool BlurIsEffectivelyIdentity(float sigma) { return sigma <= 0.03f; }

// Convert from a sigma Gaussian standard deviation to a pixel radius such that pixels outside the
// radius would have an insignificant contribution to the final blurred value.
inline int BlurSigmaRadius(float sigma) {
    // sk_float_ceil is not constexpr
    return BlurIsEffectivelyIdentity(sigma) ? 0 : sk_float_ceil2int(3.f * sigma);
}

// Maximum sigma that can be computed with the 1D and 2D effects returned below. Larger sigmas must
// have the input image downscaled to use a lower effective sigma.
// static constexpr float kMaxSigma = 4.f;

// The maximum sigma that can be computed without downscaling is based on the number of uniforms and
// texture samples the effects will make in a single pass. For 1D passes, the number of samples
// is equal to `BlurLinearKernelWidth`; for 2D passes, it is equal to
// `BlurKernelWidth(radiusX)*BlurKernelWidth(radiusY)`. This maps back to different maximum sigmas
// depending on the approach used, as well as the ratio between the sigmas for the X and Y axes if
// a 2D blur is performed.
static constexpr int kMaxBlurSamples = 28;

// TODO: Update max linear sigma to 9; it had been 4 when a full 1D kernel was used, but never
// updated after the linear filtering optimization reduced the number of sample() calls required.
// Keep it at 4 for now to better isolate performance changes due to switching to a runtime effect
// and constant loop structure.
static constexpr float kMaxLinearBlurSigma = 4.f; // -> radius = 27 -> linear kernel width = 28
// There is no defined kMaxBlurSigma for direct 2D blurs since it is entirely dependent on the ratio
// between the two axes' sigmas, but generally it will be small on the order of a 5x5 kernel.


// FIXME should this be a pointer to kernel and offsets?
// should we accept spans?
void Compute1DBlurLinearKernel(float sigma,
                               int radius,
                               std::array<float, kMaxBlurSamples>& kernel,
                               std::array<float, kMaxBlurSamples>& offsets);

void Compute2DBlurKernel(const SkSize& sigma,
                         const SkISize& radius,
                         std::array<float, kMaxBlurSamples>& kernel);

}

#endif // skgpu_BlurUtils_DEFINED
