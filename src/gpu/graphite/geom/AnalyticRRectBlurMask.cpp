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
        //return std::nullopt;
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
    float sigmaX = localSigma;
    float sigmaY = localSigma;
    float sqrtHalfOverSigma[2] = { float{1.f/SK_FloatSqrt2} / sigmaX,
                                   float{1.f/SK_FloatSqrt2} / sigmaY };
    SkISize radius = { static_cast<int>(ceil(6 * sigmaX)) / 2,
                       static_cast<int>(ceil(6 * sigmaY)) / 2 };

    float drawPadX = std::ceil(3.f * sigmaX);
    float drawPadY = std::ceil(3.f * sigmaY);
    Rect drawBounds = srcRRect.rect().makeOutset(drawPadX, drawPadY);

    return AnalyticRRectBlurMask(drawBounds,
                                 SkM44(devToLocal),
                                 shapeData,
                                 {sqrtHalfOverSigma[0], sqrtHalfOverSigma[1]},
                                 std::to_array(newRadii),
                                 radius);
}

}  // namespace skgpu::graphite
