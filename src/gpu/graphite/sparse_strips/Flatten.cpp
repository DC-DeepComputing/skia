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
    double fVal;  // The number of `subdivisions * 2 * sqrt_tol`.
};

SK_ALWAYS_INLINE double approxParabolaIntegral(double x) {
    constexpr double kD = 0.67;
    return x / (1.0 - kD + std::sqrt(std::sqrt(std::pow(kD, 4) + 0.25 * x * x)));
}

SK_ALWAYS_INLINE double approxParabolaInvIntegral(double x) {
    constexpr double kB = 0.39;
    return x * (1.0 - kB + std::sqrt(kB * kB + 0.25 * x * x));
}

SK_ALWAYS_INLINE size_t estimate(double err_div) {
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
        if (err_div <= kLUT[i]) {
            return i + 1;
        }
    }
    return 16;
}

SK_ALWAYS_INLINE size_t estimateNumQuads(const SkPoint pts[4], float accuracy) {
    constexpr float kToQuadTol = 0.1f;
    double q_accuracy = accuracy * kToQuadTol;
    double max_hypot2 = 432.0 * q_accuracy * q_accuracy;

    SkPoint p1x2 = pts[1] * 3.0f - pts[0];
    SkPoint p2x2 = pts[2] * 3.0f - pts[3];
    SkPoint err_v = p2x2 - p1x2;

    double err = SkPoint::DotProduct(err_v, err_v);
    double err_div = err / max_hypot2;

    return estimate(err_div);
}

SK_ALWAYS_INLINE FlattenParams estimateSubdiv(const SkPoint pts[3], double sqrtTol) {
    SkPoint d01 = pts[1] - pts[0];
    SkPoint d12 = pts[2] - pts[1];
    SkPoint dd = d01 - d12;

    double cross = SkPoint::CrossProduct(pts[2] - pts[0], dd);
    double x0 = SkPoint::DotProduct(d01, dd) / cross;
    double x2 = SkPoint::DotProduct(d12, dd) / cross;
    double scale = std::abs(cross / (dd.length() * (x2 - x0)));

    double a0 = approxParabolaIntegral(x0);
    double a2 = approxParabolaIntegral(x2);
    double val = 0.0;

    if (std::isfinite(scale)) {
        double da = std::abs(a2 - a0);
        double sqrt_scale = std::sqrt(scale);

        auto signum = [](double val) -> int { return (0.0 < val) - (val < 0.0); };

        if (signum(x0) == signum(x2)) {
            val = da * sqrt_scale;
        } else {
            double xmin = sqrtTol / sqrt_scale;
            val = sqrtTol * da / approxParabolaIntegral(xmin);
        }
    }

    double u0 = approxParabolaInvIntegral(a0);
    double u2 = approxParabolaInvIntegral(a2);
    double uscale = 1.0 / (u2 - u0);

    return {a0, a2, u0, uscale, val};
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

SK_ALWAYS_INLINE skvx::float8 ptSplatSimd(SkPoint pt) {
    skvx::float4 xyxy(pt.fX, pt.fY, pt.fX, pt.fY);
    return skvx::join(xyxy, xyxy);
}

SK_ALWAYS_INLINE skvx::float8 evalSimd(
        skvx::float8 p0, skvx::float8 p1, skvx::float8 p2, skvx::float8 p3, skvx::float8 t) {
    skvx::float8 mt = 1.0f - t;
    skvx::float8 im0 = p0 * mt * mt * mt;
    skvx::float8 im1 = p1 * mt * mt * 3.0f;
    skvx::float8 im2 = p2 * mt * 3.0f;
    skvx::float8 im3 = (p3 * t + im2) * t;

    return (im1 + im3) * t + im0;
}

SK_ALWAYS_INLINE skvx::float4 approxParabolaIntegralSimd(skvx::float4 x) {
    constexpr float kD = 0.67f;
    constexpr float kD_pow4 = 0.2015112f;  // 0.67^4

    skvx::float4 x2 = x * x;
    skvx::float4 temp = skvx::sqrt(skvx::sqrt(x2 * 0.25f + kD_pow4));
    skvx::float4 denom = temp + (1.0f - kD);

    return x / denom;
}

SK_ALWAYS_INLINE skvx::float8 approxParabolaInvIntegralSimd(skvx::float8 x) {
    constexpr float kB = 0.39f;
    constexpr float kOneMinusB = 1.0f - kB;
    constexpr float kB2 = kB * kB;

    skvx::float8 x2 = x * x;
    skvx::float8 temp = skvx::sqrt(x2 * 0.25f + kB2);
    skvx::float8 factor = temp + kOneMinusB;

    return x * factor;
}

SK_ALWAYS_INLINE skvx::int4 isFiniteSimd(skvx::float4 x) {
    skvx::int4 bits = sk_bit_cast<skvx::int4>(skvx::abs(x));
    return bits < skvx::int4(0x7f800000);
}

SK_ALWAYS_INLINE float distSqToSegment(SkPoint p, SkPoint a, SkPoint b) {
    SkPoint ab = b - a;
    SkPoint ap = p - a;
    float lenSq = SkPoint::DotProduct(ab, ab);
    float t = (lenSq == 0.0f) ? 0.0f : SkPoint::DotProduct(ap, ab) / lenSq;
    t = std::clamp(t, 0.0f, 1.0f);
    SkPoint proj = a + ab * t;
    SkPoint d = p - proj;
    return SkPoint::DotProduct(d, d);
}

template <typename ProcessQuadFn, typename ProcessCubicFn>
SK_ALWAYS_INLINE void processPathsImpl(const SkPath& path,
                                       Polyline& polyline,
                                       ProcessQuadFn&& processQuad,
                                       ProcessCubicFn&& processCubic) {
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
                processQuad(pts);
                lastPt = pts[2];
                break;
            }
            case SkPath::kCubic_Verb: {
                SkASSERT(!closed);
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

void Flatten::flattenQuad(const SkPoint pts[3], Polyline& polyline) {
    FlattenParams params = estimateSubdiv(pts, kSqrtTol);
    int n = std::max(1, static_cast<int>(std::ceil(0.5 / kSqrtTol * params.fVal)));
    double step = 1.0 / n;

    for (int i = 1; i < n; ++i) {
        double u = i * step;
        double t = determineSubdivT(params, u);
        polyline.appendPoint(evalQuad(pts, static_cast<float>(t)));
    }
    polyline.appendPoint(pts[2]);
}

size_t Flatten::flattenCubicScalar(const SkPoint pts[4]) {
    constexpr float kToQuadTol = 0.1f;
    size_t n_quads = estimateNumQuads(pts, kTol);
    fContext.fNumQuads = n_quads;
    float dt = 0.5f / n_quads;
    double tol = kTol * (1.0 - kToQuadTol);
    double sqrt_tol = std::sqrt(tol);
    float sum_val = 0.0f;

    for (size_t i = 0; i < n_quads; ++i) {
        float t0 = (2 * i) * dt;
        float t1 = (2 * i + 1) * dt;
        float t2 = (2 * i + 2) * dt;

        SkPoint p0 = evalCubic(pts, t0);
        SkPoint p_onehalf = evalCubic(pts, t1);
        SkPoint p2 = evalCubic(pts, t2);
        SkPoint p1 = p_onehalf * 2.0f - (p0 + p2) * 0.5f;

        fContext.fEvenPts[i] = p0;
        fContext.fOddPts[i] = p1;
        fContext.fEvenPts[i + 1] = p2;

        SkPoint quad[3] = {p0, p1, p2};
        FlattenParams params = estimateSubdiv(quad, sqrt_tol);

        fContext.fA0[i] = params.fA0;
        fContext.fDa[i] = params.fA2 - params.fA0;
        fContext.fU0[i] = params.fU0;
        fContext.fUScale[i] = params.fUScale;
        fContext.fVal[i] = params.fVal;

        sum_val += params.fVal;
    }

    size_t n = std::max<size_t>(1, static_cast<size_t>(std::ceil(0.5 * sum_val / sqrt_tol)));
    size_t target_len = n + 4;
    if (target_len > static_cast<size_t>(fContext.fFlattenedCubics.capacity())) {
        fContext.fFlattenedCubics.reserve(target_len);
    }
    fContext.fFlattenedCubics.resize(target_len);

    float step = sum_val / n;
    float step_recip = 1.0f / step;
    float val_sum = 0.0f;
    size_t last_n = 0;
    float x0base = 0.0f;

    for (size_t i = 0; i < n_quads; ++i) {
        float val = fContext.fVal[i];
        val_sum += val;
        float this_n = val_sum * step_recip;
        float this_n_next = 1.0f + std::floor(this_n);
        size_t dn = static_cast<size_t>(this_n_next) - last_n;

        if (dn > 0) {
            float dx = step / val;
            float x0 = x0base * dx;

            SkPoint p0 = fContext.fEvenPts[i];
            SkPoint p1 = fContext.fOddPts[i];
            SkPoint p2 = fContext.fEvenPts[i + 1];

            float a0 = fContext.fA0[i];
            float da = fContext.fDa[i];
            float u0 = fContext.fU0[i];
            float uscale = fContext.fUScale[i];

            for (size_t j = 0; j < dn; ++j) {
                float x = x0 + j * dx;
                float a = a0 + da * x;
                float u = approxParabolaInvIntegral(a);
                float t = (u - u0) * uscale;

                float mt = 1.0f - t;
                SkPoint p = p0 * (mt * mt) + p1 * (2.0f * t * mt) + p2 * (t * t);
                fContext.fFlattenedCubics[last_n + j] = p;
            }
        }
        x0base = this_n_next - this_n;
        last_n = static_cast<size_t>(this_n_next);
    }

    fContext.fFlattenedCubics[n] = fContext.fEvenPts[n_quads];
    return n + 1;
}

void Flatten::evalCubicsSimd(const SkPoint pts[4], size_t n) {
    fContext.fNumQuads = n;
    float dt = 0.5f / n;

    skvx::float8 p0_128 = ptSplatSimd(pts[0]);
    skvx::float8 p1_128 = ptSplatSimd(pts[1]);
    skvx::float8 p2_128 = ptSplatSimd(pts[2]);
    skvx::float8 p3_128 = ptSplatSimd(pts[3]);

    skvx::float4 iota_lo(0.0f, 0.0f, 2.0f, 2.0f);
    skvx::float4 iota_hi(1.0f, 1.0f, 3.0f, 3.0f);
    skvx::float8 iota = skvx::join(iota_lo, iota_hi);

    skvx::float8 step = iota * dt;
    skvx::float8 t = step;
    skvx::float8 t_inc(4.0f * dt);

    float* even_pts = reinterpret_cast<float*>(fContext.fEvenPts.data());
    float* odd_pts = reinterpret_cast<float*>(fContext.fOddPts.data());

    size_t loop_count = (n + 1) / 2;
    for (size_t i = 0; i < loop_count; ++i) {
        skvx::float8 evaluated = evalSimd(p0_128, p1_128, p2_128, p3_128, t);
        evaluated.lo.store(even_pts + i * 4);
        evaluated.hi.store(odd_pts + i * 4);
        t = t + t_inc;
    }

    p3_128.store(even_pts + n * 2);
}

void Flatten::estimateSubdivSimd(float sqrtTol) {
    size_t n = fContext.fNumQuads;
    const float* even_pts = reinterpret_cast<const float*>(fContext.fEvenPts.data());
    float* odd_pts = reinterpret_cast<float*>(fContext.fOddPts.data());

    size_t chunks = (n + 3) / 4;
    for (size_t i = 0; i < chunks; ++i) {
        skvx::float8 p0 = skvx::float8::Load(even_pts + i * 8);
        skvx::float8 p_onehalf = skvx::float8::Load(odd_pts + i * 8);
        skvx::float8 p2 = skvx::float8::Load(even_pts + i * 8 + 2);

        skvx::float8 p1 = p_onehalf * 2.0f - (p0 + p2) * 0.5f;
        p1.store(odd_pts + i * 8);

        skvx::float8 d01 = p1 - p0;
        skvx::float8 d12 = p2 - p1;

        skvx::float4 d01x = skvx::shuffle<0, 2, 4, 6>(d01);
        skvx::float4 d01y = skvx::shuffle<1, 3, 5, 7>(d01);
        skvx::float4 d12x = skvx::shuffle<0, 2, 4, 6>(d12);
        skvx::float4 d12y = skvx::shuffle<1, 3, 5, 7>(d12);

        skvx::float4 ddx = d01x - d12x;
        skvx::float4 ddy = d01y - d12y;

        skvx::float4 cross = (d01x + d12x) * ddy - (d01y + d12y) * ddx;
        skvx::float4 x0 = (d01x * ddx + d01y * ddy) / cross;
        skvx::float4 x2 = (d12x * ddx + d12y * ddy) / cross;

        skvx::float4 dd_hypot = skvx::sqrt(ddx * ddx + ddy * ddy);
        skvx::float4 scale = skvx::abs(cross / (dd_hypot * (x2 - x0)));

        skvx::float4 a0 = approxParabolaIntegralSimd(x0);
        skvx::float4 a2 = approxParabolaIntegralSimd(x2);
        skvx::float4 da = a2 - a0;
        skvx::float4 da_abs = skvx::abs(da);
        skvx::float4 sqrt_scale = skvx::sqrt(scale);

        skvx::int4 s0 = skvx::cast<int>(x0 > 0.0f) - skvx::cast<int>(x0 < 0.0f);
        skvx::int4 s2 = skvx::cast<int>(x2 > 0.0f) - skvx::cast<int>(x2 < 0.0f);
        skvx::int4 mask = (s0 == s2);

        skvx::float4 noncusp = da_abs * sqrt_scale;
        skvx::float4 xmin = sqrtTol / sqrt_scale;
        skvx::float4 approxint = approxParabolaIntegralSimd(xmin);
        skvx::float4 cusp = (sqrtTol * da_abs) / approxint;

        skvx::float4 val_raw = skvx::if_then_else(mask, noncusp, cusp);
        skvx::int4 finite_mask = isFiniteSimd(val_raw);
        skvx::float4 val = skvx::if_then_else(finite_mask, val_raw, skvx::float4(0.0f));

        skvx::float8 u0_u2 = approxParabolaInvIntegralSimd(skvx::join(a0, a2));
        skvx::float4 u0 = u0_u2.lo;
        skvx::float4 u2 = u0_u2.hi;
        skvx::float4 uscale = 1.0f / (u2 - u0);

        a0.store(fContext.fA0.data() + i * 4);
        da.store(fContext.fDa.data() + i * 4);
        u0.store(fContext.fU0.data() + i * 4);
        uscale.store(fContext.fUScale.data() + i * 4);
        val.store(fContext.fVal.data() + i * 4);
    }
}

void Flatten::outputLinesSimd(size_t i, float x0, float dx, size_t n, size_t start_idx) {
    skvx::float8 p0 = ptSplatSimd(fContext.fEvenPts[i]);
    skvx::float8 p1 = ptSplatSimd(fContext.fOddPts[i]);
    skvx::float8 p2 = ptSplatSimd(fContext.fEvenPts[i + 1]);

    skvx::float8 iota2 =
            skvx::join(skvx::float4(0.0f, 0.0f, 1.0f, 1.0f), skvx::float4(2.0f, 2.0f, 3.0f, 3.0f));

    skvx::float8 x = iota2 * dx + x0;
    skvx::float8 da(fContext.fDa[i]);
    skvx::float8 a = x * da + fContext.fA0[i];
    skvx::float8 a_inc(4.0f * dx * fContext.fDa[i]);
    skvx::float8 uscale(fContext.fUScale[i]);
    skvx::float8 u0(fContext.fU0[i]);

    size_t chunks = (n + 3) / 4;
    for (size_t j = 0; j < chunks; ++j) {
        skvx::float8 u = approxParabolaInvIntegralSimd(a);
        skvx::float8 t = (u - u0) * uscale;
        skvx::float8 mt = 1.0f - t;

        skvx::float8 p = p0 * (mt * mt) + p1 * (2.0f * t * mt) + p2 * (t * t);

        p.store(fContext.fFlattenedCubics.data() + start_idx + j * 4);
        a = a + a_inc;
    }
}

size_t Flatten::flattenCubicSimd(const SkPoint pts[4]) {
    constexpr float kToQuadTol = 0.1f;
    size_t n_quads = estimateNumQuads(pts, kTol);
    this->evalCubicsSimd(pts, n_quads);

    float tol = kTol * (1.0f - kToQuadTol);
    float sqrt_tol = std::sqrt(tol);
    this->estimateSubdivSimd(sqrt_tol);

    float sum_val = 0.0f;
    for (size_t i = 0; i < n_quads; ++i) {
        sum_val += fContext.fVal[i];
    }

    size_t n = std::max<size_t>(1, static_cast<size_t>(std::ceil(0.5f * sum_val / sqrt_tol)));

    size_t target_len = n + 4;
    if (target_len > static_cast<size_t>(fContext.fFlattenedCubics.capacity())) {
        fContext.fFlattenedCubics.reserve(target_len);
    }
    fContext.fFlattenedCubics.resize(target_len);

    float step = sum_val / n;
    float step_recip = 1.0f / step;
    float val_sum = 0.0f;
    size_t last_n = 0;
    float x0base = 0.0f;

    for (size_t i = 0; i < n_quads; ++i) {
        float val = fContext.fVal[i];
        val_sum += val;
        float this_n = val_sum * step_recip;
        float this_n_next = 1.0f + std::floor(this_n);
        size_t dn = static_cast<size_t>(this_n_next) - last_n;

        if (dn > 0) {
            float dx = step / val;
            float x0 = x0base * dx;
            this->outputLinesSimd(i, x0, dx, dn, last_n);
        }
        x0base = this_n_next - this_n;
        last_n = static_cast<size_t>(this_n_next);
    }

    fContext.fFlattenedCubics[n] = fContext.fEvenPts[n_quads];
    return n + 1;
}

void Flatten::processPathsSimd(const SkPath& path, float width, float height, Polyline& polyline) {
    fContext.fFlattenedCubics.clear();

    auto processQuad = [this, width, height, &polyline](const SkPoint pts[3]) {
        skvx::float8 P = skvx::float8::Load(pts);
        skvx::float4 X = skvx::shuffle<0, 2, 4, 4>(P);
        skvx::float4 Y = skvx::shuffle<1, 3, 5, 5>(P);

        bool culled = skvx::all(X > width) || skvx::all(Y < 0.0f) || skvx::all(Y > height);
        bool isLine =
                skvx::all(X < 0.0f) || distSqToSegment(pts[1], pts[0], pts[2]) <= 4.0f * kTol2;

        if (culled || isLine) {
            polyline.appendPoint(pts[2]);
        } else {
            this->flattenQuad(pts, polyline);
        }
    };

    auto processCubic = [this, width, height, &polyline](const SkPoint pts[4]) {
        skvx::float8 P = skvx::float8::Load(pts);
        skvx::float4 X = skvx::shuffle<0, 2, 4, 6>(P);
        skvx::float4 Y = skvx::shuffle<1, 3, 5, 7>(P);

        bool culled = skvx::all(X > width) || skvx::all(Y < 0.0f) || skvx::all(Y > height);
        bool isLine = skvx::all(X < 0.0f) ||
                      std::max(distSqToSegment(pts[1], pts[0], pts[3]),
                               distSqToSegment(pts[2], pts[0], pts[3])) <= (16.0f / 9.0f) * kTol2;

        if (culled || isLine) {
            polyline.appendPoint(pts[3]);
        } else {
            size_t max = this->flattenCubicSimd(pts);
            for (size_t i = 1; i < max; ++i) {
                polyline.appendPoint(fContext.fFlattenedCubics[i]);
            }
        }
    };

    processPathsImpl(path, polyline, processQuad, processCubic);
}

void Flatten::processPathsScalar(const SkPath& path,
                                 float width,
                                 float height,
                                 Polyline& polyline) {
    fContext.fFlattenedCubics.clear();

    auto processQuad = [this, width, height, &polyline](const SkPoint pts[3]) {
        bool culled = (pts[0].fX > width && pts[1].fX > width && pts[2].fX > width) ||
                      (pts[0].fY < 0.0f && pts[1].fY < 0.0f && pts[2].fY < 0.0f) ||
                      (pts[0].fY > height && pts[1].fY > height && pts[2].fY > height);
        bool isLine = (pts[0].fX < 0.0f && pts[1].fX < 0.0f && pts[2].fX < 0.0f) ||
                      distSqToSegment(pts[1], pts[0], pts[2]) <= 4.0f * kTol2;

        if (culled || isLine) {
            polyline.appendPoint(pts[2]);
        } else {
            this->flattenQuad(pts, polyline);
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
                std::max(distSqToSegment(pts[1], pts[0], pts[3]),
                         distSqToSegment(pts[2], pts[0], pts[3])) <= (16.0f / 9.0f) * kTol2;

        if (culled || isLine) {
            polyline.appendPoint(pts[3]);
        } else {
            size_t max = this->flattenCubicScalar(pts);
            for (size_t i = 1; i < max; ++i) {
                polyline.appendPoint(fContext.fFlattenedCubics[i]);
            }
        }
    };

    processPathsImpl(path, polyline, processQuad, processCubic);
}

}  // namespace skgpu::graphite
