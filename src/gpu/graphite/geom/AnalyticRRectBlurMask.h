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
#include "src/gpu/graphite/geom/Rect.h"

#include <array>
#include <optional>

namespace skgpu::graphite {

class Recorder;
class Transform;

class AnalyticRRectBlurMask {
public:
    AnalyticRRectBlurMask() = delete;

    static std::optional<AnalyticRRectBlurMask> Make(Recorder*,
                                                     const Transform& localToDevice,
                                                     float devSigma,
                                                     const SkRRect& srcRRect);

    const SkRRect& rrect() const { return fRRect; }
    const Rect& drawBounds() const { return fDrawBounds; }
    const SkM44& devToScaledShape() const { return fDevToScaledShape; }
    const Rect& shapeData() const { return fShapeData; }
    float localSigma() const { return fLocalSigma; }
    float sqrtHalfOverSigma() const { return fSqrtHalfOverSigma; }
    SkSpan<const SkV2> radii() const { return fRadii; }
    int radius() const { return fRadius; }

private:
    AnalyticRRectBlurMask(const SkRRect& rrect,
                          const Rect& drawBounds,
                          const SkM44& devToScaledShape,
                          const Rect& shapeData,
                          float sqrtHalfOverSigma,
                          float localSigma,
                          std::array<SkV2, 4> radii,
                          int radius)
            : fRRect(rrect)
            , fDrawBounds(drawBounds)
            , fDevToScaledShape(devToScaledShape)
            , fShapeData(shapeData)
            , fSqrtHalfOverSigma(sqrtHalfOverSigma)
            , fLocalSigma(localSigma)
            , fRadii(radii)
            , fRadius(radius) {}

    SkRRect fRRect;
    Rect fDrawBounds;
    SkM44 fDevToScaledShape;
    Rect fShapeData;
    float fSqrtHalfOverSigma;
    float fLocalSigma;
    std::array<SkV2, 4> fRadii;
    int fRadius;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_AnalyticRRectBlurMask_DEFINED
