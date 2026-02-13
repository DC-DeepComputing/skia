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

using namespace skgpu::graphite;

namespace {

template <uint16_t WIDTH, uint16_t HEIGHT> struct ClipTestRunner : IntersectionBits {
    using TileType = Tile<WIDTH, HEIGHT>;

    static constexpr float SCALE = (float)WIDTH / 4.0f;
    static constexpr float BASE_V_DIM = 100.0f;

    static std::vector<Line> scale_lines(const std::vector<Line>& input) {
        if (SCALE == 1.0f) return input;
        std::vector<Line> scaled;
        scaled.reserve(input.size());
        for (const auto& l : input) {
            scaled.push_back({{l.p0.x * SCALE, l.p0.y * SCALE}, {l.p1.x * SCALE, l.p1.y * SCALE}});
        }
        return scaled;
    }

    // LiangBarsky style as ground truth
    static std::optional<std::array<Point, 2>> ReferenceClip(
            const Line& line, float minX, float minY, float maxX, float maxY) {
        float t0 = 0.0f;
        float t1 = 1.0f;
        float dx = line.p1.x - line.p0.x;
        float dy = line.p1.y - line.p0.y;

        float p[4] = {-dx, dx, -dy, dy};
        float q[4] = {line.p0.x - minX, maxX - line.p0.x, line.p0.y - minY, maxY - line.p0.y};

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

        Point pEntry = {line.p0.x + t0 * dx, line.p0.y + t0 * dy};
        Point pExit = {line.p0.x + t1 * dx, line.p0.y + t1 * dy};

        pEntry.x -= minX;
        pEntry.y -= minY;
        pExit.x -= minX;
        pExit.y -= minY;

        float width = maxX - minX;
        float height = maxY - minY;
        auto clamp_pt = [&](Point& pt) {
            pt.x = std::clamp(pt.x, 0.0f, width);
            pt.y = std::clamp(pt.y, 0.0f, height);
        };
        clamp_pt(pEntry);
        clamp_pt(pExit);

        if (pExit.y < pEntry.y) return {{{pExit, pEntry}}};
        return {{{pEntry, pExit}}};
    }

    static void VerifyStripClip(skiatest::Reporter* reporter,
                                const Line& line,
                                const TileType& tile,
                                const char* testName) {
        float dx = line.p1.x - line.p0.x;
        float dy = line.p1.y - line.p0.y;

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
        uint32_t mask = tile.packedWindingLineIdx & INTERSECTION_MASK;

        bool canonicalXDir = (dx >= 0);
        bool canonicalYDir = (dy >= 0);

        std::array<Point, 2> result =
                Strip::clipToTile(line, bounds, derivatives, mask, canonicalXDir, canonicalYDir);

        auto lbOpt = ReferenceClip(line, tileMinX, tileMinY, tileMaxX, tileMaxY);
        float width = tileMaxX - tileMinX;
        float height = tileMaxY - tileMinY;

        Point rawP0 = {line.p0.x - tileMinX, line.p0.y - tileMinY};
        Point rawP1 = {line.p1.x - tileMinX, line.p1.y - tileMinY};

        auto clampPt = [&](Point& pt) {
            pt.x = std::clamp(pt.x, 0.0f, width);
            pt.y = std::clamp(pt.y, 0.0f, height);
        };

        clampPt(rawP0);
        clampPt(rawP1);

        std::array<Point, 2> fallbackExp;
        if (rawP1.y < rawP0.y) {
            fallbackExp = {rawP1, rawP0};
        } else {
            fallbackExp = {rawP0, rawP1};
        }

        const float kTol = 0.005f;
        auto matchPt = [&](const Point& a, const Point& b) {
            return std::abs(a.x - b.x) < kTol && std::abs(a.y - b.y) < kTol;
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
                   line.p0.x,
                   line.p0.y,
                   line.p1.x,
                   line.p1.y,
                   exp[0].x,
                   exp[0].y,
                   exp[1].x,
                   exp[1].y,
                   result[0].x,
                   result[0].y,
                   result[1].x,
                   result[1].y);
        }
    }

    static void RunClipTest(skiatest::Reporter* reporter,
                            const std::vector<Line>& lines,
                            const std::vector<TileType>& expectedTiles,
                            const char* testName) {
        for (const auto& tile : expectedTiles) {
            uint32_t lineIdx = tile.packedWindingLineIdx >> TileType::INT_MASK_SHIFT;
            if (lineIdx < lines.size()) {
                VerifyStripClip(reporter, lines[lineIdx], tile, testName);
            } else {
                ERRORF(reporter,
                       "[%s] Test setup error: Tile references invalid line index %u",
                       testName,
                       lineIdx);
            }
        }
    }

    static void RunAll(skiatest::Reporter* reporter) {
        {
            std::vector<Line> lines = {
                    {{1.0f, -7.0f}, {3.0f, -1.0f}},
                    {{1.0f, -11.0f}, {3.0f, -1.0f}},
                    {{BASE_V_DIM + 1.0f, 50.0f}, {BASE_V_DIM + 3.0f, 70.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {3.0f, BASE_V_DIM + 7.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {3.0f, BASE_V_DIM + 13.0f}},
            };
            RunClipTest(reporter, scale_lines(lines), {}, "CulledLines");
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
            RunClipTest(reporter, scale_lines(lines), expected, "SlopedLineCrossingTop");
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
            RunClipTest(reporter, scale_lines(lines), expected, "SlopedLineCrossingBot");
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
            RunClipTest(reporter, scale_lines(lines), expected, "SlopedLineCrossingTopMultiTile");
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
            RunClipTest(reporter, scale_lines(lines), expected, "SlopedLineCrossingBotMultiTile");
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
            RunClipTest(reporter, scale_lines(lines), expected, "SlopedLineCrossingRight");
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
            RunClipTest(reporter, scale_lines(lines), expected, "SlopedLineCrossingLeft");
        }

        {
            std::vector<Line> lines = {{{10.0f, -5.0f}, {90.0f, -5.0f}}};
            RunClipTest(reporter, scale_lines(lines), {}, "HorizontalLineAboveViewport");
        }

        {
            std::vector<Line> lines = {{{10.0f, BASE_V_DIM + 5.0f}, {90.0f, BASE_V_DIM + 5.0f}}};
            RunClipTest(reporter, scale_lines(lines), {}, "HorizontalLineBelowViewport");
        }

        {
            std::vector<Line> lines = {{{-10.0f, 10.0f}, {10.0f, 10.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 2, 0, L | R),
                    TileType(1, 2, 0, L | R),
                    TileType(2, 2, 0, L),
            };
            RunClipTest(
                    reporter, scale_lines(lines), expected, "HorizontalLineCrossingLeftViewport");
        }

        {
            std::vector<Line> lines = {{{BASE_V_DIM - 5.0f, 10.0f}, {BASE_V_DIM + 5.0f, 10.0f}}};
            std::vector<TileType> expected = {TileType(23, 2, 0, R), TileType(24, 2, 0, L | R)};
            RunClipTest(
                    reporter, scale_lines(lines), expected, "HorizontalLineCrossingRightViewport");
        }

        {
            std::vector<Line> lines = {
                    {{1.0f, -5.0f}, {1.0f, -1.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {1.0f, BASE_V_DIM + 5.0f}},
            };
            RunClipTest(reporter, scale_lines(lines), {}, "VerticalLinesOutsideViewport");
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
            RunClipTest(reporter, scale_lines(lines), expected, "VerticalLineCrossingTopViewport");
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
            RunClipTest(reporter, scale_lines(lines), expected, "VerticalLineCrossingBotViewport");
        }

        {
            std::vector<Line> lines = {{{-1.0f, 2.0f}, {2.0f, -1.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, W | L | T)};
            RunClipTest(reporter, scale_lines(lines), expected, "ClipTopLeftCorner");
        }

        {
            std::vector<Line> lines = {{{BASE_V_DIM + 1.0f, BASE_V_DIM - 2.0f},
                                        {BASE_V_DIM - 2.0f, BASE_V_DIM + 1.0f}}};
            std::vector<TileType> expected = {TileType(24, 24, 0, R | B)};
            RunClipTest(reporter, scale_lines(lines), expected, "ClipBottomRightCorner");
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
            RunClipTest(
                    reporter, scale_lines(lines), expected, "HorizontalLineLeftToRightThreeTile");
        }

        {
            std::vector<Line> lines = {{{8.5f, 1.0f}, {1.5f, 1.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, L),
            };
            RunClipTest(
                    reporter, scale_lines(lines), expected, "HorizontalLineRightToLeftThreeTile");
        }

        {
            std::vector<Line> lines = {{{1.5f, 1.0f}, {12.5f, 1.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, R | L),
                    TileType(3, 0, 0, L),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "HorizontalLineMultiTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 1.5f}, {1.0f, 8.5f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "VerticalLineDownThreeTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {1.0f, 13.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T | B),
                    TileType(0, 3, 0, W | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "VerticalLineDownMultiTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 13.0f}, {1.0f, 1.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T | B),
                    TileType(0, 3, 0, W | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "VerticalLineUpThreeTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 8.5f}, {1.0f, 1.5f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "VerticalLineUpMultiTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {1.0f, 8.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "VerticalLineTouchingBot");
        }

        {
            std::vector<Line> lines = {{{1.0f, 0.0f}, {1.0f, 7.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | B),
                    TileType(0, 1, 0, W | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "VerticalLineTouchingTop");
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
            RunClipTest(reporter, scale_lines(lines), expected, "TopLeftToBottomRight");
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
            RunClipTest(reporter, scale_lines(lines), expected, "BottomRightToTopLeft");
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
            RunClipTest(reporter, scale_lines(lines), expected, "BottomLeftToTopRight");
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
            RunClipTest(reporter, scale_lines(lines), expected, "TopRightToBottomLeft");
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
            RunClipTest(reporter, scale_lines(lines), expected, "TwoLinesInSingleTile");
        }

        {
            std::vector<Line> lines = {{{3.0f, 5.0f}, {5.0f, 3.0f}}};
            std::vector<TileType> expected = {
                    TileType(1, 0, 0, L),
                    TileType(0, 1, 0, R),
                    TileType(1, 1, 0, W | L | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "DiagonalCrossCorner");
        }

        {
            std::vector<Line> lines = {{{7.9f, 7.9f}, {0.1f, 0.1f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, L),
                    TileType(1, 1, 0, W | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "DiagonalCrossCornerTwo");
        }

        {
            std::vector<Line> lines = {{{5.0f, 5.0f}, {9.0f, 9.0f}}};
            std::vector<TileType> expected = {
                    TileType(1, 1, 0, R),
                    TileType(2, 1, 0, L),
                    TileType(2, 2, 0, W | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "DiagonalDownSlopeTiles");
        }

        {
            std::vector<Line> lines = {{{5.0f, 9.0f}, {9.0f, 5.0f}}};
            std::vector<TileType> expected = {
                    TileType(1, 1, 0, R | B),
                    TileType(2, 1, 0, L),
                    TileType(1, 2, 0, W | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "DiagonalUpSlopeTiles");
        }

        {
            std::vector<Line> lines = {{{0.0f, 0.0f}, {4.0f, 4.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | R),
                    TileType(1, 0, 0, L),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "DiagonalDownOneTile");
        }

        {
            std::vector<Line> lines = {{{0.0f, 4.0f}, {4.0f, 0.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, W | L),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "DiagonalUpOneTile");
        }

        {
            std::vector<Line> lines = {{{0.0f, 0.0f}, {8.0f, 8.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | R),
                    TileType(1, 0, 0, L),
                    TileType(1, 1, 0, W | R | T),
                    TileType(2, 1, 0, L),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "DiagonalDownTwoTile");
        }

        {
            std::vector<Line> lines = {{{8.0f, 8.0f}, {0.0f, 0.0f}}};
            std::vector<TileType> expected = {TileType(1, 1, 0, W | L | R)};
            RunClipTest(reporter, scale_lines(lines), expected, "BackwardsDiagonal");
        }

        {
            std::vector<Line> lines = {{{0.0f, 8.0f}, {8.0f, 0.0f}}};
            std::vector<TileType> expected = {
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, W | L),
                    TileType(0, 1, 0, R),
                    TileType(1, 1, 0, W | L | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "DiagonalUpTwoTile");
        }

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {8.0f, 2.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, L),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "SlopedEndingRight");
        }

        {
            std::vector<Line> lines = {{{0.0f, 8.0f}, {4.0f, 0.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R | B),
                    TileType(1, 0, 0, W | L),
                    TileType(0, 1, 0, W | T),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "SlopedTouchingTop");
        }

        {
            std::vector<Line> lines = {{{0.0f, 0.0f}, {4.0f, 8.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, W | B),
                    TileType(0, 1, 0, W | R | T),
                    TileType(1, 1, 0, L),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "SlopedTouchingBot");
        }

        //==========================================================================================
        // Same Tile Cases
        //==========================================================================================

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {3.0f, 3.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, 0)};
            RunClipTest(reporter, scale_lines(lines), expected, "SameTile");
        }

        {
            std::vector<Line> lines = {{{0.0f, 1.0f}, {3.0f, 1.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, 0)};
            RunClipTest(reporter, scale_lines(lines), expected, "SameTileLeft");
        }

        {
            std::vector<Line> lines = {{{1.0f, 0.0f}, {1.0f, 3.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, W)};
            RunClipTest(reporter, scale_lines(lines), expected, "SameTileTop");
        }

        {
            std::vector<Line> lines = {{{1.0f, 1.0f}, {4.0f, 1.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, L),
            };
            RunClipTest(reporter, scale_lines(lines), expected, "SameTileRight");
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
            RunClipTest(reporter, scale_lines(lines), expected, "SameTileBottom");
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
            RunClipTest(reporter, scale_lines(lines), expected, "SameTileTopLeft");
        }

        {
            std::vector<Line> lines = {{{2.0f, 2.0f}, {2.0f, 2.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, 0)};
            RunClipTest(reporter, scale_lines(lines), expected, "PointLineInside");
        }

        {
            std::vector<Line> lines = {{{4.0f, 4.0f}, {4.0f, 4.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, R),
                                              TileType(1, 0, 0, L),
                                              TileType(0, 1, 0, W | R),
                                              TileType(1, 1, 0, W | L)};
            RunClipTest(reporter, scale_lines(lines), expected, "PointLineOnCorner");
        }

        //==========================================================================================
        // Precision Stress Tests
        //==========================================================================================

        {
            std::vector<Line> lines = {{{3.999f, 0.0f}, {3.999f, 4.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, W | T | B)};
            RunClipTest(reporter, scale_lines(lines), expected, "NearVerticalGrazing");
        }

        {
            std::vector<Line> lines = {{{2.0f, -100.0f}, {2.01f, 100.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, W | T | B)};
            RunClipTest(reporter, scale_lines(lines), expected, "HighSlopeNeedle");
        }

        {
            std::vector<Line> lines = {{{-5.0f, 2.0f}, {15.0f, 2.0001f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, L | R)};
            RunClipTest(reporter, scale_lines(lines), expected, "NearParallelHorizontal");
        }
    }
};

}  // namespace

DEF_TEST(GraphiteClip_4x4, reporter) { ClipTestRunner<4, 4>::RunAll(reporter); }

DEF_TEST(GraphiteClip_8x8, reporter) { ClipTestRunner<8, 8>::RunAll(reporter); }
