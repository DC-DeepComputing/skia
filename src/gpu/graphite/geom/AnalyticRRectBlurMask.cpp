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
#include "include/private/SkFloatingPoint.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/BlurUtils.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace skgpu::graphite {

namespace {

// Fills `row` (kCdfLutWidth texels) with the normalized radial profile of a circle of radius `r`,
// in units of sigma, convolved with a Gaussian. Texel i maps to the signed distance to the edge
// u = (r - t) / sqrt(2), so t = |r - sqrt(2) * u| is the distance to the circle's center. Texels
// with u > r / sqrt(2) would be "past" the center; they are mirrored so that linear filtering
// around the center stays smooth, but are never sampled directly.
//
// The profile comes from CreateCircleProfile() in BlurUtils, the same code and resolution that the
// legacy AnalyticBlurMask uses for blurred circles: kProfileWidth texels spanning the distances
// from the circle's center to 3 sigma past its edge, which are linearly interpolated here. Each
// value is the coverage straight from the profile, not normalized by the coverage at the circle's
// center, so that the LUT holds the same values the legacy profile does and the shader can use the
// sampled value directly.
//
// This only runs once on the CPU, when the LUT is created. Returns false if the profile couldn't
// be allocated.
bool fill_circle_profile_row(double r, uint8_t* row) {
    constexpr int kWidth = AnalyticRRectBlurMask::kCdfLutWidth;
    constexpr float kRange = AnalyticRRectBlurMask::kCdfLutRange;
    constexpr int kProfileWidth = 512;

    const float radius = static_cast<float>(r);
    // Profile texels per sigma, matching the kProfileWidth texels per (radius + 3 sigma) of
    // AnalyticBlurMask::MakeCircle().
    const float scale = kProfileWidth / (radius + 3.f);
    const SkBitmap profile = CreateCircleProfile(scale, radius * scale, kProfileWidth);
    if (profile.empty()) {
        return false;
    }
    const uint8_t* profileTexels = profile.getAddr8(0, 0);

    for (int i = 0; i < kWidth; ++i) {
        const float u = -kRange + 2.f * kRange * (i + 0.5f) / kWidth;
        const float t = std::abs(radius - SK_FloatSqrt2 * u);

        // Profile texel k holds the coverage at a distance of (k + 0.5) / scale from the center.
        const float x = std::clamp(t * scale - 0.5f, 0.f, kProfileWidth - 1.f);
        const int k0 = static_cast<int>(x);
        const int k1 = std::min(k0 + 1, kProfileWidth - 1);
        const float f = x - k0;
        const float coverage0 = SK_UNSAFE_TODO(profileTexels[k0]);
        const float coverage1 = SK_UNSAFE_TODO(profileTexels[k1]);

        SK_UNSAFE_TODO(row[i]) = static_cast<uint8_t>(
                std::round(coverage0 + f * (coverage1 - coverage0)));
    }
    return true;
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
        if (!fill_circle_profile_row((1.0 - w) / w, bmp.getAddr8(0, j))) {
            return {};
        }
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
            [](const void*) -> SkBitmap { return create_cdf_lut(); });

    if (!cdfLut) {
        return std::nullopt;
    }

    // The circle case requires that the local sigma is isotrophic, this is so we can assume
    // radial symmetry of the blur.
    const bool isCircle = SkRRectPriv::IsCircle(srcRRect) &&
                          SkScalarNearlyEqual(localSigma.x, localSigma.y);

    return AnalyticRRectBlurMask(srcRRect,
                                 localSigma,
                                 isCircle,
                                 std::move(cdfLut));
}

SkV2 AnalyticRRectBlurMask::circleParams() const {
    SkASSERT(fIsCircle);
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
