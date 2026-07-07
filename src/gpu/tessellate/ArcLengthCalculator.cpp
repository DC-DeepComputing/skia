/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/tessellate/ArcLengthCalculator.h"
#include "src/gpu/tessellate/Tessellation.h"

namespace skgpu::tess {

float ArcLengthCalculator::ConicLength(skvx::float2 p0, skvx::float2 p1, skvx::float2 p2, float w) {
    float a = skvx::length(p1 - p0);
    float b = skvx::length(p2 - p1);
    if (a < b) {
        std::swap(a, b);
    }
    if (b == 0.0f) {
        return 4.0f * a;
    }
    float a_n = 1.0f;
    float b_n = b / a;
    float sum = 0.5f * (1.0f - b_n * b_n);

    // Pass 1
    float a1 = (a_n + b_n) * 0.5f;
    float b1 = std::sqrt(a_n * b_n);
    float c1 = (a_n - b_n) * 0.5f;
    sum += 1.0f * (c1 * c1);

    // Pass 2
    float a2 = (a1 + b1) * 0.5f;
    float b2 = std::sqrt(a1 * b1);
    float c2 = (a1 - b1) * 0.5f;
    sum += 2.0f * (c2 * c2);

    // Pass 3
    float c3 = (a2 - b2) * 0.5f;
    sum += 4.0f * (c3 * c3);

    return 6.283185307f * a * (1.0f - sum);
}

float ArcLengthCalculator::CubicLength(skvx::float2 c0, skvx::float2 c1, skvx::float2 c2, skvx::float2 c3) {
    skvx::float2 d1 = c1 - c0;
    skvx::float2 d2 = c2 - c0;
    skvx::float2 d3 = c3 - c0;
    float v0 = skvx::length(d1) * 0.15f;
    float v1 = skvx::length(d1 * 0.325650248872424f + d2 * 0.208983582205757f + d3 * 0.024349751127576f);
    float v2 = skvx::length(d3 + d2 - d1) * 0.26666666666666666f;
    float v3 = skvx::length(d1 * -0.208983582205757f - d2 * 0.325650248872424f + d3 * 0.558983582205757f);
    float v4 = skvx::length(d3 - d2) * 0.15f;
    return v0 + v1 + v2 + v3 + v4;
}

float ArcLengthCalculator::PathTotalLength(const SkPath& path, SkPaint::Cap cap) {
    class Precalculator {
    public:
        Precalculator() : fAccumulatedLength(0.f) {}

        void writeDeferredStrokePatch(SkPoint, SkPaint::Cap) {}
        void closeDeferredStrokePatch(SkPaint::Cap) {}

        void writeLine(SkPoint p0, SkPoint p1) {
            fAccumulatedLength += ArcLengthCalculator::LineLength({p0.fX, p0.fY}, {p1.fX, p1.fY});
        }
        void writeCircle(SkPoint) {}
        void writeQuadratic(const SkPoint pts[3]) {
            fAccumulatedLength += ArcLengthCalculator::ConicLength({pts[0].fX, pts[0].fY},
                                                                   {pts[1].fX, pts[1].fY},
                                                                   {pts[2].fX, pts[2].fY},
                                                                   1.f);
        }
        void writeConic(const SkPoint pts[3], float w) {
            fAccumulatedLength += ArcLengthCalculator::ConicLength({pts[0].fX, pts[0].fY},
                                                                   {pts[1].fX, pts[1].fY},
                                                                   {pts[2].fX, pts[2].fY},
                                                                   w);
        }
        void writeCubic(const SkPoint pts[4]) {
            fAccumulatedLength += ArcLengthCalculator::CubicLength({pts[0].fX, pts[0].fY},
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
                if (ConicHasCusp(pts)) {
                    SkPoint cusp = SkEvalQuadAt(pts, SkFindQuadMidTangent(pts));
                    precalculator.writeCircle(cusp);
                    precalculator.writeLine(pts[0], cusp);
                    precalculator.writeLine(cusp, pts[2]);
                } else {
                    precalculator.writeQuadratic(pts);
                }
                break;

            case SkPathVerb::kConic:
                if (ConicHasCusp(pts)) {
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
                int numChops = FindCubicConvex180Chops(pts, T, &areCusps);
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

} // namespace skgpu::tess
