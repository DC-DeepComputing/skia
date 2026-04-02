/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_Flatten_DEFINED
#define skgpu_graphite_sparse_strips_Flatten_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkVx.h"
#include "src/core/SkGeometry.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

namespace skgpu::graphite {

/*
 * This is a translation of the work of the Vello authors:
 * https://raphlinus.github.io/graphics/curves/2019/12/23/flatten-quadbez.html
 * https://github.com/linebender/vello/blob/main/sparse_strips/vello_common/src/flatten.rs
 *
 * A naive approach to flattening a Bézier curve evaluates points at evenly spaced parametric `t`
 * values (e.g., t=0.1, 0.2... 1.0). However, the speed at which a curve travels through space is
 * not constant relative to `t`. A uniform step in `t` might cover a huge, flat distance (needing
 * few segments) or whip around a tight corner (needing many segments).
 *
 * To fix this, this algorithm maps the quadratic curve into an "arc-length" or uniform-curvature
 * space by projecting it onto a standard y = x^2 parabola.
 *
 * 1. `estimate_subdiv`:
 *    This function analyzes the control points and transforms the quadratic into that parabolic
 *    space. It calculates the total "parabolic distance" the curve covers. By comparing this
 *    distance against our tolerance, it determines exactly how many straight line segments
 *    (subdivisions) are required to safely approximate the curve. It returns a `FlattenParams`
 *    struct containing the transformation constants.
 *
 * 2. `determine_subdiv_t`:ß
 *    Once `estimate_subdiv` tells us we need `N` segments, we iterate `i` from 1 to N-1. We divide
 *    the total parabolic distance into uniform chunks (`x = i / N`). We pass this uniform step `x`
 *    to `determine_subdiv_t`. This function uses the inverse integral to map that evenly spaced
 *    arc-length step *back* into the original curve's `t` parameter (0.0 to 1.0).
 *
 */

class Flatten {
public:
    static constexpr size_t kMaxQuads = 16;
    static constexpr double kTol = 0.25;
    static constexpr double kTol2 = kTol * kTol;
    static constexpr double kSqrtTol = 0.5;  // sqrt(0.25)

    Flatten() = default;

    template <FlattenMode Mode>
    SK_ALWAYS_INLINE void processPaths(const SkPath& path,
                                       const SkMatrix& ctm,
                                       float width,
                                       float height,
                                       Polyline& polyline) {
        SkASSERT(!ctm.hasPerspective());
        if constexpr (Mode == FlattenMode::kSimd) {
            this->processPathsSimd(path, ctm, width, height, polyline);
        } else {
            this->processPathsScalar(path, ctm, width, height, polyline);
        }
    }

private:
#if defined(GPU_TEST_UTILS)
    template <FlattenMode> friend class FlattenTestRunner;
#endif

    struct FlattenCtx {
        std::array<SkPoint, kMaxQuads + 4> fEvenPts;
        std::array<SkPoint, kMaxQuads> fOddPts;
        std::array<float, kMaxQuads> fA0;
        std::array<float, kMaxQuads> fDa;
        std::array<float, kMaxQuads> fU0;
        std::array<float, kMaxQuads> fUScale;
        std::array<float, kMaxQuads> fVal;

        size_t fNumQuads = 0;

        SkTDArray<SkPoint> fFlattenedCubics;
    };

    void processPathsSimd(const SkPath& path, const SkMatrix& ctm, float width, float height,
                          Polyline& polyline);
    void processPathsScalar(const SkPath& path, const SkMatrix& ctm, float width, float height,
                            Polyline& polyline);

    void flattenQuad(const SkPoint pts[3], Polyline& polyline);
    void flattenQuadSimd(const SkPoint pts[3], Polyline& polyline);
    size_t flattenCubicScalar(const SkPoint pts[4]);

    void evalCubicsSimd(const SkPoint pts[4], size_t n);
    void estimateSubdivSimd(float sqrtTol);
    void outputLinesSimd(size_t i, float x0, float dx, size_t n, size_t start_idx);
    size_t flattenCubicSimd(const SkPoint pts[4]);

    FlattenCtx fContext;
    SkAutoConicToQuads fQuad;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_Flatten_DEFINED
