/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/sparse_strips/Flatten.h"

namespace skgpu::graphite {

namespace {

struct FlattenParams {
    double fA0;
    double fA2;
    double fU0;
    double fUScale;
    double fVal;  // The number of `subdivisions * 2 * sqrtTol`.
};

SK_ALWAYS_INLINE double approxParabolaIntegral(double x) {
    constexpr double kD = 0.67;
    return x / (1.0 - kD + std::sqrt(std::sqrt(std::pow(kD, 4) + 0.25 * x * x)));
}

SK_ALWAYS_INLINE double approxParabolaInvIntegral(double x) {
    constexpr double kB = 0.39;
    return x * (1.0 - kB + std::sqrt(kB * kB + 0.25 * x * x));
}

SK_ALWAYS_INLINE size_t estimate(double errDiv) {
    constexpr double kLUT[16] = {
            1.0,
            64.0,
            729.0,
            4096.0,
            15625.0,
            46656.0,
            117649.0,
            262144.0,
            531441.0,
            1000000.0,
            1771561.0,
            2985984.0,
            4826809.0,
            7529536.0,
            11390625.0,
            16777216.0,
    };

    for (size_t i = 0; i < 16; ++i) {
        if (errDiv <= kLUT[i]) {
            return i + 1;
        }
    }
    return 16;
}

SK_ALWAYS_INLINE size_t estimateNumQuads(const SkPoint pts[4], float accuracy) {
    constexpr float kToQuadTol = 0.1f;
    double qAccuracy = accuracy * kToQuadTol;
    double maxHypot2 = 432.0 * qAccuracy * qAccuracy;

    SkPoint p1x2 = pts[1] * 3.0f - pts[0];
    SkPoint p2x2 = pts[2] * 3.0f - pts[3];
    SkPoint errV = p2x2 - p1x2;

    double err = SkPoint::DotProduct(errV, errV);
    double errDiv = err / maxHypot2;

    return estimate(errDiv);
}

SK_ALWAYS_INLINE FlattenParams estimateSubdiv(const SkPoint pts[3], double sqrtTol) {
    SkPoint d01 = pts[1] - pts[0];
    SkPoint d12 = pts[2] - pts[1];
    SkPoint dd = d01 - d12;

    double cross = SkPoint::CrossProduct(pts[2] - pts[0], dd);
    if (std::abs(cross) < 1e-7) {
        return {0.0, 0.0, 0.0, 1.0, 0.0};
    }

    double x0 = SkPoint::DotProduct(d01, dd) / cross;
    double x2 = SkPoint::DotProduct(d12, dd) / cross;
    double scale = std::abs(cross / (dd.length() * (x2 - x0)));

    double a0 = approxParabolaIntegral(x0);
    double a2 = approxParabolaIntegral(x2);
    double val = 0.0;

    if (std::isfinite(scale)) {
        double da = std::abs(a2 - a0);
        double sqrtScale = std::sqrt(scale);

        auto signum = [](double val) -> int { return (0.0 < val) - (val < 0.0); };

        if (signum(x0) == signum(x2)) {
            val = da * sqrtScale;
        } else {
            double xMin = sqrtTol / sqrtScale;
            val = sqrtTol * da / approxParabolaIntegral(xMin);
        }
    }

    double u0 = approxParabolaInvIntegral(a0);
    double u2 = approxParabolaInvIntegral(a2);
    double uScale = 1.0 / (u2 - u0);

    return {a0, a2, u0, uScale, val};
}

SK_ALWAYS_INLINE double determineSubdivT(const FlattenParams& params, double x) {
    double a = params.fA0 + (params.fA2 - params.fA0) * x;
    double u = approxParabolaInvIntegral(a);
    return (u - params.fU0) * params.fUScale;
}

SK_ALWAYS_INLINE SkPoint evalQuad(const SkPoint pts[3], float t) {
    float mt = 1.0f - t;
    return pts[0] * (mt * mt) + pts[1] * (2.0f * mt * t) + pts[2] * (t * t);
}

SK_ALWAYS_INLINE SkPoint evalCubic(const SkPoint pts[4], float t) {
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float t2 = t * t;
    return pts[0] * (mt2 * mt) + pts[1] * (3.0f * mt2 * t) + pts[2] * (3.0f * mt * t2) +
           pts[3] * (t2 * t);
}

SK_ALWAYS_INLINE bool isWithinDistSq(SkPoint p, SkPoint a, SkPoint b, float tolSq) {
    SkPoint ab = b - a;
    SkPoint ap = p - a;
    float lenSq = SkPoint::DotProduct(ab, ab);
    float tNum = SkPoint::DotProduct(ap, ab);

    if (lenSq == 0.0f || tNum <= 0.0f) {
        return SkPoint::DotProduct(ap, ap) <= tolSq;
    }
    if (tNum >= lenSq) {
        SkPoint bp = p - b;
        return SkPoint::DotProduct(bp, bp) <= tolSq;
    }

    // Lagrange's identity: apSq*lenSq - tNum*tNum == cross^2
    float cross = SkPoint::CrossProduct(ap, ab);
    return (cross * cross) <= (tolSq * lenSq);
}

template <bool kIsIdentity,
          typename ProcessQuadFn,
          typename ProcessConicFn,
          typename ProcessCubicFn>
SK_ALWAYS_INLINE void processPathsImpl(const SkPath& path,
                                       const SkMatrix& ctm,
                                       Polyline& polyline,
                                       ProcessQuadFn&& processQuad,
                                       ProcessConicFn&& processConic,
                                       ProcessCubicFn&& processCubic) {
    bool closed = true;
    SkPoint startPt = {0, 0};
    SkPoint lastPt = {0, 0};

    SkPath::Iter iter(path, false);
    SkPoint localPts[4];
    SkPoint mappedPts[4];
    SkPath::Verb verb;

    while ((verb = iter.next(localPts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb: {
                const SkPoint* pts = localPts;
                if constexpr (!kIsIdentity) {
                    ctm.mapPoints(SkSpan(mappedPts, 1), SkSpan(localPts, 1));
                    pts = mappedPts;
                }

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
                const SkPoint* pts = localPts;
                if constexpr (!kIsIdentity) {
                    ctm.mapPoints(SkSpan(mappedPts, 2), SkSpan(localPts, 2));
                    pts = mappedPts;
                }

                lastPt = pts[1];
                polyline.appendPoint(pts[1]);
                break;
            }
            case SkPath::kQuad_Verb: {
                SkASSERT(!closed);
                const SkPoint* pts = localPts;
                if constexpr (!kIsIdentity) {
                    ctm.mapPoints(SkSpan(mappedPts, 3), SkSpan(localPts, 3));
                    pts = mappedPts;
                }

                processQuad(pts);
                lastPt = pts[2];
                break;
            }
            case SkPath::kConic_Verb: {
                SkASSERT(!closed);
                const SkPoint* pts = localPts;
                if constexpr (!kIsIdentity) {
                    ctm.mapPoints(SkSpan(mappedPts, 3), SkSpan(localPts, 3));
                    pts = mappedPts;
                }

                processConic(pts, iter.conicWeight());
                lastPt = pts[2];
                break;
            }
            case SkPath::kCubic_Verb: {
                SkASSERT(!closed);
                const SkPoint* pts = localPts;
                if constexpr (!kIsIdentity) {
                    ctm.mapPoints(SkSpan(mappedPts, 4), SkSpan(localPts, 4));
                    pts = mappedPts;
                }

                processCubic(pts);
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

}  // anonymous namespace

SK_ALWAYS_INLINE void Flatten::flattenQuad(const SkPoint pts[3], Polyline& polyline) {
    FlattenParams params = estimateSubdiv(pts, kSqrtTol);

    if (params.fVal == 0.0) {
        // The quad is collinear or degenerate. It might fold back on itself.
        // We must find the extremum (t) to ensure the polyline covers the full spatial extent.
        float denX = pts[0].fX - 2.0f * pts[1].fX + pts[2].fX;
        float denY = pts[0].fY - 2.0f * pts[1].fY + pts[2].fY;

        float tX = -1.0f;
        float tY = -1.0f;

        if (std::abs(denX) > 1e-7f) {
            tX = (pts[0].fX - pts[1].fX) / denX;
        }
        if (std::abs(denY) > 1e-7f) {
            tY = (pts[0].fY - pts[1].fY) / denY;
        }

        // Because it's collinear, tX and tY should be roughly equal if both are valid.
        float t = tX >= 0.0f ? tX : tY;

        // If the fold point happens within the curve bounds, emit it.
        if (t > 0.0f && t < 1.0f) {
            polyline.appendPoint(evalQuad(pts, t));
        }
    } else {
        int n = std::max(1, static_cast<int>(std::ceil(0.5 / kSqrtTol * params.fVal)));
        double step = 1.0 / n;

        for (int i = 1; i < n; ++i) {
            double u = i * step;
            double t = determineSubdivT(params, u);
            polyline.appendPoint(evalQuad(pts, static_cast<float>(t)));
        }
    }

    polyline.appendPoint(pts[2]);
}

[[maybe_unused]]
SK_ALWAYS_INLINE void Flatten::flattenQuadSimd(const SkPoint pts[3], Polyline& polyline) {
    // Stub
}

SK_ALWAYS_INLINE size_t Flatten::flattenCubicScalar(const SkPoint pts[4]) {
    constexpr float kToQuadTol = 0.1f;
    size_t nQuads = estimateNumQuads(pts, kTol);
    fContext.fNumQuads = nQuads;
    float dt = 0.5f / nQuads;
    double tol = kTol * (1.0 - kToQuadTol);
    double sqrtTol = std::sqrt(tol);
    float sumVal = 0.0f;

    for (size_t i = 0; i < nQuads; ++i) {
        float t0 = (2 * i) * dt;
        float t1 = (2 * i + 1) * dt;
        float t2 = (2 * i + 2) * dt;

        SkPoint p0 = evalCubic(pts, t0);
        SkPoint pOneHalf = evalCubic(pts, t1);
        SkPoint p2 = evalCubic(pts, t2);
        SkPoint p1 = pOneHalf * 2.0f - (p0 + p2) * 0.5f;

        fContext.fEvenPts[i] = p0;
        fContext.fOddPts[i] = p1;
        fContext.fEvenPts[i + 1] = p2;

        SkPoint quad[3] = {p0, p1, p2};
        FlattenParams params = estimateSubdiv(quad, sqrtTol);

        fContext.fA0[i] = params.fA0;
        fContext.fDa[i] = params.fA2 - params.fA0;
        fContext.fU0[i] = params.fU0;
        fContext.fUScale[i] = params.fUScale;

        fContext.fVal[i] = std::max(static_cast<float>(params.fVal), 1e-7f);

        sumVal += params.fVal;
    }

    size_t n = std::max<size_t>(1, static_cast<size_t>(std::ceil(0.5 * sumVal / sqrtTol)));
    size_t targetLen = n + 4;
    if (targetLen > static_cast<size_t>(fContext.fFlattenedCubics.capacity())) {
        fContext.fFlattenedCubics.reserve(targetLen);
    }
    fContext.fFlattenedCubics.resize(targetLen);

    float step = sumVal / n;
    float stepRecip = 1.0f / step;
    float valSum = 0.0f;
    size_t lastN = 0;
    float x0Base = 0.0f;

    for (size_t i = 0; i < nQuads; ++i) {
        float val = fContext.fVal[i];
        valSum += val;
        float thisN = valSum * stepRecip;
        float thisNNext = 1.0f + std::floor(thisN);
        size_t dn = static_cast<size_t>(thisNNext) - lastN;

        if (dn > 0) {
            float dx = step / val;
            float x0 = x0Base * dx;

            SkPoint p0 = fContext.fEvenPts[i];
            SkPoint p1 = fContext.fOddPts[i];
            SkPoint p2 = fContext.fEvenPts[i + 1];

            float a0 = fContext.fA0[i];
            float da = fContext.fDa[i];
            float u0 = fContext.fU0[i];
            float uScale = fContext.fUScale[i];

            for (size_t j = 0; j < dn; ++j) {
                float x = x0 + j * dx;
                float a = a0 + da * x;
                float u = approxParabolaInvIntegral(a);
                float t = (u - u0) * uScale;

                float mt = 1.0f - t;
                SkPoint p = p0 * (mt * mt) + p1 * (2.0f * t * mt) + p2 * (t * t);
                fContext.fFlattenedCubics[lastN + j] = p;
            }
        }
        x0Base = thisNNext - thisN;
        lastN = static_cast<size_t>(thisNNext);
    }

    fContext.fFlattenedCubics[n] = fContext.fEvenPts[nQuads];
    return n + 1;
}

SK_ALWAYS_INLINE void Flatten::evalCubicsSimd(const SkPoint pts[4], size_t n) {
    // Stub
}

SK_ALWAYS_INLINE void Flatten::estimateSubdivSimd(float sqrtTol) {
    // Stub
}

SK_ALWAYS_INLINE void Flatten::outputLinesSimd(
        size_t i, float x0, float dx, size_t n, size_t startIdx) {
    // Stub
}

size_t Flatten::flattenCubicSimd(const SkPoint pts[4]) {
    // Stub
    return 0;
}

void Flatten::processPathsSimd(
        const SkPath& path, const SkMatrix& ctm, float width, float height, Polyline& polyline) {
    // Stub
}

void Flatten::processPathsScalar(
        const SkPath& path, const SkMatrix& ctm, float width, float height, Polyline& polyline) {
    fContext.fFlattenedCubics.clear();

    auto processQuad = [this, width, height, &polyline](const SkPoint pts[3]) {
        bool culled = (pts[0].fX > width && pts[1].fX > width && pts[2].fX > width) ||
                      (pts[0].fY < 0.0f && pts[1].fY < 0.0f && pts[2].fY < 0.0f) ||
                      (pts[0].fY > height && pts[1].fY > height && pts[2].fY > height);
        bool isLine = (pts[0].fX < 0.0f && pts[1].fX < 0.0f && pts[2].fX < 0.0f) ||
                      isWithinDistSq(pts[1], pts[0], pts[2], 4.0f * kTol2);

        if (culled || isLine) {
            polyline.appendPoint(pts[2]);
        } else {
            this->flattenQuad(pts, polyline);
        }
    };

    auto processConic = [this, &processQuad](const SkPoint pts[3], float weight) {
        const SkPoint* quadPts = fQuad.computeQuads(pts, weight, kTol);
        int quadCount = fQuad.countQuads();

        for (int i = 0; i < quadCount; ++i) {
            processQuad(&quadPts[i * 2]);
        }
    };

    auto processCubic = [this, width, height, &polyline](const SkPoint pts[4]) {
        bool culled =
                (pts[0].fX > width && pts[1].fX > width && pts[2].fX > width &&
                 pts[3].fX > width) ||
                (pts[0].fY < 0.0f && pts[1].fY < 0.0f && pts[2].fY < 0.0f && pts[3].fY < 0.0f) ||
                (pts[0].fY > height && pts[1].fY > height && pts[2].fY > height &&
                 pts[3].fY > height);
        bool isLine =
                (pts[0].fX < 0.0f && pts[1].fX < 0.0f && pts[2].fX < 0.0f && pts[3].fX < 0.0f) ||
                (isWithinDistSq(pts[1], pts[0], pts[3], (16.0f / 9.0f) * kTol2) &&
                 isWithinDistSq(pts[2], pts[0], pts[3], (16.0f / 9.0f) * kTol2));

        if (culled || isLine) {
            polyline.appendPoint(pts[3]);
        } else {
            size_t max = this->flattenCubicScalar(pts);
            for (size_t i = 1; i < max; ++i) {
                polyline.appendPoint(fContext.fFlattenedCubics[i]);
            }
        }
    };

    if (ctm.isIdentity()) {
        processPathsImpl<true>(path, ctm, polyline, processQuad, processConic, processCubic);
    } else {
        processPathsImpl<false>(path, ctm, polyline, processQuad, processConic, processCubic);
    }
}

}  // namespace skgpu::graphite
