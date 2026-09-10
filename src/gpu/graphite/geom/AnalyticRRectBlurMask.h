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

#include <array>
#include <cmath>
#include <optional>

namespace skgpu::graphite {

class Recorder;
class Transform;

class AnalyticRRectBlurMask {
public:
    AnalyticRRectBlurMask() = delete;

    static std::optional<AnalyticRRectBlurMask> Make(Recorder*,
                                                     const Transform& localToDevice,
                                                     float localSigma,
                                                     const SkRRect& srcRRect);

    const SkRRect& rrect() const { return fRRect; }
    const SkM44& devToScaledShape() const { return fDevToScaledShape; }
    float localSigma() const { return fLocalSigma; }
    float sqrtHalfOverSigma() const { return fSqrtHalfOverSigma; }
    SkSpan<const SkV2> radii() const { return fRadii; }
    int radius() const { return fRadius; }
    sk_sp<TextureProxy> refCdfProxy() const { return fCdfLut; }
    Rect bounds() const {
        return Rect(fRRect.getBounds()).makeOutset(this->drawPad());
    }
    float drawPad() const { return std::ceil(3.f * fLocalSigma); }

private:
    AnalyticRRectBlurMask(const SkRRect& rrect,
                          const SkM44& devToScaledShape,
                          float sqrtHalfOverSigma,
                          float localSigma,
                          std::array<SkV2, 4> radii,
                          int radius,
                          sk_sp<TextureProxy> cdfLut)
            : fRRect(rrect)
            , fDevToScaledShape(devToScaledShape)
            , fSqrtHalfOverSigma(sqrtHalfOverSigma)
            , fLocalSigma(localSigma)
            , fRadii(radii)
            , fRadius(radius)
            , fCdfLut(std::move(cdfLut)) {}

    SkRRect fRRect;
    SkM44 fDevToScaledShape;
    float fSqrtHalfOverSigma;
    float fLocalSigma;
    std::array<SkV2, 4> fRadii;
    int fRadius;
    sk_sp<TextureProxy> fCdfLut;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_AnalyticRRectBlurMask_DEFINED
