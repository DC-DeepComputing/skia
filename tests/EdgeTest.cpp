/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "src/core/SkEdge.h"
#include "tests/Test.h"
#include "src/base/SkBezierCurves.h"

#include <cstddef>

DEF_TEST(SkEdge_CWLineWithoutClip_PublicMembersMatchMathematics, reporter) {
    SkEdge e;
    e.setLine({0, 4}, {10, 20});

    REPORTER_ASSERT(reporter, e.fWinding == SkEdge::Winding::kCW);
    // The derivative of X with respect to Y of this line segment is (10 - 0) / (20 - 4) or 0.625
    REPORTER_ASSERT(reporter, e.fDX == SkFloatToFixed(0.625f));
    // The Y coordinate conceptually starts at 0.5, so make sure the X coordinate matches is
    // 0.5 * 0.625 (the slope)
    REPORTER_ASSERT(reporter, e.fX == SkFloatToFixed(0.3125f));
    // Even though these are stored as integers, they conceptually represent the midpoint of
    // the line (4.5 and 19.5).
    REPORTER_ASSERT(reporter, e.fFirstY == 4);
    REPORTER_ASSERT(reporter, e.fLastY == 19);
    // Lines are "approximated" in one shot and have no need for follow-up approximations.
    REPORTER_ASSERT(reporter, !e.hasNextSegment());
}

DEF_TEST(SkEdge_CWLineNullClip_PublicMembersMatchMathematics, reporter) {
    SkEdge e;
    e.setLine({0, 4}, {10, 20}, nullptr);

    REPORTER_ASSERT(reporter, e.fWinding == SkEdge::Winding::kCW);
    // The derivative of X with respect to Y of this line segment is (10 - 0) / (20 - 4) or 0.625
    REPORTER_ASSERT(reporter, e.fDX == SkFloatToFixed(0.625f));
    // The Y coordinate conceptually starts at 0.5, so make sure the X coordinate matches is
    // 0.5 * 0.625 (the slope)
    REPORTER_ASSERT(reporter, e.fX == SkFloatToFixed(0.3125f));
    // Even though these are stored as integers, they conceptually represent the midpoint of
    // the line (4.5 and 19.5).
    REPORTER_ASSERT(reporter, e.fFirstY == 4);
    REPORTER_ASSERT(reporter, e.fLastY == 19);
    // Lines are "approximated" in one shot and have no need for follow-up approximations.
    REPORTER_ASSERT(reporter, !e.hasNextSegment());
}

DEF_TEST(SkEdge_CCWLineWithoutClip_PublicMembersMatchMathematics, reporter) {
    SkEdge e;
    e.setLine({0, -4}, {10, -20});
    REPORTER_ASSERT(reporter, e.fWinding == SkEdge::Winding::kCCW);
    // The derivative of X with respect to Y of this line segment is (10 - 0) / (-20 - -4) or -0.625
    REPORTER_ASSERT(reporter, e.fDX == SkFloatToFixed(-0.625f));
    // The Y coordinate conceptually starts at 0.5, so make sure the X coordinate matches is
    // 10 + 0.5 * -0.625 (the slope)
    REPORTER_ASSERT(reporter, e.fX == SkFloatToFixed(9.6875f));
    // Even though these are stored as integers, they conceptually represent the midpoint of
    // the line (-19.5 and -4.5).
    REPORTER_ASSERT(reporter, e.fFirstY == -20);
    REPORTER_ASSERT(reporter, e.fLastY == -5);
    REPORTER_ASSERT(reporter, !e.hasNextSegment());
}


DEF_TEST(SkEdge_Quad, reporter) {
    SkQuadraticEdge e;
    const SkPoint points[3] = {{0, 0}, {2.5, 15}, {10, 20}};

    auto getExpectedX = [&](float y) {
        float data[2];
        auto answers = SkBezierQuad::IntersectWithHorizontalLine({points, std::size(points)},
                                                                  y, data);
        SkASSERT(answers.size() == 1);
        return answers[0];
    };

    e.setQuadratic(points);
    e.dump();

    REPORTER_ASSERT(reporter, e.fWinding == SkEdge::Winding::kCW);
    REPORTER_ASSERT(reporter, e.fDX == SkFloatToFixed(0.388885));
    REPORTER_ASSERT(reporter, e.fX == SkFloatToFixed(getExpectedX(0.5)),
                    "%g != %g", SkFixedToFloat(e.fDX), getExpectedX(0.5));

    e.nextSegment();
    e.dump();
}
