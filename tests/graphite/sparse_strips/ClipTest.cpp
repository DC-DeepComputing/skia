/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/MakeStrip_impl.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/ClipTestUtils.h"

#include <array>
#include <cmath>

namespace skgpu::graphite {

template <uint16_t WIDTH, uint16_t HEIGHT> class ClipTestRunner {
    using TileType = Tile<WIDTH, HEIGHT>;

public:
    static void runAll(skiatest::Reporter* reporter) {
        auto cases = MakeClipTestCases<WIDTH, HEIGHT>();

        for (const auto& testCase : cases) {
            for (const auto& tile : testCase.expectedTiles) {
                uint32_t lineIdx =
                        tile.packedWindingLineIdx >> SparseStripsConstants::INT_MASK_SHIFT;

                if (lineIdx >= testCase.lines.size()) {
                    ERRORF(reporter,
                           "[%s] Setup error: Tile refs invalid line index %u",
                           testCase.name.c_str(),
                           lineIdx);
                    continue;
                }

                const Line& line = testCase.lines[lineIdx];

                float dx = line.p1.fX - line.p0.fX;
                float dy = line.p1.fY - line.p0.fY;

                bool isVertical = std::abs(dx) <= std::numeric_limits<float>::epsilon();
                bool isHorizontal = std::abs(dy) <= std::numeric_limits<float>::epsilon();

                float idx = isVertical ? 0.0f : 1.0f / dx;
                float idy = isHorizontal ? 0.0f : 1.0f / dy;

                float tileMinX = tile.x * static_cast<float>(WIDTH);
                float tileMinY = tile.y * static_cast<float>(HEIGHT);
                float tileMaxX = tileMinX + static_cast<float>(WIDTH);
                float tileMaxY = tileMinY + static_cast<float>(HEIGHT);

                std::array<float, 4> bounds = {tileMinX, tileMinY, tileMaxX, tileMaxY};
                std::array<float, 4> derivatives = {dx, dy, idx, idy};
                uint32_t mask =
                        tile.packedWindingLineIdx & SparseStripsConstants::INTERSECTION_MASK;

                bool canonicalXDir = (dx >= 0);
                bool canonicalYDir = (dy >= 0);

                std::array<SkPoint, 2> result = MakeStripImpl::clipToTile(
                        line, bounds, derivatives, mask, canonicalXDir, canonicalYDir);

                VerifyClipResult<WIDTH, HEIGHT>(reporter, testCase.name, line, tile, result);
            }
        }
    }
};

DEF_TEST(SparseStrips_Clip_4x4, reporter) { ClipTestRunner<4, 4>::runAll(reporter); }
DEF_TEST(SparseStrips_Clip_8x8, reporter) { ClipTestRunner<8, 8>::runAll(reporter); }

}  // namespace skgpu::graphite
