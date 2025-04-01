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

DEF_TEST(SkEdge_Line_NoClip, reporter) {
    SkEdge e;
    e.setLine({0, 3}, {10, 20});

    REPORTER_ASSERT(reporter, e.fWinding == SkEdge::Winding::kCW);
    REPORTER_ASSERT(reporter, e.fX == SkFloatToFixed(0));
    REPORTER_ASSERT(reporter, e.fFirstY == 3);
    REPORTER_ASSERT(reporter, e.fLastY == 20);

    // The slope of this line segment is (20 - 3) / (10 - 0) or 1.7.
    REPORTER_ASSERT(reporter, e.fDX == SkFloatToFixed(1.7f));

    REPORTER_ASSERT(reporter, !e.hasNextSegment());
}

// DEF_TEST(SkEdge_Quad, reporter) {
//     SkQuadraticEdge q;
//     const SkPoint points[3] = {{0, 0}, {2.5, 15}, {10, 20}};
//     q.setQuadratic(points);
//     q.dump();

//     int count = q.fLastY - q.fFirstY;
//     SkASSERT(count >= 0);
//     SkFixed newX = q.fX + q.fDX * count;
//     q.fX = newX;
//     q.nextSegment();
//     q.dump();

// }
