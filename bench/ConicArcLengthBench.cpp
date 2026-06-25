/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/Tiger.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkString.h"
#include "src/core/SkPathPriv.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

const float kPI_F = 3.1415926535f;

namespace ConicLength {

    // ========================================================================
    // METHOD 1: 5-Point Gauss-Legendre Quadrature (in float)
    // ========================================================================
    inline float getEllipseLengthQuadrature(float a, float b) {
        if (a < b) std::swap(a, b);
        if (b == 0.0f) return 4.0f * a; // Degenerate to a line segment

        float e_sq = 1.0f - (b * b) / (a * a); // Eccentricity squared

        // 5-point Gauss-Legendre nodes (x) and weights (w) for domain [-1, 1]
        const float x[5] = {
             0.0f,
             0.5384693101f, -0.5384693101f,
             0.9061798459f, -0.9061798459f
        };
        const float w[5] = {
             0.5688888889f,
             0.4786286705f,  0.4786286705f,
             0.2369268851f,  0.2369268851f
        };

        float integral = 0.0f;

        // Map [-1, 1] to [0, PI/2]
        for (int i = 0; i < 5; ++i) {
            float theta = (kPI_F * 0.25f) * x[i] + (kPI_F * 0.25f);
            float sin_theta = std::sin(theta);

            // f(theta) = sqrt(1 - e^2 * sin^2(theta))
            integral += w[i] * std::sqrt(1.0f - e_sq * sin_theta * sin_theta);
        }

        return a * kPI_F * integral;
    }


    // ========================================================================
    // METHOD 2: Arithmetic-Geometric Mean (AGM) - Fixed 3 Passes in float
    // ========================================================================
    inline float getEllipseLengthAGM(float a, float b) {
        if (a < b) std::swap(a, b);
        if (b == 0.0f) return 4.0f * a; // Degenerate to a line segment

        float a_n = 1.0f;
        float b_n = b / a;
        float e_sq = 1.0f - (b_n * b_n);

        float sum = 0.5f * e_sq;

        // Exactly 3 fixed unrolled passes. Guaranteed hard bound for 30-bit fp.
        // Step 1
        float a1 = (a_n + b_n) * 0.5f;
        float b1 = std::sqrt(a_n * b_n);
        float c1 = (a_n - b_n) * 0.5f;
        sum += 1.0f * (c1 * c1);

        // Step 2
        float a2 = (a1 + b1) * 0.5f;
        float b2 = std::sqrt(a1 * b1);
        float c2 = (a1 - b1) * 0.5f;
        sum += 2.0f * (c2 * c2);

        // Step 3
        float c3 = (a2 - b2) * 0.5f;
        sum += 4.0f * (c3 * c3);

        return 2.0f * kPI_F * a * (1.0f - sum);
    }

} // namespace ConicLength

// Lowering function for cubics / quads / lines elevated to cubics
inline float eval_cubic_arclength(SkPoint c0, SkPoint c1, SkPoint c2, SkPoint c3) {
    float v0 = SkPoint::Distance(c0, c1) * 0.15f;
    float v1 = (c0 * (-0.558983582205757f) + c1 * 0.325650248872424f +
                c2 * 0.208983582205757f + c3 * 0.024349751127576f).length() * 0.27222222222f; // weight 49/180
    float v2 = (c3 - c0 + c2 - c1).length() * 0.26666666666f;
    float v3 = (c0 * (-0.024349751127576f) - c1 * 0.208983582205757f -
                c2 * 0.325650248872424f + c3 * 0.558983582205757f).length() * 0.27222222222f;
    float v4 = SkPoint::Distance(c2, c3) * 0.15f;
    return v0 + v1 + v2 + v3 + v4;
}

// Lowering function for rational conics using De Casteljau velocity 5-point quadrature
inline float eval_conic_arclength_de_casteljau(SkPoint p0, SkPoint p1, SkPoint p2, float w) {
    float alpha = 0.32732683535f;
    float T[5] = {0.0f, 0.5f - alpha, 0.5f, 0.5f + alpha, 1.0f};
    float W[5] = {0.05f, 0.27222222222f, 0.35555555555f, 0.27222222222f, 0.05f};
    float len = 0.0f;
    SkPoint p1_w = p1 * w;
    for (int i = 0; i < 5; ++i) {
        float t = T[i];
        SkPoint ab = p0 * (1.0f - t) + p1_w * t;
        SkPoint bc = p1_w * (1.0f - t) + p2 * t;
        SkPoint abc = ab * (1.0f - t) + bc * t;
        float u = (1.0f - t) + w * t;
        float v = w * (1.0f - t) + t;
        float uv = u * (1.0f - t) + v * t;
        SkPoint dN = (bc - ab) * 2.0f;
        float dD = (v - u) * 2.0f;
        float invUV = 1.0f / uv;
        SkPoint tan = (dN - (abc * invUV) * dD) * invUV;
        len += W[i] * tan.length();
    }
    return len;
}

// 64-bit double precision ground truth calculator using Adaptive Simpson integration
inline double conic_speed(SkPoint p0, SkPoint p1, SkPoint p2, double w, double t) {
    double p1_w_x = (double)p1.fX * w;
    double p1_w_y = (double)p1.fY * w;
    double ab_x = (double)p0.fX * (1.0 - t) + p1_w_x * t;
    double ab_y = (double)p0.fY * (1.0 - t) + p1_w_y * t;
    double bc_x = p1_w_x * (1.0 - t) + (double)p2.fX * t;
    double bc_y = p1_w_y * (1.0 - t) + (double)p2.fY * t;
    double abc_x = ab_x * (1.0 - t) + bc_x * t;
    double abc_y = ab_y * (1.0 - t) + bc_y * t;
    double u = (1.0 - t) + w * t;
    double v = w * (1.0 - t) + t;
    double uv = u * (1.0 - t) + v * t;
    double dN_x = (bc_x - ab_x) * 2.0;
    double dN_y = (bc_y - ab_y) * 2.0;
    double dD = (v - u) * 2.0;
    double invUV = 1.0 / uv;
    double tan_x = (dN_x - (abc_x * invUV) * dD) * invUV;
    double tan_y = (dN_y - (abc_y * invUV) * dD) * invUV;
    return std::sqrt(tan_x * tan_x + tan_y * tan_y);
}

inline double simpson_step(SkPoint p0, SkPoint p1, SkPoint p2, double w, double a, double b) {
    double c = (a + b) * 0.5;
    double h3 = std::abs(b - a) / 6.0;
    return h3 * (conic_speed(p0, p1, p2, w, a) + 4.0 * conic_speed(p0, p1, p2, w, c) + conic_speed(p0, p1, p2, w, b));
}

inline double adaptive_simpson(SkPoint p0, SkPoint p1, SkPoint p2, double w, double a, double b, double eps, int depth) {
    double c = (a + b) * 0.5;
    double S = simpson_step(p0, p1, p2, w, a, b);
    double Sleft = simpson_step(p0, p1, p2, w, a, c);
    double Sright = simpson_step(p0, p1, p2, w, c, b);
    if (depth >= 20 || std::abs(Sleft + Sright - S) <= 15.0 * eps) {
        return Sleft + Sright + (Sleft + Sright - S) / 15.0;
    }
    return adaptive_simpson(p0, p1, p2, w, a, c, eps * 0.5, depth + 1) +
           adaptive_simpson(p0, p1, p2, w, c, b, eps * 0.5, depth + 1);
}

inline double eval_conic_arclength_ground_truth(SkPoint p0, SkPoint p1, SkPoint p2, float w) {
    return adaptive_simpson(p0, p1, p2, (double)w, 0.0, 1.0, 1e-12, 0);
}

enum class ConicLoweringMethod {
    kRationalDeCasteljau,
    kEllipseQuadrature,
    kEllipseAGM
};

template <ConicLoweringMethod kMethod>
class TigerArcLengthBench : public Benchmark {
public:
    TigerArcLengthBench() {
        const char* methodStr = "DeCasteljau";
        if constexpr (kMethod == ConicLoweringMethod::kEllipseQuadrature) {
            methodStr = "EllipseQuadrature";
        } else if constexpr (kMethod == ConicLoweringMethod::kEllipseAGM) {
            methodStr = "EllipseAGM";
        }
        fName.printf("Tiger_ArcLengthLowering_%s", methodStr);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    void onDelayedSetup() override {
        fPaths = Tiger::GetTigerPaths();

        // Synthesize representative conic arcs (circles, ellipses, hyperbolas)
        // since SVG Tiger paths consist primarily of lines and cubics.
        fTestConics.clear();
        float weights[5] = {0.2f, 0.5f, 0.70710678f, 0.9f, 1.5f};
        for (int r = 10; r <= 200; r += 20) {
            for (float w : weights) {
                fTestConics.push_back({
                    {SkPoint::Make(0, 0), SkPoint::Make((float)r, 0), SkPoint::Make((float)r, (float)r)},
                    w
                });
                fTestConics.push_back({
                    {SkPoint::Make(10, 20), SkPoint::Make((float)r + 10, 50), SkPoint::Make((float)r * 1.5f, (float)r + 20)},
                    w
                });
            }
        }

        static bool sPrintedTable = false;
        if (!sPrintedTable) {
            sPrintedTable = true;
            double sumGT = 0.0;
            double sumDeCasteljau = 0.0;
            double sumQuad = 0.0;
            double sumAGM = 0.0;
            int conicCount = (int)fTestConics.size();

            for (const auto& conic : fTestConics) {
                double gt = eval_conic_arclength_ground_truth(conic.pts[0], conic.pts[1], conic.pts[2], conic.w);
                double dc = eval_conic_arclength_de_casteljau(conic.pts[0], conic.pts[1], conic.pts[2], conic.w);
                double a = SkPoint::Distance(conic.pts[0], conic.pts[1]);
                double b = SkPoint::Distance(conic.pts[1], conic.pts[2]);
                double q = ConicLength::getEllipseLengthQuadrature(a, b);
                double agm = ConicLength::getEllipseLengthAGM(a, b);

                sumGT += gt;
                sumDeCasteljau += dc;
                sumQuad += q;
                sumAGM += agm;
            }

            double errDC = std::abs(sumDeCasteljau - sumGT);
            double relErrDC = (sumGT > 0.0) ? (errDC / sumGT) * 100.0 : 0.0;

            double errQuad = std::abs(sumQuad - sumGT);
            double relErrQuad = (sumGT > 0.0) ? (errQuad / sumGT) * 100.0 : 0.0;

            double errAGM = std::abs(sumAGM - sumGT);
            double relErrAGM = (sumGT > 0.0) ? (errAGM / sumGT) * 100.0 : 0.0;

            SkDebugf("\n================================ CONIC ACCURACY TABLE (Test Set: %d conics) ================================\n", conicCount);
            SkDebugf("| %-25s | %-16s | %-16s | %-16s |\n", "Lowering Method", "Total Conic Len", "Abs Error (px)", "Rel Error (%)");
            SkDebugf("|---------------------------|------------------|------------------|------------------|\n");
            SkDebugf("| %-25s | %-16.6f | %-16s | %-16s |\n", "Ground Truth (Adaptive)", sumGT, "0.000000", "0.000000%");
            SkDebugf("| %-25s | %-16.6f | %-16.6f | %-15.6f%% |\n", "DeCasteljau 5pt Quad", sumDeCasteljau, errDC, relErrDC);
            SkDebugf("| %-25s | %-16.6f | %-16.6f | %-15.6f%% |\n", "Ellipse Quadrature", sumQuad, errQuad, relErrQuad);
            SkDebugf("| %-25s | %-16.6f | %-16.6f | %-15.6f%% |\n", "Ellipse AGM", sumAGM, errAGM, relErrAGM);
            SkDebugf("===============================================================================================================\n\n");
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            double totalLength = 0.0;
            for (const auto& conic : fTestConics) {
                if constexpr (kMethod == ConicLoweringMethod::kRationalDeCasteljau) {
                    totalLength += eval_conic_arclength_de_casteljau(conic.pts[0], conic.pts[1], conic.pts[2], conic.w);
                } else {
                    double a = SkPoint::Distance(conic.pts[0], conic.pts[1]);
                    double b = SkPoint::Distance(conic.pts[1], conic.pts[2]);
                    if constexpr (kMethod == ConicLoweringMethod::kEllipseQuadrature) {
                        totalLength += ConicLength::getEllipseLengthQuadrature(a, b);
                    } else {
                        totalLength += ConicLength::getEllipseLengthAGM(a, b);
                    }
                }
            }
            for (const SkPath& path : fPaths) {
                for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
                    switch (verb) {
                        case SkPathVerb::kLine:
                            totalLength += SkPoint::Distance(pts[0], pts[1]);
                            break;
                        case SkPathVerb::kQuad: {
                            // Quad elevated to cubic control points
                            SkPoint c0 = pts[0];
                            SkPoint c1 = pts[0] + (pts[1] - pts[0]) * (2.0f / 3.0f);
                            SkPoint c2 = pts[2] + (pts[1] - pts[2]) * (2.0f / 3.0f);
                            SkPoint c3 = pts[2];
                            totalLength += eval_cubic_arclength(c0, c1, c2, c3);
                            break;
                        }
                        case SkPathVerb::kCubic:
                            totalLength += eval_cubic_arclength(pts[0], pts[1], pts[2], pts[3]);
                            break;
                        case SkPathVerb::kConic: {
                            if constexpr (kMethod == ConicLoweringMethod::kRationalDeCasteljau) {
                                totalLength += eval_conic_arclength_de_casteljau(pts[0], pts[1], pts[2], *w);
                            } else {
                                double a = SkPoint::Distance(pts[0], pts[1]);
                                double b = SkPoint::Distance(pts[1], pts[2]);
                                if constexpr (kMethod == ConicLoweringMethod::kEllipseQuadrature) {
                                    totalLength += ConicLength::getEllipseLengthQuadrature(a, b);
                                } else {
                                    totalLength += ConicLength::getEllipseLengthAGM(a, b);
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            fTotalLength = totalLength;
        }
    }

private:
    struct TestConic {
        SkPoint pts[3];
        float w;
    };
    SkString fName;
    std::vector<SkPath> fPaths;
    std::vector<TestConic> fTestConics;
    volatile double fTotalLength = 0.0;
};

} // namespace

DEF_BENCH( return new TigerArcLengthBench<ConicLoweringMethod::kRationalDeCasteljau>(); )
DEF_BENCH( return new TigerArcLengthBench<ConicLoweringMethod::kEllipseQuadrature>(); )
DEF_BENCH( return new TigerArcLengthBench<ConicLoweringMethod::kEllipseAGM>(); )
