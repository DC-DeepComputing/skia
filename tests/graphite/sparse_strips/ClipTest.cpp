/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <cmath>
#include <vector>
#include <algorithm>
#include <optional>
#include <array>

using namespace skgpu::graphite;

namespace {

const uint16_t VIEW_DIM = 100;
const float F_V_DIM = static_cast<float>(VIEW_DIM);

const uint32_t T = Tile::T;
const uint32_t B = Tile::B;
const uint32_t L = Tile::L;
const uint32_t R = Tile::R;
const uint32_t W = Tile::W;

// LiangBarsky style as ground truth
std::optional<std::array<std::array<float, 2>, 2>> ReferenceClip(
    const Line& line, float min_x, float min_y, float max_x, float max_y) {

    float t0 = 0.0f;
    float t1 = 1.0f;
    float dx = line.p1.x - line.p0.x;
    float dy = line.p1.y - line.p0.y;

    float p[4] = {-dx, dx, -dy, dy};
    float q[4] = {line.p0.x - min_x, max_x - line.p0.x, line.p0.y - min_y, max_y - line.p0.y};

    for (int i = 0; i < 4; i++) {
        if (p[i] == 0) {
            if (q[i] < 0) return std::nullopt; // Parallel and outside
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

    std::array<float, 2> p_entry = {line.p0.x + t0 * dx, line.p0.y + t0 * dy};
    std::array<float, 2> p_exit = {line.p0.x + t1 * dx, line.p0.y + t1 * dy};

    p_entry[0] -= min_x; p_entry[1] -= min_y;
    p_exit[0]  -= min_x; p_exit[1]  -= min_y;


    float width = max_x - min_x;
    float height = max_y - min_y;
    auto clamp_pt = [&](std::array<float, 2>& pt) {
        pt[0] = std::clamp(pt[0], 0.0f, width);
        pt[1] = std::clamp(pt[1], 0.0f, height);
    };
    clamp_pt(p_entry);
    clamp_pt(p_exit);

    if (p_exit[1] < p_entry[1]) return {{{p_exit, p_entry}}};
    return {{{p_entry, p_exit}}};
}

void VerifyStripClip(skiatest::Reporter* reporter,
                     const Line& line,
                     const Tile& tile) {
    float dx = line.p1.x - line.p0.x;
    float dy = line.p1.y - line.p0.y;

    bool is_vertical = std::abs(dx) <= std::numeric_limits<float>::epsilon();
    bool is_horizontal = std::abs(dy) <= std::numeric_limits<float>::epsilon();

    float idx = is_vertical ? 0.0f : 1.0f / dx;
    float idy = is_horizontal ? 0.0f : 1.0f / dy;

    float tile_min_x = tile.x * static_cast<float>(Tile::WIDTH);
    float tile_min_y = tile.y * static_cast<float>(Tile::HEIGHT);
    float tile_max_x = tile_min_x + static_cast<float>(Tile::WIDTH);
    float tile_max_y = tile_min_y + static_cast<float>(Tile::HEIGHT);

    std::array<float, 4> bounds = {tile_min_x, tile_min_y, tile_max_x, tile_max_y};
    std::array<float, 4> derivatives = {dx, dy, idx, idy};
    uint32_t mask = tile.packed_winding_line_idx & Tile::INTERSECTION_MASK;

    bool canonical_x_dir = (dx >= 0);
    bool canonical_y_dir = (dy >= 0);

    auto result = Strip::ClipToTile(
        line,
        bounds,
        derivatives,
        mask,
        canonical_x_dir,
        canonical_y_dir
    );

    auto lb_opt = ReferenceClip(line, tile_min_x, tile_min_y, tile_max_x, tile_max_y);
    float width = tile_max_x - tile_min_x;
    float height = tile_max_y - tile_min_y;

    std::array<float, 2> raw_p0 = {line.p0.x - tile_min_x, line.p0.y - tile_min_y};
    std::array<float, 2> raw_p1 = {line.p1.x - tile_min_x, line.p1.y - tile_min_y};

    auto clamp_pt = [&](std::array<float, 2>& pt) {
        pt[0] = std::clamp(pt[0], 0.0f, width);
        pt[1] = std::clamp(pt[1], 0.0f, height);
    };

    clamp_pt(raw_p0);
    clamp_pt(raw_p1);
    std::array<std::array<float, 2>, 2> fallback_exp;
    if (raw_p1[1] < raw_p0[1]) {
        fallback_exp = {raw_p1, raw_p0};
    } else {
        fallback_exp = {raw_p0, raw_p1};
    }

    const float kTol = 0.005f;
    auto match_pt = [&](const std::array<float, 2>& a, const std::array<float, 2>& b) {
        return std::abs(a[0] - b[0]) < kTol && std::abs(a[1] - b[1]) < kTol;
    };

    bool match = true;
    for (int i = 0; i < 2; ++i) {
        bool matches_lb = false;
        if (lb_opt.has_value()) {
            matches_lb = match_pt(result[i], (*lb_opt)[i]);
        }

        bool matches_fallback = match_pt(result[i], fallback_exp[i]);
        if (!matches_lb && !matches_fallback) {
            match = false;
            break;
        }
    }

    if (!match) {
        auto exp = lb_opt.has_value() ? lb_opt.value() : fallback_exp;
        ERRORF(reporter,
               "Clip mismatch at Tile(%u, %u).\n"
               "Line: (%.2f, %.2f) -> (%.2f, %.2f)\n"
               "Exp: [(%.2f, %.2f), (%.2f, %.2f)]\n"
               "Got: [(%.2f, %.2f), (%.2f, %.2f)]\n"
               "Note: Result did not match Liang-Barsky OR Raw-Clamped logic.",
               tile.x, tile.y,
               line.p0.x, line.p0.y, line.p1.x, line.p1.y,
               exp[0][0], exp[0][1], exp[1][0], exp[1][1],
               result[0][0], result[0][1], result[1][0], result[1][1]);
    }
}

void RunClipTest(skiatest::Reporter* reporter,
                 const std::vector<Line>& lines,
                 const std::vector<Tile>& expected_tiles) {
    for (const auto& tile : expected_tiles) {
        uint32_t line_idx = tile.packed_winding_line_idx >> Tile::INT_MASK_SHIFT;
        if (line_idx < lines.size()) {
            VerifyStripClip(reporter, lines[line_idx], tile);
        } else {
            ERRORF(reporter, "Test setup error: Tile references invalid line index %u", line_idx);
        }
    }
}

} // namespace

DEF_TEST(TileClip_CulledLines, reporter) {
    std::vector<Line> lines = {
            {{1.0f, -7.0f}, {3.0f, -1.0f}},
            {{1.0f, -11.0f}, {3.0f, -1.0f}},
            {{F_V_DIM + 1.0f, 50.0f}, {F_V_DIM + 3.0f, 70.0f}},
            {{1.0f, F_V_DIM + 1.0f}, {3.0f, F_V_DIM + 7.0f}},
            {{1.0f, F_V_DIM + 1.0f}, {3.0f, F_V_DIM + 13.0f}},
    };
    RunClipTest(reporter, lines, {});
}

DEF_TEST(TileClip_SlopedLineCrossingTop, reporter) {
    std::vector<Line> lines = {
            {{-2.0f, -3.0f}, {2.0f, 1.0f}},
            {{6.0f, -1.0f}, {5.0f, 2.0f}},
            {{9.0f, -10.0f}, {10.0f, 3.0f}},
            {{2.0f, 1.0f}, {-2.0f, -3.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | T),
            Tile::new_tile(1, 0, 1, W | T),
            Tile::new_tile(2, 0, 2, W | T),
            Tile::new_tile(0, 0, 3, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SlopedLineCrossingBot, reporter) {
    std::vector<Line> lines = {
            {{5.0f, F_V_DIM + 3.0f}, {6.0f, F_V_DIM - 2.0f}},
            {{10.0f, F_V_DIM + 1.0f}, {9.0f, F_V_DIM - 1.0f}},
            {{2.0f, F_V_DIM - 2.0f}, {3.0f, F_V_DIM + 3.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(1, 24, 0, B),
            Tile::new_tile(2, 24, 1, B),
            Tile::new_tile(0, 24, 2, B),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SlopedLineCrossingTopMultiTile, reporter) {
    std::vector<Line> lines = {
            {{1.0f, -5.0f}, {6.0f, 7.0f}},
            {{2.5f, -10.0f}, {3.5f, 6.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | T | R),
            Tile::new_tile(1, 0, 0, L | B),
            Tile::new_tile(1, 1, 0, W | T),
            Tile::new_tile(0, 0, 1, W | T | B),
            Tile::new_tile(0, 1, 1, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SlopedLineCrossingBotMultiTile, reporter) {
    std::vector<Line> lines = {
            {{12.0f, F_V_DIM + 10.0f}, {2.0f, 94.0f}},
            {{1.5f, F_V_DIM + 5.0f}, {3.5f, 94.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 23, 0, B),
            Tile::new_tile(0, 24, 0, W | T | R),
            Tile::new_tile(1, 24, 0, B | L),
            Tile::new_tile(0, 23, 1, B),
            Tile::new_tile(0, 24, 1, W | T | B),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SlopedLineCrossingRight, reporter) {
    std::vector<Line> lines = {
            {{97.0f, 1.0f}, {F_V_DIM + 1.0f, 2.0f}},
            {{93.0f, 1.0f}, {F_V_DIM + 5.0f, 2.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(24, 0, 0, R),
            Tile::new_tile(23, 0, 1, R),
            Tile::new_tile(24, 0, 1, R | L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SlopedLineCrossingLeft, reporter) {
    std::vector<Line> lines = {
            {{-5.0f, 1.0f}, {1.0f, 2.0f}},
            {{-5.0f, 1.0f}, {5.0f, 2.0f}},
            {{-5.0f, 1.0f}, {13.0f, 9.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, L),
            Tile::new_tile(0, 0, 1, L | R),
            Tile::new_tile(1, 0, 1, L),
            Tile::new_tile(0, 0, 2, L | B),
            Tile::new_tile(0, 1, 2, W | R | T),
            Tile::new_tile(1, 1, 2, R | L),
            Tile::new_tile(2, 1, 2, L | B),
            Tile::new_tile(2, 2, 2, W | R | T),
            Tile::new_tile(3, 2, 2, L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_HorizontalLineAboveViewport, reporter) {
    std::vector<Line> lines = {{{10.0f, -5.0f}, {90.0f, -5.0f}}};
    RunClipTest(reporter, lines, {});
}

DEF_TEST(TileClip_HorizontalLineBelowViewport, reporter) {
    std::vector<Line> lines = {{{10.0f, F_V_DIM + 5.0f}, {90.0f, F_V_DIM + 5.0f}}};
    RunClipTest(reporter, lines, {});
}

DEF_TEST(TileClip_HorizontalLineCrossingLeftViewport, reporter) {
    std::vector<Line> lines = {{{-10.0f, 10.0f}, {10.0f, 10.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 2, 0, L | R),
            Tile::new_tile(1, 2, 0, L | R),
            Tile::new_tile(2, 2, 0, L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_HorizontalLineCrossingRightViewport, reporter) {
    std::vector<Line> lines = {{{F_V_DIM - 5.0f, 10.0f}, {F_V_DIM + 5.0f, 10.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(23, 2, 0, R), Tile::new_tile(24, 2, 0, L | R)};
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_VerticalLinesOutsideViewport, reporter) {
    std::vector<Line> lines = {
            {{1.0f, -5.0f}, {1.0f, -1.0f}},
            {{1.0f, F_V_DIM + 1.0f}, {1.0f, F_V_DIM + 5.0f}},
    };
    RunClipTest(reporter, lines, {});
}

DEF_TEST(TileClip_VerticalLineCrossingTopViewport, reporter) {
    std::vector<Line> lines = {
            {{1.0f, -7.0f}, {1.0f, 3.0f}},
            {{1.0f, -7.0f}, {1.0f, 7.0f}},
            {{1.0f, -7.0f}, {1.0f, 8.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | T),
            Tile::new_tile(0, 0, 1, W | B | T),
            Tile::new_tile(0, 1, 1, W | T),
            Tile::new_tile(0, 0, 2, W | B | T),
            Tile::new_tile(0, 1, 2, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_VerticalLineCrossingBotViewport, reporter) {
    std::vector<Line> lines = {
            {{1.0f, F_V_DIM - 1.0f}, {1.0f, F_V_DIM + 5.0f}},
            {{1.0f, F_V_DIM - 5.0f}, {1.0f, F_V_DIM + 5.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 24, 0, B),
            Tile::new_tile(0, 23, 1, B),
            Tile::new_tile(0, 24, 1, W | T | B),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_ClipTopLeftCorner, reporter) {
    std::vector<Line> lines = {{{-1.0f, 2.0f}, {2.0f, -1.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(0, 0, 0, W | L | T)};
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_ClipBottomRightCorner, reporter) {
    std::vector<Line> lines = {
            {{F_V_DIM + 1.0f, F_V_DIM - 2.0f}, {F_V_DIM - 2.0f, F_V_DIM + 1.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(24, 24, 0, R | B)};
    RunClipTest(reporter, lines, expected);
}

//==============================================================================================
// Axis-aligned lines
//==============================================================================================

DEF_TEST(TileClip_HorizontalLineLeftToRightThreeTile, reporter) {
    std::vector<Line> lines = {{{1.5f, 1.0f}, {8.5f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, R | L),
            Tile::new_tile(2, 0, 0, L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_HorizontalLineRightToLeftThreeTile, reporter) {
    std::vector<Line> lines = {{{8.5f, 1.0f}, {1.5f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, R | L),
            Tile::new_tile(2, 0, 0, L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_HorizontalLineMultiTile, reporter) {
    std::vector<Line> lines = {{{1.5f, 1.0f}, {12.5f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, R | L),
            Tile::new_tile(2, 0, 0, R | L),
            Tile::new_tile(3, 0, 0, L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_VerticalLineDownThreeTile, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.5f}, {1.0f, 8.5f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, B),
            Tile::new_tile(0, 1, 0, W | T | B),
            Tile::new_tile(0, 2, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_VerticalLineDownMultiTile, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {1.0f, 13.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, B),
            Tile::new_tile(0, 1, 0, W | T | B),
            Tile::new_tile(0, 2, 0, W | T | B),
            Tile::new_tile(0, 3, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_VerticalLineUpThreeTile, reporter) {
    std::vector<Line> lines = {{{1.0f, 13.0f}, {1.0f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, B),
            Tile::new_tile(0, 1, 0, W | T | B),
            Tile::new_tile(0, 2, 0, W | T | B),
            Tile::new_tile(0, 3, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_VerticalLineUpMultiTile, reporter) {
    std::vector<Line> lines = {{{1.0f, 8.5f}, {1.0f, 1.5f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, B),
            Tile::new_tile(0, 1, 0, W | T | B),
            Tile::new_tile(0, 2, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_VerticalLineTouchingBot, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {1.0f, 8.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, B),
            Tile::new_tile(0, 1, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_VerticalLineTouchingTop, reporter) {
    std::vector<Line> lines = {{{1.0f, 0.0f}, {1.0f, 7.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | B),
            Tile::new_tile(0, 1, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

//==============================================================================================
// Sloped Lines
//==============================================================================================

DEF_TEST(TileClip_TopLeftToBottomRight, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {11.0f, 9.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, L | B),
            Tile::new_tile(1, 1, 0, W | R | T),
            Tile::new_tile(2, 1, 0, L | B),
            Tile::new_tile(2, 2, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_BottomRightToTopLeft, reporter) {
    std::vector<Line> lines = {{{11.0f, 9.0f}, {1.0f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, L | B),
            Tile::new_tile(1, 1, 0, W | R | T),
            Tile::new_tile(2, 1, 0, L | B),
            Tile::new_tile(2, 2, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_BottomLeftToTopRight, reporter) {
    std::vector<Line> lines = {{{2.0f, 11.0f}, {14.0f, 6.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(2, 1, 0, R | B),
            Tile::new_tile(3, 1, 0, L),
            Tile::new_tile(0, 2, 0, R),
            Tile::new_tile(1, 2, 0, R | L),
            Tile::new_tile(2, 2, 0, W | L | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_TopRightToBottomLeft, reporter) {
    std::vector<Line> lines = {{{14.0f, 6.0f}, {2.0f, 11.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(2, 1, 0, R | B),
            Tile::new_tile(3, 1, 0, L),
            Tile::new_tile(0, 2, 0, R),
            Tile::new_tile(1, 2, 0, R | L),
            Tile::new_tile(2, 2, 0, W | L | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_TwoLinesInSingleTile, reporter) {
    std::vector<Line> lines = {
            {{1.0f, 3.0f}, {3.0f, 3.0f}},
            {{3.0f, 3.0f}, {0.0f, 1.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, 0),
            Tile::new_tile(0, 0, 1, 0),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_DiagonalCrossCorner, reporter) {
    std::vector<Line> lines = {{{3.0f, 5.0f}, {5.0f, 3.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(1, 0, 0, L),
            Tile::new_tile(0, 1, 0, R),
            Tile::new_tile(1, 1, 0, W | L | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_DiagonalCrossCornerTwo, reporter) {
    std::vector<Line> lines = {{{7.9f, 7.9f}, {0.1f, 0.1f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, L),
            Tile::new_tile(1, 1, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_DiagonalDownSlopeTiles, reporter) {
    std::vector<Line> lines = {{{5.0f, 5.0f}, {9.0f, 9.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(1, 1, 0, R),
            Tile::new_tile(2, 1, 0, L),
            Tile::new_tile(2, 2, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_DiagonalUpSlopeTiles, reporter) {
    std::vector<Line> lines = {{{5.0f, 9.0f}, {9.0f, 5.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(1, 1, 0, R | B),
            Tile::new_tile(2, 1, 0, L),
            Tile::new_tile(1, 2, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_DiagonalDownOneTile, reporter) {
    std::vector<Line> lines = {{{0.0f, 0.0f}, {4.0f, 4.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | R),
            Tile::new_tile(1, 0, 0, L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_DiagonalUpOneTile, reporter) {
    std::vector<Line> lines = {{{0.0f, 4.0f}, {4.0f, 0.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, W | L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_DiagonalDownTwoTile, reporter) {
    std::vector<Line> lines = {{{0.0f, 0.0f}, {8.0f, 8.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | R),
            Tile::new_tile(1, 0, 0, L),
            Tile::new_tile(1, 1, 0, W | R | T),
            Tile::new_tile(2, 1, 0, L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_BackwardsDiagonal, reporter) {
    std::vector<Line> lines = {{{8.0f, 8.0f}, {0.0f, 0.0f}}};
    std::vector<Tile> expected = {
        Tile::new_tile(1, 1, 0, W | L | R)
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_DiagonalUpTwoTile, reporter) {
    std::vector<Line> lines = {{{0.0f, 8.0f}, {8.0f, 0.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(1, 0, 0, R | L),
            Tile::new_tile(2, 0, 0, W | L),
            Tile::new_tile(0, 1, 0, R),
            Tile::new_tile(1, 1, 0, W | L | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SlopedEndingRight, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {8.0f, 2.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, R | L),
            Tile::new_tile(2, 0, 0, L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SlopedTouchingTop, reporter) {
    std::vector<Line> lines = {{{0.0f, 8.0f}, {4.0f, 0.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R | B),
            Tile::new_tile(1, 0, 0, W | L),
            Tile::new_tile(0, 1, 0, W | T),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SlopedTouchingBot, reporter) {
    std::vector<Line> lines = {{{0.0f, 0.0f}, {4.0f, 8.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | B),
            Tile::new_tile(0, 1, 0, W | R | T),
            Tile::new_tile(1, 1, 0, L),
    };
    RunClipTest(reporter, lines, expected);
}

//==============================================================================================
// Same Tile Cases
//==============================================================================================

DEF_TEST(TileClip_SameTile, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {3.0f, 3.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(0, 0, 0, 0)};
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SameTileLeft, reporter) {
    std::vector<Line> lines = {{{0.0f, 1.0f}, {3.0f, 1.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(0, 0, 0, 0)};
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SameTileTop, reporter) {
    std::vector<Line> lines = {{{1.0f, 0.0f}, {1.0f, 3.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(0, 0, 0, W)};
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SameTileRight, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {4.0f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, L),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SameTileBottom, reporter) {
    std::vector<Line> lines = {
            {{1.0f, 1.0f}, {1.0f, 4.0f}},
            {{1.0f, 1.0f}, {2.0f, 4.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, 0),
            Tile::new_tile(0, 0, 1, 0),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_SameTileTopLeft, reporter) {
    std::vector<Line> lines = {
            {{0.0f, 1.0f}, {1.0f, 0.0f}},
            {{0.0f, 0.0001f}, {0.0001f, 0.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W),
            Tile::new_tile(0, 0, 1, W),
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_PointLineInside, reporter) {
    std::vector<Line> lines = {{{2.0f, 2.0f}, {2.0f, 2.0f}}};
    std::vector<Tile> expected = {
        Tile::new_tile(0, 0, 0, 0)
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_PointLineOnCorner, reporter) {
    std::vector<Line> lines = {{{4.0f, 4.0f}, {4.0f, 4.0f}}};
    std::vector<Tile> expected = {
        Tile::new_tile(0, 0, 0, R),
        Tile::new_tile(1, 0, 0, L),
        Tile::new_tile(0, 1, 0, W | R),
        Tile::new_tile(1, 1, 0, W | L)
    };
    RunClipTest(reporter, lines, expected);
}

//==============================================================================================
// Precision Stress Tests
//==============================================================================================

DEF_TEST(TileClip_NearVerticalGrazing, reporter) {
    std::vector<Line> lines = {{{3.999f, 0.0f}, {3.999f, 4.0f}}};
    std::vector<Tile> expected = {
        Tile::new_tile(0, 0, 0, W | T | B)
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_HighSlopeNeedle, reporter) {
    std::vector<Line> lines = {{{2.0f, -100.0f}, {2.01f, 100.0f}}};
    std::vector<Tile> expected = {
        Tile::new_tile(0, 0, 0, W | T | B)
    };
    RunClipTest(reporter, lines, expected);
}

DEF_TEST(TileClip_NearParallelHorizontal, reporter) {
    std::vector<Line> lines = {{{-5.0f, 2.0f}, {15.0f, 2.0001f}}};
    std::vector<Tile> expected = {
        Tile::new_tile(0, 0, 0, L | R)
    };
    RunClipTest(reporter, lines, expected);
}
