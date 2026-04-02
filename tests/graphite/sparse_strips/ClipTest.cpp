/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <vector>

namespace skgpu::graphite {

namespace {

std::vector<Line> scale_lines(const std::vector<Line>& input, float scale) {
    if (scale == 1.0f) return input;
    std::vector<Line> scaled;
    scaled.reserve(input.size());
    for (const auto& l : input) {
        scaled.push_back({{l.p0.fX * scale, l.p0.fY * scale}, {l.p1.fX * scale, l.p1.fY * scale}});
    }
    return scaled;
}

// LiangBarsky style as ground truth
std::optional<std::array<SkPoint, 2>> reference_clip(
        const Line& line, float minX, float minY, float maxX, float maxY) {
    float t0 = 0.0f;
    float t1 = 1.0f;
    float dx = line.p1.fX - line.p0.fX;
    float dy = line.p1.fY - line.p0.fY;

    float p[4] = {-dx, dx, -dy, dy};
    float q[4] = {line.p0.fX - minX, maxX - line.p0.fX, line.p0.fY - minY, maxY - line.p0.fY};

    for (int i = 0; i < 4; i++) {
        if (p[i] == 0) {
            if (q[i] < 0) return std::nullopt;  // Parallel and outside
        } else {
            float t = q[i] / p[i];
            if (p[i] < 0) {
                if (t > t1) return std::nullopt;
                if (t > t0) t0 = t;
            } else {
                if (t < t0) return std::nullopt;
                if (t < t1) t1 = t;
            }
        }
    }

    if (t0 > t1) return std::nullopt;

    SkPoint pEntry = {line.p0.fX + t0 * dx, line.p0.fY + t0 * dy};
    SkPoint pExit = {line.p0.fX + t1 * dx, line.p0.fY + t1 * dy};

    pEntry.fX -= minX;
    pEntry.fY -= minY;
    pExit.fX -= minX;
    pExit.fY -= minY;

    float width = maxX - minX;
    float height = maxY - minY;
    auto clamp_pt = [&](SkPoint& pt) {
        pt.fX = std::clamp(pt.fX, 0.0f, width);
        pt.fY = std::clamp(pt.fY, 0.0f, height);
    };
    clamp_pt(pEntry);
    clamp_pt(pExit);

    if (pExit.fY < pEntry.fY) return {{{pExit, pEntry}}};
    return {{{pEntry, pExit}}};
}
}  // namespace

template <uint16_t WIDTH, uint16_t HEIGHT>
class ClipTestRunner : IntersectionBits {
    using TileType = Tile<WIDTH, HEIGHT>;

    static constexpr float SCALE = (float)WIDTH / 4.0f;
    static constexpr float BASE_V_DIM = 100.0f;

    //static and not anonymous namespace function for ease of friending ClipTestRunner
    static void verifyClip(skiatest::Reporter* reporter,
                            const Line& line,
                            const TileType& tile,
                            const char* testName) {
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
        uint32_t mask = tile.packedWindingLineIdx & IntersectionBits::INTERSECTION_MASK;

        bool canonicalXDir = (dx >= 0);
        bool canonicalYDir = (dy >= 0);

        std::array<SkPoint, 2> result =
                Strip::clipToTile(line, bounds, derivatives, mask, canonicalXDir, canonicalYDir);

        auto lbOpt = reference_clip(line, tileMinX, tileMinY, tileMaxX, tileMaxY);
        float width = tileMaxX - tileMinX;
        float height = tileMaxY - tileMinY;

        SkPoint rawP0 = {line.p0.fX - tileMinX, line.p0.fY - tileMinY};
        SkPoint rawP1 = {line.p1.fX - tileMinX, line.p1.fY - tileMinY};

        auto clampPt = [&](SkPoint& pt) {
            pt.fX = std::clamp(pt.fX, 0.0f, width);
            pt.fY = std::clamp(pt.fY, 0.0f, height);
        };

        clampPt(rawP0);
        clampPt(rawP1);

        std::array<SkPoint, 2> fallbackExp;
        if (rawP1.fY < rawP0.fY) {
            fallbackExp = {rawP1, rawP0};
        } else {
            fallbackExp = {rawP0, rawP1};
        }

        const float kTol = 0.005f;
        auto matchPt = [&](const SkPoint& a, const SkPoint& b) {
            return std::abs(a.fX - b.fX) < kTol && std::abs(a.fY - b.fY) < kTol;
        };

        bool match = true;
        for (int i = 0; i < 2; ++i) {
            bool matchesLb = false;
            if (lbOpt.has_value()) {
                matchesLb = matchPt(result[i], (*lbOpt)[i]);
            }

            bool matchesFallback = matchPt(result[i], fallbackExp[i]);
            if (!matchesLb && !matchesFallback) {
                match = false;
                break;
            }
        }

        if (!match) {
            auto exp = lbOpt.has_value() ? lbOpt.value() : fallbackExp;
            ERRORF(reporter,
                   "[%s] Clip mismatch at Tile(%u, %u).\n"
                   "Line: (%.2f, %.2f) -> (%.2f, %.2f)\n"
                   "Exp: [(%.2f, %.2f), (%.2f, %.2f)]\n"
                   "Got: [(%.2f, %.2f), (%.2f, %.2f)]\n",
                   testName,
                   tile.x,
                   tile.y,
                   line.p0.fX,
                   line.p0.fY,
                   line.p1.fX,
                   line.p1.fY,
                   exp[0].fX,
                   exp[0].fY,
                   exp[1].fX,
                   exp[1].fY,
                   result[0].fX,
                   result[0].fY,
                   result[1].fX,
                   result[1].fY);
        }
    }

public:
    static void runClipTest(skiatest::Reporter* reporter,
                            const std::vector<Line>& lines,
                            const std::vector<TileType>& expectedTiles,
                            const char* testName) {
        for (const auto& tile : expectedTiles) {
            uint32_t lineIdx = tile.packedWindingLineIdx >> TileType::INT_MASK_SHIFT;
            if (lineIdx < lines.size()) {
                verifyClip(reporter, lines[lineIdx], tile, testName);
            } else {
                ERRORF(reporter,
                       "[%s] Test setup error: Tile references invalid line index %u",
                       testName,
                       lineIdx);
            }
        }
    }

    static void runAll(skiatest::Reporter* reporter) {
        {
            std::vector<Line> lines = {
                    {{1.0f, -7.0f}, {3.0f, -1.0f}},
                    {{1.0f, -11.0f}, {3.0f, -1.0f}},
                    {{BASE_V_DIM + 1.0f, 50.0f}, {BASE_V_DIM + 3.0f, 70.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {3.0f, BASE_V_DIM + 7.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {3.0f, BASE_V_DIM + 13.0f}},
            };
            runClipTest(reporter, scale_lines(lines, SCALE), {}, "CulledLines");
        }

        {
            std::vector<Line> lines = {
                    {{-2.0f, -3.0f}, {2.0f, 1.0f}},
                    {{6.0f, -1.0f}, {5.0f, 2.0f}},
                    {{9.0f, -10.0f}, {10.0f, 3.0f}},
                    {{2.0f, 1.0f}, {-2.0f, -3.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | T),
                    TileType(1, 0, 1, W | T),
                    TileType(2, 0, 2, W | T),
                    TileType(0, 0, 3, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SlopedLineCrossingTop");
        }

        {
            std::vector<Line> lines = {
                    {{5.0f, BASE_V_DIM + 3.0f}, {6.0f, BASE_V_DIM - 2.0f}},
                    {{10.0f, BASE_V_DIM + 1.0f}, {9.0f, BASE_V_DIM - 1.0f}},
                    {{2.0f, BASE_V_DIM - 2.0f}, {3.0f, BASE_V_DIM + 3.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(1, 24, 0, B),
                    TileType(2, 24, 1, B),
                    TileType(0, 24, 2, B),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SlopedLineCrossingBot");
        }

        {
            std::vector<Line> lines = {
                    {{1.0f, -5.0f}, {6.0f, 7.0f}},
                    {{2.5f, -10.0f}, {3.5f, 6.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | T | R),
                    TileType(1, 0, 0, L | B),
                    TileType(1, 1, 0, W | T),
                    TileType(0, 0, 1, W | T | B),
                    TileType(0, 1, 1, W | T),
            };
            runClipTest(reporter,
                        scale_lines(lines, SCALE),
                        expected,
                        "SlopedLineCrossingTopMultiTile");
        }

        {
            std::vector<Line> lines = {
                    {{12.0f, BASE_V_DIM + 10.0f}, {2.0f, 94.0f}},
                    {{1.5f, BASE_V_DIM + 5.0f}, {3.5f, 94.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 23, 0, B),
                    TileType(0, 24, 0, W | T | R),
                    TileType(1, 24, 0, B | L),
                    TileType(0, 23, 1, B),
                    TileType(0, 24, 1, W | T | B),
            };
            runClipTest(reporter,
                        scale_lines(lines, SCALE),
                        expected,
                        "SlopedLineCrossingBotMultiTile");
        }

        {
            std::vector<Line> lines = {
                    {{97.0f, 1.0f}, {BASE_V_DIM + 1.0f, 2.0f}},
                    {{93.0f, 1.0f}, {BASE_V_DIM + 5.0f, 2.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(24, 0, 0, R),
                    TileType(23, 0, 1, R),
                    TileType(24, 0, 1, R | L),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SlopedLineCrossingRight");
        }

        {
            std::vector<Line> lines = {
                    {{-5.0f, 1.0f}, {1.0f, 2.0f}},
                    {{-5.0f, 1.0f}, {5.0f, 2.0f}},
                    {{-5.0f, 1.0f}, {13.0f, 9.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, L),
                    TileType(0, 0, 1, L | R),
                    TileType(1, 0, 1, L),
                    TileType(0, 0, 2, L | B),
                    TileType(0, 1, 2, W | R | T),
                    TileType(1, 1, 2, R | L),
                    TileType(2, 1, 2, L | B),
                    TileType(2, 2, 2, W | R | T),
                    TileType(3, 2, 2, L),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SlopedLineCrossingLeft");
        }

        {
            std::vector<Line> lines = {{{10.0f, -5.0f}, {90.0f, -5.0f}}};
            runClipTest(reporter, scale_lines(lines, SCALE), {}, "HorizontalLineAboveViewport");
        }

        {
            std::vector<Line> lines = {{{10.0f, BASE_V_DIM + 5.0f}, {90.0f, BASE_V_DIM + 5.0f}}};
            runClipTest(reporter, scale_lines(lines, SCALE), {}, "HorizontalLineBelowViewport");
        }

        {
            std::vector<Line> lines = {{{-10.0f, 10.0f}, {10.0f, 10.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 2, 0, L | R),
                    TileType(1, 2, 0, L | R),
                    TileType(2, 2, 0, L),
            };
            runClipTest(reporter,
                        scale_lines(lines, SCALE),
                        expected,
                        "HorizontalLineCrossingLeftViewport");
        }

        {
            std::vector<Line> lines = {{{BASE_V_DIM - 5.0f, 10.0f}, {BASE_V_DIM + 5.0f, 10.0f}}};
            std::vector<TileType> expected = {TileType(23, 2, 0, R), TileType(24, 2, 0, L | R)};
            runClipTest(reporter,
                        scale_lines(lines, SCALE),
                        expected,
                        "HorizontalLineCrossingRightViewport");
        }

        {
            std::vector<Line> lines = {
                    {{1.0f, -5.0f}, {1.0f, -1.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {1.0f, BASE_V_DIM + 5.0f}},
            };
            runClipTest(reporter, scale_lines(lines, SCALE), {}, "VerticalLinesOutsideViewport");
        }

        {
            std::vector<Line> lines = {
                    {{1.0f, -7.0f}, {1.0f, 3.0f}},
                    {{1.0f, -7.0f}, {1.0f, 7.0f}},
                    {{1.0f, -7.0f}, {1.0f, 8.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | T),
                    TileType(0, 0, 1, W | B | T),
                    TileType(0, 1, 1, W | T),
                    TileType(0, 0, 2, W | B | T),
                    TileType(0, 1, 2, W | T),
            };
            runClipTest(reporter,
                        scale_lines(lines, SCALE),
                        expected,
                        "VerticalLineCrossingTopViewport");
        }

        {
            std::vector<Line> lines = {
                    {{1.0f, BASE_V_DIM - 1.0f}, {1.0f, BASE_V_DIM + 5.0f}},
                    {{1.0f, BASE_V_DIM - 5.0f}, {1.0f, BASE_V_DIM + 5.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 24, 0, B),
                    TileType(0, 23, 1, B),
                    TileType(0, 24, 1, W | T | B),
            };
            runClipTest(reporter,
                        scale_lines(lines, SCALE),
                        expected,
                        "VerticalLineCrossingBotViewport");
        }

        {
            std::vector<Line> lines = {{{-1.0f, 2.0f}, {2.0f, -1.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, W | L | T)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "ClipTopLeftCorner");
        }

        {
            std::vector<Line> lines = {{{BASE_V_DIM + 1.0f, BASE_V_DIM - 2.0f},
                                        {BASE_V_DIM - 2.0f, BASE_V_DIM + 1.0f}}};
            std::vector<TileType> expected = {TileType(24, 24, 0, R | B)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "ClipBottomRightCorner");
        }

        //==========================================================================================
        // Axis-aligned lines
        //==========================================================================================

        {
            std::vector<Line> lines = {{{1.5f, 1.0f}, {8.5f, 1.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, L),
            };
            runClipTest(reporter,
                        scale_lines(lines, SCALE),
                        expected,
                        "HorizontalLineLeftToRightThreeTile");
        }

        {
            std::vector<Line> lines = {{{8.5f, 1.0f}, {1.5f, 1.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, L),
            };
            runClipTest(reporter,
                        scale_lines(lines, SCALE),
                        expected,
                        "HorizontalLineRightToLeftThreeTile");
        }

        {
            std::vector<Line> lines = {{{1.5f, 1.0f}, {12.5f, 1.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, R | L),
                    TileType(3, 0, 0, L),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "HorizontalLineMultiTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 1.5f}, {1.0f, 8.5f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "VerticalLineDownThreeTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {1.0f, 13.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T | B),
                    TileType(0, 3, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "VerticalLineDownMultiTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 13.0f}, {1.0f, 1.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T | B),
                    TileType(0, 3, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "VerticalLineUpThreeTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 8.5f}, {1.0f, 1.5f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "VerticalLineUpMultiTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {1.0f, 8.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "VerticalLineTouchingBot");
        }

        {
            std::vector<Line> lines = {{{1.0f, 0.0f}, {1.0f, 7.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | B),
                    TileType(0, 1, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "VerticalLineTouchingTop");
        }

        {
            std::vector<Line> lines = {{{4.0f, 0.0f}, {4.0f, 7.0f}}};
            std::vector<TileType> expected = {
                    TileType(1, 0, 0, W | L | B),
                    TileType(1, 1, 0, W | L | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "VerticalLineTouchingLeft");
        }

        //==========================================================================================
        // Sloped Lines
        //==========================================================================================

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {11.0f, 9.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, L | B),
                    TileType(1, 1, 0, W | R | T),
                    TileType(2, 1, 0, L | B),
                    TileType(2, 2, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "TopLeftToBottomRight");
        }

        {
            std::vector<Line> lines = {{{11.0f, 9.0f}, {1.0f, 1.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, L | B),
                    TileType(1, 1, 0, W | R | T),
                    TileType(2, 1, 0, L | B),
                    TileType(2, 2, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "BottomRightToTopLeft");
        }

        {
            std::vector<Line> lines = {{{2.0f, 11.0f}, {14.0f, 6.0f}}};
            std::vector<TileType> expected = {
                    TileType(2, 1, 0, R | B),
                    TileType(3, 1, 0, L),
                    TileType(0, 2, 0, R),
                    TileType(1, 2, 0, R | L),
                    TileType(2, 2, 0, W | L | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "BottomLeftToTopRight");
        }

        {
            std::vector<Line> lines = {{{14.0f, 6.0f}, {2.0f, 11.0f}}};
            std::vector<TileType> expected = {
                    TileType(2, 1, 0, R | B),
                    TileType(3, 1, 0, L),
                    TileType(0, 2, 0, R),
                    TileType(1, 2, 0, R | L),
                    TileType(2, 2, 0, W | L | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "TopRightToBottomLeft");
        }

        {
            std::vector<Line> lines = {
                    {{1.0f, 3.0f}, {3.0f, 3.0f}},
                    {{3.0f, 3.0f}, {0.0f, 1.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, 0),
                    TileType(0, 0, 1, 0),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "TwoLinesInSingleTile");
        }

        {
            std::vector<Line> lines = {{{3.0f, 5.0f}, {5.0f, 3.0f}}};
            std::vector<TileType> expected = {
                    TileType(1, 0, 0, L),
                    TileType(0, 1, 0, R),
                    TileType(1, 1, 0, W | L | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "DiagonalCrossCorner");
        }

        {
            std::vector<Line> lines = {{{7.9f, 7.9f}, {0.1f, 0.1f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, L),
                    TileType(1, 1, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "DiagonalCrossCornerTwo");
        }

        {
            std::vector<Line> lines = {{{5.0f, 5.0f}, {9.0f, 9.0f}}};
            std::vector<TileType> expected = {
                    TileType(1, 1, 0, R),
                    TileType(2, 1, 0, L),
                    TileType(2, 2, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "DiagonalDownSlopeTiles");
        }

        {
            std::vector<Line> lines = {{{5.0f, 9.0f}, {9.0f, 5.0f}}};
            std::vector<TileType> expected = {
                    TileType(1, 1, 0, R | B),
                    TileType(2, 1, 0, L),
                    TileType(1, 2, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "DiagonalUpSlopeTiles");
        }

        {
            std::vector<Line> lines = {{{0.0f, 0.0f}, {4.0f, 4.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | R),
                    TileType(1, 0, 0, L),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "DiagonalDownOneTile");
        }

        {
            std::vector<Line> lines = {{{0.0f, 4.0f}, {4.0f, 0.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, W | L),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "DiagonalUpOneTile");
        }

        {
            std::vector<Line> lines = {{{0.0f, 0.0f}, {8.0f, 8.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | R),
                    TileType(1, 0, 0, L),
                    TileType(1, 1, 0, W | R | T),
                    TileType(2, 1, 0, L),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "DiagonalDownTwoTile");
        }

        {
            std::vector<Line> lines = {{{8.0f, 8.0f}, {0.0f, 0.0f}}};
            std::vector<TileType> expected = {TileType(1, 1, 0, W | L | R)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "BackwardsDiagonal");
        }

        {
            std::vector<Line> lines = {{{0.0f, 8.0f}, {8.0f, 0.0f}}};
            std::vector<TileType> expected = {
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, W | L),
                    TileType(0, 1, 0, R),
                    TileType(1, 1, 0, W | L | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "DiagonalUpTwoTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {8.0f, 2.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, L),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SlopedEndingRight");
        }

        {
            std::vector<Line> lines = {{{0.0f, 8.0f}, {4.0f, 0.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R | B),
                    TileType(1, 0, 0, W | L),
                    TileType(0, 1, 0, W | T),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SlopedTouchingTop");
        }

        {
            std::vector<Line> lines = {{{0.0f, 0.0f}, {4.0f, 8.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | B),
                    TileType(0, 1, 0, W | R | T),
                    TileType(1, 1, 0, L),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SlopedTouchingBot");
        }

        //==========================================================================================
        // Same Tile Cases
        //==========================================================================================

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {3.0f, 3.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, 0)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SameTile");
        }

        {
            std::vector<Line> lines = {{{0.0f, 1.0f}, {3.0f, 1.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, 0)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SameTileLeft");
        }

        {
            std::vector<Line> lines = {{{1.0f, 0.0f}, {1.0f, 3.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, W)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SameTileTop");
        }

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {4.0f, 1.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, L),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SameTileRight");
        }

        {
            std::vector<Line> lines = {
                    {{1.0f, 1.0f}, {1.0f, 4.0f}},
                    {{1.0f, 1.0f}, {2.0f, 4.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, 0),
                    TileType(0, 0, 1, 0),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SameTileBottom");
        }

        {
            std::vector<Line> lines = {
                    {{0.0f, 1.0f}, {1.0f, 0.0f}},
                    {{0.0f, 0.0001f}, {0.0001f, 0.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W),
                    TileType(0, 0, 1, W),
            };
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "SameTileTopLeft");
        }

        {
            std::vector<Line> lines = {{{2.0f, 2.0f}, {2.0f, 2.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, 0)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "PointLineInside");
        }

        {
            std::vector<Line> lines = {{{4.0f, 4.0f}, {4.0f, 4.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, R),
                                              TileType(1, 0, 0, L),
                                              TileType(0, 1, 0, W | R),
                                              TileType(1, 1, 0, W | L)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "PointLineOnCorner");
        }

        //==========================================================================================
        // Precision Stress Tests
        //==========================================================================================

        {
            std::vector<Line> lines = {{{3.999f, 0.0f}, {3.999f, 4.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, W | T | B)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "NearVerticalGrazing");
        }

        {
            std::vector<Line> lines = {{{2.0f, -100.0f}, {2.01f, 100.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, W | T | B)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "HighSlopeNeedle");
        }

        {
            std::vector<Line> lines = {{{-5.0f, 2.0f}, {15.0f, 2.0001f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, L | R)};
            runClipTest(reporter, scale_lines(lines, SCALE), expected, "NearParallelHorizontal");
        }
    }
};

DEF_TEST(SparseStrips_Clip_4x4, reporter) { ClipTestRunner<4, 4>::runAll(reporter); }

DEF_TEST(SparseStrips_Clip_8x8, reporter) { ClipTestRunner<8, 8>::runAll(reporter); }

}  // namespace skgpu::graphite
