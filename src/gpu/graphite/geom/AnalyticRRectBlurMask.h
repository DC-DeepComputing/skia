/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_AnalyticRRectBlurMask_DEFINED
#define skgpu_graphite_geom_AnalyticRRectBlurMask_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Rect.h"

#include <cmath>
#include <optional>

namespace skgpu::graphite {

class Recorder;
class Transform;

class AnalyticRRectBlurMask {
public:
    // All blur profiles are stored in a single cached R8 texture of kCdfLutWidth x kCdfLutHeight.
    // The x axis spans u in [-kCdfLutRange, kCdfLutRange], where u is the signed distance to the
    // shape's edge (positive inside) divided by sqrt(2) * sigma.
    //
    // Row 0 holds the Gaussian CDF of a straight edge (a half-plane), which is all that rects and
    // rrects need. The remaining rows hold the radial profile of a blurred circle, parameterized by
    // w = sigma / (sigma + radius): row 0 is the half-plane limit (w = 0, radius >> sigma) and the
    // last row is the limit of a single point (w = 1, sigma >> radius). Each circle row is
    // normalized by the coverage at the circle's center so that it uses the full 8 bits, and the
    // shader scales the sampled value back by that coverage (see circleParams()).
    static constexpr int   kCdfLutWidth  = 256;
    static constexpr int   kCdfLutHeight = 32;
    static constexpr float kCdfLutRange  = 4.f;

    AnalyticRRectBlurMask() = delete;

    static std::optional<AnalyticRRectBlurMask> Make(Recorder*,
                                                     const Transform& localToDevice,
                                                     SkV2 localSigma,
                                                     const SkRRect& srcRRect);

    const SkRRect& rrect() const { return fRRect; }
    SkV2 localSigma() const { return fLocalSigma; }
    sk_sp<TextureProxy> refCdfProxy() const { return fCdfLut; }
    Rect bounds() const {
        return Rect(fRRect.getBounds()).makeOutset(skvx::float2{this->drawPadX(),
                                                                this->drawPadY()});
    }
    float drawPadX() const { return std::ceil(3.f * fLocalSigma.x); }
    float drawPadY() const { return std::ceil(3.f * fLocalSigma.y); }
    SkV2 drawPad() const { return {this->drawPadX(), this->drawPadY()}; }

    // True when the rrect is a circle blurred by an isotropic (local space) sigma, in which case
    // the coverage only depends on the distance to the circle's center and can be read directly
    // from one of the circle rows of the CDF LUT.
    bool isCircle() const { return fIsCircle; }

    // Only valid when isCircle() is true. Returns:
    //   x: the v texture coordinate of the CDF LUT for this circle's sigma / radius ratio, which
    //      lies between two rows that the sampler linearly interpolates.
    //   y: the coverage at the center of the blurred circle, 1 - exp(-radius^2 / (2 * sigma^2)),
    //      which the normalized circle profile is multiplied by.
    SkV2 circleParams() const;

private:
    AnalyticRRectBlurMask(const SkRRect& rrect,
                          SkV2 localSigma,
                          bool isCircle,
                          sk_sp<TextureProxy> cdfLut)
            : fRRect(rrect)
            , fLocalSigma(localSigma)
            , fIsCircle(isCircle)
            , fCdfLut(std::move(cdfLut)) {}

    SkRRect fRRect;
    SkV2 fLocalSigma;
    bool fIsCircle;
    sk_sp<TextureProxy> fCdfLut;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_AnalyticRRectBlurMask_DEFINED
