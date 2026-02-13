/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <cmath>
#include <sstream>
#include <vector>

using namespace skgpu::graphite;

namespace {

template <uint16_t WIDTH, uint16_t HEIGHT> struct TestRunner : IntersectionBits {
    using TileType = Tile<WIDTH, HEIGHT>;
    using TilesType = Tiles<WIDTH, HEIGHT>;
    static_assert(WIDTH == HEIGHT); // only support square tiles for now
    static constexpr float SCALE = (float)WIDTH / 4.0f;
    static constexpr uint16_t VIEW_DIM = (uint16_t)(100 * SCALE);
    static constexpr float F_V_DIM = (float)VIEW_DIM;

    static std::vector<Line> scale_lines(const std::vector<Line>& input) {
        if (SCALE == 1.0f) return input;
        std::vector<Line> scaled;
        scaled.reserve(input.size());
        for (const auto& l : input) {
            scaled.push_back({{l.p0.x * SCALE, l.p0.y * SCALE}, {l.p1.x * SCALE, l.p1.y * SCALE}});
        }
        return scaled;
    }

    static void check_tiles_match(skiatest::Reporter* reporter,
                                  TilesType& tiles,
                                  const std::vector<Line>& rawLines,
                                  const std::vector<TileType>& expected,
                                  const char* testName) {
        tiles.reset();

        std::vector<Line> lines = scale_lines(rawLines);
        tiles.makeTilesMSAA(lines, VIEW_DIM, VIEW_DIM);
        const std::vector<TileType>& actual = tiles.getTiles();

        REPORTER_ASSERT(reporter,
                        actual.size() == expected.size(),
                        "[%s] Tile count mismatch. Expected %zu, got %zu",
                        testName,
                        expected.size(),
                        actual.size());

        size_t limit = std::min(actual.size(), expected.size());
        for (size_t i = 0; i < limit; ++i) {
            const TileType& got = actual[i];
            const TileType& want = expected[i];

            REPORTER_ASSERT(reporter,
                            got.x == want.x,
                            "[%s] Tile[%zu] X mismatch. Want %u, Got %u",
                            testName,
                            i,
                            want.x,
                            got.x);
            REPORTER_ASSERT(reporter,
                            got.y == want.y,
                            "[%s] Tile[%zu] Y mismatch. Want %u, Got %u",
                            testName,
                            i,
                            want.y,
                            got.y);

            uint32_t gotLine = got.packedWindingLineIdx >> TileType::INT_MASK_SHIFT;
            uint32_t wantLine = want.packedWindingLineIdx >> TileType::INT_MASK_SHIFT;
            REPORTER_ASSERT(reporter,
                            gotLine == wantLine,
                            "[%s] Tile[%zu] Line Index mismatch. Want %u, Got %u",
                            testName,
                            i,
                            wantLine,
                            gotLine);

            uint32_t gotMask = got.packedWindingLineIdx & TileType::INTERSECTION_MASK;
            uint32_t wantMask = want.packedWindingLineIdx & TileType::INTERSECTION_MASK;

            if (gotMask != wantMask) {
                std::string got_str = maskToString(gotMask);
                std::string want_str = maskToString(wantMask);
                ERRORF(reporter,
                       "[%s] Tile[%zu] Mask mismatch. Want [%s], Got [%s]",
                       testName,
                       i,
                       want_str.c_str(),
                       got_str.c_str());
            }
        }
    }

    static void check_sorted(skiatest::Reporter* reporter, const std::vector<TileType>& buf) {
        if (buf.empty()) return;

        for (size_t i = 0; i < buf.size() - 1; ++i) {
            const TileType& current = buf[i];
            const TileType& next = buf[i + 1];

            if (current.y > next.y) {
                ERRORF(reporter, "Sort Failure [Y]: Tile[%zu] > Tile[%zu]", i, i + 1);
            }
            if (current.y == next.y) {
                if (current.x > next.x) {
                    ERRORF(reporter, "Sort Failure [X]: Tile[%zu] > Tile[%zu]", i, i + 1);
                }
                if (current.x == next.x &&
                    current.packedWindingLineIdx > next.packedWindingLineIdx) {
                    ERRORF(reporter, "Sort Failure [Payload]: Tile[%zu] > Tile[%zu]", i, i + 1);
                }
            }
        }
    }

    static void RunAll(skiatest::Reporter* reporter) {
        TilesType tiles;

        const float BASE_V_DIM = 100.0f;
        {
            std::vector<Line> lines = {
                    {{1.0f, -7.0f}, {3.0f, -1.0f}},
                    {{1.0f, -11.0f}, {3.0f, -1.0f}},
                    {{BASE_V_DIM + 1.0f, 50.0f}, {BASE_V_DIM + 3.0f, 70.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {3.0f, BASE_V_DIM + 7.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {3.0f, BASE_V_DIM + 13.0f}},
            };
            check_tiles_match(reporter, tiles, lines, {}, "CulledLines");
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
            check_tiles_match(reporter, tiles, lines, expected, "SlopedLineCrossingTop");
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
            check_tiles_match(reporter, tiles, lines, expected, "SlopedLineCrossingBot");
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
            check_tiles_match(reporter, tiles, lines, expected, "SlopedLineCrossingTopMultiTile");
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
            check_tiles_match(reporter, tiles, lines, expected, "SlopedLineCrossingBotMultiTile");
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
            check_tiles_match(reporter, tiles, lines, expected, "SlopedLineCrossingRight");
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
            check_tiles_match(reporter, tiles, lines, expected, "SlopedLineCrossingLeft");
        }

        {
            std::vector<Line> lines = {{{10.0f, -5.0f}, {90.0f, -5.0f}}};
            check_tiles_match(reporter, tiles, lines, {}, "HorizontalLineAboveViewport");
        }

        {
            std::vector<Line> lines = {{{10.0f, BASE_V_DIM + 5.0f}, {90.0f, BASE_V_DIM + 5.0f}}};
            check_tiles_match(reporter, tiles, lines, {}, "HorizontalLineBelowViewport");
        }

        {
            std::vector<Line> lines = {{{-10.0f, 10.0f}, {10.0f, 10.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 2, 0, L | R),
                    TileType(1, 2, 0, L | R),
                    TileType(2, 2, 0, L),
            };
            check_tiles_match(
                    reporter, tiles, lines, expected, "HorizontalLineCrossingLeftViewport");
        }

        {
            std::vector<Line> lines = {{{BASE_V_DIM - 5.0f, 10.0f}, {BASE_V_DIM + 5.0f, 10.0f}}};
            std::vector<TileType> expected = {TileType(23, 2, 0, R),
                                              TileType(24, 2, 0, L | R)};
            check_tiles_match(
                    reporter, tiles, lines, expected, "HorizontalLineCrossingRightViewport");
        }

        {
            std::vector<Line> lines = {
                    {{1.0f, -5.0f}, {1.0f, -1.0f}},
                    {{1.0f, BASE_V_DIM + 1.0f}, {1.0f, BASE_V_DIM + 5.0f}},
            };
            check_tiles_match(reporter, tiles, lines, {}, "VerticalLinesOutsideViewport");
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
            check_tiles_match(reporter, tiles, lines, expected, "VerticalLineCrossingTopViewport");
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
            check_tiles_match(reporter, tiles, lines, expected, "VerticalLineCrossingBotViewport");
        }

        {
            std::vector<Line> lines = {{{-1.0f, 2.0f}, {2.0f, -1.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, W | L | T)};
            check_tiles_match(reporter, tiles, lines, expected, "ClipTopLeftCorner");
        }
        {
            std::vector<Line> lines = {{{BASE_V_DIM + 1.0f, BASE_V_DIM - 2.0f},
                                        {BASE_V_DIM - 2.0f, BASE_V_DIM + 1.0f}}};
            std::vector<TileType> expected = {TileType(24, 24, 0, R | B)};
            check_tiles_match(reporter, tiles, lines, expected, "ClipBottomRightCorner");
        }

        {
            check_tiles_match(reporter,
                              tiles,
                              {{{1.5f, 1.0f}, {8.5f, 1.0f}}},
                              {
                                      TileType(0, 0, 0, R),
                                      TileType(1, 0, 0, R | L),
                                      TileType(2, 0, 0, L),
                              },
                              "HorizontalLineLeftToRightThreeTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{8.5f, 1.0f}, {1.5f, 1.0f}}},
                              {
                                      TileType(0, 0, 0, R),
                                      TileType(1, 0, 0, R | L),
                                      TileType(2, 0, 0, L),
                              },
                              "HorizontalLineRightToLeftThreeTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.5f, 1.0f}, {12.5f, 1.0f}}},
                              {
                                      TileType(0, 0, 0, R),
                                      TileType(1, 0, 0, R | L),
                                      TileType(2, 0, 0, R | L),
                                      TileType(3, 0, 0, L),
                              },
                              "HorizontalLineMultiTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 1.5f}, {1.0f, 8.5f}}},
                              {
                                      TileType(0, 0, 0, B),
                                      TileType(0, 1, 0, W | T | B),
                                      TileType(0, 2, 0, W | T),
                              },
                              "VerticalLineDownThreeTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 1.0f}, {1.0f, 13.0f}}},
                              {
                                      TileType(0, 0, 0, B),
                                      TileType(0, 1, 0, W | T | B),
                                      TileType(0, 2, 0, W | T | B),
                                      TileType(0, 3, 0, W | T),
                              },
                              "VerticalLineDownMultiTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 13.0f}, {1.0f, 1.0f}}},
                              {
                                      TileType(0, 0, 0, B),
                                      TileType(0, 1, 0, W | T | B),
                                      TileType(0, 2, 0, W | T | B),
                                      TileType(0, 3, 0, W | T),
                              },
                              "VerticalLineUpThreeTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 8.5f}, {1.0f, 1.5f}}},
                              {
                                      TileType(0, 0, 0, B),
                                      TileType(0, 1, 0, W | T | B),
                                      TileType(0, 2, 0, W | T),
                              },
                              "VerticalLineUpMultiTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 1.0f}, {1.0f, 8.0f}}},
                              {
                                      TileType(0, 0, 0, B),
                                      TileType(0, 1, 0, W | T),
                              },
                              "VerticalLineTouchingBot");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 0.0f}, {1.0f, 7.0f}}},
                              {
                                      TileType(0, 0, 0, W | B),
                                      TileType(0, 1, 0, W | T),
                              },
                              "VerticalLineTouchingTop");
        }

        {
            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 1.0f}, {11.0f, 9.0f}}},
                              {
                                      TileType(0, 0, 0, R),
                                      TileType(1, 0, 0, L | B),
                                      TileType(1, 1, 0, W | R | T),
                                      TileType(2, 1, 0, L | B),
                                      TileType(2, 2, 0, W | T),
                              },
                              "TopLeftToBottomRight");

            check_tiles_match(reporter,
                              tiles,
                              {{{11.0f, 9.0f}, {1.0f, 1.0f}}},
                              {
                                      TileType(0, 0, 0, R),
                                      TileType(1, 0, 0, L | B),
                                      TileType(1, 1, 0, W | R | T),
                                      TileType(2, 1, 0, L | B),
                                      TileType(2, 2, 0, W | T),
                              },
                              "BottomRightToTopLeft");

            check_tiles_match(reporter,
                              tiles,
                              {{{2.0f, 11.0f}, {14.0f, 6.0f}}},
                              {
                                      TileType(2, 1, 0, R | B),
                                      TileType(3, 1, 0, L),
                                      TileType(0, 2, 0, R),
                                      TileType(1, 2, 0, R | L),
                                      TileType(2, 2, 0, W | L | T),
                              },
                              "BottomLeftToTopRight");

            check_tiles_match(reporter,
                              tiles,
                              {{{14.0f, 6.0f}, {2.0f, 11.0f}}},
                              {
                                      TileType(2, 1, 0, R | B),
                                      TileType(3, 1, 0, L),
                                      TileType(0, 2, 0, R),
                                      TileType(1, 2, 0, R | L),
                                      TileType(2, 2, 0, W | L | T),
                              },
                              "TopRightToBottomLeft");
        }

        {
            check_tiles_match(reporter,
                              tiles,
                              {
                                      {{1.0f, 3.0f}, {3.0f, 3.0f}},
                                      {{3.0f, 3.0f}, {0.0f, 1.0f}},
                              },
                              {
                                      TileType(0, 0, 0, 0),
                                      TileType(0, 0, 1, 0),
                              },
                              "TwoLinesInSingleTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{3.0f, 5.0f}, {5.0f, 3.0f}}},
                              {
                                      TileType(1, 0, 0, L),
                                      TileType(0, 1, 0, R),
                                      TileType(1, 1, 0, W | L | T),
                              },
                              "DiagonalCrossCorner");

            check_tiles_match(reporter,
                              tiles,
                              {{{7.9f, 7.9f}, {0.1f, 0.1f}}},
                              {
                                      TileType(0, 0, 0, R),
                                      TileType(1, 0, 0, L),
                                      TileType(1, 1, 0, W | T),
                              },
                              "DiagonalCrossCornerTwo");

            check_tiles_match(reporter,
                              tiles,
                              {{{5.0f, 5.0f}, {9.0f, 9.0f}}},
                              {
                                      TileType(1, 1, 0, R),
                                      TileType(2, 1, 0, L),
                                      TileType(2, 2, 0, W | T),
                              },
                              "DiagonalDownSlopeTiles");

            check_tiles_match(reporter,
                              tiles,
                              {{{5.0f, 9.0f}, {9.0f, 5.0f}}},
                              {
                                      TileType(1, 1, 0, R | B),
                                      TileType(2, 1, 0, L),
                                      TileType(1, 2, 0, W | T),
                              },
                              "DiagonalUpSlopeTiles");

            check_tiles_match(reporter,
                              tiles,
                              {{{0.0f, 0.0f}, {4.0f, 4.0f}}},
                              {
                                      TileType(0, 0, 0, W | R),
                                      TileType(1, 0, 0, L),
                              },
                              "DiagonalDownOneTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{0.0f, 4.0f}, {4.0f, 0.0f}}},
                              {
                                      TileType(0, 0, 0, R),
                                      TileType(1, 0, 0, W | L),
                              },
                              "DiagonalUpOneTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{0.0f, 0.0f}, {8.0f, 8.0f}}},
                              {
                                      TileType(0, 0, 0, W | R),
                                      TileType(1, 0, 0, L),
                                      TileType(1, 1, 0, W | R | T),
                                      TileType(2, 1, 0, L),
                              },
                              "DiagonalDownTwoTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{0.0f, 8.0f}, {8.0f, 0.0f}}},
                              {
                                      TileType(1, 0, 0, R | L),
                                      TileType(2, 0, 0, W | L),
                                      TileType(0, 1, 0, R),
                                      TileType(1, 1, 0, W | L | T),
                              },
                              "DiagonalUpTwoTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 1.0f}, {8.0f, 2.0f}}},
                              {
                                      TileType(0, 0, 0, R),
                                      TileType(1, 0, 0, R | L),
                                      TileType(2, 0, 0, L),
                              },
                              "SlopedEndingRight");

            check_tiles_match(reporter,
                              tiles,
                              {{{0.0f, 8.0f}, {4.0f, 0.0f}}},
                              {
                                      TileType(0, 0, 0, R | B),
                                      TileType(1, 0, 0, W | L),
                                      TileType(0, 1, 0, W | T),
                              },
                              "SlopedTouchingTop");

            check_tiles_match(reporter,
                              tiles,
                              {{{0.0f, 0.0f}, {4.0f, 8.0f}}},
                              {
                                      TileType(0, 0, 0, W | B),
                                      TileType(0, 1, 0, W | R | T),
                                      TileType(1, 1, 0, L),
                              },
                              "SlopedTouchingBot");
        }

        {
            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 1.0f}, {3.0f, 3.0f}}},
                              {TileType(0, 0, 0, 0)},
                              "SameTile");

            check_tiles_match(reporter,
                              tiles,
                              {{{0.0f, 1.0f}, {3.0f, 1.0f}}},
                              {TileType(0, 0, 0, 0)},
                              "SameTileLeft");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 0.0f}, {1.0f, 3.0f}}},
                              {TileType(0, 0, 0, W)},
                              "SameTileTop");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 1.0f}, {4.0f, 1.0f}}},
                              {
                                      TileType(0, 0, 0, R),
                                      TileType(1, 0, 0, L),
                              },
                              "SameTileRight");

            check_tiles_match(reporter,
                              tiles,
                              {
                                      {{1.0f, 1.0f}, {1.0f, 4.0f}},
                                      {{1.0f, 1.0f}, {2.0f, 4.0f}},
                              },
                              {
                                      TileType(0, 0, 0, 0),
                                      TileType(0, 0, 1, 0),
                              },
                              "SameTileBottom");

            check_tiles_match(reporter,
                              tiles,
                              {
                                      {{0.0f, 1.0f}, {1.0f, 0.0f}},
                                      {{0.0f, 0.0001f}, {0.0001f, 0.0f}},
                              },
                              {
                                      TileType(0, 0, 0, W),
                                      TileType(0, 0, 1, W),
                              },
                              "SameTileTopLeft");

            check_tiles_match(reporter,
                              tiles,
                              {{{4.0f, 1.0f}, {4.0f, 3.0f}}},
                              {TileType(1, 0, 0, 0)},
                              "ExactRightEdgeCoincidence");

            check_tiles_match(reporter,
                              tiles,
                              {{{1.0f, 4.0f}, {3.0f, 4.0f}}},
                              {},
                              "ExactBottomEdgeCoincidence");
        }

        {
            std::vector<Line> lines = {
                    {{22.0f * SCALE, 552.0f * SCALE}, {224.0f * SCALE, 388.0f * SCALE}}};
            tiles.reset();
            tiles.makeTilesMSAA(lines, (uint16_t)(600 * SCALE), (uint16_t)(600 * SCALE));

            lines = {{{59.60001f * SCALE, 40.78f * SCALE}, {520599.6f * SCALE, 100.18f * SCALE}}};
            tiles.reset();
            tiles.makeTilesMSAA(lines, (uint16_t)(200 * SCALE), (uint16_t)(100 * SCALE));
        }

        {
            std::vector<Line> lines;
            tiles.reset();

            float step = 4.0f * SCALE;
            float y = F_V_DIM - (10.0f * SCALE);
            float limit = 10.0f * SCALE;

            while (y > limit) {
                lines.push_back({{F_V_DIM - (10.0f * SCALE), y}, {10.0f * SCALE, y}});
                lines.push_back({{F_V_DIM - (12.0f * SCALE), y}, {12.0f * SCALE, y}});
                y -= step;
            }

            tiles.makeTilesMSAA(lines, VIEW_DIM, VIEW_DIM);
            const auto& buf = tiles.getTiles();

            REPORTER_ASSERT(reporter, !buf.empty(), "SortTest produced no tiles");
            if (!buf.empty()) {
                REPORTER_ASSERT(
                        reporter, buf.front().y > buf.back().y, "SortTest initial state invalid");
            }

            tiles.sortTiles();
            check_sorted(reporter, tiles.getTiles());
        }
    }
};

}  // namespace

DEF_TEST(GraphiteTiler_4x4, reporter) { TestRunner<4, 4>::RunAll(reporter); }

DEF_TEST(GraphiteTiler_8x8, reporter) { TestRunner<8, 8>::RunAll(reporter); }
