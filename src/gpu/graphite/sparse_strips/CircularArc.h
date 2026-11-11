/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_CircularArc_DEFINED
#define skgpu_graphite_sparse_strips_CircularArc_DEFINED

#include "include/core/SkPoint.h"
#include "include/private/SkAttributes.h"
#include "src/core/SkVx.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>

namespace skgpu::graphite {

/*
 * A conic that is a circular arc, as produced by circles, round rects and arcTo. Such arcs are
 * flattened exactly, at uniform angles, instead of being approximated by cubics and quads. Used by
 * Flatten, and kept separate so that the stroker can share it.
 */
struct CircularArc {
    // Conics whose control polygon and weight match a circular arc this closely (relative) are
    // treated as one.
    static constexpr float kEpsilon = 1e-5f;
    // Arcs with larger radii are left to the general path, avoiding precision issues with their
    // far away centers.
    static constexpr float kMaxRadius = 1e5f;
    // Guards against pathological inputs.
    static constexpr int kMaxSteps = 1 << 16;

    SkPoint fCenter;
    float fRadius;
    float fTheta;  // the central angle, below 3 radians
    float fTurn;   // 1 if the tangent turns counterclockwise (positive cross product), else -1
    float fCosTheta;
    SkVector fU0;  // the unit tangents at the two ends
    SkVector fU1;

    // Returns false if the conic isn't a circular arc, or one too close to a half circle or too
    // large to handle precisely.
    static bool Make(const SkPoint pts[3], float weight, CircularArc* arc) {
        // A circular arc has an isosceles control polygon, with legs of nonzero length...
        const SkVector a = pts[1] - pts[0];
        const SkVector b = pts[2] - pts[1];
        const float aa = SkPoint::DotProduct(a, a);
        const float bb = SkPoint::DotProduct(b, b);
        if (!(aa > 0) || !(std::abs(aa - bb) <= kEpsilon * (aa + bb))) {
            return false;
        }
        const float la = std::sqrt(aa);
        const SkVector u0 = a * (1.0f / la);
        const SkVector u1 = b * (1.0f / std::sqrt(bb));
        // ...and, since its tangent turns by the central angle theta, a weight of cos(theta / 2),
        // i.e. weight^2 = (1 + cos(theta)) / 2.
        const float cosTheta = SkPoint::DotProduct(u0, u1);
        const float sinTheta = SkPoint::CrossProduct(u0, u1);
        if (!(cosTheta > -0.99f) || sinTheta == 0 ||
            !(std::abs(weight * weight - 0.5f * (1.0f + cosTheta)) <= kEpsilon)) {
            return false;
        }
        // The tangent length is R tan(theta / 2), with tan(theta / 2) = |sin| / (1 + cos).
        const float absSin = std::abs(sinTheta);
        const float radius = la * (1.0f + cosTheta) / absSin;
        if (!(radius < kMaxRadius)) {
            return false;
        }
        const float turn = sinTheta > 0 ? 1.0f : -1.0f;
        arc->fCenter = pts[0] + SkVector{-u0.fY, u0.fX} * (turn * radius);
        arc->fRadius = radius;
        arc->fTheta = std::atan2(absSin, cosTheta);
        arc->fTurn = turn;
        arc->fCosTheta = cosTheta;
        arc->fU0 = u0;
        arc->fU1 = u1;
        return true;
    }

    // The number of uniform angular steps for which the chords of the circle of 'radius' about the
    // center stay within 'tolerance' of it: the chord error radius * (1 - cos(step / 2)) is within
    // tolerance iff step <= 2 acos(1 - tolerance / radius), and acos(1 - x) >= sqrt(2 x).
    int steps(float radius, float tolerance) const {
        const float n = std::ceil(fTheta * std::sqrt(radius / (8.0f * tolerance)));
        return static_cast<int>(std::clamp(n, 1.0f, static_cast<float>(kMaxSteps)));
    }

    // Writes the points at the angles k * theta / n from the start, k = 1 .. n - 1, on the circles
    // of radii[i] about the center to dst[i][k - 1]. Writes whole vectors of 8 points, so each
    // dst[i] needs room for n + 6 points.
    template <int N> void sample(int n, const float (&radii)[N], SkPoint* const (&dst)[N]) const {
        using skvx::float8;
        // Point k sits at the angle psi_k = (k - n / 2) * step from the direction m of the arc's
        // middle, so |psi| < theta / 2 < 1.5 and sin_cos() applies. Each point is evaluated
        // independently, so no rounding error accumulates along the arc.
        const float step = fTheta / static_cast<float>(n);
        // m bisects the unit vectors from the center to the two ends, -turn * perp(u0 + u1), and
        // |u0 + u1| = 2 cos(theta / 2) = sqrt(2 + 2 cos(theta)).
        const SkVector sum = fU0 + fU1;
        const float scale = -fTurn / std::sqrt(2.0f + 2.0f * fCosTheta);
        const SkVector m = {-sum.fY * scale, sum.fX * scale};
        const SkVector mp = {-m.fY * fTurn, m.fX * fTurn};  // the direction at psi = 90 degrees
        const float8 index = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        const float halfN = 0.5f * static_cast<float>(n);
        for (int k = 0; k < n - 1; k += 8) {
            const float8 psi = (index + (static_cast<float>(k) - halfN)) * step;
            float8 s, c;
            sin_cos(psi, &s, &c);
            const float8 vx = skvx::fma(c, float8(m.fX), s * mp.fX);
            const float8 vy = skvx::fma(c, float8(m.fY), s * mp.fY);
            for (int i = 0; i < N; ++i) {
                const float8 x = skvx::fma(vx, float8(radii[i]), float8(fCenter.fX));
                const float8 y = skvx::fma(vy, float8(radii[i]), float8(fCenter.fY));
                skvx::shuffle<0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15>(
                        skvx::join(x, y))
                        .store(dst[i] + k);
            }
        }
    }

private:
    // sin(x) and cos(x) for |x| <= 1.5, by their Taylor series through x^13 and x^14. The
    // remainders are below 4e-10, so float rounding dominates the error.
    SK_ALWAYS_INLINE static void sin_cos(skvx::float8 x, skvx::float8* s, skvx::float8* c) {
        static constexpr float kSin[] = {1.0f / 6227020800.0f,
                                         -1.0f / 39916800.0f,
                                         1.0f / 362880.0f,
                                         -1.0f / 5040.0f,
                                         1.0f / 120.0f,
                                         -1.0f / 6.0f,
                                         1.0f};
        static constexpr float kCos[] = {-1.0f / 87178291200.0f,
                                         1.0f / 479001600.0f,
                                         -1.0f / 3628800.0f,
                                         1.0f / 40320.0f,
                                         -1.0f / 720.0f,
                                         1.0f / 24.0f,
                                         -0.5f,
                                         1.0f};
        const skvx::float8 x2 = x * x;
        skvx::float8 sp(kSin[0]);
        for (size_t i = 1; i < std::size(kSin); ++i) {
            sp = skvx::fma(sp, x2, skvx::float8(kSin[i]));
        }
        skvx::float8 cp(kCos[0]);
        for (size_t i = 1; i < std::size(kCos); ++i) {
            cp = skvx::fma(cp, x2, skvx::float8(kCos[i]));
        }
        *s = x * sp;
        *c = cp;
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_CircularArc_DEFINED
