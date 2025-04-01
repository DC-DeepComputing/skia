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

DEF_TEST(SkEdge_Quad, reporter) {
    SkQuadraticEdge q;
    const SkPoint points[3] = {{0, 0}, {2.5, 15}, {10, 20}};
    q.setQuadratic(points);
    q.dump();

    int count = q.fLastY - q.fFirstY;
    SkASSERT(count >= 0);
    SkFixed newX = q.fX + q.fDX * count;
    q.fX = newX;
    q.updateQuadratic();
    q.dump();

}
