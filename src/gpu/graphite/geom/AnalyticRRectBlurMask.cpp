/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/AnalyticRRectBlurMask.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkScalar.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkAssert.h"
#include "include/private/SkAttributes.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>

namespace skgpu::graphite {

namespace {

// Returns e^-z * I1(z), the exponentially scaled modified Bessel function of the first kind of
// order one, for z >= 0. This comes from "Handbook of Mathematical Functions" by Abramowitz and
// Stegun in chapters 9.8.3 and 9.8.4, |error| < 2.2e-7.
double scaled_bessel_i1(double z) {
    SkASSERT(z >= 0.0);
    if (z < 3.75) {
        const double y = (z / 3.75) * (z / 3.75);
        return std::exp(-z) * z *
               (0.5 + y * (0.87890594 + y * (0.51498869 + y * (0.15084934 +
                y * (0.02658733 + y * (0.00301532 + y * 0.00032411))))));
    }
    const double y = 3.75 / z;
    const double p = 0.02282967 + y * (-0.02895312 + y * (0.01787654 - y * 0.00420059));
    return (0.39894228 + y * (-0.03988024 + y * (-0.00362018 + y * (0.00163801 +
                         y * (-0.01031555 + y * p))))) / std::sqrt(z);
}

// Fills `row` (kCdfLutWidth texels) with the normalized radial profile of a circle of radius `r`,
// in units of sigma, convolved with a Gaussian. Texel i maps to the signed distance to the edge
// u = (r - t) / sqrt(2), so t = r - sqrt(2) * u is the distance to the circle's center. Texels
// with u > r / sqrt(2) would be "past" the center and are only reached by linear filtering (across
// the center, or when blending with the next larger circle's row). The profile is flat at the
// center, so they are clamped to the center's coverage.
//
// The coverage at distance t from the center is the CDF of the Rice distribution,
// P(t) = 1 - Q1(t, r) where Q1 is the Marcum Q-function. It has no closed form, but
//     P(0)  = 1 - e^(-r^2 / 2)
//     dP/dt = -r * e^(-(t^2 + r^2) / 2) * I1(r * t) = -r * e^(-(t - r)^2 / 2) * e^(-rt) * I1(r * t)
// so the row is computed by integrating dP/dt outward from the center. Each value is normalized by
// P(0) so that every row uses the full 8 bits, even when the blurred circle's peak coverage is
// tiny (sigma >> r). The shader scales the normalized value back by P(0).
void fill_circle_profile_row(double r, uint8_t* row) {
    constexpr int kWidth = AnalyticRRectBlurMask::kCdfLutWidth;
    constexpr float kRange = AnalyticRRectBlurMask::kCdfLutRange;
    const float sqrt2 = std::sqrt(2.0);

    auto texelU = [](int i) { return -kRange + 2.0 * kRange * (i + 0.5) / kWidth; };
    auto negDPdt = [r](double s) {
        return r * std::exp(-0.5 * (s - r) * (s - r)) * scaled_bessel_i1(r * s);
    };
    const float centerCoverage = -std::expm1(-0.5 * r * r);

    // Walking from the last texel to the first, the distance to the center never decreases (it is
    // 0 for every texel past the center), so a single pass accumulates the coverage lost so far.
    // dP/dt is negligible more than 12 sigma inside the circle's edge, so start there.
    float s = std::max(0.0, r - 12.0);
    float negDPdtAtS = negDPdt(s);
    float integral = 0.0;
    for (int i = kWidth - 1; i >= 0; --i) {
        const float t = std::max(0.0, r - sqrt2 * texelU(i));
        if (t > s) {
            // Composite Simpson's rule with an even number of steps no larger than 1/32 sigma.
            const int n = 2 * std::max(1, static_cast<int>(std::ceil((t - s) * 16.0)));
            const float h = (t - s) / n;
            const float negDPdtAtT = negDPdt(t);
            float sum = negDPdtAtS + negDPdtAtT;
            for (int k = 1; k < n; ++k) {
                sum += ((k & 1) ? 4.0 : 2.0) * negDPdt(s + k * h);
            }
            integral += sum * h / 3.0;
            s = t;
            negDPdtAtS = negDPdtAtT;
        }
        const float normalizedCoverage = (centerCoverage - integral) / centerCoverage;
        SK_UNSAFE_TODO(row[i]) = static_cast<uint8_t>(
                std::round(255.0 * std::clamp(normalizedCoverage, 0.0f, 1.0f)));
    }
}

SkBitmap create_cdf_lut() {
    constexpr int kWidth = AnalyticRRectBlurMask::kCdfLutWidth;
    constexpr int kHeight = AnalyticRRectBlurMask::kCdfLutHeight;
    constexpr float kRange = AnalyticRRectBlurMask::kCdfLutRange;

    SkBitmap bmp;
    if (!bmp.tryAllocPixels(SkImageInfo::Make(kWidth, kHeight,
                                              kR8_unorm_SkColorType,
                                              kOpaque_SkAlphaType))) {
        return bmp;
    }

    // For row 0, we have the gaussian CDF of a straight edge (half-plane) for x in
    // [-kRange, kRange], sampled with a half pixel offset. This is used by rects and rrects, and
    // for circles can be considered the case where radius is very large as taking the limit of
    // radius -> inf then sigma/radius = 0.
    uint8_t* halfPlaneRow = bmp.getAddr8(0, 0);
    for (int i = 0; i < kWidth; i++) {
        float x = -kRange + 2.0f * kRange * (i + 0.5f) / kWidth;
        float cdf = 0.5f + 0.5f * std::erf(x);
        SK_UNSAFE_TODO(halfPlaneRow[i]) = static_cast<uint8_t>(std::round(cdf * 255.0f));
    }

    // Rows [1, kHeight - 2]: blurred circles where row j corresponds to
    // w = sigma / (sigma + radius) = j / (kHeight - 1), i.e. radius / sigma = (1 - w) / w. This
    // spaces the rows linearly in sigma / radius near the half-plane limit and linearly in
    // radius / sigma near the point limit, where the profiles respectively vary the most.
    for (int j = 1; j < kHeight - 1; j++) {
        const double w = static_cast<double>(j) / (kHeight - 1);
        fill_circle_profile_row((1.0 - w) / w, bmp.getAddr8(0, j));
    }

    // For the last row, we have the case where the sigma is much larger than the radius, then we
    // can simplify the blur profile by treating it as a blur from a point. So we can treat each
    // x value as the gaussian weight based off of the distance to the center of the circle, x = 0.
    uint8_t* pointRow = bmp.getAddr8(0, kHeight - 1);
    for (int i = 0; i < kWidth; i++) {
        const float x = -kRange + 2.0 * kRange * (i + 0.5) / kWidth;
        SK_UNSAFE_TODO(pointRow[i]) = static_cast<uint8_t>(std::round(255.0 * std::exp(-x * x)));
    }

    bmp.setImmutable();
    return bmp;
}

}  // anonymous namespace

std::optional<AnalyticRRectBlurMask> AnalyticRRectBlurMask::Make(
                                            Recorder* recorder,
                                            const Transform& localToDeviceTransform,
                                            SkV2 localSigma,
                                            const SkRRect& srcRRect) {
    UniqueKey key;
    {
        static const UniqueKey::Domain kCdfLutDomain = UniqueKey::GenerateDomain();
        UniqueKey::Builder builder(&key, kCdfLutDomain, 0, "AnalyticRRectBlurCdfLut");
    }
    sk_sp<TextureProxy> cdfLut = recorder->priv().proxyCache()->findOrCreateCachedProxy(
            recorder, key, nullptr,
            [](const void*) -> SkBitmap {
                // TEMP timing - DO NOT LAND
                constexpr int kIterations = 2000;
                SkBitmap bmp;
                const auto start = std::chrono::steady_clock::now();
                for (int iter = 0; iter < kIterations; ++iter) {
                    bmp = create_cdf_lut();
                }
                const std::chrono::duration<double, std::micro> elapsed =
                        std::chrono::steady_clock::now() - start;
                printf("AnalyticRRectBlurMask CDF LUT: %.3f us avg over %d iterations (%.3f ms total)\n",
                       elapsed.count() / kIterations, kIterations, elapsed.count() / 1000.0);
                fflush(stdout);
                return bmp;
            });

    if (!cdfLut) {
        return std::nullopt;
    }

    return AnalyticRRectBlurMask(srcRRect,
                                 localSigma,
                                 std::move(cdfLut));
}

SkV2 AnalyticRRectBlurMask::circleParams() const {
    SkASSERT(this->isCircleBlur());
    const float sigma = fLocalSigma.x;
    const float radius = fRRect.radii(SkRRect::kUpperLeft_Corner).fX;

    // Map w = sigma / (sigma + radius) in [0, 1] onto the centers of rows [0, kCdfLutHeight - 1]
    // (see create_cdf_lut()); the sampler interpolates between the two closest rows.
    const float w = sigma / (sigma + radius);
    const float lutRow = (w * (kCdfLutHeight - 1) + 0.5f) / kCdfLutHeight;

    // The coverage at the center of the blurred circle, which the normalized rows are scaled by.
    const float r = radius / sigma;
    const float centerCoverage = -std::expm1(-0.5f * r * r);

    return {lutRow, centerCoverage};
}

}  // namespace skgpu::graphite
