/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradientShader_DEFINED
#define SkGradientShader_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"

class SkColorSpace;

/** \class SkGradientShader

    SkGradientShader hosts factories for creating subclasses of SkShader that
    render linear and radial gradients. In general, degenerate cases should not
    produce surprising results, but there are several types of degeneracies:

     * A linear gradient made from the same two points.
     * A radial gradient with a radius of zero.
     * A sweep gradient where the start and end angle are the same.
     * A two point conical gradient where the two centers and the two radii are
       the same.

    For any degenerate gradient with a decal tile mode, it will draw empty since the interpolating
    region is zero area and the outer region is discarded by the decal mode.

    For any degenerate gradient with a repeat or mirror tile mode, it will draw a solid color that
    is the average gradient color, since infinitely many repetitions of the gradients will fill the
    shape.

    For a clamped gradient, every type is well-defined at the limit except for linear gradients. The
    radial gradient with zero radius becomes the last color. The sweep gradient draws the sector
    from 0 to the provided angle with the first color, with a hardstop switching to the last color.
    When the provided angle is 0, this is just the solid last color again. Similarly, the two point
    conical gradient becomes a circle filled with the first color, sized to the provided radius,
    with a hardstop switching to the last color. When the two radii are both zero, this is just the
    solid last color.

    As a linear gradient approaches the degenerate case, its shader will approach the appearance of
    two half planes, each filled by the first and last colors of the gradient. The planes will be
    oriented perpendicular to the vector between the two defining points of the gradient. However,
    once they become the same point, Skia cannot reconstruct what that expected orientation is. To
    provide a stable and predictable color in this case, Skia just uses the last color as a solid
    fill to be similar to many of the other degenerate gradients' behaviors in clamp mode.
*/
class SK_API SkGradientShader {
public:
    enum Flags {
        /** By default gradients will interpolate their colors in unpremul space
         *  and then premultiply each of the results. By setting this flag, the
         *  gradients will premultiply their colors first, and then interpolate
         *  between them.
         *  example: https://fiddle.skia.org/c/@GradientShader_MakeLinear
         */
        kInterpolateColorsInPremul_Flag = 1 << 0,
    };

    // (DEPRECATED) All these factor fucntions are deprecated. Please use the ones in
    // SkShaders.
    static sk_sp<SkShader> MakeLinear(const SkPoint pts[2],
                                      const SkColor colors[], const SkScalar pos[], int count,
                                      SkTileMode mode,
                                      uint32_t flags, const SkMatrix* localMatrix) {
        return SkShaders::LinearGradient(pts, colors, pos, count, mode, flags, localMatrix);
    }

    static sk_sp<SkShader> MakeLinear(const SkPoint pts[2],
                                      const SkColor colors[], const SkScalar pos[], int count,
                                      SkTileMode mode) {
        return SkShaders::LinearGradient(pts, colors, pos, count, mode);
    }

    static sk_sp<SkShader> MakeLinear(const SkPoint pts[2],
                                      const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                                      const SkScalar pos[], int count, SkTileMode mode,
                                      uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeLinear(const SkPoint pts[2],
                                      const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                                      const SkScalar pos[], int count, SkTileMode mode);

    static sk_sp<SkShader> MakeRadial(const SkPoint& center, SkScalar radius,
                                      const SkColor colors[], const SkScalar pos[], int count,
                                      SkTileMode mode,
                                      uint32_t flags, const SkMatrix* localMatrix) {
        return SkShaders::RadialGradient(center, radius, colors, pos, count, mode,
                                         flags, localMatrix);
    }
    static sk_sp<SkShader> MakeRadial(const SkPoint& center, SkScalar radius,
                                      const SkColor colors[], const SkScalar pos[], int count,
                                      SkTileMode mode) {
        return SkShaders::RadialGradient(center, radius, colors, pos, count, mode);
    }

    static sk_sp<SkShader> MakeRadial(const SkPoint& center, SkScalar radius,
                                      const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                                      const SkScalar pos[], int count, SkTileMode mode,
                                      uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeRadial(const SkPoint& center, SkScalar radius,
                                      const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                                      const SkScalar pos[], int count, SkTileMode mode);

    static sk_sp<SkShader> MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                               const SkPoint& end, SkScalar endRadius,
                                               const SkColor colors[], const SkScalar pos[],
                                               int count, SkTileMode mode,
                                               uint32_t flags, const SkMatrix* localMatrix) {
        return SkShaders::TwoPointConicalGradient(start, startRadius, end, endRadius, colors, pos,
                                                  count, mode, flags, localMatrix);
    }
    static sk_sp<SkShader> MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                               const SkPoint& end, SkScalar endRadius,
                                               const SkColor colors[], const SkScalar pos[],
                                               int count, SkTileMode mode) {
        return SkShaders::TwoPointConicalGradient(start, startRadius, end, endRadius, colors, pos,
                                                  count, mode);
    }

    static sk_sp<SkShader> MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                               const SkPoint& end, SkScalar endRadius,
                                               const SkColor4f colors[],
                                               sk_sp<SkColorSpace> colorSpace, const SkScalar pos[],
                                               int count, SkTileMode mode,
                                               uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                               const SkPoint& end, SkScalar endRadius,
                                               const SkColor4f colors[],
                                               sk_sp<SkColorSpace> colorSpace, const SkScalar pos[],
                                               int count, SkTileMode mode);

    static sk_sp<SkShader> MakeSweep(SkScalar cx, SkScalar cy,
                                     const SkColor colors[], const SkScalar pos[], int count,
                                     SkTileMode mode,
                                     SkScalar startAngle, SkScalar endAngle,
                                     uint32_t flags, const SkMatrix* localMatrix) {
        return SkShaders::SweepGradient(cx, cy, colors, pos, count, mode, startAngle, endAngle,
                                        flags, localMatrix);
    }
    static sk_sp<SkShader> MakeSweep(SkScalar cx, SkScalar cy,
                                     const SkColor colors[], const SkScalar pos[], int count,
                                     uint32_t flags, const SkMatrix* localMatrix) {
        return SkShaders::SweepGradient(cx, cy, colors, pos, count, flags, localMatrix);
    }
    static sk_sp<SkShader> MakeSweep(SkScalar cx, SkScalar cy,
                                     const SkColor colors[], const SkScalar pos[], int count) {
        return SkShaders::SweepGradient(cx, cy, colors, pos, count);
    }

    static sk_sp<SkShader> MakeSweep(SkScalar cx, SkScalar cy,
                                     const SkColor4f colors[],
                                     sk_sp<SkColorSpace> colorSpace,
                                     const SkScalar pos[],
                                     int count,
                                     SkTileMode mode,
                                     SkScalar startAngle, SkScalar endAngle,
                                     uint32_t flags,
                                     const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeSweep(SkScalar cx, SkScalar cy,
                                     const SkColor4f colors[],
                                     sk_sp<SkColorSpace> colorSpace,
                                     const SkScalar pos[],
                                     int count,
                                     uint32_t flags,
                                     const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeSweep(SkScalar cx, SkScalar cy,
                                     const SkColor4f colors[],
                                     sk_sp<SkColorSpace> colorSpace,
                                     const SkScalar pos[],
                                     int count);
};

#endif
