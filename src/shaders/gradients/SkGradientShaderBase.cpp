/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/gradients/SkGradientShaderBase.h"

SkGradientShaderBase::ColorStopOptimizer::ColorStopOptimizer(const SkColor4f* colors,
                                                             const SkScalar* pos,
                                                             int count,
                                                             SkTileMode mode)
        : fColors(colors)
        , fPos(pos)
        , fCount(count) {

    if (!pos || count != 3) {
        return;
    }

    if (SkScalarNearlyEqual(pos[0], 0.0f) &&
        SkScalarNearlyEqual(pos[1], 0.0f) &&
        SkScalarNearlyEqual(pos[2], 1.0f)) {

        if (SkTileMode::kRepeat == mode || SkTileMode::kMirror == mode ||
            colors[0] == colors[1]) {

            // Ignore the leftmost color/pos.
            fColors += 1;
            fPos    += 1;
            fCount   = 2;
        }
    } else if (SkScalarNearlyEqual(pos[0], 0.0f) &&
               SkScalarNearlyEqual(pos[1], 1.0f) &&
               SkScalarNearlyEqual(pos[2], 1.0f)) {

        if (SkTileMode::kRepeat == mode || SkTileMode::kMirror == mode ||
            colors[1] == colors[2]) {

            // Ignore the rightmost color/pos.
            fCount  = 2;
        }
    }
}
