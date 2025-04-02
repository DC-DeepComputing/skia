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

#include <cstddef>

DEF_TEST(SkEdge_CWLineWithoutClip_PublicMembersMatchMathematics, reporter) {
    SkEdge e;
    e.setLine({0, 4}, {10, 20});

    REPORTER_ASSERT(reporter, e.fWinding == SkEdge::Winding::kCW);
    // The slope of X with respect to Y of this line segment is (10 - 0) / (20 - 4) or 0.625
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
    // The slope of X with respect to Y of this line segment is (10 - 0) / (20 - 4) or 0.625
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
    // The slope of X with respect to Y of this line segment is (10 - 0) / (-20 - -4) or -0.625
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


DEF_TEST(SkEdge_Quad_ApproximatedWithMultipleLineSegments, reporter) {
    SkQuadraticEdge e;
    const SkPoint points[3] = {{0.f, 0.f}, {2.5f, 15.f}, {10.f, 20.f}};

    e.setQuadratic(points);

    REPORTER_ASSERT(reporter, e.fWinding == SkEdge::Winding::kCW);
    // SkQuadraticEdge will approximate a quadratic Bezier curve with N line segments.
    // There are some heuristics to determine how many segments to use. For this line, there are
    // 4 total segments - the current segment followed by 3 more.
    REPORTER_ASSERT(reporter, e.segmentsLeft() == 3);
    const float kSegmentWidth = 1/4.0f;  // each segment is 0.25 units (of t) wide
    // A quadratic Bezier curve can be written in polynomial form as
    //   Q(t) = At^2 + Bt + C
    // Where A = p_0 - 2p_1 + p_2, B = 2(p_1 - p_0), C = p_0
    // and p_0 is the start point, p_2 is the end point and p_1 is the control point.
    // With some derivatives and algebra, the slope of X with respect to Y is
    // ((2x_0 - 4x_1 + 2x_2)t + (2x_1 - 2x_0)) / (2y_0 - 4y_1 + 2y_2)t + (2y_1 - 2y_0)
    //
    // The starting value for t is the midpoint of the first segment or (1/N) / 2.
    // Evaluating the above formula for t = 1/8 yields the following
    const float kSegment1DxDt = 6.25f;
    const float kSegment1DyDt = 27.5f;
    const float kSegment1Slope = kSegment1DxDt / kSegment1DyDt;  // about 0.22727

    REPORTER_ASSERT(reporter, e.fDX == SkFloatToFixed(kSegment1Slope));
    // The first Y coordinate is 0.0, so trace the straight line down to that coordinate
    const float kSegment1NextHalfPixel = 0.5f;
    const float kSegment1StartXPrecise =
        0.0f + (kSegment1NextHalfPixel * kSegment1Slope);  // about 0.11364
    // The math is currently done internally in FDot6, so there's some precision lost.
    const SkFDot6 kSegment1StartX = SkFloatToFDot6(kSegment1StartXPrecise);  // 0.109375
    REPORTER_ASSERT(reporter, e.fX == SkFDot6ToFixed(kSegment1StartX),
                    "%g != %g", SkFixedToFloat(e.fX), SkFDot6ToFloat(kSegment1StartX));

    REPORTER_ASSERT(reporter, e.fFirstY == 0);
    // Our segment covers 1/N (in units of t), so to get the bottom Y value covered by this range
    // we multiply DyDt by 1/4. After converting to an int, we subtract 1 because there's an
    // implicit + 0.5 on fLastY and fLastY is inclusive.
    const float kSegment1EndYPrecise = 0.0f + kSegment1DyDt * kSegmentWidth;
    const int kSegment1LastY = sk_float_round2int(kSegment1EndYPrecise) - 1; // 6
    REPORTER_ASSERT(reporter, e.fLastY == kSegment1LastY);


    e.nextSegment();
    REPORTER_ASSERT(reporter, e.segmentsLeft() == 2);
    REPORTER_ASSERT(reporter, e.fWinding == SkEdge::Winding::kCW);

    // The second segment covers the range of t from [0.25, 0.5], picking up where the previous
    // segment ended.
    // DxDt and DyDt are computed using the above formula and t = 3/8 (1/8 + 1/4)
    const float kSegment2DxDt = 8.75f;
    const float kSegment2DyDt = 22.5f;
    const float kSegment2Slope = kSegment2DxDt / kSegment2DyDt;  // about 0.38888
    REPORTER_ASSERT(reporter, e.fDX == SkFloatToFixed(kSegment2Slope));

    // The next segment builds off the ends of the previous segments. Advance x by 1/4 t
    const float kSegment1EndXPrecise = 0 + kSegment1DxDt * kSegmentWidth;

    // Then add an amount to get to the next half pixel in the y direction
    const float kSegment2NextHalfPixel =
        sk_float_round2int(kSegment1EndYPrecise) + 0.5f - kSegment1EndYPrecise; // 0.625
    const float kSegment2StartXPrecise =
        kSegment1EndXPrecise + (kSegment2NextHalfPixel * kSegment2Slope);  // about 1.80556

    const SkFDot6 kSegment2StartX = SkFloatToFDot6(kSegment2StartXPrecise);  // 1.79688
    REPORTER_ASSERT(reporter, e.fX == SkFDot6ToFixed(kSegment2StartX),
                    "%g != %g", SkFixedToFloat(e.fX), SkFDot6ToFloat(kSegment2StartX));

    // The next segment picks up on the pixel after the last segment.
    REPORTER_ASSERT(reporter, e.fFirstY == kSegment1LastY + 1);
    const float kSegment2EndYPrecise = kSegment1EndYPrecise + kSegment2DyDt * kSegmentWidth;
    const int kSegment2LastY = sk_float_round2int(kSegment2EndYPrecise) - 1; // 12
    REPORTER_ASSERT(reporter, e.fLastY == kSegment2LastY);


    e.nextSegment();
    REPORTER_ASSERT(reporter, e.segmentsLeft() == 1);
    REPORTER_ASSERT(reporter, e.fWinding == SkEdge::Winding::kCW);
    // math omitted on remaining segments for brevity, but it follows the same pattern.


    e.nextSegment();
    REPORTER_ASSERT(reporter, !e.hasNextSegment());
    REPORTER_ASSERT(reporter, e.fWinding == SkEdge::Winding::kCW);
    // we should go down to 19.5 to cover our curve going to 20
    REPORTER_ASSERT(reporter, e.fLastY == 19);
}
