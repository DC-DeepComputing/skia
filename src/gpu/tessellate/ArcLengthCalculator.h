/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_ArcLengthCalculator_DEFINED
#define skgpu_tessellate_ArcLengthCalculator_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "src/core/SkVx.h"

namespace skgpu::tess {

class ArcLengthCalculator {
public:
    static inline float LineLength(skvx::float2 p0, skvx::float2 p1) {
        return skvx::length(p1 - p0);
    }

    static float ConicLength(skvx::float2 p0, skvx::float2 p1, skvx::float2 p2, float w);

    static float CubicLength(skvx::float2 c0, skvx::float2 c1, skvx::float2 c2, skvx::float2 c3);

    static float PathTotalLength(const SkPath& path, SkPaint::Cap cap);
};

} // namespace skgpu::tess

#endif // skgpu_tessellate_ArcLengthCalculator_DEFINED
