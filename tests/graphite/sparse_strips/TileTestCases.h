/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TileTestCases_DEFINED
#define skgpu_graphite_TileTestCases_DEFINED

#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include <vector>

namespace skgpu::graphite {

namespace TileTestCases {

inline SkTDArray<Line> scale_lines(const SkTDArray<Line>& input, float scale) {
    if (scale == 1.0f) {
        return input;
    }
    SkTDArray<Line> scaled;
    scaled.reserve(input.size());
    for (const auto& line : input) {
        scaled.push_back({{line.p0.fX * scale, line.p0.fY * scale},
                          {line.p1.fX * scale, line.p1.fY * scale}});
    }
    return scaled;
}

struct TileTestCase {
    const char* name;
    SkTDArray<Line> lines;
    SkTDArray<Tile> expected;
};

// TODO (thomsmit): update this when we put the constants into the header
struct TestCases : public IntersectionBits {
    static std::vector<TileTestCase> Get() {
        constexpr float kUnscaledDim = 100.0f;
        return {
            {
                "CulledLines",
                {
                    {{1.0f, -7.0f}, {3.0f, -1.0f}},
                    {{1.0f, -11.0f}, {3.0f, -1.0f}},
                    {{kUnscaledDim + 1.0f, 50.0f}, {kUnscaledDim + 3.0f, 70.0f}},
                    {{1.0f, kUnscaledDim + 1.0f}, {3.0f, kUnscaledDim + 7.0f}},
                    {{1.0f, kUnscaledDim + 1.0f}, {3.0f, kUnscaledDim + 13.0f}},
                },
                {}
            },
            {
                "SlopedLineCrossingTop",
                {
                    {{-2.0f, -3.0f}, {2.0f, 1.0f}},
                    {{6.0f, -1.0f}, {5.0f, 2.0f}},
                    {{9.0f, -10.0f}, {10.0f, 3.0f}},
                    {{2.0f, 1.0f}, {-2.0f, -3.0f}},
                },
                {
                    Tile(0, 0, 0, W | T),
                    Tile(1, 0, 1, W | T),
                    Tile(2, 0, 2, W | T),
                    Tile(0, 0, 3, W | T),
                }
            },
            {
                "SlopedLineCrossingBot",
                {
                    {{5.0f, kUnscaledDim + 3.0f}, {6.0f, kUnscaledDim - 2.0f}},
                    {{10.0f, kUnscaledDim + 1.0f}, {9.0f, kUnscaledDim - 1.0f}},
                    {{2.0f, kUnscaledDim - 2.0f}, {3.0f, kUnscaledDim + 3.0f}},
                },
                {
                    Tile(1, 24, 0, B),
                    Tile(2, 24, 1, B),
                    Tile(0, 24, 2, B),
                }
            },
            {
                "SlopedLineCrossingTopMultiTile",
                {
                    {{1.0f, -5.0f}, {6.0f, 7.0f}},
                    {{2.5f, -10.0f}, {3.5f, 6.0f}},
                },
                {
                    Tile(0, 0, 0, W | T | R),
                    Tile(1, 0, 0, L | B),
                    Tile(1, 1, 0, W | T),
                    Tile(0, 0, 1, W | T | B),
                    Tile(0, 1, 1, W | T),
                }
            },
            {
                "SlopedLineCrossingBotMultiTile",
                {
                    {{12.0f, kUnscaledDim + 10.0f}, {2.0f, 94.0f}},
                    {{1.5f, kUnscaledDim + 5.0f}, {3.5f, 94.0f}},
                },
                {
                    Tile(0, 23, 0, B),
                    Tile(0, 24, 0, W | T | R),
                    Tile(1, 24, 0, B | L),
                    Tile(0, 23, 1, B),
                    Tile(0, 24, 1, W | T | B),
                }
            },
            {
                "SlopedLineCrossingRight",
                {
                    {{97.0f, 1.0f}, {kUnscaledDim + 1.0f, 2.0f}},
                    {{93.0f, 1.0f}, {kUnscaledDim + 5.0f, 2.0f}},
                },
                {
                    Tile(24, 0, 0, R),
                    Tile(23, 0, 1, R),
                    Tile(24, 0, 1, R | L),
                }
            },
            {
                "SlopedLineCrossingLeft",
                {
                    {{-5.0f, 1.0f}, {1.0f, 2.0f}},
                    {{-5.0f, 1.0f}, {5.0f, 2.0f}},
                    {{-5.0f, 1.0f}, {13.0f, 9.0f}},
                },
                {
                    Tile(0, 0, 0, L),
                    Tile(0, 0, 1, L | R),
                    Tile(1, 0, 1, L),
                    Tile(0, 0, 2, L | B),
                    Tile(0, 1, 2, W | R | T),
                    Tile(1, 1, 2, R | L),
                    Tile(2, 1, 2, L | B),
                    Tile(2, 2, 2, W | R | T),
                    Tile(3, 2, 2, L),
                }
            },
            {
                "HorizontalLineAboveViewport",
                {{{10.0f, -5.0f}, {90.0f, -5.0f}}},
                {}
            },
            {
                "HorizontalLineBelowViewport",
                {{{10.0f, kUnscaledDim + 5.0f}, {90.0f, kUnscaledDim + 5.0f}}},
                {}
            },
            {
                "HorizontalLineCrossingLeftViewport",
                {{{-10.0f, 10.0f}, {10.0f, 10.0f}}},
                {
                    Tile(0, 2, 0, L | R),
                    Tile(1, 2, 0, L | R),
                    Tile(2, 2, 0, L),
                }
            },
            {
                "HorizontalLineCrossingRightViewport",
                {{{kUnscaledDim - 5.0f, 10.0f}, {kUnscaledDim + 5.0f, 10.0f}}},
                {Tile(23, 2, 0, R), Tile(24, 2, 0, L | R)}
            },
            {
                "VerticalLinesOutsideViewport",
                {
                    {{1.0f, -5.0f}, {1.0f, -1.0f}},
                    {{1.0f, kUnscaledDim + 1.0f}, {1.0f, kUnscaledDim + 5.0f}},
                },
                {}
            },
            {
                "VerticalLineCrossingTopViewport",
                {
                    {{1.0f, -7.0f}, {1.0f, 3.0f}},
                    {{1.0f, -7.0f}, {1.0f, 7.0f}},
                    {{1.0f, -7.0f}, {1.0f, 8.0f}},
                },
                {
                    Tile(0, 0, 0, W | T),
                    Tile(0, 0, 1, W | B | T),
                    Tile(0, 1, 1, W | T),
                    Tile(0, 0, 2, W | B | T),
                    Tile(0, 1, 2, W | T),
                }
            },
            {
                "VerticalLineCrossingBotViewport",
                {
                    {{1.0f, kUnscaledDim - 1.0f}, {1.0f, kUnscaledDim + 5.0f}},
                    {{1.0f, kUnscaledDim - 5.0f}, {1.0f, kUnscaledDim + 5.0f}},
                },
                {
                    Tile(0, 24, 0, B),
                    Tile(0, 23, 1, B),
                    Tile(0, 24, 1, W | T | B),
                }
            },
            {
                "ClipTopLeftCorner",
                {{{-1.0f, 2.0f}, {2.0f, -1.0f}}},
                {Tile(0, 0, 0, W | L | T)}
            },
            {
                "ClipBottomRightCorner",
                {{{kUnscaledDim + 1.0f, kUnscaledDim - 2.0f},
                  {kUnscaledDim - 2.0f, kUnscaledDim + 1.0f}}},
                {Tile(24, 24, 0, R | B)}
            },
            {
                "HorizontalLineLeftToRightThreeTile",
                {{{1.5f, 1.0f}, {8.5f, 1.0f}}},
                {
                    Tile(0, 0, 0, R),
                    Tile(1, 0, 0, R | L),
                    Tile(2, 0, 0, L),
                }
            },
            {
                "HorizontalLineRightToLeftThreeTile",
                {{{8.5f, 1.0f}, {1.5f, 1.0f}}},
                {
                    Tile(0, 0, 0, R),
                    Tile(1, 0, 0, R | L),
                    Tile(2, 0, 0, L),
                }
            },
            {
                "HorizontalLineMultiTile",
                {{{1.5f, 1.0f}, {12.5f, 1.0f}}},
                {
                    Tile(0, 0, 0, R),
                    Tile(1, 0, 0, R | L),
                    Tile(2, 0, 0, R | L),
                    Tile(3, 0, 0, L),
                }
            },
            {
                "VerticalLineDownThreeTile",
                {{{1.0f, 1.5f}, {1.0f, 8.5f}}},
                {
                    Tile(0, 0, 0, B),
                    Tile(0, 1, 0, W | T | B),
                    Tile(0, 2, 0, W | T),
                }
            },
            {
                "VerticalLineDownMultiTile",
                {{{1.0f, 1.0f}, {1.0f, 13.0f}}},
                {
                    Tile(0, 0, 0, B),
                    Tile(0, 1, 0, W | T | B),
                    Tile(0, 2, 0, W | T | B),
                    Tile(0, 3, 0, W | T),
                }
            },
            {
                "VerticalLineUpThreeTile",
                {{{1.0f, 13.0f}, {1.0f, 1.0f}}},
                {
                    Tile(0, 0, 0, B),
                    Tile(0, 1, 0, W | T | B),
                    Tile(0, 2, 0, W | T | B),
                    Tile(0, 3, 0, W | T),
                }
            },
            {
                "VerticalLineUpMultiTile",
                {{{1.0f, 8.5f}, {1.0f, 1.5f}}},
                {
                    Tile(0, 0, 0, B),
                    Tile(0, 1, 0, W | T | B),
                    Tile(0, 2, 0, W | T),
                }
            },
            {
                "VerticalLineTouchingBot",
                {{{1.0f, 1.0f}, {1.0f, 8.0f}}},
                {
                    Tile(0, 0, 0, B),
                    Tile(0, 1, 0, W | T),
                }
            },
            {
                "VerticalLineTouchingTop",
                {{{1.0f, 0.0f}, {1.0f, 7.0f}}},
                {
                    Tile(0, 0, 0, W | B),
                    Tile(0, 1, 0, W | T),
                }
            },
            {
                "VerticalLineTouchingLeft",
                {{{4.0f, 0.0f}, {4.0f, 7.0f}}},
                {
                    Tile(1, 0, 0, W | B),
                    Tile(1, 1, 0, W | T),
                }
            },
            {
                "TopLeftToBottomRight",
                {{{1.0f, 1.0f}, {11.0f, 9.0f}}},
                {
                    Tile(0, 0, 0, R),
                    Tile(1, 0, 0, L | B),
                    Tile(1, 1, 0, W | R | T),
                    Tile(2, 1, 0, L | B),
                    Tile(2, 2, 0, W | T),
                }
            },
            {
                "BottomRightToTopLeft",
                {{{11.0f, 9.0f}, {1.0f, 1.0f}}},
                {
                    Tile(0, 0, 0, R),
                    Tile(1, 0, 0, L | B),
                    Tile(1, 1, 0, W | R | T),
                    Tile(2, 1, 0, L | B),
                    Tile(2, 2, 0, W | T),
                }
            },
            {
                "BottomLeftToTopRight",
                {{{2.0f, 11.0f}, {14.0f, 6.0f}}},
                {
                    Tile(2, 1, 0, R | B),
                    Tile(3, 1, 0, L),
                    Tile(0, 2, 0, R),
                    Tile(1, 2, 0, R | L),
                    Tile(2, 2, 0, W | L | T),
                }
            },
            {
                "TopRightToBottomLeft",
                {{{14.0f, 6.0f}, {2.0f, 11.0f}}},
                {
                    Tile(2, 1, 0, R | B),
                    Tile(3, 1, 0, L),
                    Tile(0, 2, 0, R),
                    Tile(1, 2, 0, R | L),
                    Tile(2, 2, 0, W | L | T),
                }
            },
            {
                "TwoLinesInSingleTile",
                {
                    {{1.0f, 3.0f}, {3.0f, 3.0f}},
                    {{3.0f, 3.0f}, {0.0f, 1.0f}},
                },
                {
                    Tile(0, 0, 0, 0),
                    Tile(0, 0, 1, 0),
                }
            },
            {
                "DiagonalCrossCorner",
                {{{3.0f, 5.0f}, {5.0f, 3.0f}}},
                {
                    Tile(1, 0, 0, L),
                    Tile(0, 1, 0, R),
                    Tile(1, 1, 0, W | L | T),
                }
            },
            {
                "DiagonalCrossCornerTwo",
                {{{7.9f, 7.9f}, {0.1f, 0.1f}}},
                {
                    Tile(0, 0, 0, R),
                    Tile(1, 0, 0, L),
                    Tile(1, 1, 0, W | T),
                }
            },
            {
                "DiagonalDownSlopeTiles",
                {{{5.0f, 5.0f}, {9.0f, 9.0f}}},
                {
                    Tile(1, 1, 0, R),
                    Tile(2, 1, 0, L),
                    Tile(2, 2, 0, W | T),
                }
            },
            {
                "DiagonalUpSlopeTiles",
                {{{5.0f, 9.0f}, {9.0f, 5.0f}}},
                {
                    Tile(1, 1, 0, R | B),
                    Tile(2, 1, 0, L),
                    Tile(1, 2, 0, W | T),
                }
            },
            {
                "DiagonalDownOneTile",
                {{{0.0f, 0.0f}, {4.0f, 4.0f}}},
                {
                    Tile(0, 0, 0, W | R),
                    Tile(1, 0, 0, L),
                }
            },
            {
                "DiagonalUpOneTile",
                {{{0.0f, 4.0f}, {4.0f, 0.0f}}},
                {
                    Tile(0, 0, 0, R),
                    Tile(1, 0, 0, W | L),
                }
            },
            {
                "DiagonalDownTwoTile",
                {{{0.0f, 0.0f}, {8.0f, 8.0f}}},
                {
                    Tile(0, 0, 0, W | R),
                    Tile(1, 0, 0, L),
                    Tile(1, 1, 0, W | R | T),
                    Tile(2, 1, 0, L),
                }
            },
            {
                "DiagonalUpTwoTile",
                {{{0.0f, 8.0f}, {8.0f, 0.0f}}},
                {
                    Tile(1, 0, 0, R | L),
                    Tile(2, 0, 0, W | L),
                    Tile(0, 1, 0, R),
                    Tile(1, 1, 0, W | L | T),
                }
            },
            {
                "SlopedEndingRight",
                {{{1.0f, 1.0f}, {8.0f, 2.0f}}},
                {
                    Tile(0, 0, 0, R),
                    Tile(1, 0, 0, R | L),
                    Tile(2, 0, 0, L),
                }
            },
            {
                "SlopedTouchingTop",
                {{{0.0f, 8.0f}, {4.0f, 0.0f}}},
                {
                    Tile(0, 0, 0, R | B),
                    Tile(1, 0, 0, W | L),
                    Tile(0, 1, 0, W | T),
                }
            },
            {
                "SlopedTouchingBot",
                {{{0.0f, 0.0f}, {4.0f, 8.0f}}},
                {
                    Tile(0, 0, 0, W | B),
                    Tile(0, 1, 0, W | R | T),
                    Tile(1, 1, 0, L),
                }
            },
            {
                "SlopedDownThree",
                {{{1.0f, 1.0f}, {5.0f, 11.0f}}},
                {
                    Tile(0, 0, 0, B),
                    Tile(0, 1, 0, W | T | B),
                    Tile(0, 2, 0, W | T | R),
                    Tile(1, 2, 0, L),
                }
            },
            {
                "SameTile",
                {{{1.0f, 1.0f}, {3.0f, 3.0f}}},
                {Tile(0, 0, 0, 0)}
            },
            {
                "SameTileLeft",
                {{{0.0f, 1.0f}, {3.0f, 1.0f}}},
                {Tile(0, 0, 0, 0)}
            },
            {
                "SameTileTop",
                {{{1.0f, 0.0f}, {1.0f, 3.0f}}},
                {Tile(0, 0, 0, W)}
            },
            {
                "SameTileRight",
                {{{1.0f, 1.0f}, {4.0f, 1.0f}}},
                {
                    Tile(0, 0, 0, R),
                    Tile(1, 0, 0, L),
                }
            },
            {
                "SameTileBottom",
                {
                    {{1.0f, 1.0f}, {1.0f, 4.0f}},
                    {{1.0f, 1.0f}, {2.0f, 4.0f}},
                },
                {
                    Tile(0, 0, 0, 0),
                    Tile(0, 0, 1, 0),
                }
            },
            {
                "SameTileTopLeft",
                {
                    {{0.0f, 1.0f}, {1.0f, 0.0f}},
                    {{0.0f, 0.0001f}, {0.0001f, 0.0f}},
                },
                {
                    Tile(0, 0, 0, W),
                    Tile(0, 0, 1, W),
                }
            },
            {
                "ExactRightEdgeCoincidence",
                {{{4.0f, 1.0f}, {4.0f, 3.0f}}},
                {Tile(1, 0, 0, 0)}
            },
            {
                "ExactBottomEdgeCoincidence",
                {{{1.0f, 4.0f}, {3.0f, 4.0f}}},
                {}
            },
            {
                "Tricky Precision",
                {{{0.000000, 15.856406}, {8.000000, 2.000000}}},
                {
                    Tile(1, 0, 0, B | R),
                    Tile(2, 0, 0, L),
                    Tile(1, 1, 0, W | T | B),
                    Tile(0, 2, 0, B | R),
                    Tile(1, 2, 0, W | T | L),
                    Tile(0, 3, 0, W | T)
                }
            }
        };
    }
};

} // namespace TileTestCases

} // namespace skgpu::graphite

#endif // skgpu_graphite_TileTestCases_DEFINED
