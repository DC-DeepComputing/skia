/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/RectBlurData.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRRect.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/BlurUtils.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {

namespace {

std::optional<Rect> outset_bounds(const SkMatrix& localToDevice,
                                  float devSigma,
                                  const SkRect& srcRect) {
    float outsetX = 3.0f * devSigma;
    float outsetY = 3.0f * devSigma;
    if (localToDevice.isScaleTranslate()) {
        outsetX /= std::fabs(localToDevice.getScaleX());
        outsetY /= std::fabs(localToDevice.getScaleY());
    } else {
        SkSize scale;
        if (!localToDevice.decomposeScale(&scale, nullptr)) {
            return std::nullopt;
        }
        outsetX /= scale.width();
        outsetY /= scale.height();
    }
    return srcRect.makeOutset(outsetX, outsetY);
}

}  // anonymous namespace

std::optional<RectBlurData> RectBlurData::Make(Recorder* recorder,
                                               const Transform& localToDeviceTransform,
                                               float deviceSigma,
                                               const SkRRect& srcRRect) {
    // TODO: Implement SkMatrix functionality used below for Transform.
    SkMatrix localToDevice = localToDeviceTransform;

    const bool canBeRect = srcRRect.isRect() && localToDevice.preservesRightAngles();
    if (canBeRect) {
        return MakeRect(recorder, localToDevice, deviceSigma, srcRRect.rect());
    }

    SkRRect devRRect;
    const bool devRRectIsCircle = srcRRect.transform(localToDevice, &devRRect) &&
                                  SkRRectPriv::IsCircle(devRRect);
    const bool canBeCircle = devRRectIsCircle ||
                             (SkRRectPriv::IsCircle(srcRRect) && localToDevice.isSimilarity());
    if (canBeCircle) {
        const SkRect srcRect = srcRRect.rect();
        SkRect devRect;
        if (devRRectIsCircle) {
            devRect = devRRect.rect();
        } else {
            const SkPoint center = localToDevice.mapPoint(srcRect.center());
            const float radius = localToDevice.mapVector(0, srcRect.width() / 2.0f).length();
            devRect = {center.x() - radius,
                       center.y() - radius,
                       center.x() + radius,
                       center.y() + radius};
        }
        return MakeCircle(recorder, localToDevice, deviceSigma, srcRect, devRect);
    }

    // TODO(b/238762890) Support analytic blurring with rrects.
    return std::nullopt;
}

std::optional<RectBlurData> RectBlurData::MakeRect(Recorder* recorder,
                                                   const SkMatrix& localToDevice,
                                                   float devSigma,
                                                   const SkRect& srcRect) {
    SkASSERT(srcRect.isSorted());

    SkRect devRect;
    SkMatrix devToScaledShape;
    if (localToDevice.rectStaysRect()) {
        // We can do everything in device space when the src rect projects to a rect in device
        // space.
        SkAssertResult(localToDevice.mapRect(&devRect, srcRect));

    } else {
        // The view matrix may scale, perhaps anisotropically. But we want to apply our device space
        // sigma to the delta of frag coord from the rect edges. Factor out the scaling to define a
        // space that is purely rotation / translation from device space (and scale from src space).
        // We'll meet in the middle: pre-scale the src rect to be in this space and then apply the
        // inverse of the rotation / translation portion to the frag coord.
        SkMatrix m;
        SkSize scale;
        if (!localToDevice.decomposeScale(&scale, &m)) {
            return std::nullopt;
        }
        if (!m.invert(&devToScaledShape)) {
            return std::nullopt;
        }
        devRect = {srcRect.left() * scale.width(),
                   srcRect.top() * scale.height(),
                   srcRect.right() * scale.width(),
                   srcRect.bottom() * scale.height()};
    }

    if (!recorder->priv().caps()->shaderCaps()->fFloatIs32Bits) {
        // We promote the math that gets us into the Gaussian space to full float when the rect
        // coords are large. If we don't have full float then fail. We could probably clip the rect
        // to an outset device bounds instead.
        if (std::fabs(devRect.left()) > 16000.0f || std::fabs(devRect.top()) > 16000.0f ||
            std::fabs(devRect.right()) > 16000.0f || std::fabs(devRect.bottom()) > 16000.0f) {
            return std::nullopt;
        }
    }

    const float sixSigma = 6.0f * devSigma;
    SkBitmap integralBitmap = skgpu::CreateIntegralTable(sixSigma);
    if (integralBitmap.empty()) {
        return std::nullopt;
    }

    sk_sp<TextureProxy> integral = RecorderPriv::CreateCachedProxy(recorder, integralBitmap);
    if (!integral) {
        return std::nullopt;
    }

    // In the fast variant we think of the midpoint of the integral texture as aligning with the
    // closest rect edge both in x and y. To simplify texture coord calculation we inset the rect so
    // that the edge of the inset rect corresponds to t = 0 in the texture. It actually simplifies
    // things a bit in the !isFast case, too.
    const float threeSigma = 3.0f * devSigma;
    const Rect shapeData = Rect(devRect.left() + threeSigma,
                                devRect.top() + threeSigma,
                                devRect.right() - threeSigma,
                                devRect.bottom() - threeSigma);

    // In our fast variant we find the nearest horizontal and vertical edges and for each do a
    // lookup in the integral texture for each and multiply them. When the rect is less than 6*sigma
    // wide then things aren't so simple and we have to consider both the left and right edge of the
    // rectangle (and similar in y).
    const bool isFast = shapeData.left() <= shapeData.right() && shapeData.top() <= shapeData.bot();

    const float invSixSigma = 1.0f / sixSigma;

    // Determine how much to outset the draw bounds to ensure we hit pixels within 3*sigma.
    std::optional<Rect> drawBounds = outset_bounds(localToDevice, devSigma, srcRect);
    if (!drawBounds) {
        return std::nullopt;
    }

    return RectBlurData(*drawBounds,
                        SkM44(devToScaledShape),
                        shapeData,
                        ShapeType::kRect,
                        isFast,
                        invSixSigma,
                        integral);
}

std::optional<RectBlurData> RectBlurData::MakeCircle(Recorder* recorder,
                                                     const SkMatrix& localToDevice,
                                                     float devSigma,
                                                     const SkRect& srcRect,
                                                     const SkRect& devRect) {
    const float radius = devRect.width() / 2.0f;
    if (!sk_float_isfinite(radius) || radius < SK_ScalarNearlyZero) {
        return std::nullopt;
    }

    // When sigma is really small this becomes a equivalent to convolving a Gaussian with a
    // half-plane. Similarly, in the extreme high ratio cases circle becomes a point WRT to the
    // Guassian and the profile texture is a just a Gaussian evaluation. However, we haven't yet
    // implemented this latter optimization.
    constexpr float kHalfPlaneThreshold = 0.1f;
    const float sigmaToRadiusRatio = std::min(devSigma / radius, 8.0f);
    const bool useHalfPlaneApprox = sigmaToRadiusRatio <= kHalfPlaneThreshold;

    float solidRadius;
    float textureRadius;
    if (useHalfPlaneApprox) {
        solidRadius = radius - 3.0f * devSigma;
        textureRadius = 6.0f * devSigma;
    } else {
        devSigma = radius * sigmaToRadiusRatio;
        solidRadius = 0.0f;
        textureRadius = radius + 3.0f * devSigma;
    }

    constexpr int kProfileTextureWidth = 512;

    SkBitmap profileBitmap;
    if (useHalfPlaneApprox) {
        profileBitmap = skgpu::CreateHalfPlaneProfile(kProfileTextureWidth);
    } else {
        // Rescale params to the size of the texture we're creating.
        const float scale = kProfileTextureWidth / textureRadius;
        profileBitmap =
                skgpu::CreateCircleProfile(devSigma * scale, radius * scale, kProfileTextureWidth);
    }
    profileBitmap.setImmutable();

    sk_sp<TextureProxy> profile = RecorderPriv::CreateCachedProxy(recorder, profileBitmap);
    if (!profile) {
        return std::nullopt;
    }

    const Rect shapeData =
            Rect(devRect.centerX(), devRect.centerY(), solidRadius, 1.0f / textureRadius);

    // Determine how much to outset the draw bounds to ensure we hit pixels within 3*sigma.
    std::optional<Rect> drawBounds = outset_bounds(localToDevice, devSigma, srcRect);
    if (!drawBounds) {
        return std::nullopt;
    }

    constexpr float kUnusedBlurData = 0.0f;
    return RectBlurData(*drawBounds,
                        SkM44(),
                        shapeData,
                        ShapeType::kCircle,
                        kUnusedBlurData,
                        kUnusedBlurData,
                        profile);
}

}  // namespace skgpu::graphite
