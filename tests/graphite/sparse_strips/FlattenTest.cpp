/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkPathBuilder.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"

#include <cmath>
#include <string>
#include <vector>

namespace skgpu::graphite {

namespace {

SkPoint evalQuad(const SkPoint pts[3], float t) {
    float mt = 1.0f - t;
    return pts[0] * (mt * mt) + pts[1] * (2.0f * mt * t) + pts[2] * (t * t);
}

SkPoint evalCubic(const SkPoint pts[4], float t) {
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float t2 = t * t;
    return pts[0] * (mt2 * mt) + pts[1] * (3.0f * mt2 * t) + pts[2] * (3.0f * mt * t2) +
           pts[3] * (t2 * t);
}

float distSqPointToSegment(SkPoint p, SkPoint a, SkPoint b) {
    SkPoint ab = b - a;
    SkPoint ap = p - a;
    float lenSq = SkPoint::DotProduct(ab, ab);
    float t = (lenSq == 0.0f) ? 0.0f : SkPoint::DotProduct(ap, ab) / lenSq;
    t = std::clamp(t, 0.0f, 1.0f);
    SkPoint proj = a + ab * t;
    SkPoint d = p - proj;
    return SkPoint::DotProduct(d, d);
}

float distSqPointToPath(SkPoint pt, const SkPath& path) {
    float minDistSq = 1e10f;
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;

    const int kSamples = 100;
    const float dt = 1.0f / kSamples;

    SkPoint lastPt = {0, 0};
    SkPoint contourStart = {0, 0};
    bool hasContour = false;

    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                if (hasContour && lastPt != contourStart) {
                    minDistSq = std::min(minDistSq, distSqPointToSegment(pt, lastPt, contourStart));
                }
                contourStart = pts[0];
                lastPt = pts[0];
                hasContour = true;
                break;
            case SkPath::kLine_Verb:
                minDistSq = std::min(minDistSq, distSqPointToSegment(pt, pts[0], pts[1]));
                lastPt = pts[1];
                break;
            case SkPath::kQuad_Verb: {
                SkPoint p0 = pts[0];
                for (int i = 1; i <= kSamples; ++i) {
                    SkPoint p1 = evalQuad(pts, i * dt);
                    minDistSq = std::min(minDistSq, distSqPointToSegment(pt, p0, p1));
                    p0 = p1;
                }
                lastPt = pts[2];
                break;
            }
            case SkPath::kCubic_Verb: {
                SkPoint p0 = pts[0];
                for (int i = 1; i <= kSamples; ++i) {
                    SkPoint p1 = evalCubic(pts, i * dt);
                    minDistSq = std::min(minDistSq, distSqPointToSegment(pt, p0, p1));
                    p0 = p1;
                }
                lastPt = pts[3];
                break;
            }
            case SkPath::kClose_Verb:
                minDistSq = std::min(minDistSq, distSqPointToSegment(pt, lastPt, contourStart));
                lastPt = contourStart;
                hasContour = false;  // Mark as safely closed
                break;
            default:
                break;
        }
    }

    // Check the implicit close for the very last contour in the path
    if (hasContour && lastPt != contourStart) {
        minDistSq = std::min(minDistSq, distSqPointToSegment(pt, lastPt, contourStart));
    }

    return minDistSq;
}

template <FlattenMode Mode>
void checkFlattenedPath(skiatest::Reporter* reporter,
                        const SkPath& path,
                        float tolerance,
                        const char* testName) {
    FlattenCtx ctx;
    Polyline polyline;

    const float kViewDim = 10000.0f;
    Flatten::processPaths<Mode>(path, kViewDim, kViewDim, ctx, polyline);

    if (path.isEmpty()) {
        REPORTER_ASSERT(reporter, polyline.empty(), "[%s] Expected empty output", testName);
        return;
    }

    // Watertightness check, should be guaranteed by deduplication but check anyways.
    const SkTDArray<SkPoint>& rawPts = polyline.points();
    SkPoint contourStart = {0, 0};
    SkPoint lastPt = {0, 0};
    bool inContour = false;

    for (int i = 0; i < rawPts.size(); ++i) {
        if (std::isnan(rawPts[i].fX)) {
            if (inContour) {
                REPORTER_ASSERT(
                        reporter,
                        lastPt == contourStart,
                        "[%s] Contour did not close tightly. End: (%f, %f), Start: (%f, %f)",
                        testName,
                        lastPt.fX,
                        lastPt.fY,
                        contourStart.fX,
                        contourStart.fY);
            }
            inContour = false;
        } else {
            if (!inContour) {
                contourStart = rawPts[i];
                inContour = true;
            }
            lastPt = rawPts[i];
        }
    }

    // Tolerance check
    const float tolSq = tolerance * tolerance;
    const float allowedTolSq = tolSq + 0.0001f;
    const int kLineSamples = 4;
    const float lineDt = 1.0f / kLineSamples;

    for (auto [edge, index] : polyline) {
        for (int j = 0; j <= kLineSamples; ++j) {
            float t = j * lineDt;
            SkPoint samplePt = edge.p0 * (1.0f - t) + edge.p1 * t;
            float minDistSq = distSqPointToPath(samplePt, path);

            REPORTER_ASSERT(reporter,
                            minDistSq <= allowedTolSq,
                            "[%s] Interpolated line point (%f, %f) deviated from true curve by %f "
                            "(tol: %f)",
                            testName,
                            samplePt.fX,
                            samplePt.fY,
                            std::sqrt(minDistSq),
                            tolerance);
        }
    }
}

}  // namespace

template <FlattenMode Mode> class FlattenTestRunner {
public:
    static void runAll(skiatest::Reporter* reporter) {
        const float tol = Flatten::kTol;

        {
            SkPath emptyPath;
            checkFlattenedPath<Mode>(reporter, emptyPath, tol, "EmptyPath");
        }

        {
            SkPathBuilder builder;
            builder.moveTo(10, 10);
            builder.lineTo(50, 50);
            builder.lineTo(90, 10);
            checkFlattenedPath<Mode>(reporter, builder.detach(), tol, "SimpleLines");
        }

        {
            SkPathBuilder builder;
            builder.moveTo(10, 10);
            builder.quadTo(50, 100, 90, 10);
            checkFlattenedPath<Mode>(reporter, builder.detach(), tol, "SimpleQuad");
        }

        {
            SkPathBuilder builder;
            builder.moveTo(10, 10);
            builder.cubicTo(30, 100, 70, 100, 90, 10);
            checkFlattenedPath<Mode>(reporter, builder.detach(), tol, "SimpleCubic");
        }

        {
            SkPathBuilder builder;
            builder.moveTo(0, 0);
            builder.quadTo(50, 100, 100, 0);
            builder.close();
            checkFlattenedPath<Mode>(reporter, builder.detach(), tol, "ClosedQuad");
        }

        {
            SkPathBuilder builder;
            builder.moveTo(10, 10);
            builder.cubicTo(100, 10, 10, 100, 100, 100);
            checkFlattenedPath<Mode>(reporter, builder.detach(), tol, "SCurveCubic");
        }

        {
            SkPathBuilder builder;
            builder.moveTo(10, 10);
            builder.quadTo(10, 10, 10, 10);
            builder.cubicTo(10, 10, 10, 10, 10, 10);
            checkFlattenedPath<Mode>(reporter, builder.detach(), tol, "DegenerateCurves");
        }

        {
            SkPathBuilder builder;
            builder.moveTo(10, 10);
            builder.cubicTo(40, 10, 70, 10, 100, 10);
            checkFlattenedPath<Mode>(reporter, builder.detach(), tol, "CollinearCubic");
        }

        {
            SkPathBuilder builder;
            builder.moveTo(50, 50);
            builder.cubicTo(150, -50, 150, 150, 50, 50);
            checkFlattenedPath<Mode>(reporter, builder.detach(), tol, "LoopCubic");
        }

        {
            SkPathBuilder builder;
            builder.moveTo(10, 10);
            builder.lineTo(20, 20);

            builder.moveTo(30, 30);
            builder.quadTo(50, 80, 70, 30);
            builder.close();

            builder.moveTo(100, 100);
            builder.cubicTo(120, 150, 180, 150, 200, 100);
            checkFlattenedPath<Mode>(reporter, builder.detach(), tol, "MultiContour");
        }
    }
};

DEF_TEST(GraphiteFlatten_Scalar, reporter) {
    skgpu::graphite::FlattenTestRunner<FlattenMode::kScalar>::runAll(reporter);
}

DEF_TEST(GraphiteFlatten_Simd, reporter) {
    skgpu::graphite::FlattenTestRunner<FlattenMode::kSimd>::runAll(reporter);
}

}  // namespace skgpu::graphite
