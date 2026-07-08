/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tess_ArcLengthCalculator_DEFINED
#define skgpu_tess_ArcLengthCalculator_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkVx.h"
#include "src/gpu/tessellate/Tessellation.h"

namespace skgpu::tess {

class ArcLengthCalculator {
public:
    static constexpr float kConicTolerance = 0.25f;
    SK_ALWAYS_INLINE static float LineLength(skvx::float2 p0, skvx::float2 p1) {
        return skvx::length(p1 - p0);
    }

    // Approximates a rational conic using a cubic bezier. This implements the G1 mid-point
    // approximation from Proposition 5 of:
    // Floater, M. S. (1996). An analysis of cubic approximation schemes for conic sections.
    // Advances in Computational Mathematics, 5(1), 361–379. https://doi.org/10.1007/bf02124751
    SK_ALWAYS_INLINE static float ConicLength(skvx::float2 p0,
                                              skvx::float2 p1,
                                              skvx::float2 p2,
                                              float w) {
        float k = (4.0f / 3.0f) * (w / (1.0f + w));
        skvx::float2 v1 = p1 - p0;
        skvx::float2 v2 = p2 - p0;
        skvx::float2 d1 = k * v1;
        skvx::float2 d2 = v2 + k * (v1 - v2);
        skvx::float2 d3d2 = k * (p2 - p1);
        return CubicLengthFromDerivatives(d1, d2, v2, d3d2);
    }

    // Adaptively subdivides a conic until the approximation error is below `kConicTolerance`.
    // Currently, this is unused because Floater's single-segment G1 approximation provides
    // sufficient precision for arc length calculations.
    //
    // For a realistic worst-case unchopped conic---a 90-degree circular arc (w ≈ 0.707), scaled to
    // an 8K screen (radius = 8192px)---the maximum deviation from the true curve is only ~2.24
    // pixels. Over a total path length of ~12,800 pixels, this geometric bulge results in a
    // sub-pixel difference in the integrated arc length.
    //
    // Floater's G1 spatial error bound for a 90-degree arc is known to be a constant fraction of
    // the radius: ~0.000273 * R, so:
    // E_max = 0.000273 * 8192 px = ~2.24 pixels
    // True 90-deg Arc Length = (PI / 2) * 8192  = ~12,867.96 pixels
    [[maybe_unused]]
    SK_ALWAYS_INLINE static float AdaptiveConicLength(skvx::float2 p0,
                                                      skvx::float2 p1,
                                                      skvx::float2 p2,
                                                      float w) {
        // The Hausdorff distance (d_H) between the cubic approximation (q) and the true rational
        // conic (r) is bounded by Theorem 1 from Floater:
        //
        // d_H(q, r) <= (1/4) * max(1/w^2, 1) * max_{t in [0,1]} |f_r(q(t))| * ||p0 - 2*p1 + p2||
        // d_H(q, r) <= algFactor * geomFactor
        float err;
        if (w == 0.0f) {
            err = 0.0f;
        } else {
            // To evaluate if the curve needs subdivision, we calculate a combined error.
            //
            // 1. geomFactor: The 2nd forward difference of the control polygon
            //    (p0 - 2p1 + p2). This measures the physical "bulge" or curvature.
            //
            // 2. algFactor:  For elliptical arcs (w < 1), the unweighted rational math artificially
            //    inflates projective error as the curve flattens out. Scaling by 1 / w^2
            //    counteracts this, preventing over-chopping visually flat curves.
            float algFactor = std::max(1.0f / (w * w), 1.0f);
            float geomFactor = skvx::length(p0 - 2.0f * p1 + p2);
            err = algFactor * geomFactor;
        }

        // TODO (thomsmit): Maybe also check if the sweep is > 90 degrees (w < ~0.707) for
        // quadrature accuracy? Due to tessellator preprocessing, this is currently unecessary.
        if (err > kConicTolerance) {
            // Note: Floater's approximation has an error bound of O(h^6), where h is the parameter
            // interval. Halving the interval decreases the error by a factor of 64. For all
            // expected screen resolutions, a single round of subdivision is sufficient to reach the
            // error threshold, so we use a specialized splitting function rather than recursively
            // calling this one. (The recursive is left here in comment form.)
            //
            // Calculate new weight for both subdivided halves.
            // float newWeight = std::sqrt((1.0f + w) * 0.5f);
            //
            // Compute the subdivided rational control points at t = 0.5.
            // float inv = 1.0f / (1.0f + w);
            // skvx::float2 p01 = (p0 + p1 * w) * inv;
            // skvx::float2 p12 = (p1 * w + p2) * inv;
            // skvx::float2 M   = (p01 + p12) * 0.5f; // M is the on-curve midpoint
            //
            // return AdaptiveConicLength(p0, p01, M, newWeight) +
            //        AdaptiveConicLength(M, p12, p2, newWeight);
            return ConicLengthTwoSegments(p0, p1, p2, w);
        }

        // If the error from the single segment approximation is below tolerance, no subdivision is
        // necessary so make the conversion to cubic and return the length.
        return ConicLength(p0, p1, p2, w);
    }

    SK_ALWAYS_INLINE static float CubicLength(skvx::float2 c0,
                                              skvx::float2 c1,
                                              skvx::float2 c2,
                                              skvx::float2 c3) {
        skvx::float2 d1 = c1 - c0;
        skvx::float2 d2 = c2 - c0;
        skvx::float2 d3 = c3 - c0;
        return CubicLengthFromDerivatives(d1, d2, d3, d3 - d2);
    }

    // 5-point Gaussian-Lobatto
    SK_ALWAYS_INLINE static float CubicLengthFromDerivatives(skvx::float2 d1,
                                                             skvx::float2 d2,
                                                             skvx::float2 d3,
                                                             skvx::float2 d3d2) {
        const skvx::float4 cx1(
                0.15f,  0.325650248872424f, -0.26666666666666666f, -0.208983582205757f);
        const skvx::float4 cx2(
                0.0f,   0.208983582205757f,  0.26666666666666666f, -0.325650248872424f);
        const skvx::float4 cx3(
                0.0f,   0.024349751127576f,  0.26666666666666666f,  0.558983582205757f);

        skvx::float4 vx = d1[0] * cx1 + d2[0] * cx2 + d3[0] * cx3;
        skvx::float4 vy = d1[1] * cx1 + d2[1] * cx2 + d3[1] * cx3;

        skvx::float4 len = skvx::sqrt(vx * vx + vy * vy);

        float v4 = skvx::length(d3d2) * 0.15f;
        return len[0] + len[1] + len[2] + len[3] + v4;
    }

    static float PathTotalLength(const SkPath& path, SkPaint::Cap cap) {
        class Precalculator : public ArcLengthCalculator {
        public:
            Precalculator() : fAccumulatedLength(0.f) {}

            void writeDeferredStrokePatch(SkPoint, SkPaint::Cap) {}
            void closeDeferredStrokePatch(SkPaint::Cap) {}

            void writeLine(SkPoint p0, SkPoint p1) {
                fAccumulatedLength += this->LineLength({p0.fX, p0.fY}, {p1.fX, p1.fY});
            }
            void writeCircle(SkPoint) {}
            void writeQuadratic(const SkPoint pts[3]) {
                fAccumulatedLength += this->ConicLength({pts[0].fX, pts[0].fY},
                                                        {pts[1].fX, pts[1].fY},
                                                        {pts[2].fX, pts[2].fY},
                                                        1.f);
            }
            void writeConic(const SkPoint pts[3], float w) {
                fAccumulatedLength += this->ConicLength(
                        {pts[0].fX, pts[0].fY}, {pts[1].fX, pts[1].fY}, {pts[2].fX, pts[2].fY}, w);
            }
            void writeCubic(const SkPoint pts[4]) {
                fAccumulatedLength += this->CubicLength({pts[0].fX, pts[0].fY},
                                                        {pts[1].fX, pts[1].fY},
                                                        {pts[2].fX, pts[2].fY},
                                                        {pts[3].fX, pts[3].fY});
            }

            float totalLength() const { return fAccumulatedLength; }

        private:
            float fAccumulatedLength;
        };

        Precalculator precalculator;

        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kMove:
                    precalculator.writeDeferredStrokePatch(pts[0], cap);
                    break;

                case SkPathVerb::kClose:
                    precalculator.closeDeferredStrokePatch(cap);
                    break;

                case SkPathVerb::kLine:
                    precalculator.writeLine(pts[0], pts[1]);
                    break;

                case SkPathVerb::kQuad:
                    if (tess::ConicHasCusp(pts)) {
                        SkPoint cusp = SkEvalQuadAt(pts, SkFindQuadMidTangent(pts));
                        precalculator.writeCircle(cusp);
                        precalculator.writeLine(pts[0], cusp);
                        precalculator.writeLine(cusp, pts[2]);
                    } else {
                        precalculator.writeQuadratic(pts);
                    }
                    break;

                case SkPathVerb::kConic:
                    if (tess::ConicHasCusp(pts)) {
                        SkConic conic(pts, *w);
                        SkPoint cusp = conic.evalAt(conic.findMidTangent());
                        precalculator.writeCircle(cusp);
                        precalculator.writeLine(pts[0], cusp);
                        precalculator.writeLine(cusp, pts[2]);
                    } else {
                        precalculator.writeConic(pts, *w);
                    }
                    break;

                case SkPathVerb::kCubic: {
                    SkPoint chops[10];
                    float T[2];
                    bool areCusps;
                    int numChops = tess::FindCubicConvex180Chops(pts, T, &areCusps);
                    if (numChops == 0) {
                        precalculator.writeCubic(pts);
                    } else if (numChops == 1) {
                        SkChopCubicAt(pts, chops, T[0]);
                        if (areCusps) {
                            precalculator.writeCircle(chops[3]);
                            chops[2] = chops[4] = chops[3];
                        }
                        precalculator.writeCubic(chops);
                        precalculator.writeCubic(chops + 3);
                    } else {
                        SkASSERT(numChops == 2);
                        SkChopCubicAt(pts, chops, T[0], T[1]);
                        if (areCusps) {
                            precalculator.writeCircle(chops[3]);
                            precalculator.writeCircle(chops[6]);
                            precalculator.writeLine(chops[0], chops[3]);
                            precalculator.writeLine(chops[3], chops[6]);
                            precalculator.writeLine(chops[6], chops[9]);
                        } else {
                            precalculator.writeCubic(chops);
                            precalculator.writeCubic(chops + 3);
                            precalculator.writeCubic(chops + 6);
                        }
                    }
                    break;
                }
            }
        }
        return precalculator.totalLength();
    }

private:
    // Dedicated version for the bounded recursion on the adaptive subdivision. Designed to take
    // advantage of: 1) the Gaussian-Lobatto quadrature for cubics can be processed in parallel 2)
    // the two cubics share an endpoint.
    SK_ALWAYS_INLINE static float ConicLengthTwoSegments(skvx::float2 p0,
                                                         skvx::float2 p1,
                                                         skvx::float2 p2,
                                                         float w) {
        float w1 = std::sqrt((1.0f + w) * 0.5f);
        float inv = 1.0f / (1.0f + w);
        float k = (4.0f / 3.0f) * (w1 / (1.0f + w1));

        // Expand the subdivided conics and their converted cubic control points, and express
        // derivatives in terms of three base vectors and the mid-point.
        skvx::float2 vP1P0 = (p1 - p0) * (w * inv);
        skvx::float2 vP1P2 = (p1 - p2) * (w * inv);
        skvx::float2 v1M   = (p0 - p2) * (0.5f * inv);
        skvx::float2 M     = (p0 + p2 + 2.0f * w * p1) * (0.5f * inv);

        skvx::float2 d3A = M - p0;
        skvx::float2 d3B = p2 - M;

        // Setup for the internal 3 points of the 5-point Lobatto integration
        const skvx::float4 C1(0.325650248872424f, -0.26666666666666666f, -0.208983582205757f, 0.0f);
        const skvx::float4 C2(0.208983582205757f,  0.26666666666666666f, -0.325650248872424f, 0.0f);
        const skvx::float4 C23(7.0f/30.0f, 16.0f/30.0f, 7.0f/30.0f, 0.0f); // Exactly C2 + C3

        skvx::float4 K1 = C1 * k;
        skvx::float4 K2 = C2 * k;

        // The shared middle endpoint is premultiplied
        skvx::float4 t1Mx = v1M[0] * K1;
        skvx::float4 t1My = v1M[1] * K1;

        // X and Y components for Conic A's 3 intermediate vectors
        skvx::float4 vxA = vP1P0[0] * K1 + v1M[0] * K2 + d3A[0] * C23;
        skvx::float4 vyA = vP1P0[1] * K1 + v1M[1] * K2 + d3A[1] * C23;

        // X and Y components for Conic B's 3 intermediate vectors (reusing the symmetric t1M)
        skvx::float4 vxB = -t1Mx + vP1P2[0] * K2 + d3B[0] * C23;
        skvx::float4 vyB = -t1My + vP1P2[1] * K2 + d3B[1] * C23;

        // X and Y components for the boundary vectors V0 and V4
        skvx::float4 vx04(vP1P0[0], v1M[0], vP1P2[0], 0.0f);
        skvx::float4 vy04(vP1P0[1], v1M[1], vP1P2[1], 0.0f);

        // Compute ALL lengths in parallel
        skvx::float4 lenA  = skvx::sqrt(vxA * vxA + vyA * vyA);
        skvx::float4 lenB  = skvx::sqrt(vxB * vxB + vyB * vyB);
        skvx::float4 len04 = skvx::sqrt(vx04 * vx04 + vy04 * vy04);

        // Sum V1, V2, V3 for both halves
        skvx::float4 lenSum = lenA + lenB;
        float lengthV123 = lenSum[0] + lenSum[1] + lenSum[2];

        // Factor out the collapsed endpoints. Since the two subdivided cubic segments meet at the
        // shared midpoint, their boundary tangent vector magnitudes (V4_A and V0_B) are both equal
        // to ||v1M||. We sum all four endpoint terms directly:
        // V0_A = k * ||vP1P0|| * 0.15
        // V4_A = V0_B = k * ||v1M|| * 0.15 (shared midpoint, counted twice)
        // V4_B = k * ||vP1P2|| * 0.15
        float lengthV04 = (len04[0] + 2.0f * len04[1] + len04[2]) * (k * 0.15f);

        return lengthV123 + lengthV04;
    }
};

}  // namespace skgpu::tess

#endif  // skgpu_tess_ArcLengthCalculator_DEFINED

