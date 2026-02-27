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

}  // namespace

template <FlattenMode kMode> class FlattenTestRunner {
    static float distSqPointToPath(SkPoint pt, const SkPath& path, float earlyOutTolSq) {
        float minDistSq = 1e10f;
        SkPath::Iter iter(path, false);
        SkPoint pts[4];
        SkPath::Verb verb;

        const int kSamples = 100;
        const float dt = 1.0f / kSamples;

        SkPoint lastPt = {0, 0};
        SkPoint contourStart = {0, 0};
        bool hasContour = false;

        // Lambda to deduplicate the check and early return logic
        auto checkSegment = [&](SkPoint p0, SkPoint p1) {
            minDistSq = std::min(minDistSq, Flatten::distSqToSegment(pt, p0, p1));
            return minDistSq <= earlyOutTolSq;
        };

        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                    if (hasContour && lastPt != contourStart) {
                        if (checkSegment(lastPt, contourStart)) return minDistSq;
                    }
                    contourStart = pts[0];
                    lastPt = pts[0];
                    hasContour = true;
                    break;
                case SkPath::kLine_Verb:
                    if (checkSegment(pts[0], pts[1])) return minDistSq;
                    lastPt = pts[1];
                    break;
                case SkPath::kQuad_Verb: {
                    SkPoint p0 = pts[0];
                    for (int i = 1; i <= kSamples; ++i) {
                        SkPoint p1 = evalQuad(pts, i * dt);
                        if (checkSegment(p0, p1)) return minDistSq;
                        p0 = p1;
                    }
                    lastPt = pts[2];
                    break;
                }
                case SkPath::kCubic_Verb: {
                    SkPoint p0 = pts[0];
                    for (int i = 1; i <= kSamples; ++i) {
                        SkPoint p1 = evalCubic(pts, i * dt);
                        if (checkSegment(p0, p1)) return minDistSq;
                        p0 = p1;
                    }
                    lastPt = pts[3];
                    break;
                }
                case SkPath::kClose_Verb:
                    if (checkSegment(lastPt, contourStart)) return minDistSq;
                    lastPt = contourStart;
                    hasContour = false;  // Mark as safely closed
                    break;
                default:
                    break;
            }
        }

        // Check the implicit close for the very last contour in the path
        if (hasContour && lastPt != contourStart) {
            checkSegment(lastPt, contourStart);
        }

        return minDistSq;
    }

    static void checkFlattenedPath(skiatest::Reporter* reporter,
                                   const SkPath& path,
                                   float tolerance,
                                   const char* testName) {
        Flatten flattener;
        Polyline polyline;
        const float kViewDim = 10000.0f;
        flattener.processPaths<kMode>(path, kViewDim, kViewDim, polyline);

        if (path.isEmpty()) {
            REPORTER_ASSERT(reporter, polyline.empty(), "[%s] Expected empty output", testName);
            return;
        }

        // Watertightness check
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
        const float allowedTolSq = Flatten::kTol2 + 0.0001f;
        const int kLineSamples = 4;
        const float lineDt = 1.0f / kLineSamples;

        for (auto [edge, index] : polyline) {
            for (int j = 0; j <= kLineSamples; ++j) {
                float t = j * lineDt;
                SkPoint samplePt = edge.p0 * (1.0f - t) + edge.p1 * t;
                float minDistSq = distSqPointToPath(samplePt, path, allowedTolSq);

                REPORTER_ASSERT(
                        reporter,
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

    static void testCulling(skiatest::Reporter* reporter) {
        const float kViewW = 100.0f;
        const float kViewH = 100.0f;

        auto checkCull = [&](const SkPath& path, bool expectCulled, const char* testName) {
            Flatten flattener;
            Polyline polyline;
            flattener.processPaths<kMode>(path, kViewW, kViewH, polyline);

            // TODO (thomsmit): check that the simplified produces the correct equivalent winding
            // A single contour that is culled should output exactly 4 points:
            // 1. Sentinel (NaN)
            // 2. MoveTo (start point)
            // 3. End point of the curve
            // 4. Implicit close back to the start point
            const int kCulledPointSize = 4;

            if (expectCulled) {
                REPORTER_ASSERT(
                        reporter,
                        polyline.points().size() == kCulledPointSize,
                        "[%s] Expected curve to be culled/simplified to 1 edge, got %d points",
                        testName,
                        polyline.points().size());
            } else {
                REPORTER_ASSERT(
                        reporter,
                        polyline.points().size() > kCulledPointSize,
                        "[%s] Expected curve to cross viewport and be subdivided, got %d points",
                        testName,
                        polyline.points().size());
            }
        };

        checkCull(SkPathBuilder().moveTo(10, -20).quadTo(50, -80, 90, -20).detach(),
                  true,
                  "Cull_Top");
        checkCull(SkPathBuilder().moveTo(10, 120).cubicTo(30, 180, 70, 180, 90, 120).detach(),
                  true,
                  "Cull_Bottom");
        checkCull(SkPathBuilder().moveTo(120, 10).quadTo(180, 50, 120, 90).detach(),
                  true,
                  "Cull_Right");
        checkCull(SkPathBuilder().moveTo(-20, -20).quadTo(120, -20, 120, 120).detach(),
                  false,
                  "Crosses_Viewport");
        checkCull(SkPathBuilder().moveTo(-50, 10).cubicTo(-20, 30, -20, 70, -50, 90).detach(),
                  true,
                  "Simplify_Left");
    }

public:
    static void runAll(skiatest::Reporter* reporter) {
        const float tol = Flatten::kTol;

        checkFlattenedPath(reporter, SkPath(), tol, "EmptyPath");

        checkFlattenedPath(reporter,
                           SkPathBuilder().moveTo(10, 10).lineTo(50, 50).lineTo(90, 10).detach(),
                           tol,
                           "SimpleLines");

        checkFlattenedPath(reporter,
                           SkPathBuilder().moveTo(10, 10).quadTo(50, 100, 90, 10).detach(),
                           tol,
                           "SimpleQuad");

        checkFlattenedPath(
                reporter,
                SkPathBuilder().moveTo(10, 10).cubicTo(30, 100, 70, 100, 90, 10).detach(),
                tol,
                "SimpleCubic");

        checkFlattenedPath(reporter,
                           SkPathBuilder().moveTo(0, 0).quadTo(50, 100, 100, 0).close().detach(),
                           tol,
                           "ClosedQuad");

        checkFlattenedPath(
                reporter,
                SkPathBuilder().moveTo(10, 10).cubicTo(100, 10, 10, 100, 100, 100).detach(),
                tol,
                "SCurveCubic");

        checkFlattenedPath(reporter,
                           SkPathBuilder()
                                   .moveTo(10, 10)
                                   .quadTo(10, 10, 10, 10)
                                   .cubicTo(10, 10, 10, 10, 10, 10)
                                   .detach(),
                           tol,
                           "DegenerateCurves");

        checkFlattenedPath(reporter,
                           SkPathBuilder().moveTo(10, 10).cubicTo(40, 10, 70, 10, 100, 10).detach(),
                           tol,
                           "CollinearCubic");

        checkFlattenedPath(
                reporter,
                SkPathBuilder().moveTo(50, 50).cubicTo(150, -50, 150, 150, 50, 50).detach(),
                tol,
                "LoopCubic");

        checkFlattenedPath(reporter,
                           SkPathBuilder()
                                   .moveTo(10, 10)
                                   .lineTo(20, 20)
                                   .moveTo(30, 30)
                                   .quadTo(50, 80, 70, 30)
                                   .close()
                                   .moveTo(100, 100)
                                   .cubicTo(120, 150, 180, 150, 200, 100)
                                   .detach(),
                           tol,
                           "MultiContour");

        testCulling(reporter);
    }
};

DEF_TEST(GraphiteFlatten_Scalar, reporter) {
    skgpu::graphite::FlattenTestRunner<FlattenMode::kScalar>::runAll(reporter);
}

DEF_TEST(GraphiteFlatten_Simd, reporter) {
    skgpu::graphite::FlattenTestRunner<FlattenMode::kSimd>::runAll(reporter);
}

}  // namespace skgpu::graphite
