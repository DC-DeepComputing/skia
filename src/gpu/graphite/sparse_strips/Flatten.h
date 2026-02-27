/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_Flatten_DEFINED
#define skgpu_graphite_Flatten_DEFINED

#include "include/core/SkPath.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

namespace skgpu::graphite {

// ==============================================================================
// A naive approach to flattening a Bézier curve evaluates points at evenly spaced
// parametric `t` values (e.g., t=0.1, 0.2... 1.0). However, the speed at which a
// curve travels through space is not constant relative to `t`. A uniform step in
// `t` might cover a huge, flat distance (needing few segments) or whip around a
// tight corner (needing many segments).
//
// To fix this, this algorithm maps the quadratic curve into an "arc-length" or
// uniform-curvature space by projecting it onto a standard y = x^2 parabola.
//
// 1. `estimate_subdiv`:
//    This function analyzes the control points and transforms the quadratic
//    into that parabolic space. It calculates the total "parabolic distance"
//    the curve covers. By comparing this distance against our tolerance, it
//    determines exactly how many straight line segments (subdivisions) are
//    required to safely approximate the curve. It returns a `FlattenParams`
//    struct containing the transformation constants.
//
// 2. `determine_subdiv_t`:
//    Once `estimate_subdiv` tells us we need `N` segments, we iterate `i` from
//    1 to N-1. We divide the total parabolic distance into uniform chunks
//    (`x = i / N`). We pass this uniform step `x` to `determine_subdiv_t`.
//    This function uses the inverse integral to map that evenly spaced arc-length
//    step *back* into the original curve's `t` parameter (0.0 to 1.0).
//
// ==============================================================================

class Flatten {
public:
    Flatten() = default;

    template <FlattenMode Mode>
    void processPaths(
            const SkPath& path, float width, float height, Polyline& polyline) {
        fContext.fFlattenedCubics.clear();

        bool closed = true;
        SkPoint startPt = {0, 0};
        SkPoint lastPt = {0, 0};

        SkPath::Iter iter(path, false);
        SkPoint pts[4];
        SkPath::Verb verb;

        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb: {
                    if (!closed && lastPt != startPt) {
                        polyline.appendPoint(startPt);
                    }
                    polyline.appendSentinel();

                    closed = false;
                    lastPt = pts[0];
                    startPt = pts[0];
                    polyline.appendPoint(pts[0]);
                    break;
                }
                case SkPath::kLine_Verb: {
                    SkASSERT(!closed);
                    lastPt = pts[1];
                    polyline.appendPoint(pts[1]);
                    break;
                }
                case SkPath::kQuad_Verb: {
                    SkASSERT(!closed);

                    bool culled = false;
                    bool isLine = false;

                    if constexpr (Mode == FlattenMode::kSimd) {
                        skvx::float8 P = skvx::float8::Load(pts);
                        skvx::float4 X = skvx::shuffle<0, 2, 4, 4>(P);
                        skvx::float4 Y = skvx::shuffle<1, 3, 5, 5>(P);

                        culled = skvx::all(X > width) || skvx::all(Y < 0.0f) ||
                                 skvx::all(Y > height);
                        isLine = skvx::all(X < 0.0f) ||
                                 distSqToSegment(pts[1], pts[0], pts[2]) <= 4.0f * kTol2;
                    } else {
                        culled = (pts[0].fX > width && pts[1].fX > width && pts[2].fX > width) ||
                                 (pts[0].fY < 0.0f && pts[1].fY < 0.0f && pts[2].fY < 0.0f) ||
                                 (pts[0].fY > height && pts[1].fY > height && pts[2].fY > height);
                        isLine = (pts[0].fX < 0.0f && pts[1].fX < 0.0f && pts[2].fX < 0.0f) ||
                                 distSqToSegment(pts[1], pts[0], pts[2]) <= 4.0f * kTol2;
                    }

                    if (culled || isLine) {
                        polyline.appendPoint(pts[2]);
                    } else {
                        this->flattenQuad(pts, polyline);
                    }
                    lastPt = pts[2];
                    break;
                }
                case SkPath::kCubic_Verb: {
                    SkASSERT(!closed);

                    bool culled = false;
                    bool isLine = false;

                    if constexpr (Mode == FlattenMode::kSimd) {
                        skvx::float8 P = skvx::float8::Load(pts);
                        skvx::float4 X = skvx::shuffle<0, 2, 4, 6>(P);
                        skvx::float4 Y = skvx::shuffle<1, 3, 5, 7>(P);

                        culled = skvx::all(X > width) || skvx::all(Y < 0.0f) ||
                                 skvx::all(Y > height);
                        isLine = skvx::all(X < 0.0f) ||
                                 std::max(distSqToSegment(pts[1], pts[0], pts[3]),
                                          distSqToSegment(pts[2], pts[0], pts[3])) <=
                                         (16.0f / 9.0f) * kTol2;
                    } else {
                        culled = (pts[0].fX > width && pts[1].fX > width && pts[2].fX > width &&
                                  pts[3].fX > width) ||
                                 (pts[0].fY < 0.0f && pts[1].fY < 0.0f && pts[2].fY < 0.0f &&
                                  pts[3].fY < 0.0f) ||
                                 (pts[0].fY > height && pts[1].fY > height && pts[2].fY > height &&
                                  pts[3].fY > height);
                        isLine = (pts[0].fX < 0.0f && pts[1].fX < 0.0f && pts[2].fX < 0.0f &&
                                  pts[3].fX < 0.0f) ||
                                 std::max(distSqToSegment(pts[1], pts[0], pts[3]),
                                          distSqToSegment(pts[2], pts[0], pts[3])) <=
                                         (16.0f / 9.0f) * kTol2;
                    }

                    if (culled || isLine) {
                        polyline.appendPoint(pts[3]);
                    } else {
                        size_t max = 0;
                        if constexpr (Mode == FlattenMode::kSimd) {
                            max = this->flattenCubicSimd(pts);
                        } else {
                            max = this->flattenCubicScalar(pts);
                        }
                        for (size_t i = 1; i < max; ++i) {
                            polyline.appendPoint(fContext.fFlattenedCubics[i]);
                        }
                    }
                    lastPt = pts[3];
                    break;
                }
                case SkPath::kClose_Verb: {
                    closed = true;
                    if (lastPt != startPt) {
                        polyline.appendPoint(startPt);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        if (!closed && lastPt != startPt) {
            polyline.appendPoint(startPt);
        }

        polyline.appendSentinel();
    }

private:
#if defined(GPU_TEST_UTILS)
    template <FlattenMode> friend class FlattenTestRunner;
#endif
    static constexpr size_t kMaxQuads = 16;
    static constexpr double kTol = 0.25;
    static constexpr double kTol2 = kTol * kTol;
    static constexpr double kSqrtTol = 0.5;  // sqrt(0.25)

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

    SK_ALWAYS_INLINE static float distSqToSegment(SkPoint p, SkPoint a, SkPoint b) {
        SkPoint ab = b - a;
        SkPoint ap = p - a;
        float lenSq = SkPoint::DotProduct(ab, ab);
        float t = (lenSq == 0.0f) ? 0.0f : SkPoint::DotProduct(ap, ab) / lenSq;
        t = std::clamp(t, 0.0f, 1.0f);
        SkPoint proj = a + ab * t;
        SkPoint d = p - proj;
        return SkPoint::DotProduct(d, d);
    }

    void flattenQuad(const SkPoint pts[3], Polyline& polyline);
    size_t flattenCubicScalar(const SkPoint pts[4]);

    void evalCubicsSimd(const SkPoint pts[4], size_t n);
    void estimateSubdivSimd(float sqrtTol);
    void outputLinesSimd(size_t i, float x0, float dx, size_t n, size_t start_idx);
    size_t flattenCubicSimd(const SkPoint pts[4]);

    FlattenCtx fContext;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_Flatten_DEFINED
