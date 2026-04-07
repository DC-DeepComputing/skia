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
 * The naive approach to flattening a Bézier curve evaluates points at evenly spaced parametric `t`
 * values (e.g., t=0.1, 0.2, ..., 1.0) and recursively subdivides the resulting lines until each
 * segment is within the desired tolerance (https://en.wikipedia.org/wiki/Hausdorff_distance).
 * However, the rate of change in both arc length and curvature is not evenly distributed across
 * `t`. A uniform step in `t` might span a long, mostly flat distance (needing few segments), or it
 * might whip around a tight loop (needing many).
 *
 * Instead, this algorithm maps the quadratic curve into an "arc-length" or uniform-curvature space
 * by projecting it onto a standard y = x^2 parabola. By dividing the curve uniformly in this
 * parabolic space, we can determine the parametric steps upfront without recursion. This allows the
 * exact `t` values to be calculated without data dependencies, increasing ILP and enabling SIMD.
 *
 * 1. `estimateSubdiv`:
 *    This function analyzes the control points and transforms the quadratic into the parabolic
 *    space. It calculates the total "parabolic distance" the curve covers. By comparing this
 *    distance against the tolerance, it determines exactly subdivsions are required to safely
 *    approximate the curve. It returns a `FlattenParams` struct containing the transformation
 *    constants.
 *
 * 2. `determineSubdivT`:
 *    Once we know *how many segments* we need to approximate the curve, we then need to find
 *    *where* to divide it. Using the inverse integral of the parabolic approximation, we map the
 *    `N` evenly spaced parabolic steps *back* into the curve's parametric space (`t`).
 *
 * 3. Culling and Winding:
 *    All out-of-viewport curves are simplified into straight lines. We simplify instead of cull
 *    because curves to the left of the viewport may still contribute winding for left-to-right
 *    scanline renderers (sparse strips). This simplification avoids subdvision and reduces the
 *    number of tiles generated downstream, while preserving the winding the curve would have
 *    contributed otherwise. Theoretically, curves entirely above, below, or to the right of the
 *    viewport could be completely culled, but to reduce branching, we fold them into the
 *    simplification case.
 *
 * 4. Lowering Conics:
 *    Conics (rational quadratics) are degree-reduced to standard quadratics using
 *    SkAutoConicToQuads. The curve is recursively chopped in half until the error between the
 *    weighted conic segment and a standard quadratic falls below our visual tolerance.
 *
 * 5. Lowering Cubics:
 *    Cubics are approximated by a series of quadratics. We estimate the required number of segments
 *    by measuring the curve's geometric deviation from a pure quadratic. The cubic is then sliced
 *    into equal parametric intervals, and the resulting quadratics are subdivided.
 */

class Flatten {
public:
    static constexpr size_t kMaxQuads = 16;
    static constexpr double kTol = 0.25;
    static constexpr double kTol2 = kTol * kTol;
    static constexpr double kSqrtTol = 0.5;  // sqrt(0.25)

    Flatten() = default;

    template <FlattenMode kMode>
    SK_ALWAYS_INLINE void processPaths(const SkPath& path,
                                       const SkMatrix& ctm,
                                       float width,
                                       float height,
                                       Polyline& polyline) {
        SkASSERT(!ctm.hasPerspective());
        if constexpr (kMode == FlattenMode::kSimd) {
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
