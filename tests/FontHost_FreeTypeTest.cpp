/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/ports/SkFontHost_FreeType_common.h"
#include "tests/Test.h"

#include <optional>

using OutlineShift = SkScalerContextFTUtils::OutlineShift;

// Checks the ordinary case: the shift applies the subpixel offset and moves the floored
// left/bottom edge to the bitmap origin, so the translated minimum lands in [0, 64)
DEF_TEST(FreeType_ComputeOutlineShift_Normal, reporter) {
    std::optional<OutlineShift> shift =
            SkScalerContextFTUtils::ComputeOutlineShift(-100, -200, 500, 700, 10, -20);
    REPORTER_ASSERT(reporter, shift.has_value());
    if (shift) {
        REPORTER_ASSERT(reporter, shift->fX == 138, "%ld", static_cast<long>(shift->fX));
        REPORTER_ASSERT(reporter, shift->fY == 236, "%ld", static_cast<long>(shift->fY));
    }
}

// Checks that a control box whose ends both fit in 32 bits, but whose extent does not, is
// rejected: shifting it to the origin leaves the 32-bit range where FT_Pos is 32 bits wide
DEF_TEST(FreeType_ComputeOutlineShift_HugeExtent, reporter) {
    std::optional<OutlineShift> shift = SkScalerContextFTUtils::ComputeOutlineShift(
            0, -2130706496, 640, 2147483584, 0, 0);
    REPORTER_ASSERT(reporter, !shift.has_value());
}

// Checks the edge of the accepted range: FT_Outline_Render rejects control boxes beyond
// 0x1000000, so the translated outline must stay within it on every platform
DEF_TEST(FreeType_ComputeOutlineShift_RenderLimit, reporter) {
    std::optional<OutlineShift> atLimit =
            SkScalerContextFTUtils::ComputeOutlineShift(0, 0, 0x1000000, 64, 0, 0);
    REPORTER_ASSERT(reporter, atLimit.has_value());
    if (atLimit) {
        REPORTER_ASSERT(reporter, atLimit->fX == 0 && atLimit->fY == 0);
    }

    std::optional<OutlineShift> pastLimit =
            SkScalerContextFTUtils::ComputeOutlineShift(0, 0, 0x1000001, 64, 0, 0);
    REPORTER_ASSERT(reporter, !pastLimit.has_value());
}
