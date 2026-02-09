/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <cmath>
#include <vector>

using namespace skgpu::graphite;

namespace {

const uint16_t VIEW_DIM = 100;
const float F_V_DIM = (float)VIEW_DIM;

const uint32_t T = Tile::T;
const uint32_t B = Tile::B;
const uint32_t L = Tile::L;
const uint32_t R = Tile::R;
const uint32_t W = Tile::W;

std::string mask_to_string(uint32_t mask) {
    if (mask == 0) return "0";

    std::vector<std::string> parts;
    if (mask & W) parts.push_back("W");
    if (mask & T) parts.push_back("T");
    if (mask & B) parts.push_back("B");
    if (mask & L) parts.push_back("L");
    if (mask & R) parts.push_back("R");

    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i) {
        oss << parts[i];
        if (i < parts.size() - 1) oss << " | ";
    }
    return oss.str();
}

void check_tiles_match(skiatest::Reporter* reporter,
                       Tiles& tiles,
                       const std::vector<Line>& lines,
                       const std::vector<Tile>& expected) {
    tiles.reset();

    tiles.make_tiles_msaa(lines, VIEW_DIM, VIEW_DIM);

    const std::vector<Tile>& actual = tiles.get_tiles();

    REPORTER_ASSERT(reporter,
                    actual.size() == expected.size(),
                    "Tile count mismatch. Expected %zu, got %zu",
                    expected.size(),
                    actual.size());

    size_t limit = std::min(actual.size(), expected.size());
    for (size_t i = 0; i < limit; ++i) {
        const Tile& got = actual[i];
        const Tile& want = expected[i];

        REPORTER_ASSERT(reporter,
                        got.x == want.x,
                        "Tile[%zu] X mismatch. Want %u, Got %u",
                        i,
                        want.x,
                        got.x);
        REPORTER_ASSERT(reporter,
                        got.y == want.y,
                        "Tile[%zu] Y mismatch. Want %u, Got %u",
                        i,
                        want.y,
                        got.y);

        uint32_t got_line = got.packed_winding_line_idx >> Tile::INT_MASK_SHIFT;
        uint32_t want_line = want.packed_winding_line_idx >> Tile::INT_MASK_SHIFT;
        REPORTER_ASSERT(reporter,
                        got_line == want_line,
                        "Tile[%zu] Line Index mismatch. Want %u, Got %u",
                        i,
                        want_line,
                        got_line);

        uint32_t got_mask = got.packed_winding_line_idx & Tile::INTERSECTION_MASK;
        uint32_t want_mask = want.packed_winding_line_idx & Tile::INTERSECTION_MASK;
        if (got_mask != want_mask) {
            std::string got_str = mask_to_string(got_mask);
            std::string want_str = mask_to_string(want_mask);
            ERRORF(reporter,
                   "Tile[%zu] Mask mismatch. Want [%s], Got [%s]",
                   i,
                   want_str.c_str(),
                   got_str.c_str());
        }
    }
}

void check_sorted(skiatest::Reporter* reporter, const std::vector<Tile>& buf) {
    if (buf.empty()) return;

    for (size_t i = 0; i < buf.size() - 1; ++i) {
        const Tile& current = buf[i];
        const Tile& next = buf[i + 1];

        if (current.y > next.y) {
            ERRORF(reporter,
                   "Sort Failure [Y]: Tile[%zu] (y=%d) > Tile[%zu] (y=%d)",
                   i,
                   current.y,
                   i + 1,
                   next.y);
        }

        if (current.y == next.y) {
            if (current.x > next.x) {
                ERRORF(reporter,
                       "Sort Failure [X]: at Row y=%d, Tile[%zu] (x=%d) > Tile[%zu] (x=%d)",
                       current.y,
                       i,
                       current.x,
                       i + 1,
                       next.x);
            }

            if (current.x == next.x &&
                current.packed_winding_line_idx > next.packed_winding_line_idx) {
                ERRORF(reporter,
                       "Sort Failure [Payload]: at %dx%d, Tile[%zu] > Tile[%zu]",
                       current.x,
                       current.y,
                       i,
                       i + 1);
            }
        }
    }
}

}  // namespace

DEF_TEST(GraphiteTiler_CulledLines, reporter) {
    std::vector<Line> lines = {
            {{1.0f, -7.0f}, {3.0f, -1.0f}},
            {{1.0f, -11.0f}, {3.0f, -1.0f}},
            {{F_V_DIM + 1.0f, 50.0f}, {F_V_DIM + 3.0f, 70.0f}},
            {{1.0f, F_V_DIM + 1.0f}, {3.0f, F_V_DIM + 7.0f}},
            {{1.0f, F_V_DIM + 1.0f}, {3.0f, F_V_DIM + 13.0f}},
    };

    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, {});
}

DEF_TEST(GraphiteTiler_SlopedLineCrossingTop, reporter) {
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

    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SlopedLineCrossingBot, reporter) {
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

    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SlopedLineCrossingTopMultiTile, reporter) {
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

    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SlopedLineCrossingBotMultiTile, reporter) {
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

    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SlopedLineCrossingRight, reporter) {
    std::vector<Line> lines = {
            {{97.0f, 1.0f}, {F_V_DIM + 1.0f, 2.0f}},
            {{93.0f, 1.0f}, {F_V_DIM + 5.0f, 2.0f}},
    };

    std::vector<Tile> expected = {
            Tile::new_tile(24, 0, 0, R),
            Tile::new_tile(23, 0, 1, R),
            Tile::new_tile(24, 0, 1, R | L),
    };

    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SlopedLineCrossingLeft, reporter) {
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

    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_HorizontalLineAboveViewport, reporter) {
    std::vector<Line> lines = {{{10.0f, -5.0f}, {90.0f, -5.0f}}};
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, {});
}

DEF_TEST(GraphiteTiler_HorizontalLineBelowViewport, reporter) {
    std::vector<Line> lines = {{{10.0f, F_V_DIM + 5.0f}, {90.0f, F_V_DIM + 5.0f}}};
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, {});
}

DEF_TEST(GraphiteTiler_HorizontalLineCrossingLeftViewport, reporter) {
    std::vector<Line> lines = {{{-10.0f, 10.0f}, {10.0f, 10.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 2, 0, L | R),
            Tile::new_tile(1, 2, 0, L | R),
            Tile::new_tile(2, 2, 0, L),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_HorizontalLineCrossingRightViewport, reporter) {
    std::vector<Line> lines = {{{F_V_DIM - 5.0f, 10.0f}, {F_V_DIM + 5.0f, 10.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(23, 2, 0, R), Tile::new_tile(24, 2, 0, L | R)};
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_VerticalLinesOutsideViewport, reporter) {
    std::vector<Line> lines = {
            {{1.0f, -5.0f}, {1.0f, -1.0f}},
            {{1.0f, F_V_DIM + 1.0f}, {1.0f, F_V_DIM + 5.0f}},
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, {});
}

DEF_TEST(GraphiteTiler_VerticalLineCrossingTopViewport, reporter) {
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
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_VerticalLineCrossingBotViewport, reporter) {
    std::vector<Line> lines = {
            {{1.0f, F_V_DIM - 1.0f}, {1.0f, F_V_DIM + 5.0f}},
            {{1.0f, F_V_DIM - 5.0f}, {1.0f, F_V_DIM + 5.0f}},
    };

    std::vector<Tile> expected = {
            Tile::new_tile(0, 24, 0, B),
            Tile::new_tile(0, 23, 1, B),
            Tile::new_tile(0, 24, 1, W | T | B),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_ClipTopLeftCorner, reporter) {
    std::vector<Line> lines = {{{-1.0f, 2.0f}, {2.0f, -1.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(0, 0, 0, W | L | T)};
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_ClipBottomRightCorner, reporter) {
    std::vector<Line> lines = {
            {{F_V_DIM + 1.0f, F_V_DIM - 2.0f}, {F_V_DIM - 2.0f, F_V_DIM + 1.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(24, 24, 0, R | B)};
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

//==============================================================================================
// Axis-aligned lines
//==============================================================================================

DEF_TEST(GraphiteTiler_HorizontalLineLeftToRightThreeTile, reporter) {
    std::vector<Line> lines = {{{1.5f, 1.0f}, {8.5f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, R | L),
            Tile::new_tile(2, 0, 0, L),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_HorizontalLineRightToLeftThreeTile, reporter) {
    std::vector<Line> lines = {{{8.5f, 1.0f}, {1.5f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, R | L),
            Tile::new_tile(2, 0, 0, L),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_HorizontalLineMultiTile, reporter) {
    std::vector<Line> lines = {{{1.5f, 1.0f}, {12.5f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, R | L),
            Tile::new_tile(2, 0, 0, R | L),
            Tile::new_tile(3, 0, 0, L),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_VerticalLineDownThreeTile, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.5f}, {1.0f, 8.5f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, B),
            Tile::new_tile(0, 1, 0, W | T | B),
            Tile::new_tile(0, 2, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_VerticalLineDownMultiTile, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {1.0f, 13.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, B),
            Tile::new_tile(0, 1, 0, W | T | B),
            Tile::new_tile(0, 2, 0, W | T | B),
            Tile::new_tile(0, 3, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_VerticalLineUpThreeTile, reporter) {
    std::vector<Line> lines = {{{1.0f, 13.0f}, {1.0f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, B),
            Tile::new_tile(0, 1, 0, W | T | B),
            Tile::new_tile(0, 2, 0, W | T | B),
            Tile::new_tile(0, 3, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_VerticalLineUpMultiTile, reporter) {
    std::vector<Line> lines = {{{1.0f, 8.5f}, {1.0f, 1.5f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, B),
            Tile::new_tile(0, 1, 0, W | T | B),
            Tile::new_tile(0, 2, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_VerticalLineTouchingBot, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {1.0f, 8.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, B),
            Tile::new_tile(0, 1, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_VerticalLineTouchingTop, reporter) {
    std::vector<Line> lines = {{{1.0f, 0.0f}, {1.0f, 7.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | B),
            Tile::new_tile(0, 1, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

//==============================================================================================
// Sloped Lines
//==============================================================================================

DEF_TEST(GraphiteTiler_TopLeftToBottomRight, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {11.0f, 9.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, L | B),
            Tile::new_tile(1, 1, 0, W | R | T),
            Tile::new_tile(2, 1, 0, L | B),
            Tile::new_tile(2, 2, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_BottomRightToTopLeft, reporter) {
    std::vector<Line> lines = {{{11.0f, 9.0f}, {1.0f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, L | B),
            Tile::new_tile(1, 1, 0, W | R | T),
            Tile::new_tile(2, 1, 0, L | B),
            Tile::new_tile(2, 2, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_BottomLeftToTopRight, reporter) {
    std::vector<Line> lines = {{{2.0f, 11.0f}, {14.0f, 6.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(2, 1, 0, R | B),
            Tile::new_tile(3, 1, 0, L),
            Tile::new_tile(0, 2, 0, R),
            Tile::new_tile(1, 2, 0, R | L),
            Tile::new_tile(2, 2, 0, W | L | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_TopRightToBottomLeft, reporter) {
    std::vector<Line> lines = {{{14.0f, 6.0f}, {2.0f, 11.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(2, 1, 0, R | B),
            Tile::new_tile(3, 1, 0, L),
            Tile::new_tile(0, 2, 0, R),
            Tile::new_tile(1, 2, 0, R | L),
            Tile::new_tile(2, 2, 0, W | L | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_TwoLinesInSingleTile, reporter) {
    std::vector<Line> lines = {
            {{1.0f, 3.0f}, {3.0f, 3.0f}},
            {{3.0f, 3.0f}, {0.0f, 1.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, 0),
            Tile::new_tile(0, 0, 1, 0),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_DiagonalCrossCorner, reporter) {
    std::vector<Line> lines = {{{3.0f, 5.0f}, {5.0f, 3.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(1, 0, 0, L),
            Tile::new_tile(0, 1, 0, R),
            Tile::new_tile(1, 1, 0, W | L | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_DiagonalCrossCornerTwo, reporter) {
    std::vector<Line> lines = {{{7.9f, 7.9f}, {0.1f, 0.1f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, L),
            Tile::new_tile(1, 1, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_DiagonalDownSlopeTiles, reporter) {
    std::vector<Line> lines = {{{5.0f, 5.0f}, {9.0f, 9.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(1, 1, 0, R),
            Tile::new_tile(2, 1, 0, L),
            Tile::new_tile(2, 2, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_DiagonalUpSlopeTiles, reporter) {
    std::vector<Line> lines = {{{5.0f, 9.0f}, {9.0f, 5.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(1, 1, 0, R | B),
            Tile::new_tile(2, 1, 0, L),
            Tile::new_tile(1, 2, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_DiagonalDownOneTile, reporter) {
    std::vector<Line> lines = {{{0.0f, 0.0f}, {4.0f, 4.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | R),
            Tile::new_tile(1, 0, 0, L),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_DiagonalUpOneTile, reporter) {
    std::vector<Line> lines = {{{0.0f, 4.0f}, {4.0f, 0.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, W | L),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_DiagonalDownTwoTile, reporter) {
    std::vector<Line> lines = {{{0.0f, 0.0f}, {8.0f, 8.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | R),
            Tile::new_tile(1, 0, 0, L),
            Tile::new_tile(1, 1, 0, W | R | T),
            Tile::new_tile(2, 1, 0, L),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_DiagonalUpTwoTile, reporter) {
    std::vector<Line> lines = {{{0.0f, 8.0f}, {8.0f, 0.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(1, 0, 0, R | L),
            Tile::new_tile(2, 0, 0, W | L),
            Tile::new_tile(0, 1, 0, R),
            Tile::new_tile(1, 1, 0, W | L | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SlopedEndingRight, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {8.0f, 2.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, R | L),
            Tile::new_tile(2, 0, 0, L),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SlopedTouchingTop, reporter) {
    std::vector<Line> lines = {{{0.0f, 8.0f}, {4.0f, 0.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R | B),
            Tile::new_tile(1, 0, 0, W | L),
            Tile::new_tile(0, 1, 0, W | T),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SlopedTouchingBot, reporter) {
    std::vector<Line> lines = {{{0.0f, 0.0f}, {4.0f, 8.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W | B),
            Tile::new_tile(0, 1, 0, W | R | T),
            Tile::new_tile(1, 1, 0, L),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

//==============================================================================================
// Same Tile Cases
//==============================================================================================

DEF_TEST(GraphiteTiler_SameTile, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {3.0f, 3.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(0, 0, 0, 0)};
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SameTileLeft, reporter) {
    std::vector<Line> lines = {{{0.0f, 1.0f}, {3.0f, 1.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(0, 0, 0, 0)};
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SameTileTop, reporter) {
    std::vector<Line> lines = {{{1.0f, 0.0f}, {1.0f, 3.0f}}};
    std::vector<Tile> expected = {Tile::new_tile(0, 0, 0, W)};
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SameTileRight, reporter) {
    std::vector<Line> lines = {{{1.0f, 1.0f}, {4.0f, 1.0f}}};
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, R),
            Tile::new_tile(1, 0, 0, L),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SameTileBottom, reporter) {
    std::vector<Line> lines = {
            {{1.0f, 1.0f}, {1.0f, 4.0f}},
            {{1.0f, 1.0f}, {2.0f, 4.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, 0),
            Tile::new_tile(0, 0, 1, 0),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_SameTileTopLeft, reporter) {
    std::vector<Line> lines = {
            {{0.0f, 1.0f}, {1.0f, 0.0f}},
            {{0.0f, 0.0001f}, {0.0001f, 0.0f}},
    };
    std::vector<Tile> expected = {
            Tile::new_tile(0, 0, 0, W),
            Tile::new_tile(0, 0, 1, W),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_ExactRightEdgeCoincidence, reporter) {
    std::vector<Line> lines = {{{4.0f, 1.0f}, {4.0f, 3.0f}}};
    std::vector<Tile> expected = {
        Tile::new_tile(1, 0, 0, 0),
    };
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

DEF_TEST(GraphiteTiler_ExactBottomEdgeCoincidence, reporter) {
    std::vector<Line> lines = {{{1.0f, 4.0f}, {3.0f, 4.0f}}};
    std::vector<Tile> expected = {};
    Tiles tiles;
    check_tiles_match(reporter, tiles, lines, expected);
}

//==============================================================================================
// Miscellaneous Cases
//==============================================================================================

DEF_TEST(GraphiteTiler_InfiniteLoop, reporter) {
    std::vector<Line> lines = {{{22.0f, 552.0f}, {224.0f, 388.0f}}};
    Tiles tiles;
    // Just verify it doesn't hang/crash
    tiles.make_tiles_msaa(lines, 600, 600);
}

DEF_TEST(GraphiteTiler_Overflow, reporter) {
    std::vector<Line> lines = {{{59.60001f, 40.78f}, {520599.6f, 100.18f}}};
    Tiles tiles;
    // Verify it handles large coordinates without crashing
    tiles.make_tiles_msaa(lines, 200, 100);
}

DEF_TEST(GraphiteTiler_SortTest, reporter) {
    std::vector<Line> lines;
    Tiles tiles;

    float step = 4.0f;
    float y = F_V_DIM - 10.0f;
    while (y > 10.0f) {
        lines.push_back({{F_V_DIM - 10.0f, y}, {10.0f, y}});
        lines.push_back({{F_V_DIM - 12.0f, y}, {12.0f, y}});
        y -= step;
    }

    tiles.make_tiles_msaa(lines, VIEW_DIM, VIEW_DIM);
    const auto& buf = tiles.get_tiles();

    REPORTER_ASSERT(reporter, !buf.empty());
    REPORTER_ASSERT(reporter, buf.front().y > buf.back().y);

    tiles.sort_tiles();
    check_sorted(reporter, tiles.get_tiles());
}
