/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/sparse_strips/CoverageTestUtils.h"

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkString.h"
#include "src/core/SkCubics.h"
#include "src/core/SkQuads.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace skgpu::graphite {

void ScanlineOracle8x::IntersectLine(
        const SkPoint pts[2], skvx::float8 targetY, std::vector<CurveIntersection>* out) {
    float dy = pts[1].fY - pts[0].fY;
    if (dy != 0.0f) {
        float invDy = 1.0f / dy;
        float dx = pts[1].fX - pts[0].fX;
        int32_t dir = dy > 0.0f ? 1 : -1;

        skvx::float8 t = (targetY - pts[0].fY) * invDy;
        auto valid = (t >= 0.0f) & (t < 1.0f);
        if (any(valid)) {
            CurveIntersection hit;
            hit.x = pts[0].fX + t * dx;
            hit.dir = if_then_else(valid, skvx::int8(dir), skvx::int8(0));
            out->push_back(hit);
        }
    }
}

void ScanlineOracle8x::IntersectQuad(
        const SkPoint pts[3], skvx::float8 targetY, std::vector<CurveIntersection>* out) {
    double Ay = pts[2].fY - 2.0 * pts[1].fY + pts[0].fY;
    double By = 2.0 * (pts[1].fY - pts[0].fY);
    double Cy = pts[0].fY;

    double Ax = pts[2].fX - 2.0 * pts[1].fX + pts[0].fX;
    double Bx = 2.0 * (pts[1].fX - pts[0].fX);
    double Cx = pts[0].fX;

    CurveIntersection hits[2];
    for (int r = 0; r < 2; ++r) {
        hits[r].x = 0.0f;
        hits[r].dir = 0;
    }

    for (int k = 0; k < 8; ++k) {
        double roots[2];
        int numRoots = SkQuads::RootsReal(Ay, By, Cy - targetY[k], roots);
        int validCount = 0;
        for (int i = 0; i < numRoots; ++i) {
            double t = roots[i];
            if (t >= 0.0 && t < 1.0) {
                float derivativeY = static_cast<float>(2.0 * Ay * t + By);
                if (std::abs(derivativeY) > 1e-5f) {
                    float x = static_cast<float>(Ax * t * t + Bx * t + Cx);
                    int32_t dir = derivativeY > 0.0f ? 1 : -1;
                    if (validCount < 2) {
                        hits[validCount].x[k] = x;
                        hits[validCount].dir[k] = dir;
                        validCount++;
                    }
                }
            }
        }
    }

    for (int r = 0; r < 2; ++r) {
        if (any(hits[r].dir != 0)) {
            out->push_back(hits[r]);
        }
    }
}

void ScanlineOracle8x::IntersectConic(
        const SkPoint pts[3], float weight, skvx::float8 targetY, std::vector<CurveIntersection>* out) {
    CurveIntersection hits[2];
    for (int r = 0; r < 2; ++r) {
        hits[r].x = 0.0f;
        hits[r].dir = 0;
    }

    for (int k = 0; k < 8; ++k) {
        double d0 = pts[0].fY - targetY[k];
        double d1 = pts[1].fY - targetY[k];
        double d2 = pts[2].fY - targetY[k];

        double Ay = d2 - 2.0 * weight * d1 + d0;
        double By = 2.0 * (weight * d1 - d0);
        double Cy = d0;

        double roots[2];
        int numRoots = SkQuads::RootsReal(Ay, By, Cy, roots);
        int validCount = 0;
        for (int i = 0; i < numRoots; ++i) {
            double t = roots[i];
            if (t >= 0.0 && t < 1.0) {
                double denom = (1.0 - t) * (1.0 - t) + 2.0 * weight * t * (1.0 - t) + t * t;
                if (denom != 0.0) {
                    double numX = (1.0 - t) * (1.0 - t) * pts[0].fX +
                                   2.0 * weight * t * (1.0 - t) * pts[1].fX +
                                   t * t * pts[2].fX;
                    float x = static_cast<float>(numX / denom);

                    // denom is always positive, so we only need the numerator to find the sign
                    float derivativeY = static_cast<float>(2.0 * Ay * t + By);
                    if (std::abs(derivativeY) > 1e-5f) {
                        int32_t dir = derivativeY > 0.0f ? 1 : -1;
                        if (validCount < 2) {
                            hits[validCount].x[k] = x;
                            hits[validCount].dir[k] = dir;
                            validCount++;
                        }
                    }
                }
            }
        }
    }

    for (int r = 0; r < 2; ++r) {
        if (any(hits[r].dir != 0)) {
            out->push_back(hits[r]);
        }
    }
}

void ScanlineOracle8x::IntersectCubic(
        const SkPoint pts[4], skvx::float8 targetY, std::vector<CurveIntersection>* out) {
    double Ay = pts[3].fY - 3.0 * pts[2].fY + 3.0 * pts[1].fY - pts[0].fY;
    double By = 3.0 * (pts[2].fY - 2.0 * pts[1].fY + pts[0].fY);
    double Cy = 3.0 * (pts[1].fY - pts[0].fY);
    double Dy = pts[0].fY;

    double Ax = pts[3].fX - 3.0 * pts[2].fX + 3.0 * pts[1].fX - pts[0].fX;
    double Bx = 3.0 * (pts[2].fX - 2.0 * pts[1].fX + pts[0].fX);
    double Cx = 3.0 * (pts[1].fX - pts[0].fX);
    double Dx = pts[0].fX;

    CurveIntersection hits[3];
    for (int r = 0; r < 3; ++r) {
        hits[r].x = 0.0f;
        hits[r].dir = 0;
    }

    for (int k = 0; k < 8; ++k) {
        double roots[3];
        int numRoots = SkCubics::RootsValidT(Ay, By, Cy, Dy - targetY[k], roots);
        int validCount = 0;
        for (int i = 0; i < numRoots; ++i) {
            double t = roots[i];
            if (t >= 0.0 && t < 1.0) {
                float x = static_cast<float>(SkCubics::EvalAt(Ax, Bx, Cx, Dx, t));
                float derivativeY = static_cast<float>(3.0 * Ay * t * t + 2.0 * By * t + Cy);
                if (std::abs(derivativeY) > 1e-5f) {
                    int32_t dir = derivativeY > 0.0f ? 1 : -1;
                    if (validCount < 3) {
                        hits[validCount].x[k] = x;
                        hits[validCount].dir[k] = dir;
                        validCount++;
                    }
                }
            }
        }
    }

    for (int r = 0; r < 3; ++r) {
        if (any(hits[r].dir != 0)) {
            out->push_back(hits[r]);
        }
    }
}

void ScanlineOracle8x::CollectPathIntersections(
        const SkPath& path, skvx::float8 targetY, std::vector<CurveIntersection>* out) {
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;

    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                IntersectLine(pts, targetY, out);
                break;
            case SkPath::kQuad_Verb:
                IntersectQuad(pts, targetY, out);
                break;
            case SkPath::kConic_Verb:
                IntersectConic(pts, iter.conicWeight(), targetY, out);
                break;
            case SkPath::kCubic_Verb:
                IntersectCubic(pts, targetY, out);
                break;
            default:
                break;
        }
    }
}

skvx::int8 ScanlineOracle8x::EvaluatePixelWinding(
        float px, const std::vector<CurveIntersection>& hits) {
    static const skvx::float8 kSubX =
            (skvx::cast<float>(skvx::byte8::Load(kMsaaPattern<uint8_t>.data())) + 0.5f) / 8.0f;
    skvx::float8 vPx = px + kSubX;
    skvx::int8 pixelWinding(0);

    for (const auto& hit : hits) {
        pixelWinding += if_then_else(vPx < hit.x, hit.dir, skvx::int8(0));
    }
    return pixelWinding;
}

std::vector<ScanlineOracle8x::RowWindingInterval> ScanlineOracle8x::BuildRowIntervals(
        int py, const SkPath& path) {
    static const skvx::float8 kSubY =
            (skvx::float8{0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f} + 0.5f) / 8.0f;
    skvx::float8 vPy = static_cast<float>(py) + kSubY;

    std::vector<CurveIntersection> hits;
    CollectPathIntersections(path, vPy, &hits);
    if (hits.empty()) {
        return {};
    }

    SkRect bounds = path.getBounds();
    int minX = static_cast<int>(std::floor(bounds.fLeft));
    int maxX = static_cast<int>(std::ceil(bounds.fRight));

    std::vector<RowWindingInterval> intervals;
    skvx::int8 lastWinding(0);
    bool hasLast = false;

    for (int px = minX; px <= maxX; ++px) {
        skvx::int8 w = EvaluatePixelWinding(static_cast<float>(px), hits);
        if (!hasLast || any(w != lastWinding)) {
            intervals.push_back({px, w});
            lastWinding = w;
            hasLast = true;
        }
    }

    // Append terminator at maxX + 1 to define the upper bound of the final span [x_k, maxX + 1)
    intervals.push_back({maxX + 1, skvx::int8(0)});

    return intervals;
}

skvx::int8 ScanlineOracle8x::Evaluate(SkPoint pt, const SkPath& path) {
    static const skvx::float8 kSubY =
            (skvx::float8{0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f} + 0.5f) / 8.0f;
    skvx::float8 vPy = pt.fY + kSubY;

    std::vector<CurveIntersection> hits;
    CollectPathIntersections(path, vPy, &hits);

    return EvaluatePixelWinding(pt.fX, hits);
}

}  // namespace skgpu::graphite
