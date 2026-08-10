/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/AnalyticRRectBlurMask.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkScalar.h"
#include "src/gpu/graphite/geom/Transform.h"

namespace skgpu::graphite {

std::optional<AnalyticRRectBlurMask> AnalyticRRectBlurMask::Make(Recorder* recorder,
                                                                 const Transform& localToDeviceTransform,
                                                                 float devSigma,
                                                                 const SkRRect& srcRRect) {
    if (devSigma <= 0.f) {
        return std::nullopt;
    }

    SkMatrix localToDevice = localToDeviceTransform;
    SkMatrix devToLocal;
    if (!localToDevice.invert(&devToLocal)) {
        return std::nullopt;
    }

    SkV2 newRadii[4];
    newRadii[0] = {srcRRect.radii(SkRRect::kUpperLeft_Corner).fX, srcRRect.radii(SkRRect::kUpperLeft_Corner).fY};
    newRadii[1] = {srcRRect.radii(SkRRect::kUpperRight_Corner).fX, srcRRect.radii(SkRRect::kUpperRight_Corner).fY};
    newRadii[2] = {srcRRect.radii(SkRRect::kLowerRight_Corner).fX, srcRRect.radii(SkRRect::kLowerRight_Corner).fY};
    newRadii[3] = {srcRRect.radii(SkRRect::kLowerLeft_Corner).fX, srcRRect.radii(SkRRect::kLowerLeft_Corner).fY};

    const Rect shapeData = Rect(srcRRect.rect());

    float sx = localToDevice.mapVector(1.f, 0.f).length();
    float sy = localToDevice.mapVector(0.f, 1.f).length();
    float localSigma = devSigma / std::sqrt(sx * sy);
    float sqrtHalfOverSigma = (1.f / SK_FloatSqrt2) / localSigma;
    int radius = static_cast<int>(ceil(6 * localSigma)) / 2;

    float drawPad = std::ceil(3.f * localSigma);
    Rect drawBounds = srcRRect.rect().makeOutset(drawPad, drawPad);

    return AnalyticRRectBlurMask(drawBounds,
                                 SkM44(devToLocal),
                                 shapeData,
                                 sqrtHalfOverSigma,
                                 std::to_array(newRadii),
                                 radius);
}

}  // namespace skgpu::graphite
