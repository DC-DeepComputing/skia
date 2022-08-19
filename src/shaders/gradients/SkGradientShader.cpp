/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkGradientShader.h"

#include "include/core/SkColorSpace.h"

namespace {

SkGradientFlags convert_flags(uint32_t oldFlags) {
    static_assert(static_cast<uint32_t>(SkGradientFlags::kInterpolateColorsInPremul) ==
                  SkGradientShader::Flags::kInterpolateColorsInPremul_Flag);

    return static_cast<SkGradientFlags>(oldFlags);
}

} // anonymous namespace

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor colors[],
                                             const SkScalar pos[],
                                             int count,
                                             SkTileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    return SkShaders::LinearGradient(pts, colors, pos, count, mode,
                                     convert_flags(flags), localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[],
                                             int count,
                                             SkTileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    return SkShaders::LinearGradient(pts, colors, std::move(colorSpace), pos, count, mode,
                                     convert_flags(flags), localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[],
                                             int count,
                                             SkTileMode mode) {
    return SkShaders::LinearGradient(pts, colors, std::move(colorSpace), pos, count, mode);
}

sk_sp<SkShader> SkGradientShader::MakeRadial(const SkPoint& center, SkScalar radius,
                                             const SkColor colors[],
                                             const SkScalar pos[],
                                             int count,
                                             SkTileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    return SkShaders::RadialGradient(center, radius, colors, pos, count, mode,
                                     convert_flags(flags), localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeRadial(const SkPoint& center, SkScalar radius,
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[],
                                             int count, SkTileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    return SkShaders::RadialGradient(center, radius, colors, std::move(colorSpace),
                                     pos, count, mode, convert_flags(flags), localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeRadial(const SkPoint& center, SkScalar radius,
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[],
                                             int count,
                                             SkTileMode mode) {
    return SkShaders::RadialGradient(center, radius, colors, std::move(colorSpace),
                                     pos, count, mode);
}

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                                      const SkPoint& end, SkScalar endRadius,
                                                      const SkColor colors[],
                                                      const SkScalar pos[],
                                                      int count,
                                                      SkTileMode mode,
                                                      uint32_t flags,
                                                      const SkMatrix* localMatrix) {
    return SkShaders::TwoPointConicalGradient(start, startRadius, end, endRadius, colors, pos,
                                              count, mode, convert_flags(flags), localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                                      const SkPoint& end, SkScalar endRadius,
                                                      const SkColor4f colors[],
                                                      sk_sp<SkColorSpace> colorSpace,
                                                      const SkScalar pos[],
                                                      int count,
                                                      SkTileMode mode,
                                                      uint32_t flags,
                                                      const SkMatrix* localMatrix) {
    return SkShaders::TwoPointConicalGradient(start, startRadius, end, endRadius, colors,
                                              std::move(colorSpace), pos,
                                              count, mode, convert_flags(flags), localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                                      const SkPoint& end, SkScalar endRadius,
                                                      const SkColor4f colors[],
                                                      sk_sp<SkColorSpace> colorSpace,
                                                      const SkScalar pos[],
                                                      int count,
                                                      SkTileMode mode) {
    return SkShaders::TwoPointConicalGradient(start, startRadius, end, endRadius, colors,
                                              std::move(colorSpace), pos,
                                              count, mode);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor colors[],
                                            const SkScalar pos[],
                                            int count,
                                            SkTileMode mode,
                                            SkScalar startAngle, SkScalar endAngle,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    return SkShaders::SweepGradient(cx, cy, colors, pos, count, mode, startAngle, endAngle,
                                    convert_flags(flags), localMatrix);
}
sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor colors[],
                                            const SkScalar pos[],
                                            int count,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    return SkShaders::SweepGradient(cx, cy, colors, pos, count, convert_flags(flags), localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor4f colors[],
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkScalar pos[],
                                            int count,
                                            SkTileMode mode,
                                            SkScalar startAngle, SkScalar endAngle,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    return SkShaders::SweepGradient(cx, cy, colors, std::move(colorSpace), pos, count,
                                    mode, startAngle, endAngle, convert_flags(flags), localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor4f colors[],
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkScalar pos[],
                                            int count,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    return SkShaders::SweepGradient(cx, cy, colors, std::move(colorSpace), pos, count,
                                    convert_flags(flags), localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor4f colors[],
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkScalar pos[],
                                            int count) {
    return SkShaders::SweepGradient(cx, cy, colors, std::move(colorSpace), pos, count);
}
