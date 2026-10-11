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
#include "src/core/SkUtils.h"
#include "src/core/SkVx.h"
#include "src/gpu/tessellate/Tessellation.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

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
    //
    // The vertex shader's $conic_length() is a port of this function; keep them in sync.
    SK_ALWAYS_INLINE static float ConicLengthOneSegment(skvx::float2 p0,
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
    // Floater's single-segment G1 approximation (ConicLengthOneSegment) is usually precise enough
    // for arc length calculations on its own; this only splits conics that bulge noticeably.
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
    SK_ALWAYS_INLINE static float ConicLength(skvx::float2 p0,
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
        return ConicLengthOneSegment(p0, p1, p2, w);
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
    //
    // The vertex shader's $cubic_length_from_derivatives() is a port of this function; keep them
    // in sync.
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

// A bend in a curve that's tighter than a join's corner radius (see ArcLengthAccumulator), where
// the inner side of the curve's stroke overlaps itself, like in the swallowtails and sharp apexes
// of curves stroked wider than they bend. A ball with the corner radius, rolling along the inside
// of the curve, can't follow the bend: it stops touching the curve at fT1, pivots around its
// center C by the angle between the curve's tangents there, and touches the curve again at fT2.
// The arc length does the same. It follows the curve up to fT1, sweeps around C the way it does
// around a join's inner corner, and continues along the curve from fT2. The pieces of the curve in
// between are all parameterized by the sweep, so their strokes agree wherever they overlap.
//
// C is where the curve's inner offset at the corner radius crosses itself, so the corner's
// cross-sections through C are the curve's normals at fT1 and fT2. C is also the corner radius away
// from the tangent lines at fT1 and fT2. So a fold is the corner a join between those tangents
// would get where they cross, with trims that reach exactly to fT1 and fT2.
struct ArcLengthFold {
    float fT1;        // Where the fold starts on its curve
    float fT2;        // Where it ends: 0 <= fT1 < fT2 <= 1
    SkPoint fCorner;  // C
    SkVector fA;      // The curve's unit tangent at fT1
    SkVector fB;      // The curve's unit tangent at fT2
    // The corner radius: the stroke radius, unless the fold had to shrink to fit within its curve
    float fRadius;

    // The angle between fA and fB (less than 180 degrees), which the sweep turns by.
    float angle() const { return std::atan2(std::abs(fA.cross(fB)), fA.dot(fB)); }
    // How much arc length the sweep covers.
    float length() const { return this->angle() * fRadius; }
};

// The most folds FindArcLengthFolds() finds in one curve.
inline constexpr int kMaxArcLengthFolds = 2;

// Implementation details of FindArcLengthFolds(), which works in double precision. The script that
// produced the expectations in ArcLengthTest follows the same steps.
namespace arclength_folds {

using double2 = skvx::double2;

constexpr int kCurvatureSamples = 32;  // Curvature samples (plus one) that find the tightest bends
constexpr int kGoldenSteps = 32;       // Steps of the search that refines each tightest point
constexpr int kBranchSegments = 40;    // Segments that approximate each branch of an offset
constexpr int kNewtonSteps = 8;
constexpr int kShrinkSteps = 16;       // Steps of the bisection that shrinks folds to fit
constexpr double kGap = 1e-4;          // Keeps an offset's branches apart at the tightest point
constexpr double kMinAngle = 1e-3;     // Folds that turn less than this (in radians) are ignored
// How much bigger than the curvature radius at the tightest point a fold's corner radius must be.
constexpr double kMinRadiusMargin = 1e-3;
// The number of segments of the polygon that approximates a loop, to see if its hole shows.
constexpr int kLoopSamples = 16;
// Like FindCubicConvex180Chops(), curves aren't chopped this close to their ends, or to each
// other's chops, where the pieces would be too short for their tangents to be stable. Folds that
// would start or end that close to the ends of their curves start or end at the ends instead.
constexpr double kChopEpsilon = 1.0 / (1 << 11);

// A quad, conic, or cubic.
class Curve {
public:
    Curve(SkPathVerb verb, const SkPoint pts[], float w)
            : fIsCubic(verb == SkPathVerb::kCubic)
            , fW(verb == SkPathVerb::kConic ? w : 1.0) {
        for (int i = 0; i < (fIsCubic ? 4 : 3); ++i) {
            fP[i] = {pts[i].fX, pts[i].fY};
        }
    }

    // Returns the position at 't', and sets the first and second derivatives there.
    double2 eval(double t, double2* d1, double2* d2) const {
        const double u = 1 - t;
        if (fIsCubic) {
            *d1 = 3.0 * (u * u * (fP[1] - fP[0]) + 2.0 * u * t * (fP[2] - fP[1]) +
                         t * t * (fP[3] - fP[2]));
            *d2 = 6.0 * (u * (fP[2] - 2.0 * fP[1] + fP[0]) + t * (fP[3] - 2.0 * fP[2] + fP[1]));
            return u * u * u * fP[0] + 3.0 * u * u * t * fP[1] + 3.0 * u * t * t * fP[2] +
                   t * t * t * fP[3];
        }
        // A rational quadratic N / D (a quad if w is 1).
        const double w = fW;
        double2 N = u * u * fP[0] + 2.0 * w * u * t * fP[1] + t * t * fP[2];
        double D = u * u + 2.0 * w * u * t + t * t;
        double2 dN = 2.0 * (-u * fP[0] + w * (u - t) * fP[1] + t * fP[2]);
        double dD = 2.0 * (1 - w) * (2.0 * t - 1);
        double2 ddN = 2.0 * (fP[0] - 2.0 * w * fP[1] + fP[2]);
        double ddD = 4.0 * (1 - w);
        double2 f = dN * D - N * dD;     // The first derivative is f / D^2
        double2 df = ddN * D - N * ddD;  // f'
        *d1 = f / (D * D);
        *d2 = (df * D - 2.0 * f * dD) / (D * D * D);
        return N / D;
    }

    // The unit tangent at 't'.
    double2 unitTangent(double t) const {
        double2 d1, d2;
        this->eval(t, &d1, &d2);
        return Direction(t, d1, d2);
    }

    // The signed curvature at 't', which is positive where the curve turns towards its left side
    // (see offset()).
    double curvature(double t) const {
        double2 d1, d2;
        this->eval(t, &d1, &d2);
        double speed2 = dot(d1, d1);
        return speed2 > 0 ? cross(d1, d2) / (speed2 * std::sqrt(speed2)) : 0.0;
    }

    // The curve's offset at 'radius' on its left side, rotated 90 degrees from its tangent towards
    // +y (side = 1), or on its right side (side = -1).
    double2 offset(double t, double radius, double side) const {
        double2 d1, d2;
        double2 p = this->eval(t, &d1, &d2);
        double2 u = Direction(t, d1, d2);
        return p + (radius * side) * double2{-u[1], u[0]};
    }

    // The derivative of offset(), which is parallel to the curve's. It points backwards where the
    // curve bends tighter than 'radius' towards 'side'.
    double2 offsetDerivative(double t, double radius, double side) const {
        double2 d1, d2;
        this->eval(t, &d1, &d2);
        double speed2 = dot(d1, d1);
        double k = speed2 > 0 ? cross(d1, d2) / (speed2 * std::sqrt(speed2)) : 0.0;
        return d1 * (1 - side * radius * k);
    }

private:
    // Where the curve stops moving (at an end whose control point coincides with it), its
    // direction is that of the second derivative, which it's about to move in (or just moved in).
    static double2 Direction(double t, double2 d1, double2 d2) {
        double2 v = dot(d1, d1) > 0 ? d1 : (t < 0.5 ? d2 : -d2);
        return v / length(v);
    }

    double2 fP[4];
    bool fIsCubic;
    double fW;
};

// Where a fold touches its curve (see ArcLengthFold).
struct Touch {
    double fT1;
    double fT2;
};

// Returns whether segments p0p1 and q0q1 cross, and sets how far along each one they do.
inline bool SegmentsCross(double2 p0, double2 p1, double2 q0, double2 q1, double* s, double* u) {
    double2 dp = p1 - p0;
    double2 dq = q1 - q0;
    double den = cross(dp, dq);
    if (den == 0) {
        return false;
    }
    double2 v = q0 - p0;
    *s = cross(v, dq) / den;
    *u = cross(v, dp) / den;
    return *s >= 0 && *s <= 1 && *u >= 0 && *u <= 1;
}

// Finds where the two branches of the curve's offset at 'radius' towards 'side' (before and after
// 'tStar', the tightest point of a bend) cross. Each branch is approximated by a polyline that's
// denser towards tStar, where small folds start and end. Returns the innermost crossing, where
// T2 - T1 is smallest.
inline std::optional<Touch> FindCrossing(const Curve& curve,
                                         double tStar,
                                         double radius,
                                         double side) {
    constexpr int N = kBranchSegments;
    double t1[N + 1], t2[N + 1];
    double2 o1[N + 1], o2[N + 1];
    const double end1 = tStar - kGap;
    const double start2 = tStar + kGap;
    for (int i = 0; i <= N; ++i) {
        double f = 1 - double(i) / N;
        double g = double(i) / N;
        t1[i] = end1 * (1 - f * f);
        t2[i] = start2 + (1 - start2) * (g * g);
        o1[i] = curve.offset(t1[i], radius, side);
        o2[i] = curve.offset(t2[i], radius, side);
    }
    std::optional<Touch> best;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double s, u;
            if (SegmentsCross(o1[i], o1[i + 1], o2[j], o2[j + 1], &s, &u)) {
                Touch touch = {t1[i] + s * (t1[i + 1] - t1[i]), t2[j] + u * (t2[j + 1] - t2[j])};
                if (!best || touch.fT2 - touch.fT1 < best->fT2 - best->fT1) {
                    best = touch;
                }
            }
        }
    }
    return best;
}

// Refines 'touch' with Newton's method so that the offset's branches meet there. Returns the best
// estimate, and sets 'error' to how far apart the branches are there.
inline Touch PolishCrossing(const Curve& curve,
                            double tStar,
                            double radius,
                            double side,
                            Touch touch,
                            double* error) {
    Touch best = touch;
    *error = std::numeric_limits<double>::infinity();
    for (int i = 0; i <= kNewtonSteps; ++i) {
        double2 f = curve.offset(touch.fT1, radius, side) - curve.offset(touch.fT2, radius, side);
        double err = length(f);
        if (err < *error) {
            best = touch;
            *error = err;
        }
        if (!(err > 1e-9 * std::max(1.0, radius))) {
            break;
        }
        // The columns of the Jacobian are the derivatives of the two branches.
        double2 j1 = curve.offsetDerivative(touch.fT1, radius, side);
        double2 j2 = -curve.offsetDerivative(touch.fT2, radius, side);
        double det = cross(j1, j2);
        if (det == 0 || !std::isfinite(det)) {
            break;
        }
        touch.fT1 = std::clamp(touch.fT1 + cross(-f, j2) / det, 0.0, tStar - kGap);
        touch.fT2 = std::clamp(touch.fT2 + cross(j1, -f) / det, tStar + kGap, 1.0);
    }
    return best;
}

// Returns where |curvature| peaks within [a, b], assuming it has one peak there.
inline double FindTightestPoint(const Curve& curve, double a, double b) {
    constexpr double g = 0.6180339887498949;  // (sqrt(5) - 1) / 2
    double c = b - g * (b - a);
    double d = a + g * (b - a);
    double kc = std::abs(curve.curvature(c));
    double kd = std::abs(curve.curvature(d));
    for (int i = 0; i < kGoldenSteps; ++i) {
        if (kc > kd) {
            b = d;
            d = c;
            kd = kc;
            c = b - g * (b - a);
            kc = std::abs(curve.curvature(c));
        } else {
            a = c;
            c = d;
            kc = kd;
            d = a + g * (b - a);
            kd = std::abs(curve.curvature(d));
        }
    }
    return (a + b) / 2;
}

// A fold, in double precision.
struct Fold {
    Touch fTouch;
    double2 fCorner;
    double2 fA;
    double2 fB;
    double fRadius;
};

// Finds the fold around 'tStar', a point where the curve bends tighter than 'radius'. Its inside
// can be on any of 'sides', and the offset can't cross itself there with a corner radius of
// 'minRadius' or less. 'samples' are the curve's curvatures at kCurvatureSamples + 1 uniform T
// values.
inline std::optional<Fold> FindFold(const Curve& curve,
                                    double tStar,
                                    const double sides[],
                                    int numSides,
                                    double minRadius,
                                    double radius,
                                    const double samples[]) {
    std::optional<Fold> best;
    for (int s = 0; s < numSides; ++s) {
        const double side = sides[s];
        // The branches can only cross where both of them move forwards, i.e. where the curve
        // doesn't bend tighter than the corner radius towards the inside. That limits how big
        // the corner radius can get before one of them has no such part left.
        double maxRadius = radius;
        double minK1 = std::numeric_limits<double>::infinity();
        double minK2 = minK1;
        for (int i = 0; i <= kCurvatureSamples; ++i) {
            double t = double(i) / kCurvatureSamples;
            if (t < tStar) {
                minK1 = std::min(minK1, side * samples[i]);
            } else if (t > tStar) {
                minK2 = std::min(minK2, side * samples[i]);
            }
        }
        for (double minK : {minK1, minK2}) {
            if (minK > 0) {
                maxRadius = std::min(maxRadius, 1 / minK);
            }
        }
        // At the curvature radius, the offset collapses towards the center of curvature, where
        // rounding errors can make its branches cross. (Folds are vanishingly small near there.)
        const double lowest = minRadius * (1 + kMinRadiusMargin);
        if (!(maxRadius > lowest)) {
            continue;
        }
        double rho = maxRadius;
        std::optional<Touch> touch;
        if (maxRadius == radius) {
            touch = FindCrossing(curve, tStar, rho, side);
        }
        if (!touch) {
            // The fold doesn't fit within the curve at full size, so shrink it until it does. (The
            // branches' crossing moves towards tStar as the corner radius shrinks.)
            //
            // F3: Alternatively, such folds could be skipped, or reach into the verbs on either
            // side (which would need the same lookahead as folds that span verbs; see
            // WriteStrokedPathVerbs()).
            double lo = lowest;
            double hi = maxRadius;
            for (int i = 0; i < kShrinkSteps; ++i) {
                double mid = (lo + hi) / 2;
                if (FindCrossing(curve, tStar, mid, side)) {
                    lo = mid;
                } else {
                    hi = mid;
                }
            }
            rho = lo;
            if (rho > lowest) {
                touch = FindCrossing(curve, tStar, rho, side);
            }
            if (!touch) {
                continue;
            }
        }
        double error;
        Touch t = PolishCrossing(curve, tStar, rho, side, *touch, &error);
        if (!(error <= 1e-4 * rho)) {
            continue;
        }
        double2 corner =
                (curve.offset(t.fT1, rho, side) + curve.offset(t.fT2, rho, side)) * 0.5;
        double2 a = curve.unitTangent(t.fT1);
        double2 b = curve.unitTangent(t.fT2);
        double sin = cross(a, b);
        double angle = std::atan2(std::abs(sin), dot(a, b));
        // Like a join's corner, the fold has to turn towards its inside.
        if (!(side * sin > 0 && angle >= kMinAngle)) {
            continue;
        }
        if (t.fT1 < kChopEpsilon) {
            t.fT1 = 0;
        }
        if (t.fT2 > 1 - kChopEpsilon) {
            t.fT2 = 1;
        }
        // The fold's ends must be far enough apart that they don't get merged when chopping.
        if (!(t.fT2 - t.fT1 >= 2 * kChopEpsilon)) {
            continue;
        }
        if (!best || t.fT2 - t.fT1 < best->fTouch.fT2 - best->fTouch.fT1) {
            best = Fold{t, corner, a, b, rho};
        }
    }
    return best;
}

}  // namespace arclength_folds

// Finds the folds (see ArcLengthFold) in a quad, conic, or cubic ('verb', with control points 'pts'
// and conic weight 'w') for a corner radius of 'radius', and writes them to 'folds' in order.
// Returns how many it found. 'cuspT' is where a cubic has a cusp, if it does (see
// FindCubicConvex180Chops()). Its fold is the limit of the folds of curves that nearly have a cusp
// there, and so is the fold of a small loop (see F7 below).
//
// The search starts from each point where the curve bends tightest (up to kMaxArcLengthFolds of
// them), if it bends tighter than 'radius' there. It finds where the curve's offset at 'radius' on
// the inside of the bend crosses itself, by intersecting polylines that approximate the offset's
// branches on either side of the tightest point, and refines that with Newton's method. A fold
// that doesn't fit within its curve shrinks until it does (see FindFold()). Everything is
// measured in local coordinates, so the folds don't depend on the transform.
inline int FindArcLengthFolds(SkPathVerb verb,
                              const SkPoint pts[],
                              float w,
                              float radius,
                              std::optional<float> cuspT,
                              ArcLengthFold folds[kMaxArcLengthFolds]) {
    using namespace arclength_folds;
    if (!(radius > 0.f)) {
        return 0;
    }
    const Curve curve(verb, pts, w);
    const double r = radius;
    // The middle of a loop that folds like a cusp (see below).
    std::optional<double> loopT;
    if (verb == SkPathVerb::kCubic && !cuspT) {
        double t[2], s[2];
        if (SkClassifyCubic(pts, t, s) == SkCubicType::kLoop) {
            // The (homogeneous) parameters where the cubic passes through its double point.
            double t0 = t[0] / s[0];
            double t1 = t[1] / s[1];
            if (t0 >= 0 && t0 <= 1 && t1 >= 0 && t1 <= 1) {
                // F7: While the stroke covers a whole loop, the loop looks like a cusp, and folds
                // like one: around the inside of the spike formed by the curve on either side of
                // it, where the stroke's sides cross. Its offsets don't cross where the search
                // expects otherwise. Loops whose holes show don't fold at all, since that fold
                // would replace the loop's arc length with a sweep around one corner. So the total
                // jumps where a loop's hole opens up (from ~205 to ~355 for the swallowtail family
                // in the arclengthfolds GM). Alternatives:
                //   - Loops could never fold. The total would jump where they become cusps instead
                //     (by ~24 for that family), and covered loops would keep their cliffs.
                //   - A loop's fold could sweep over more arc length as its hole grows, so that the
                //     total never jumps. This was tried, and looked worse: as the loop grows, its
                //     fan squeezes more and more of the gradient into one wedge. To do it, give
                //     ArcLengthFold a sweep radius next to fRadius, for length() and for the radius
                //     that PatchWriter encodes. The fragment shader only uses a corner's radius to
                //     scale its sweep, so C, fA, fB, and the shaders stay as they are. Set it so
                //     that the sweep covers angle * fRadius + s * (L - angle * fRadius), where L
                //     is the length of the curve between fT1 and fT2, and s is
                //     smoothstep(holeRadius / r). That's a cusp's sweep for a tiny loop, and the
                //     loop's own length as its hole opens up and it stops folding. Then the
                //     swallowtail family's total changes by at most ~1 per unit of P2.x.
                //
                // The hole shows if a disk bigger than the stroke radius fits inside the loop
                // (approximated by a polygon). This tries disks centered halfway between points on
                // either side of the loop's tip.
                double2 d1, d2;
                double2 loop[kLoopSamples + 1];
                for (int i = 0; i <= kLoopSamples; ++i) {
                    loop[i] = curve.eval(t0 + (t1 - t0) * i / kLoopSamples, &d1, &d2);
                }
                auto insideLoop = [&](double2 p) {
                    bool inside = false;
                    for (int i = 0; i < kLoopSamples; ++i) {
                        double2 a = loop[i], b = loop[(i + 1) % kLoopSamples];
                        if ((a[1] > p[1]) != (b[1] > p[1]) &&
                            p[0] < a[0] + (p[1] - a[1]) / (b[1] - a[1]) * (b[0] - a[0])) {
                            inside = !inside;
                        }
                    }
                    return inside;
                };
                auto distanceToLoop = [&](double2 p) {
                    double distance = std::numeric_limits<double>::infinity();
                    for (int i = 0; i < kLoopSamples; ++i) {
                        double2 a = loop[i], edge = loop[(i + 1) % kLoopSamples] - a;
                        double edgeLength2 = dot(edge, edge);
                        double u = edgeLength2 > 0
                                ? std::clamp(dot(p - a, edge) / edgeLength2, 0.0, 1.0) : 0.0;
                        distance = std::min(distance, length(p - (a + u * edge)));
                    }
                    return distance;
                };
                double holeRadius = 0;
                for (int i = 1; i <= kLoopSamples / 2; ++i) {
                    for (int j = kLoopSamples / 2; j < kLoopSamples; ++j) {
                        double2 center = (loop[i] + loop[j]) * 0.5;
                        if (insideLoop(center)) {
                            holeRadius = std::max(holeRadius, distanceToLoop(center));
                        }
                    }
                }
                if (!(holeRadius < r)) {
                    return 0;
                }
                loopT = (t0 + t1) / 2;
            }
        }
    }
    double samples[kCurvatureSamples + 1];
    double maxK = 0;
    for (int i = 0; i <= kCurvatureSamples; ++i) {
        samples[i] = curve.curvature(double(i) / kCurvatureSamples);
        if (!std::isfinite(samples[i])) {
            return 0;
        }
        maxK = std::max(maxK, std::abs(samples[i]));
    }
    if (maxK == 0) {
        // The curve is flat, including any cusp. Its offsets on either side of a cusp overlap
        // instead of crossing.
        return 0;
    }

    // The points where the curve bends tightest: |curvature|, T, and the side the curve turns
    // towards there (or 0 at a cusp, whose inside can be on either side).
    struct Candidate {
        double fK;
        double fT;
        double fSide;
    };
    Candidate candidates[2 * kMaxArcLengthFolds + 3];
    int numCandidates = 0;
    constexpr double h = 1.0 / kCurvatureSamples;
    if (cuspT) {
        candidates[numCandidates++] = {std::numeric_limits<double>::infinity(), *cuspT, 0};
    } else if (loopT) {
        // The spike's inside is the opposite of where the loop turns.
        candidates[numCandidates++] = {std::numeric_limits<double>::infinity(), *loopT,
                                       curve.curvature(*loopT) > 0 ? -1.0 : 1.0};
    } else {
        // Start from the biggest local maxima among the samples, and from guesses that catch
        // peaks too narrow for the samples, which only refining finds the height of.
        struct Seed {
            double fK;
            double fT;
        };
        Seed seeds[kCurvatureSamples + 1];
        int numSeeds = 0;
        for (int i = 0; i <= kCurvatureSamples; ++i) {
            double k = std::abs(samples[i]);
            double left = i > 0 ? std::abs(samples[i - 1]) : -1;
            double right = i < kCurvatureSamples ? std::abs(samples[i + 1]) : -1;
            if (k > left && k >= right && r * k > 0.25) {
                seeds[numSeeds++] = {k, double(i) / kCurvatureSamples};
            }
        }
        std::sort(seeds, seeds + numSeeds, [](const Seed& a, const Seed& b) {
            return a.fK != b.fK ? a.fK > b.fK : a.fT > b.fT;
        });
        numSeeds = std::min(numSeeds, kMaxArcLengthFolds);
        float guesses[3];
        int numGuesses = 0;
        if (verb == SkPathVerb::kCubic) {
            // Where the cubic slows down the most, like near a cusp.
            numGuesses = SkFindCubicMaxCurvature(pts, guesses);
        } else if (verb == SkPathVerb::kQuad) {
            guesses[numGuesses++] = SkFindQuadMaxCurvature(pts);
        } else {
            guesses[numGuesses++] = SkConic(pts, w).findMidTangent();
        }
        for (int i = 0; i < numGuesses; ++i) {
            if (r * std::abs(curve.curvature(guesses[i])) > 0.25) {
                seeds[numSeeds++] = {0, guesses[i]};
            }
        }
        for (int i = 0; i < numSeeds; ++i) {
            double t = seeds[i].fT;
            double tStar = FindTightestPoint(curve, std::max(0.0, t - h), std::min(1.0, t + h));
            double k = curve.curvature(tStar);
            if (!(r * std::abs(k) > 1 && tStar >= kChopEpsilon && tStar <= 1 - kChopEpsilon)) {
                continue;
            }
            Candidate candidate = {std::abs(k), tStar, k > 0 ? 1.0 : -1.0};
            bool merged = false;
            for (int j = 0; j < numCandidates; ++j) {
                if (std::abs(tStar - candidates[j].fT) < h) {
                    // The same peak; keep the tighter estimate.
                    if (candidate.fK > candidates[j].fK) {
                        candidates[j] = candidate;
                    }
                    merged = true;
                }
            }
            if (!merged) {
                candidates[numCandidates++] = candidate;
            }
        }
        std::stable_sort(candidates, candidates + numCandidates,
                         [](const Candidate& a, const Candidate& b) { return a.fK > b.fK; });
    }

    // Find each candidate's fold, starting with the tightest, unless it overlaps an earlier one.
    Fold found[kMaxArcLengthFolds];
    int numFolds = 0;
    for (int i = 0; i < numCandidates && numFolds < kMaxArcLengthFolds; ++i) {
        const Candidate& c = candidates[i];
        auto contains = [&](const Fold& f) {
            return f.fTouch.fT1 <= c.fT && c.fT <= f.fTouch.fT2;
        };
        if (std::any_of(found, found + numFolds, contains)) {
            continue;
        }
        static constexpr double kBothSides[] = {1, -1};
        std::optional<Fold> fold =
                c.fSide == 0 ? FindFold(curve, c.fT, kBothSides, 2, 0, r, samples)
                             : FindFold(curve, c.fT, &c.fSide, 1, 1 / c.fK, r, samples);
        auto overlaps = [&](const Fold& f) {
            return !(fold->fTouch.fT2 < f.fTouch.fT1 || fold->fTouch.fT1 > f.fTouch.fT2);
        };
        if (fold && !std::any_of(found, found + numFolds, overlaps)) {
            found[numFolds++] = *fold;
        }
    }
    if (numFolds == 2 && found[1].fTouch.fT1 < found[0].fTouch.fT1) {
        std::swap(found[0], found[1]);
    }
    for (int i = 0; i < numFolds; ++i) {
        const Fold& f = found[i];
        folds[i] = {(float)f.fTouch.fT1,
                    (float)f.fTouch.fT2,
                    {(float)f.fCorner[0], (float)f.fCorner[1]},
                    {(float)f.fA[0], (float)f.fA[1]},
                    {(float)f.fB[0], (float)f.fB[1]},
                    (float)f.fRadius};
    }
    return numFolds;
}

// Accumulates the arc length of a stroked path one verb at a time, in local (path) coordinates.
// PatchWriter (when writing kArcLength) and ArcLengthPrecalculator share it, so the total computed
// up front for devices without vertex storage matches the one accumulated while writing patches.
//
// Lines and curves add their own length, and caps add nothing. A join is parameterized as if the
// path were rounded off there by an arc of radius R (the join's "corner radius", normally the
// stroke radius) around the join's inner corner C, where the inner edges of the strokes on either
// side of it cross:
//   - The two strokes' straight parts end at their cross-sections through C. Those are
//     R * tan(angle / 2) (the join's "trim") before and after the path vertex.
//   - Between the two cross-sections, the arc length sweeps around C by angle, over a total of
//     angle * R (the join's length).
// The arc length then only depends on the position, so it's continuous across the join, and the
// strokes on either side and the join between them all agree on it where they overlap. None of
// that depends on the join's type, since round, miter, and bevel joins all lie within the sweep.
// Each join changes the total by angle * R - 2 * R * tan(angle / 2), which is never positive.
//
// A fold (see ArcLengthFold) is the same kind of corner partway through a curve that bends tighter
// than the stroke radius. WriteStrokedPathVerbs() chops the curve where the fold starts and ends,
// and brackets the pieces in between with beginFold() and endFold(). Those pieces add no length;
// the fold's sweep replaces them.
//
// A join has no corner, and adds no length, if:
//   - The stroke is a hairline.
//   - It follows a cusp (see cusp()), closes a contour (see close()), or ends a fold.
//   - Its tangents point in opposite directions, where its inner corner is undefined.
//
// How far a join can extend depends on the joins at the other ends of the verbs on either side of
// it (see Share()), so a verb's span is only known once the verb after it arrives, its contour
// ends, a cusp follows it, or a fold starts. Adding a verb therefore finishes the verb before it,
// and returns that verb's span.
//
// Everything is measured on the verbs as WriteStrokedPathVerbs() writes them, before PatchWriter
// chops curves to fit the tessellation limits, so the totals don't depend on the transform.
class ArcLengthAccumulator {
public:
    using float2 = skvx::float2;

    // The arc lengths of one verb and its preceding join. The join covers
    // [fStart, fStart + fJoinLength] and extends fJoinTrim into the verb. Past that, the arc length
    // grows with the distance s along the verb, as (fStart + fJoinLength) + (s - fJoinTrim), and
    // fEnd is where that reaches the end of the verb. The next join can extend back into the verb,
    // so the next verb's span can start before fEnd.
    struct Span {
        float fStart;
        float fJoinLength;
        float fCurveLength;
        float fEnd;
        float fJoinTrim;
        float fJoinRadius;  // The join's corner radius, or 0 if it has no corner
    };

    // What adding a verb did.
    struct Added {
        // Degenerate verbs change nothing, since PatchWriter doesn't write them.
        bool fDegenerate = false;
        // The span of the verb before it, if adding this verb finished that one.
        std::optional<Span> fFinished;
    };

    // The shape of a join, which only depends on the directions on either side of it.
    struct JoinShape {
        float fAngle;         // The angle between the two directions
        float fTanHalfAngle;  // tan(fAngle / 2): a corner of radius R has a trim of R * this
    };

    // Returns the radius used to parameterize joins: the stroke radius, so hairlines (whose radius
    // is 0) get no corners.
    static float JoinRadius(const StrokeParams& strokeParams) { return strokeParams.fRadius; }

    // Returns the shape of the join at 'p' between a verb arriving from 'prevTangentPt' and one
    // leaving towards 'nextTangentPt'. Returns nothing if either direction is undefined, or if
    // they're opposite; such joins can't have a corner.
    static std::optional<JoinShape> Shape(float2 prevTangentPt, float2 p, float2 nextTangentPt) {
        float2 in = p - prevTangentPt;
        float2 out = nextTangentPt - p;
        float inScale = std::max(std::abs(in[0]), std::abs(in[1]));
        float outScale = std::max(std::abs(out[0]), std::abs(out[1]));
        if (!(inScale > 0.f) || !(outScale > 0.f)) {
            return std::nullopt;
        }
        // Like the shader's $robust_normalize_diff(), scale both directions first so squaring them
        // can't overflow.
        in *= 1.f / inScale;
        out *= 1.f / outScale;
        in *= 1.f / skvx::length(in);
        out *= 1.f / skvx::length(out);
        float sin = std::abs(in[0] * out[1] - in[1] * out[0]);
        float cos = in[0] * out[0] + in[1] * out[1];
        // tan(angle / 2), computed whichever way avoids cancellation (like the vertex shader's
        // $arclength_corner()). It's infinite for opposite tangents.
        float tanHalfAngle = cos >= 0.f ? sin / (1.f + cos) : (1.f - cos) / sin;
        if (!std::isfinite(tanHalfAngle)) {
            return std::nullopt;
        }
        // Unlike acos(cos), atan2() is also accurate for nearly parallel tangents, so G1-continuous
        // joins measure ~0 instead of ~sqrt(epsilon) each.
        return JoinShape{std::atan2(sin, cos), tanHalfAngle};
    }

    // Returns how far a join can extend into a verb of length 'length'. 'trim' is the join's full
    // trim (with the stroke radius as its corner radius), and 'otherTrim' is the full trim of the
    // join at the verb's other end. Each join gets its full trim if both fit. Otherwise they split
    // the verb in proportion to their full trims.
    //
    // A join's trim is the smaller of what it gets from the verbs on either side of it. If that's
    // less than its full trim, its corner radius shrinks until the trim fits, which moves C towards
    // the path vertex. Then the join no longer covers the whole inner overlap of the two strokes,
    // and the rest of the overlap is resolved by the later stroke drawing on top, like other
    // self-overlaps.
    //
    // E1: Full trims grow without bound as turns get sharper. They don't fit where two sharp turns
    // share a short verb, at near-reversals, or where a stroke is wider than the curve it follows.
    // The inner sides of three or more verbs overlap there, so a parameterization built from one
    // join at a time can't be continuous there anyway. A sharp turn can also sweep over a long
    // stretch of the strokes. Along the path, the arc length only advances
    // (angle/2) / tan(angle/2) times as fast there as it does on a straight stroke. A near-reversal
    // with no other join on its verbs sweeps over the whole shorter verb. Alternatives:
    //   - Also fade the corner radius to 0 as the turn goes from N degrees to 180, e.g. by
    //     multiplying it by min(1, (cos(angle/2) / cos(N/2))^2) before computing the full trim.
    //     That bounds how far the sweep extends, and hairpins look like the stroke folding back
    //     over itself (with a cliff where the later stroke covers the earlier one), the way joins
    //     without a corner do. N = 150 degrees leaves the 140 degree turns of the zigzag in the
    //     arclengthjoins GM unchanged.
    //   - Always use the stroke radius. The trims can then run past the ends of the verbs, so
    //     that would need yet another fallback.
    //   - Limit each trim to half of each verb on either side. That needs no lookahead, but it
    //     also shrinks corners whose full trim would fit next to a smaller one.
    static float Share(float trim, float otherTrim, float length) {
        return trim + otherTrim <= length ? trim : length * (trim / (trim + otherTrim));
    }

    void setJoinRadius(float joinRadius) { fJoinRadius = joinRadius; }

    // The arc length at the end of the last finished verb.
    float accumulated() const { return fAccumulated; }

    // Starts a new contour at 'p'. The previous one must have been ended (see endContour()).
    void moveTo(float2 p) {
        SkASSERT(!fPending);
        SkASSERT(!fInFold);
        fContourStart = fLastPoint = p;
        fHasPrevTangent = false;
        fSuppressNextJoin = false;
    }

    // Ends the current contour by finishing its last verb, and returns that verb's span (if it
    // wasn't already finished). Nothing joins to the end of that verb: either a cap follows it, or
    // the closing join, which has no corner (see close()).
    std::optional<Span> endContour() {
        SkASSERT(!fInFold);
        std::optional<Span> span = this->finish(/*nextFullTrim=*/0.f);
        fHasPrevTangent = false;
        return span;
    }

    // Closes the current contour with a line back to its start (like
    // PatchWriter::closeDeferredStrokePatch()), ends it, and starts a new contour at the same
    // point. This drops the spans of the verbs it finishes, so it's only for
    // ArcLengthPrecalculator, which just needs the total.
    //
    // E4: The closing join (between the closing line, or the last verb if that line is degenerate,
    // and the first verb) has no corner and adds no length; it keeps the arc length of the start
    // of the contour. The seam where the arc length jumps from the end of the contour back to its
    // start then runs through the overlap of the strokes on either side of it. Alternatively, the
    // closing join could get a corner whose sweep belongs to the end of the contour. That would
    // put the seam on the corner's cross-section into the first verb, which is a straight line.
    void close() {
        this->line(fLastPoint, fContourStart);
        this->endContour();
        this->moveTo(fContourStart);
    }

    // A cusp is where a curve reverses direction. It adds no length, and the join that follows it
    // is treated as zero-length too. Nothing joins to the end of the verb before the cusp, so this
    // finishes that verb, and returns its span (if it wasn't already finished).
    //
    // Unless the stroke is a hairline, the cusp of a curve that isn't flat lies inside a fold (see
    // ArcLengthFold), whose sweep parameterizes the cusp and the pieces of the curve around it, and
    // whose pieces this doesn't affect. The folds of near-cusps converge to the cusp's fold, so the
    // total doesn't jump as a curve moves through a cusp. Flat cusps, where a line doubles back on
    // itself, can't fold.
    std::optional<Span> cusp() {
        std::optional<Span> span = this->finish(/*nextFullTrim=*/0.f);
        fSuppressNextJoin = true;
        return span;
    }

    // Starts a fold (see ArcLengthFold) partway through a curve, once the pieces of the curve
    // before it have been added. The fold's sweep starts where those end, and nothing joins to
    // their end (the boundary is smooth), so this finishes the last one and returns its span (if it
    // wasn't already finished). The pieces of the curve inside the fold add no length until
    // endFold().
    std::optional<Span> beginFold(const ArcLengthFold& fold) {
        SkASSERT(!fInFold);
        std::optional<Span> span = this->finish(/*nextFullTrim=*/0.f);
        fInFold = true;
        fFoldStart = fAccumulated;
        fFoldLength = fold.length();
        return span;
    }

    // Ends the fold started by beginFold(). The arc length continues from the end of the fold's
    // sweep, and the smooth join to the piece of the curve after it gets no corner.
    void endFold() {
        SkASSERT(fInFold);
        fInFold = false;
        fAccumulated = fFoldStart + fFoldLength;
        fSuppressNextJoin = true;
    }

    Added line(float2 p0, float2 p1) {
        if (all(p0 == p1)) {
            return {/*fDegenerate=*/true, std::nullopt};
        }
        return this->add(p0, p1, p0, p1, ArcLengthCalculator::LineLength(p0, p1));
    }

    Added quad(float2 p0, float2 p1, float2 p2) {
        if (all((p0 == p1) & (p1 == p2))) {
            return {/*fDegenerate=*/true, std::nullopt};
        }
        return this->add(p0, TangentPoint(p0, p1, p2), TangentPoint(p2, p1, p0), p2,
                         ArcLengthCalculator::ConicLength(p0, p1, p2, 1.f));
    }

    Added conic(float2 p0, float2 p1, float2 p2, float w) {
        if (all((p0 == p1) & (p1 == p2))) {
            return {/*fDegenerate=*/true, std::nullopt};
        }
        return this->add(p0, TangentPoint(p0, p1, p2), TangentPoint(p2, p1, p0), p2,
                         ArcLengthCalculator::ConicLength(p0, p1, p2, w));
    }

    Added cubic(float2 p0, float2 p1, float2 p2, float2 p3) {
        if (all((p0 == p1) & (p1 == p2) & (p2 == p3))) {
            return {/*fDegenerate=*/true, std::nullopt};
        }
        return this->add(p0, TangentPoint(p0, p1, p2, p3), TangentPoint(p3, p2, p1, p0), p3,
                         ArcLengthCalculator::CubicLength(p0, p1, p2, p3));
    }

private:
    // A verb that can't be finished until the full trim of the join after it is known.
    struct Pending {
        float fLength;
        // Its start join:
        float fJoinAngle = 0.f;
        float fJoinTanHalfAngle = 0.f;
        float fJoinRadius = 0.f;    // The full corner radius, or 0 if the join has no corner
        float fJoinFullTrim = 0.f;  // fJoinRadius * fJoinTanHalfAngle
    };

    // Returns the first candidate that differs from 'p' (or the last candidate). This is how
    // PatchWriter and the vertex shader pick the control point that defines a tangent at 'p'.
    static float2 TangentPoint(float2 p, float2 a, float2 b) {
        return any(a != p) ? a : b;
    }
    static float2 TangentPoint(float2 p, float2 a, float2 b, float2 c) {
        return any(a != p) ? a : (any(b != p) ? b : c);
    }

    Added add(float2 p0,
              float2 startTangentPt,
              float2 endTangentPt,
              float2 end,
              float curveLength) {
        if (fInFold) {
            // The fold's sweep covers this piece of its curve, so it adds no length, and nothing
            // that follows changes how it's parameterized.
            fPrevTangentPt = endTangentPt;
            fLastPoint = end;
            fHasPrevTangent = true;
            return {/*fDegenerate=*/false, std::nullopt};
        }
        Pending verb{curveLength};
        if (fHasPrevTangent && !fSuppressNextJoin && fJoinRadius > 0.f) {
            if (std::optional<JoinShape> shape = Shape(fPrevTangentPt, p0, startTangentPt)) {
                // A turn so close to 180 degrees that its full trim overflows gets no corner, like
                // an exact reversal.
                float fullTrim = fJoinRadius * shape->fTanHalfAngle;
                if (std::isfinite(fullTrim)) {
                    verb.fJoinAngle = shape->fAngle;
                    verb.fJoinTanHalfAngle = shape->fTanHalfAngle;
                    verb.fJoinRadius = fJoinRadius;
                    verb.fJoinFullTrim = fullTrim;
                }
            }
        }
        // The previous verb was waiting for this join's full trim.
        Added added = {/*fDegenerate=*/false, this->finish(verb.fJoinFullTrim)};
        fPending = verb;
        fPrevTangentPt = endTangentPt;
        fLastPoint = end;
        fHasPrevTangent = true;
        fSuppressNextJoin = false;
        return added;
    }

    // Finishes the pending verb (if there is one) now that the full trim of the join after it is
    // known, and returns its span.
    std::optional<Span> finish(float nextFullTrim) {
        if (!fPending) {
            return std::nullopt;
        }
        const Pending verb = *fPending;
        fPending.reset();
        // The verb's start join gets as much of its full trim as the verbs on either side of it
        // allow, and its corner radius shrinks to match.
        float trim = std::min({verb.fJoinFullTrim,
                               Share(verb.fJoinFullTrim, fPrevFullTrim, fPrevLength),
                               Share(verb.fJoinFullTrim, nextFullTrim, verb.fLength)});
        float radius = trim < verb.fJoinFullTrim ? trim / verb.fJoinTanHalfAngle
                                                 : verb.fJoinRadius;
        float joinLength = verb.fJoinAngle * radius;
        // The previous verb ends where the join's sweep starts, the join's trim before its end.
        float start = fAccumulated - trim;
        // The vertex shader evaluates positions as (start + join) + (s - trim); associate the same
        // way.
        Span span = {start,
                     joinLength,
                     verb.fLength,
                     (start + joinLength) + (verb.fLength - trim),
                     trim,
                     radius};
        fAccumulated = span.fEnd;
        fPrevLength = verb.fLength;
        fPrevFullTrim = verb.fJoinFullTrim;
        return span;
    }

    float fAccumulated = 0.f;  // Where the last finished verb ends
    float fJoinRadius = 0.f;
    std::optional<Pending> fPending;     // The contour's last verb, until it's finished
    float fPrevLength = 0.f;             // The length of the last finished verb
    float fPrevFullTrim = 0.f;           // The full trim of that verb's start join
    float2 fContourStart = {0.f, 0.f};
    float2 fLastPoint = {0.f, 0.f};      // End point of the contour's last non-degenerate verb
    float2 fPrevTangentPt = {0.f, 0.f};  // Defines the tangent at the end of that verb
    bool fHasPrevTangent = false;        // False until the contour has a non-degenerate verb
    bool fSuppressNextJoin = false;      // Set by cusp() and endFold()
    bool fInFold = false;                // Between beginFold() and endFold()
    float fFoldStart = 0.f;              // Where the current fold's sweep starts
    float fFoldLength = 0.f;             // How much arc length it covers
};

// Computes the total arc length that a PatchWriter writing kArcLength accumulates for the same
// stroke, without writing any patches. It implements the subset of PatchWriter's interface used by
// WriteStrokedPathVerbs(), so both see exactly the same verbs.
class ArcLengthPrecalculator {
public:
    explicit ArcLengthPrecalculator(const StrokeParams& strokeParams) {
        fAccumulator.setJoinRadius(ArcLengthAccumulator::JoinRadius(strokeParams));
    }

    void writeDeferredStrokePatch(SkPoint moveTo, std::optional<SkPaint::Cap>) {
        fAccumulator.endContour();
        fAccumulator.moveTo(sk_bit_cast<skvx::float2>(moveTo));
    }
    void closeDeferredStrokePatch(SkPaint::Cap) { fAccumulator.close(); }

    void writeLine(SkPoint p0, SkPoint p1) {
        fAccumulator.line(sk_bit_cast<skvx::float2>(p0), sk_bit_cast<skvx::float2>(p1));
    }
    void writeQuadratic(const SkPoint p[3]) {
        fAccumulator.quad(sk_bit_cast<skvx::float2>(p[0]),
                          sk_bit_cast<skvx::float2>(p[1]),
                          sk_bit_cast<skvx::float2>(p[2]));
    }
    void writeConic(const SkPoint p[3], float w) {
        fAccumulator.conic(sk_bit_cast<skvx::float2>(p[0]),
                           sk_bit_cast<skvx::float2>(p[1]),
                           sk_bit_cast<skvx::float2>(p[2]),
                           w);
    }
    void writeCubic(const SkPoint p[4]) {
        fAccumulator.cubic(sk_bit_cast<skvx::float2>(p[0]),
                           sk_bit_cast<skvx::float2>(p[1]),
                           sk_bit_cast<skvx::float2>(p[2]),
                           sk_bit_cast<skvx::float2>(p[3]));
    }
    // WriteStrokedPathVerbs() only writes circles for cusps.
    void writeCircle(SkPoint) { fAccumulator.cusp(); }

    void beginFold(const ArcLengthFold& fold) { fAccumulator.beginFold(fold); }
    void endFold() { fAccumulator.endFold(); }

    // The total, once the last contour has ended (which WriteStrokedPathVerbs() does).
    float totalLength() const { return fAccumulator.accumulated(); }

private:
    ArcLengthAccumulator fAccumulator;
};

// Writes a quad, conic, or cubic that has folds (see FindArcLengthFolds()) for
// WriteStrokedPathVerbs(). It's chopped where each fold starts and ends, and at 'chopTs' (a cubic's
// chops from FindCubicConvex180Chops(), which are cusps if 'areCusps'), and the pieces inside each
// fold are bracketed with beginFold() and endFold().
template <typename Writer>
void WriteFoldedCurve(Writer& writer,
                      SkPathVerb verb,
                      const SkPoint pts[],
                      float w,
                      const float chopTs[],
                      int numChops,
                      bool areCusps,
                      const ArcLengthFold folds[],
                      int numFolds) {
    struct Chop {
        float fT;
        bool fIsCusp;
        bool fEndsFold;
        int fBeginsFold;  // The fold that starts here, or -1

        // When chops are merged, the merged one keeps the T of a cusp, or else of a fold's end.
        int priority() const { return fIsCusp ? 2 : (fEndsFold || fBeginsFold >= 0 ? 1 : 0); }
    };
    constexpr int kMaxChops = 2 + 2 * kMaxArcLengthFolds;
    Chop chops[kMaxChops];
    int count = 0;
    for (int i = 0; i < numChops; ++i) {
        chops[count++] = {chopTs[i], areCusps, false, -1};
    }
    int foldAtStart = -1;
    bool foldAtEnd = false;
    for (int i = 0; i < numFolds; ++i) {
        if (folds[i].fT1 > 0) {
            chops[count++] = {folds[i].fT1, false, false, i};
        } else {
            foldAtStart = i;
        }
        if (folds[i].fT2 < 1) {
            chops[count++] = {folds[i].fT2, false, true, -1};
        } else {
            foldAtEnd = true;
        }
    }
    std::sort(chops, chops + count, [](const Chop& a, const Chop& b) { return a.fT < b.fT; });
    // Merge chops that are too close together to leave a stable piece between them. (This can
    // only drop a chop where the cubic inflects or rotates 180 degrees, which the tessellation
    // shaders tolerate overstepping a little; see FindCubicConvex180Chops().)
    int merged = 0;
    for (int i = 0; i < count; ++i) {
        if (merged > 0 && chops[i].fT - chops[merged - 1].fT < arclength_folds::kChopEpsilon) {
            Chop& m = chops[merged - 1];
            if (chops[i].priority() > m.priority()) {
                m.fT = chops[i].fT;
            }
            m.fIsCusp |= chops[i].fIsCusp;
            m.fEndsFold |= chops[i].fEndsFold;
            if (chops[i].fBeginsFold >= 0) {
                m.fBeginsFold = chops[i].fBeginsFold;
            }
        } else {
            chops[merged++] = chops[i];
        }
    }
    count = merged;
    float tValues[kMaxChops];
    for (int i = 0; i < count; ++i) {
        tValues[i] = chops[i].fT;
    }

    // Chop the curve into count + 1 pieces.
    SkPoint cubics[3 * kMaxChops + 4];
    SkPoint quads[kMaxChops + 1][3];
    SkConic conics[kMaxChops + 1];
    if (verb == SkPathVerb::kCubic) {
        SkChopCubicAt(pts, cubics, tValues, count);
        for (int i = 0; i < count; ++i) {
            if (chops[i].fIsCusp) {
                // In a perfect world, these 3 points would be equal after chopping on a cusp.
                cubics[3 * i + 2] = cubics[3 * i + 4] = cubics[3 * i + 3];
            }
        }
    } else if (verb == SkPathVerb::kQuad) {
        SkPoint rest[3] = {pts[0], pts[1], pts[2]};
        float prevT = 0.f;
        for (int i = 0; i < count; ++i) {
            SkPoint halves[5];
            SkChopQuadAt(rest, halves, (tValues[i] - prevT) / (1.f - prevT));
            std::copy(halves, halves + 3, quads[i]);
            std::copy(halves + 2, halves + 5, rest);
            prevT = tValues[i];
        }
        std::copy(rest, rest + 3, quads[count]);
    } else {
        SkASSERT(verb == SkPathVerb::kConic);
        // Unlike polynomial curves, conics can't be chopped one piece after another with rescaled
        // T values, since each chop reparameterizes the rest of the conic.
        const SkConic conic(pts, w);
        for (int i = 0; i <= count; ++i) {
            conic.chopAt(i > 0 ? tValues[i - 1] : 0.f, i < count ? tValues[i] : 1.f, &conics[i]);
            if (i > 0) {
                // Pieces chopped separately can disagree slightly on the point they share.
                conics[i].fPts[0] = conics[i - 1].fPts[2];
            }
        }
    }

    if (foldAtStart >= 0) {
        writer.beginFold(folds[foldAtStart]);
    }
    for (int i = 0; i <= count; ++i) {
        if (i > 0) {
            const Chop& chop = chops[i - 1];
            if (chop.fEndsFold) {
                writer.endFold();
            }
            if (chop.fIsCusp) {
                writer.writeCircle(cubics[3 * i]);
            }
            if (chop.fBeginsFold >= 0) {
                writer.beginFold(folds[chop.fBeginsFold]);
            }
        }
        if (verb == SkPathVerb::kCubic) {
            writer.writeCubic(cubics + 3 * i);
        } else if (verb == SkPathVerb::kQuad) {
            writer.writeQuadratic(quads[i]);
        } else {
            writer.writeConic(conics[i].fPts, conics[i].fW);
        }
    }
    if (foldAtEnd) {
        writer.endFold();
    }
}

// Writes the verbs of 'path' to 'writer' the way stroke tessellation expects them: cubics are
// chopped where they inflect or rotate 180 degrees, and cusps (in cubics, or in flat quads and
// conics) become a circle between two curves or lines. 'writer' is either a PatchWriter set up for
// strokes or an ArcLengthPrecalculator, so that both see exactly the same verbs.
//
// Curves are also chopped where their folds (see ArcLengthFold) for a corner radius of
// 'foldRadius' start and end, and the pieces inside each fold are bracketed with beginFold() and
// endFold(). 'foldRadius' should be ArcLengthAccumulator::JoinRadius() of the stroke, which is 0
// for hairlines, so they don't fold.
//
// F2: Folds only cover bends within one curve. A bend made of several verbs can also be tighter
// than the stroke radius, e.g. a short curve or line between two sharp joins, or a rounded corner
// (a conic) whose radius is less than the stroke's, since the offset of a circular arc doesn't
// cross itself. The inner sides of three or more verbs overlap there, and each join's corner is
// limited to the verbs on either side of it (see ArcLengthAccumulator::Share()), so the overlap
// still has cliffs. Folds could also span verbs, found on the offsets of all of them, but that
// needs more lookahead than one verb.
template <typename Writer>
void WriteStrokedPathVerbs(Writer& writer, const SkPath& path, SkPaint::Cap cap, float foldRadius) {
    ArcLengthFold folds[kMaxArcLengthFolds];
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                // This automatically joins the last contour with the first contour (deferred) if
                // the contour is closed. If the contour is not closed, it automatically adds
                // additional patches for the end cap of the last patch and the beginning cap of the
                // deferred patch. This does nothing if this is the beginning of the first contour.
                writer.writeDeferredStrokePatch(pts[0], cap);
                break;

            case SkPathVerb::kClose:
                // Draws a line back to the starting point of the contour and writes any deferred
                // patch with a join (instead of caps). Or if the contour was empty, draws a cap.
                // Since any deferred patch is consumed, the next moveTo's writeDeferredStrokePatch
                // will do nothing but record the beginning of the new contour.
                writer.closeDeferredStrokePatch(cap);
                break;

            case SkPathVerb::kLine:
                writer.writeLine(pts[0], pts[1]);
                break;

            case SkPathVerb::kQuad:
                if (ConicHasCusp(pts)) {
                    // The cusp is always at the midtangent.
                    SkPoint cusp = SkEvalQuadAt(pts, SkFindQuadMidTangent(pts));
                    // A quad can only have a cusp if it's flat with a 180-degree turnaround.
                    writer.writeLine(pts[0], cusp);
                    writer.writeCircle(cusp);
                    writer.writeLine(cusp, pts[2]);
                } else if (int numFolds = FindArcLengthFolds(verb, pts, 1.f, foldRadius,
                                                             std::nullopt, folds)) {
                    WriteFoldedCurve(writer, verb, pts, 1.f, nullptr, 0, false, folds, numFolds);
                } else {
                    writer.writeQuadratic(pts);
                }
                break;

            case SkPathVerb::kConic:
                if (ConicHasCusp(pts)) {
                    // The cusp is always at the midtangent.
                    SkConic conic(pts, *w);
                    SkPoint cusp = conic.evalAt(conic.findMidTangent());
                    // A conic can only have a cusp if it's flat with a 180-degree turnaround.
                    writer.writeLine(pts[0], cusp);
                    writer.writeCircle(cusp);
                    writer.writeLine(cusp, pts[2]);
                } else if (int numFolds = FindArcLengthFolds(verb, pts, *w, foldRadius,
                                                             std::nullopt, folds)) {
                    WriteFoldedCurve(writer, verb, pts, *w, nullptr, 0, false, folds, numFolds);
                } else {
                    writer.writeConic(pts, *w);
                }
                break;

            case SkPathVerb::kCubic: {
                SkPoint chops[10];
                float T[2];
                bool areCusps;
                int numChops = FindCubicConvex180Chops(pts, T, &areCusps);
                int numFolds = 0;
                if (numChops < 2 || !areCusps) {
                    // Unless the cubic is flat, its cusp is the tightest bend there is, so it
                    // folds (F5).
                    std::optional<float> cuspT;
                    if (numChops == 1 && areCusps) {
                        cuspT = T[0];
                    }
                    numFolds = FindArcLengthFolds(verb, pts, 1.f, foldRadius, cuspT, folds);
                }
                if (numFolds > 0) {
                    WriteFoldedCurve(writer, verb, pts, 1.f, T, numChops, areCusps, folds,
                                     numFolds);
                } else if (numChops == 0) {
                    writer.writeCubic(pts);
                } else if (numChops == 1) {
                    SkChopCubicAt(pts, chops, T[0]);
                    if (areCusps) {
                        // In a perfect world, these 3 points would be be equal after chopping
                        // on a cusp.
                        chops[2] = chops[4] = chops[3];
                        writer.writeCubic(chops);
                        writer.writeCircle(chops[3]);
                        writer.writeCubic(chops + 3);
                    } else {
                        writer.writeCubic(chops);
                        writer.writeCubic(chops + 3);
                    }
                } else {
                    SkASSERT(numChops == 2);
                    SkChopCubicAt(pts, chops, T[0], T[1]);
                    if (areCusps) {
                        // Two cusps are only possible if it's a flat line with two 180-degree
                        // turnarounds.
                        writer.writeLine(chops[0], chops[3]);
                        writer.writeCircle(chops[3]);
                        writer.writeLine(chops[3], chops[6]);
                        writer.writeCircle(chops[6]);
                        writer.writeLine(chops[6], chops[9]);
                    } else {
                        writer.writeCubic(chops);
                        writer.writeCubic(chops + 3);
                        writer.writeCubic(chops + 6);
                    }
                }
                break;
            }
        }
    }
    // Finish the last contour (next moveTo point doesn't matter)
    writer.writeDeferredStrokePatch({0.f, 0.f}, cap);
}

}  // namespace skgpu::tess

#endif  // skgpu_tess_ArcLengthCalculator_DEFINED

