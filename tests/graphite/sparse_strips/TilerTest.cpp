/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRect.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <cmath>
#include <cstring>
#include <sstream>
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

Polyline make_polyline(const std::vector<Line>& lines) {
    Polyline p;
    for (const auto& l : lines) {
        p.appendPoint({l.p0.fX, l.p0.fY});
        p.appendPoint({l.p1.fX, l.p1.fY});
        p.appendSentinel();
    }
    return p;
}

void build_polyline_and_mapping(const std::vector<Line>& lines,
                                Polyline& outPolyline,
                                std::vector<uint32_t>& outMapping) {
    outPolyline.reset();
    outMapping.clear();
    outMapping.reserve(lines.size());
    for (const auto& l : lines) {
        outMapping.push_back(outPolyline.count());
        outPolyline.appendPoint({l.p0.fX, l.p0.fY});
        outPolyline.appendPoint({l.p1.fX, l.p1.fY});
        outPolyline.appendSentinel();
    }
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
void check_tiles_match(skiatest::Reporter* reporter,
                       Tiles<kTileWidth, kTileHeight>& tiles,
                       const std::vector<Line>& rawLines,
                       const std::vector<Tile<kTileWidth, kTileHeight>>& expected,
                       const char* testName,
                       float scale) {
    using TileType = Tile<kTileWidth, kTileHeight>;

    Polyline polyline;
    std::vector<uint32_t> expectedToActualIdx;
    std::vector<Line> lines = scale_lines(rawLines, scale);
    build_polyline_and_mapping(lines, polyline, expectedToActualIdx);

    uint16_t kScaledDim = static_cast<uint16_t>(100.0f * scale);
    int32_t numRows = (kScaledDim + kTileHeight - 1) / kTileHeight;
    WindingHistogram culled(numRows);
    culled.resize(numRows);
    culled.clear();

    SkIRect clip = SkIRect::MakeWH(kScaledDim, kScaledDim);

    tiles.reset();
    tiles.makeTilesMSAA(polyline, clip, culled);

    const std::vector<TileType>& actual = tiles.getTiles();

    if (actual.size() != expected.size()) {
        SkString dump;
        dump.appendf("\n--- [%s] TEST CASE DUMP ---\n", testName);

        dump.append("Lines:\n{\n");
        for (size_t i = 0; i < rawLines.size(); ++i) {
            const auto& l = rawLines[i];
            dump.appendf("    {{%gf, %gf}, {%gf, %gf}}%s\n",
                         l.p0.fX,
                         l.p0.fY,
                         l.p1.fX,
                         l.p1.fY,
                         (i < rawLines.size() - 1) ? "," : "");
        }
        dump.append("}\n\n");

        auto dumpTiles =
                [&](const std::vector<TileType>& tileArray, const char* label, bool isActual) {
                    dump.appendf("%s:\n", label);
                    for (size_t i = 0; i < tileArray.size(); ++i) {
                        const auto& t = tileArray[i];
                        uint32_t rawLine = t.packedWindingLineIdx >> TileType::INT_MASK_SHIFT;
                        uint32_t mask = t.packedWindingLineIdx & TileType::INTERSECTION_MASK;

                        uint32_t logicalLine = rawLine;
                        if (isActual) {
                            for (size_t j = 0; j < expectedToActualIdx.size(); ++j) {
                                if (expectedToActualIdx[j] == rawLine) {
                                    logicalLine = static_cast<uint32_t>(j);
                                    break;
                                }
                            }
                        }

                        std::string maskStr = IntersectionBits::maskToString(mask);
                        if (maskStr.empty()) {
                            maskStr = "0";
                        }

                        dump.appendf("    Tile (%u,%u) LineId %u IntersectionMask : %s\n",
                                     t.x,
                                     t.y,
                                     logicalLine,
                                     maskStr.c_str());
                    }
                    dump.append("\n");
                };

        dumpTiles(expected, "Expected", false);
        dumpTiles(actual, "Actual", true);

        dump.append("---------------------------\n");

        INFOF(reporter, "%s", dump.c_str());
    }

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
        uint32_t wantLogicalLine = want.packedWindingLineIdx >> TileType::INT_MASK_SHIFT;

        uint32_t wantLine = wantLogicalLine < expectedToActualIdx.size()
                                    ? expectedToActualIdx[wantLogicalLine]
                                    : wantLogicalLine;

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
            std::string got_str = IntersectionBits::maskToString(gotMask);
            std::string want_str = IntersectionBits::maskToString(wantMask);
            ERRORF(reporter,
                   "[%s] Tile[%zu] Mask mismatch. Want [%s], Got [%s]",
                   testName,
                   i,
                   want_str.c_str(),
                   got_str.c_str());
        }
    }
}

template <typename TileType>
void check_sorted(skiatest::Reporter* reporter, const std::vector<TileType>& buf) {
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
            if (current.x == next.x && current.packedWindingLineIdx > next.packedWindingLineIdx) {
                ERRORF(reporter, "Sort Failure [Payload]: Tile[%zu] > Tile[%zu]", i, i + 1);
            }
        }
    }
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
void check_culling(skiatest::Reporter* reporter,
                   Tiles<kTileWidth, kTileHeight>& tiles,
                   const std::vector<Line>& rawLines,
                   const std::vector<int16_t>& expectedCulled,
                   const char* testName,
                   float scale) {
    tiles.reset();

    Polyline polyline;
    std::vector<uint32_t> expectedToActualIdx;
    std::vector<Line> lines = scale_lines(rawLines, scale);
    build_polyline_and_mapping(lines, polyline, expectedToActualIdx);

    uint16_t kScaledDim = static_cast<uint16_t>(100.0f * scale);
    int32_t numRows = (kScaledDim + kTileHeight - 1) / kTileHeight;
    WindingHistogram culled(numRows);
    culled.resize(numRows);
    culled.clear();

    SkIRect clip = SkIRect::MakeWH(kScaledDim, kScaledDim);
    bool cullingEventOccurred = tiles.makeTilesMSAA(polyline, clip, culled);

    // If we expected culling data, we should have seen a culling event flag
    if (!expectedCulled.empty() && !cullingEventOccurred) {
        ERRORF(reporter,
               "[%s] Expected culling to occur, but makeTilesMSAA returned false.",
               testName);
    }

    for (size_t i = 0; i < expectedCulled.size(); ++i) {
        REPORTER_ASSERT(reporter,
                        culled[i] == expectedCulled[i],
                        "[%s] Row[%zu] Cull mismatch. Want %d, Got %d",
                        testName,
                        i,
                        expectedCulled[i],
                        culled[i]);
    }

    // Ensure the rest of the array remains untouched (0)
    for (size_t i = expectedCulled.size(); i < static_cast<size_t>(numRows); ++i) {
        REPORTER_ASSERT(reporter,
                        culled[i] == 0,
                        "[%s] Row[%zu] Cull mismatch. Want 0, Got %d",
                        testName,
                        i,
                        culled[i]);
    }
}

}  // namespace

template <uint16_t kTileWidth, uint16_t kTileHeight> class TileTestRunner : IntersectionBits {
public:
    using TileType = Tile<kTileWidth, kTileHeight>;
    using TilesType = Tiles<kTileWidth, kTileHeight>;

    static_assert(kTileWidth == kTileHeight);  // only support square tiles for now
    static constexpr float    kScale        = static_cast<float>(kTileWidth) / 4.0f;
    static constexpr float    kUnscaledDim  = 100.0f;
    static constexpr uint16_t kScaledDim    = static_cast<uint16_t>(kUnscaledDim * kScale);
    static constexpr float    kScaledDimF   = static_cast<float>(kScaledDim);

    static void runAll(skiatest::Reporter* reporter) {
        TilesType tiles;

        {
            std::vector<Line> lines = {
                    {{1.0f, -7.0f}, {3.0f, -1.0f}},
                    {{1.0f, -11.0f}, {3.0f, -1.0f}},
                    {{kUnscaledDim + 1.0f, 50.0f}, {kUnscaledDim + 3.0f, 70.0f}},
                    {{1.0f, kUnscaledDim + 1.0f}, {3.0f, kUnscaledDim + 7.0f}},
                    {{1.0f, kUnscaledDim + 1.0f}, {3.0f, kUnscaledDim + 13.0f}},
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "CulledLines", kScale);
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
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "SlopedLineCrossingTop", kScale);
        }

        {
            std::vector<Line> lines = {
                    {{5.0f, kUnscaledDim + 3.0f}, {6.0f, kUnscaledDim - 2.0f}},
                    {{10.0f, kUnscaledDim + 1.0f}, {9.0f, kUnscaledDim - 1.0f}},
                    {{2.0f, kUnscaledDim - 2.0f}, {3.0f, kUnscaledDim + 3.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(1, 24, 0, B),
                    TileType(2, 24, 1, B),
                    TileType(0, 24, 2, B),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "SlopedLineCrossingBot", kScale);
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
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "SlopedLineCrossingTopMultiTile", kScale);
        }

        {
            std::vector<Line> lines = {
                    {{12.0f, kUnscaledDim + 10.0f}, {2.0f, 94.0f}},
                    {{1.5f, kUnscaledDim + 5.0f}, {3.5f, 94.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 23, 0, B),
                    TileType(0, 24, 0, W | T | R),
                    TileType(1, 24, 0, B | L),
                    TileType(0, 23, 1, B),
                    TileType(0, 24, 1, W | T | B),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "SlopedLineCrossingBotMultiTile", kScale);
        }

        {
            std::vector<Line> lines = {
                    {{97.0f, 1.0f}, {kUnscaledDim + 1.0f, 2.0f}},
                    {{93.0f, 1.0f}, {kUnscaledDim + 5.0f, 2.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(24, 0, 0, R),
                    TileType(23, 0, 1, R),
                    TileType(24, 0, 1, R | L),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "SlopedLineCrossingRight", kScale);
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
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "SlopedLineCrossingLeft", kScale);
        }

        {
            std::vector<Line> lines = {{{10.0f, -5.0f}, {90.0f, -5.0f}}};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "HorizontalLineAboveViewport", kScale);
        }

        {
            std::vector<Line> lines = {
                    {{10.0f, kUnscaledDim + 5.0f}, {90.0f, kUnscaledDim + 5.0f}}};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "HorizontalLineBelowViewport", kScale);
        }

        {
            std::vector<Line> lines = {{{-10.0f, 10.0f}, {10.0f, 10.0f}}};
            std::vector<TileType> expected = {
                    TileType(0, 2, 0, L | R),
                    TileType(1, 2, 0, L | R),
                    TileType(2, 2, 0, L),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "HorizontalLineCrossingLeftViewport", kScale);
        }

        {
            std::vector<Line> lines = {
                    {{kUnscaledDim - 5.0f, 10.0f}, {kUnscaledDim + 5.0f, 10.0f}}};
            std::vector<TileType> expected = {TileType(23, 2, 0, R), TileType(24, 2, 0, L | R)};
            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       lines,
                                                       expected,
                                                       "HorizontalLineCrossingRightViewport",
                                                       kScale);
        }

        {
            std::vector<Line> lines = {
                    {{1.0f, -5.0f}, {1.0f, -1.0f}},
                    {{1.0f, kUnscaledDim + 1.0f}, {1.0f, kUnscaledDim + 5.0f}},
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "VerticalLinesOutsideViewport", kScale);
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
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "VerticalLineCrossingTopViewport", kScale);
        }

        {
            std::vector<Line> lines = {
                    {{1.0f, kUnscaledDim - 1.0f}, {1.0f, kUnscaledDim + 5.0f}},
                    {{1.0f, kUnscaledDim - 5.0f}, {1.0f, kUnscaledDim + 5.0f}},
            };
            std::vector<TileType> expected = {
                    TileType(0, 24, 0, B),
                    TileType(0, 23, 1, B),
                    TileType(0, 24, 1, W | T | B),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "VerticalLineCrossingBotViewport", kScale);
        }

        {
            std::vector<Line> lines = {{{-1.0f, 2.0f}, {2.0f, -1.0f}}};
            std::vector<TileType> expected = {TileType(0, 0, 0, W | L | T)};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "ClipTopLeftCorner", kScale);
        }
        {
            std::vector<Line> lines = {{{kUnscaledDim + 1.0f, kUnscaledDim - 2.0f},
                                        {kUnscaledDim - 2.0f, kUnscaledDim + 1.0f}}};
            std::vector<TileType> expected = {TileType(24, 24, 0, R | B)};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expected, "ClipBottomRightCorner", kScale);
        }

        {
            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.5f, 1.0f}, {8.5f, 1.0f}}},
                                                       {
                                                               TileType(0, 0, 0, R),
                                                               TileType(1, 0, 0, R | L),
                                                               TileType(2, 0, 0, L),
                                                       },
                                                       "HorizontalLineLeftToRightThreeTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{8.5f, 1.0f}, {1.5f, 1.0f}}},
                                                       {
                                                               TileType(0, 0, 0, R),
                                                               TileType(1, 0, 0, R | L),
                                                               TileType(2, 0, 0, L),
                                                       },
                                                       "HorizontalLineRightToLeftThreeTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.5f, 1.0f}, {12.5f, 1.0f}}},
                                                       {
                                                               TileType(0, 0, 0, R),
                                                               TileType(1, 0, 0, R | L),
                                                               TileType(2, 0, 0, R | L),
                                                               TileType(3, 0, 0, L),
                                                       },
                                                       "HorizontalLineMultiTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 1.5f}, {1.0f, 8.5f}}},
                                                       {
                                                               TileType(0, 0, 0, B),
                                                               TileType(0, 1, 0, W | T | B),
                                                               TileType(0, 2, 0, W | T),
                                                       },
                                                       "VerticalLineDownThreeTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 1.0f}, {1.0f, 13.0f}}},
                                                       {
                                                               TileType(0, 0, 0, B),
                                                               TileType(0, 1, 0, W | T | B),
                                                               TileType(0, 2, 0, W | T | B),
                                                               TileType(0, 3, 0, W | T),
                                                       },
                                                       "VerticalLineDownMultiTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 13.0f}, {1.0f, 1.0f}}},
                                                       {
                                                               TileType(0, 0, 0, B),
                                                               TileType(0, 1, 0, W | T | B),
                                                               TileType(0, 2, 0, W | T | B),
                                                               TileType(0, 3, 0, W | T),
                                                       },
                                                       "VerticalLineUpThreeTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 8.5f}, {1.0f, 1.5f}}},
                                                       {
                                                               TileType(0, 0, 0, B),
                                                               TileType(0, 1, 0, W | T | B),
                                                               TileType(0, 2, 0, W | T),
                                                       },
                                                       "VerticalLineUpMultiTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 1.0f}, {1.0f, 8.0f}}},
                                                       {
                                                               TileType(0, 0, 0, B),
                                                               TileType(0, 1, 0, W | T),
                                                       },
                                                       "VerticalLineTouchingBot",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 0.0f}, {1.0f, 7.0f}}},
                                                       {
                                                               TileType(0, 0, 0, W | B),
                                                               TileType(0, 1, 0, W | T),
                                                       },
                                                       "VerticalLineTouchingTop",
                                                       kScale);
            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{4.0f, 0.0f}, {4.0f, 7.0f}}},
                                                       {
                                                               TileType(1, 0, 0, W | B),
                                                               TileType(1, 1, 0, W | T),
                                                       },
                                                       "VerticalLineTouchingLeft",
                                                       kScale);
        }

        {
            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 1.0f}, {11.0f, 9.0f}}},
                                                       {
                                                               TileType(0, 0, 0, R),
                                                               TileType(1, 0, 0, L | B),
                                                               TileType(1, 1, 0, W | R | T),
                                                               TileType(2, 1, 0, L | B),
                                                               TileType(2, 2, 0, W | T),
                                                       },
                                                       "TopLeftToBottomRight",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{11.0f, 9.0f}, {1.0f, 1.0f}}},
                                                       {
                                                               TileType(0, 0, 0, R),
                                                               TileType(1, 0, 0, L | B),
                                                               TileType(1, 1, 0, W | R | T),
                                                               TileType(2, 1, 0, L | B),
                                                               TileType(2, 2, 0, W | T),
                                                       },
                                                       "BottomRightToTopLeft",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{2.0f, 11.0f}, {14.0f, 6.0f}}},
                                                       {
                                                               TileType(2, 1, 0, R | B),
                                                               TileType(3, 1, 0, L),
                                                               TileType(0, 2, 0, R),
                                                               TileType(1, 2, 0, R | L),
                                                               TileType(2, 2, 0, W | L | T),
                                                       },
                                                       "BottomLeftToTopRight",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{14.0f, 6.0f}, {2.0f, 11.0f}}},
                                                       {
                                                               TileType(2, 1, 0, R | B),
                                                               TileType(3, 1, 0, L),
                                                               TileType(0, 2, 0, R),
                                                               TileType(1, 2, 0, R | L),
                                                               TileType(2, 2, 0, W | L | T),
                                                       },
                                                       "TopRightToBottomLeft",
                                                       kScale);
        }

        {
            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {
                                                               {{1.0f, 3.0f}, {3.0f, 3.0f}},
                                                               {{3.0f, 3.0f}, {0.0f, 1.0f}},
                                                       },
                                                       {
                                                               TileType(0, 0, 0, 0),
                                                               TileType(0, 0, 1, 0),
                                                       },
                                                       "TwoLinesInSingleTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{3.0f, 5.0f}, {5.0f, 3.0f}}},
                                                       {
                                                               TileType(1, 0, 0, L),
                                                               TileType(0, 1, 0, R),
                                                               TileType(1, 1, 0, W | L | T),
                                                       },
                                                       "DiagonalCrossCorner",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{7.9f, 7.9f}, {0.1f, 0.1f}}},
                                                       {
                                                               TileType(0, 0, 0, R),
                                                               TileType(1, 0, 0, L),
                                                               TileType(1, 1, 0, W | T),
                                                       },
                                                       "DiagonalCrossCornerTwo",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{5.0f, 5.0f}, {9.0f, 9.0f}}},
                                                       {
                                                               TileType(1, 1, 0, R),
                                                               TileType(2, 1, 0, L),
                                                               TileType(2, 2, 0, W | T),
                                                       },
                                                       "DiagonalDownSlopeTiles",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{5.0f, 9.0f}, {9.0f, 5.0f}}},
                                                       {
                                                               TileType(1, 1, 0, R | B),
                                                               TileType(2, 1, 0, L),
                                                               TileType(1, 2, 0, W | T),
                                                       },
                                                       "DiagonalUpSlopeTiles",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{0.0f, 0.0f}, {4.0f, 4.0f}}},
                                                       {
                                                               TileType(0, 0, 0, W | R),
                                                               TileType(1, 0, 0, L),
                                                       },
                                                       "DiagonalDownOneTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{0.0f, 4.0f}, {4.0f, 0.0f}}},
                                                       {
                                                               TileType(0, 0, 0, R),
                                                               TileType(1, 0, 0, W | L),
                                                       },
                                                       "DiagonalUpOneTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{0.0f, 0.0f}, {8.0f, 8.0f}}},
                                                       {
                                                               TileType(0, 0, 0, W | R),
                                                               TileType(1, 0, 0, L),
                                                               TileType(1, 1, 0, W | R | T),
                                                               TileType(2, 1, 0, L),
                                                       },
                                                       "DiagonalDownTwoTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{0.0f, 8.0f}, {8.0f, 0.0f}}},
                                                       {
                                                               TileType(1, 0, 0, R | L),
                                                               TileType(2, 0, 0, W | L),
                                                               TileType(0, 1, 0, R),
                                                               TileType(1, 1, 0, W | L | T),
                                                       },
                                                       "DiagonalUpTwoTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 1.0f}, {8.0f, 2.0f}}},
                                                       {
                                                               TileType(0, 0, 0, R),
                                                               TileType(1, 0, 0, R | L),
                                                               TileType(2, 0, 0, L),
                                                       },
                                                       "SlopedEndingRight",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{0.0f, 8.0f}, {4.0f, 0.0f}}},
                                                       {
                                                               TileType(0, 0, 0, R | B),
                                                               TileType(1, 0, 0, W | L),
                                                               TileType(0, 1, 0, W | T),
                                                       },
                                                       "SlopedTouchingTop",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{0.0f, 0.0f}, {4.0f, 8.0f}}},
                                                       {
                                                               TileType(0, 0, 0, W | B),
                                                               TileType(0, 1, 0, W | R | T),
                                                               TileType(1, 1, 0, L),
                                                       },
                                                       "SlopedTouchingBot",
                                                       kScale);
        }

        {
            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 1.0f}, {3.0f, 3.0f}}},
                                                       {TileType(0, 0, 0, 0)},
                                                       "SameTile",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{0.0f, 1.0f}, {3.0f, 1.0f}}},
                                                       {TileType(0, 0, 0, 0)},
                                                       "SameTileLeft",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 0.0f}, {1.0f, 3.0f}}},
                                                       {TileType(0, 0, 0, W)},
                                                       "SameTileTop",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 1.0f}, {4.0f, 1.0f}}},
                                                       {
                                                               TileType(0, 0, 0, R),
                                                               TileType(1, 0, 0, L),
                                                       },
                                                       "SameTileRight",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {
                                                               {{1.0f, 1.0f}, {1.0f, 4.0f}},
                                                               {{1.0f, 1.0f}, {2.0f, 4.0f}},
                                                       },
                                                       {
                                                               TileType(0, 0, 0, 0),
                                                               TileType(0, 0, 1, 0),
                                                       },
                                                       "SameTileBottom",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {
                                                               {{0.0f, 1.0f}, {1.0f, 0.0f}},
                                                               {{0.0f, 0.0001f}, {0.0001f, 0.0f}},
                                                       },
                                                       {
                                                               TileType(0, 0, 0, W),
                                                               TileType(0, 0, 1, W),
                                                       },
                                                       "SameTileTopLeft",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{4.0f, 1.0f}, {4.0f, 3.0f}}},
                                                       {TileType(1, 0, 0, 0)},
                                                       "ExactRightEdgeCoincidence",
                                                       kScale);

            check_tiles_match<kTileWidth, kTileHeight>(reporter,
                                                       tiles,
                                                       {{{1.0f, 4.0f}, {3.0f, 4.0f}}},
                                                       {},
                                                       "ExactBottomEdgeCoincidence",
                                                       kScale);
        }

        {
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter,
                    tiles,
                    {{{0.000000, 15.856406}, {8.000000, 2.000000}}},
                    {TileType(1, 0, 0, B | R),
                     TileType(2, 0, 0, L),
                     TileType(1, 1, 0, W | T | B),
                     TileType(0, 2, 0, B | R),
                     TileType(1, 2, 0, W | T | L),
                     TileType(0, 3, 0, W | T)},
                    "Tricky Precision",
                    kScale);
        }

        {
            WindingHistogram culled;
            std::vector<Line> lines = {
                    {{22.0f * kScale, 552.0f * kScale}, {224.0f * kScale, 388.0f * kScale}}};
            tiles.reset();
            tiles.makeTilesMSAA(make_polyline(lines),
                                SkIRect::MakeWH((int)(600 * kScale), (int)(600 * kScale)),
                                culled);

            lines = {{{59.60001f * kScale, 40.78f * kScale},
                      {520599.6f * kScale, 100.18f * kScale}}};
            tiles.reset();
            tiles.makeTilesMSAA(make_polyline(lines),
                                SkIRect::MakeWH((int)(200 * kScale), (int)(100 * kScale)),
                                culled);
        }

        {
            WindingHistogram culled;
            Polyline polyline;

            float step = 4.0f * kScale;
            float y = kScaledDimF - (10.0f * kScale);
            float limit = 10.0f * kScale;

            while (y > limit) {
                polyline.appendPoint({kScaledDimF - (10.0f * kScale), y});
                polyline.appendPoint({10.0f * kScale, y});
                polyline.appendSentinel();

                polyline.appendPoint({kScaledDimF - (12.0f * kScale), y});
                polyline.appendPoint({12.0f * kScale, y});
                polyline.appendSentinel();
                y -= step;
            }

            tiles.reset();
            tiles.makeTilesMSAA(polyline, SkIRect::MakeWH(kScaledDim, kScaledDim), culled);
            const auto& buf = tiles.getTiles();

            REPORTER_ASSERT(reporter, !buf.empty(), "SortTest produced no tiles");
            if (!buf.empty()) {
                REPORTER_ASSERT(
                        reporter, buf.front().y > buf.back().y, "SortTest initial state invalid");
            }

            tiles.sortTiles();
            check_sorted(reporter, tiles.getTiles());
        }

        {
            std::vector<Line> lines = {{{-1.0f, -4.0f}, {-1.0f, -0.1f}}};
            std::vector<int16_t> expectedCulled = {};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_Vertical_OneTile_AboveViewport",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-1.0f, -4.0f}, {-1.0f, 0.0f}}};
            std::vector<int16_t> expectedCulled = {};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_Vertical_OneTile_TouchViewport",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-1.0f, -4.0f}, {-1.0f, 0.1f}}};
            std::vector<int16_t> expectedCulled = {1};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_Vertical_SingleTile_CrossingViewport",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-1.0f, -4.0f}, {-1.0f, 4.0f}}};
            std::vector<int16_t> expectedCulled = {1};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_Vertical_OneTile_CrossingViewport",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-1.0f, -4.0f}, {-1.0f, 8.0f}}};
            std::vector<int16_t> expectedCulled = {1, 1};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_Vertical_TwoTile_CrossingViewport",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-1.0f, 0.0f}, {-1.0f, 8.0f}}};
            std::vector<int16_t> expectedCulled = {1, 1, 0};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_VerticalLeft_TwoTiles_TopTouch",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-1.0f, 0.0f}, {-1.0f, 3.9f}}};
            std::vector<int16_t> expectedCulled = {1, 0};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_VerticalLeft_SingleTiles_TopTouch",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-1.0f, 0.0f}, {-1.0f, 4.0f}}};
            std::vector<int16_t> expectedCulled = {1, 0};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_VerticalLeft_OneTiles_TopTouch",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-4.0f, 1.0f}, {-1.0f, 9.0f}}};
            std::vector<int16_t> expectedCulled = {0, 1, 1};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_SlopedLeft_Down_MultiTile",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-1.0f, 9.0f}, {-4.0f, 1.0f}}};
            std::vector<int16_t> expectedCulled = {0, -1, -1};
            check_culling<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expectedCulled, "Cull_SlopedLeft_Up_MultiTile", kScale);
        }

        {
            std::vector<Line> lines = {{{-4.0f, 4.0f}, {4.0f, 4.1f}}};
            std::vector<int16_t> expectedCulled = {0, 1};
            check_culling<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expectedCulled, "Cull_SlopedLeft_OneTile", kScale);
        }

        {
            std::vector<Line> lines = {{{-8.0f, 0.0f}, {0.1f, 14.0f}}};
            std::vector<int16_t> expectedCulled = {1, 1, 1, 1};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_Sloped_EnterViewport_Down",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-8.0f, 0.0f}, {8.0f, 16.0f}}};
            std::vector<int16_t> expectedCulled = {1, 1, 0, 0};
            check_culling<kTileWidth, kTileHeight>(reporter,
                                                   tiles,
                                                   lines,
                                                   expectedCulled,
                                                   "Cull_Sloped_EnterViewport_Down",
                                                   kScale);
        }

        {
            std::vector<Line> lines = {{{-8.0f, 16.0f}, {8.0f, 0.0f}}};
            std::vector<int16_t> expectedCulled = {0, 0, 0, -1};
            check_culling<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expectedCulled, "Cull_Sloped_EnterViewport_Up", kScale);
        }

        {
            std::vector<Line> lines = {{{-3.88f, 10.95f}, {-0.73f, 10.70f}}};
            check_culling<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "Cull_GhostWindingRepro_NoYCross", kScale);
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "Tiles_GhostWindingRepro_NoYCross", kScale);
        }

        {
            std::vector<Line> lines = {{{-20.0f, 5.0f}, {-2.0f, 5.0f}}};
            check_culling<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "Cull_Offscreen_Horizontal", kScale);
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "Tiles_Offscreen_Horizontal", kScale);
        }

        {
            std::vector<Line> lines = {{{-5.0f, 2.0f}, {-2.0f, 6.0f}}};
            std::vector<int16_t> expectedCulled = {0, 1};
            check_culling<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, expectedCulled, "Cull_OffscreenLeft_YCross", kScale);
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "Tiles_OffscreenLeft_YCross", kScale);
        }

        {
            std::vector<Line> lines = {{{kUnscaledDim + 5.0f, 2.0f}, {kUnscaledDim + 10.0f, 6.0f}}};
            check_culling<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "Cull_OffscreenRight", kScale);
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, tiles, lines, {}, "Tiles_OffscreenRight", kScale);
        }
    }
};

DEF_TEST(SparseStrips_Tiler_4x4, reporter) {
    skgpu::graphite::TileTestRunner<4, 4>::runAll(reporter);
}

DEF_TEST(SparseStrips_Tiler_8x8, reporter) {
    skgpu::graphite::TileTestRunner<8, 8>::runAll(reporter);
}

}  // namespace skgpu::graphite
