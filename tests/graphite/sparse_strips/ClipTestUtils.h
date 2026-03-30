/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SparseStrips_ClipTestUtils_DEFINED
#define SparseStrips_ClipTestUtils_DEFINED

#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

namespace skgpu::graphite {

namespace {
static constexpr uint32_t W = SparseStripsConstants::W;
static constexpr uint32_t T = SparseStripsConstants::T;
static constexpr uint32_t B = SparseStripsConstants::B;
static constexpr uint32_t L = SparseStripsConstants::L;
static constexpr uint32_t R = SparseStripsConstants::R;

// LiangBarsky style as ground truth
inline std::optional<std::array<SkPoint, 2>> reference_clip(
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

inline std::vector<Line> scale_lines(const std::vector<Line>& input, float scale) {
    if (scale == 1.0f) return input;
    std::vector<Line> scaled;
    scaled.reserve(input.size());
    for (const auto& l : input) {
        scaled.push_back({{l.p0.fX * scale, l.p0.fY * scale}, {l.p1.fX * scale, l.p1.fY * scale}});
    }
    return scaled;
}

template <uint16_t WIDTH, uint16_t HEIGHT> struct ClipTestCase {
    using TileType = Tile<WIDTH, HEIGHT>;
    std::string name;
    std::vector<Line> lines;
    std::vector<TileType> expectedTiles;
};
}  // namespace

template <uint16_t WIDTH, uint16_t HEIGHT>
std::vector<ClipTestCase<WIDTH, HEIGHT>> MakeClipTestCases() {
    using TileType = Tile<WIDTH, HEIGHT>;

    float scale = static_cast<float>(WIDTH) / 4.0f;
    constexpr float BASE_V_DIM = 100.0f;

    std::vector<ClipTestCase<WIDTH, HEIGHT>> cases;

    // Helper lambda to clean up insertion
    auto addCase = [&](const std::vector<Line>& lines,
                       const std::vector<TileType>& expected,
                       const char* name) {
        cases.push_back({name, scale_lines(lines, scale), expected});
    };

    addCase(
            {
                    {{1.0f, -7.0f}, {3.0f, -1.0f}},
                    {{1.0f, -11.0f}, {3.0f, -1.0f}},
                    {{BASE_V_DIM + 1.0f, 50.0f}, {BASE_V_DIM + 3.0f, 70.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {3.0f, BASE_V_DIM + 7.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {3.0f, BASE_V_DIM + 13.0f}},
            },
            {},
            "CulledLines");

    addCase(
            {
                    {{-2.0f, -3.0f}, {2.0f, 1.0f}},
                    {{6.0f, -1.0f}, {5.0f, 2.0f}},
                    {{9.0f, -10.0f}, {10.0f, 3.0f}},
                    {{2.0f, 1.0f}, {-2.0f, -3.0f}},
            },
            {
                    TileType(0, 0, 0, W | T),
                    TileType(1, 0, 1, W | T),
                    TileType(2, 0, 2, W | T),
                    TileType(0, 0, 3, W | T),
            },
            "SlopedLineCrossingTop");

    addCase(
            {
                    {{5.0f, BASE_V_DIM + 3.0f}, {6.0f, BASE_V_DIM - 2.0f}},
                    {{10.0f, BASE_V_DIM + 1.0f}, {9.0f, BASE_V_DIM - 1.0f}},
                    {{2.0f, BASE_V_DIM - 2.0f}, {3.0f, BASE_V_DIM + 3.0f}},
            },
            {
                    TileType(1, 24, 0, B),
                    TileType(2, 24, 1, B),
                    TileType(0, 24, 2, B),
            },
            "SlopedLineCrossingBot");

    addCase(
            {
                    {{1.0f, -5.0f}, {6.0f, 7.0f}},
                    {{2.5f, -10.0f}, {3.5f, 6.0f}},
            },
            {
                    TileType(0, 0, 0, W | T | R),
                    TileType(1, 0, 0, L | B),
                    TileType(1, 1, 0, W | T),
                    TileType(0, 0, 1, W | T | B),
                    TileType(0, 1, 1, W | T),
            },
            "SlopedLineCrossingTopMultiTile");

    addCase(
            {
                    {{12.0f, BASE_V_DIM + 10.0f}, {2.0f, 94.0f}},
                    {{1.5f, BASE_V_DIM + 5.0f}, {3.5f, 94.0f}},
            },
            {
                    TileType(0, 23, 0, B),
                    TileType(0, 24, 0, W | T | R),
                    TileType(1, 24, 0, B | L),
                    TileType(0, 23, 1, B),
                    TileType(0, 24, 1, W | T | B),
            },
            "SlopedLineCrossingBotMultiTile");

    addCase(
            {
                    {{97.0f, 1.0f}, {BASE_V_DIM + 1.0f, 2.0f}},
                    {{93.0f, 1.0f}, {BASE_V_DIM + 5.0f, 2.0f}},
            },
            {
                    TileType(24, 0, 0, R),
                    TileType(23, 0, 1, R),
                    TileType(24, 0, 1, R | L),
            },
            "SlopedLineCrossingRight");

    addCase(
            {
                    {{-5.0f, 1.0f}, {1.0f, 2.0f}},
                    {{-5.0f, 1.0f}, {5.0f, 2.0f}},
                    {{-5.0f, 1.0f}, {13.0f, 9.0f}},
            },
            {
                    TileType(0, 0, 0, L),
                    TileType(0, 0, 1, L | R),
                    TileType(1, 0, 1, L),
                    TileType(0, 0, 2, L | B),
                    TileType(0, 1, 2, W | R | T),
                    TileType(1, 1, 2, R | L),
                    TileType(2, 1, 2, L | B),
                    TileType(2, 2, 2, W | R | T),
                    TileType(3, 2, 2, L),
            },
            "SlopedLineCrossingLeft");

    addCase({{{10.0f, -5.0f}, {90.0f, -5.0f}}}, {}, "HorizontalLineAboveViewport");

    addCase({{{10.0f, BASE_V_DIM + 5.0f}, {90.0f, BASE_V_DIM + 5.0f}}},
            {},
            "HorizontalLineBelowViewport");

    addCase({{{-10.0f, 10.0f}, {10.0f, 10.0f}}},
            {
                    TileType(0, 2, 0, L | R),
                    TileType(1, 2, 0, L | R),
                    TileType(2, 2, 0, L),
            },
            "HorizontalLineCrossingLeftViewport");

    addCase({{{BASE_V_DIM - 5.0f, 10.0f}, {BASE_V_DIM + 5.0f, 10.0f}}},
            {TileType(23, 2, 0, R), TileType(24, 2, 0, L | R)},
            "HorizontalLineCrossingRightViewport");

    addCase(
            {
                    {{1.0f, -5.0f}, {1.0f, -1.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {1.0f, BASE_V_DIM + 5.0f}},
            },
            {},
            "VerticalLinesOutsideViewport");

    addCase(
            {
                    {{1.0f, -7.0f}, {1.0f, 3.0f}},
                    {{1.0f, -7.0f}, {1.0f, 7.0f}},
                    {{1.0f, -7.0f}, {1.0f, 8.0f}},
            },
            {
                    TileType(0, 0, 0, W | T),
                    TileType(0, 0, 1, W | B | T),
                    TileType(0, 1, 1, W | T),
                    TileType(0, 0, 2, W | B | T),
                    TileType(0, 1, 2, W | T),
            },
            "VerticalLineCrossingTopViewport");

    addCase(
            {
                    {{1.0f, BASE_V_DIM - 1.0f}, {1.0f, BASE_V_DIM + 5.0f}},
                    {{1.0f, BASE_V_DIM - 5.0f}, {1.0f, BASE_V_DIM + 5.0f}},
            },
            {
                    TileType(0, 24, 0, B),
                    TileType(0, 23, 1, B),
                    TileType(0, 24, 1, W | T | B),
            },
            "VerticalLineCrossingBotViewport");

    addCase({{{-1.0f, 2.0f}, {2.0f, -1.0f}}}, {TileType(0, 0, 0, W | L | T)}, "ClipTopLeftCorner");

    addCase({{{BASE_V_DIM + 1.0f, BASE_V_DIM - 2.0f}, {BASE_V_DIM - 2.0f, BASE_V_DIM + 1.0f}}},
            {TileType(24, 24, 0, R | B)},
            "ClipBottomRightCorner");

    //==========================================================================================
    // Axis-aligned lines
    //==========================================================================================

    addCase({{{1.5f, 1.0f}, {8.5f, 1.0f}}},
            {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, L),
            },
            "HorizontalLineLeftToRightThreeTile");

    addCase({{{8.5f, 1.0f}, {1.5f, 1.0f}}},
            {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, L),
            },
            "HorizontalLineRightToLeftThreeTile");

    addCase({{{1.5f, 1.0f}, {12.5f, 1.0f}}},
            {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, R | L),
                    TileType(3, 0, 0, L),
            },
            "HorizontalLineMultiTile");

    addCase({{{1.0f, 1.5f}, {1.0f, 8.5f}}},
            {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T),
            },
            "VerticalLineDownThreeTile");

    addCase({{{1.0f, 1.0f}, {1.0f, 13.0f}}},
            {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T | B),
                    TileType(0, 3, 0, W | T),
            },
            "VerticalLineDownMultiTile");

    addCase({{{1.0f, 13.0f}, {1.0f, 1.0f}}},
            {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T | B),
                    TileType(0, 3, 0, W | T),
            },
            "VerticalLineUpThreeTile");

    addCase({{{1.0f, 8.5f}, {1.0f, 1.5f}}},
            {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T | B),
                    TileType(0, 2, 0, W | T),
            },
            "VerticalLineUpMultiTile");

    addCase({{{1.0f, 1.0f}, {1.0f, 8.0f}}},
            {
                    TileType(0, 0, 0, B),
                    TileType(0, 1, 0, W | T),
            },
            "VerticalLineTouchingBot");

    addCase({{{1.0f, 0.0f}, {1.0f, 7.0f}}},
            {
                    TileType(0, 0, 0, W | B),
                    TileType(0, 1, 0, W | T),
            },
            "VerticalLineTouchingTop");

    addCase({{{4.0f, 0.0f}, {4.0f, 7.0f}}},
            {
                    TileType(1, 0, 0, W | L | B),
                    TileType(1, 1, 0, W | L | T),
            },
            "VerticalLineTouchingLeft");

    //==========================================================================================
    // Sloped Lines
    //==========================================================================================

    addCase({{{1.0f, 1.0f}, {11.0f, 9.0f}}},
            {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, L | B),
                    TileType(1, 1, 0, W | R | T),
                    TileType(2, 1, 0, L | B),
                    TileType(2, 2, 0, W | T),
            },
            "TopLeftToBottomRight");

    addCase({{{11.0f, 9.0f}, {1.0f, 1.0f}}},
            {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, L | B),
                    TileType(1, 1, 0, W | R | T),
                    TileType(2, 1, 0, L | B),
                    TileType(2, 2, 0, W | T),
            },
            "BottomRightToTopLeft");

    addCase({{{2.0f, 11.0f}, {14.0f, 6.0f}}},
            {
                    TileType(2, 1, 0, R | B),
                    TileType(3, 1, 0, L),
                    TileType(0, 2, 0, R),
                    TileType(1, 2, 0, R | L),
                    TileType(2, 2, 0, W | L | T),
            },
            "BottomLeftToTopRight");

    addCase({{{14.0f, 6.0f}, {2.0f, 11.0f}}},
            {
                    TileType(2, 1, 0, R | B),
                    TileType(3, 1, 0, L),
                    TileType(0, 2, 0, R),
                    TileType(1, 2, 0, R | L),
                    TileType(2, 2, 0, W | L | T),
            },
            "TopRightToBottomLeft");

    addCase(
            {
                    {{1.0f, 3.0f}, {3.0f, 3.0f}},
                    {{3.0f, 3.0f}, {0.0f, 1.0f}},
            },
            {
                    TileType(0, 0, 0, 0),
                    TileType(0, 0, 1, 0),
            },
            "TwoLinesInSingleTile");

    addCase({{{3.0f, 5.0f}, {5.0f, 3.0f}}},
            {
                    TileType(1, 0, 0, L),
                    TileType(0, 1, 0, R),
                    TileType(1, 1, 0, W | L | T),
            },
            "DiagonalCrossCorner");

    addCase({{{7.9f, 7.9f}, {0.1f, 0.1f}}},
            {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, L),
                    TileType(1, 1, 0, W | T),
            },
            "DiagonalCrossCornerTwo");

    addCase({{{5.0f, 5.0f}, {9.0f, 9.0f}}},
            {
                    TileType(1, 1, 0, R),
                    TileType(2, 1, 0, L),
                    TileType(2, 2, 0, W | T),
            },
            "DiagonalDownSlopeTiles");

    addCase({{{5.0f, 9.0f}, {9.0f, 5.0f}}},
            {
                    TileType(1, 1, 0, R | B),
                    TileType(2, 1, 0, L),
                    TileType(1, 2, 0, W | T),
            },
            "DiagonalUpSlopeTiles");

    addCase({{{0.0f, 0.0f}, {4.0f, 4.0f}}},
            {
                    TileType(0, 0, 0, W | R),
                    TileType(1, 0, 0, L),
            },
            "DiagonalDownOneTile");

    addCase({{{0.0f, 4.0f}, {4.0f, 0.0f}}},
            {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, W | L),
            },
            "DiagonalUpOneTile");

    addCase({{{0.0f, 0.0f}, {8.0f, 8.0f}}},
            {
                    TileType(0, 0, 0, W | R),
                    TileType(1, 0, 0, L),
                    TileType(1, 1, 0, W | R | T),
                    TileType(2, 1, 0, L),
            },
            "DiagonalDownTwoTile");

    addCase({{{8.0f, 8.0f}, {0.0f, 0.0f}}}, {TileType(1, 1, 0, W | L | R)}, "BackwardsDiagonal");

    addCase({{{0.0f, 8.0f}, {8.0f, 0.0f}}},
            {
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, W | L),
                    TileType(0, 1, 0, R),
                    TileType(1, 1, 0, W | L | T),
            },
            "DiagonalUpTwoTile");

    addCase({{{1.0f, 1.0f}, {8.0f, 2.0f}}},
            {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, R | L),
                    TileType(2, 0, 0, L),
            },
            "SlopedEndingRight");

    addCase({{{0.0f, 8.0f}, {4.0f, 0.0f}}},
            {
                    TileType(0, 0, 0, R | B),
                    TileType(1, 0, 0, W | L),
                    TileType(0, 1, 0, W | T),
            },
            "SlopedTouchingTop");

    addCase({{{0.0f, 0.0f}, {4.0f, 8.0f}}},
            {
                    TileType(0, 0, 0, W | B),
                    TileType(0, 1, 0, W | R | T),
                    TileType(1, 1, 0, L),
            },
            "SlopedTouchingBot");

    //==========================================================================================
    // Same Tile Cases
    //==========================================================================================

    addCase({{{1.0f, 1.0f}, {3.0f, 3.0f}}}, {TileType(0, 0, 0, 0)}, "SameTile");

    addCase({{{0.0f, 1.0f}, {3.0f, 1.0f}}}, {TileType(0, 0, 0, 0)}, "SameTileLeft");

    addCase({{{1.0f, 0.0f}, {1.0f, 3.0f}}}, {TileType(0, 0, 0, W)}, "SameTileTop");

    addCase({{{1.0f, 1.0f}, {4.0f, 1.0f}}},
            {
                    TileType(0, 0, 0, R),
                    TileType(1, 0, 0, L),
            },
            "SameTileRight");

    addCase(
            {
                    {{1.0f, 1.0f}, {1.0f, 4.0f}},
                    {{1.0f, 1.0f}, {2.0f, 4.0f}},
            },
            {
                    TileType(0, 0, 0, 0),
                    TileType(0, 0, 1, 0),
            },
            "SameTileBottom");

    addCase(
            {
                    {{0.0f, 1.0f}, {1.0f, 0.0f}},
                    {{0.0f, 0.0001f}, {0.0001f, 0.0f}},
            },
            {
                    TileType(0, 0, 0, W),
                    TileType(0, 0, 1, W),
            },
            "SameTileTopLeft");

    addCase({{{2.0f, 2.0f}, {2.0f, 2.0f}}}, {TileType(0, 0, 0, 0)}, "PointLineInside");

    addCase({{{4.0f, 4.0f}, {4.0f, 4.0f}}},
            {TileType(0, 0, 0, R),
             TileType(1, 0, 0, L),
             TileType(0, 1, 0, W | R),
             TileType(1, 1, 0, W | L)},
            "PointLineOnCorner");

    //==========================================================================================
    // Precision Stress Tests
    //==========================================================================================

    addCase({{{3.999f, 0.0f}, {3.999f, 4.0f}}},
            {TileType(0, 0, 0, W | T | B)},
            "NearVerticalGrazing");

    addCase({{{2.0f, -100.0f}, {2.01f, 100.0f}}},
            {TileType(0, 0, 0, W | T | B)},
            "HighSlopeNeedle");

    addCase({{{-5.0f, 2.0f}, {15.0f, 2.0001f}}},
            {TileType(0, 0, 0, L | R)},
            "NearParallelHorizontal");

    return cases;
}

template <uint16_t WIDTH, uint16_t HEIGHT>
inline void VerifyClipResult(skiatest::Reporter* reporter,
                             const std::string& testName,
                             const Line& line,
                             const Tile<WIDTH, HEIGHT>& tile,
                             const std::array<SkPoint, 2>& computedResult) {
    float tileMinX = tile.x * static_cast<float>(WIDTH);
    float tileMinY = tile.y * static_cast<float>(HEIGHT);
    float tileMaxX = tileMinX + static_cast<float>(WIDTH);
    float tileMaxY = tileMinY + static_cast<float>(HEIGHT);

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
        bool matchesLb = lbOpt.has_value() && matchPt(computedResult[i], (*lbOpt)[i]);
        bool matchesFallback = matchPt(computedResult[i], fallbackExp[i]);
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
               testName.c_str(),
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
               computedResult[0].fX,
               computedResult[0].fY,
               computedResult[1].fX,
               computedResult[1].fY);
    }
}

}  // namespace skgpu::graphite

#endif  // SparseStrips_ClipTestUtils_DEFINED
