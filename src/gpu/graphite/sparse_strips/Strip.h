/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_Strip_DEFINED
#define skgpu_graphite_Strip_DEFINED

#include "include/private/base/SkTDArray.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>

namespace skgpu::graphite {

#if defined(GPU_TEST_UTILS)
using MsaaExactMaskObserver = std::function<void(uint8_t exactMask)>;
#endif

class Strip : IntersectionBits {
public:
    Strip(uint16_t x, uint16_t y, uint32_t alphaIdx, bool shouldFill)
            : x(x)
            , y(y)
            , packedAlphaIdx(alphaIdx | (static_cast<uint32_t>(shouldFill) << FILL_SHIFT)) {}

    template <uint16_t TileWidth, uint16_t TileHeight>
    static void MakeStripsMsaa(const Tiles<TileWidth, TileHeight>& tileContainer,
                               SkTDArray<Strip>& stripBuf,
                               SkTDArray<uint8_t>& alphaBuf,
                               SkPathFillType fillType,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>& maskLut,
                               WideTiles<TileHeight>& wides,
                               uint16_t viewportWidth,
                               uint16_t viewportHeight
#if defined(GPU_TEST_UTILS)
                               , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            MakeStripsImpl<TileWidth, TileHeight, kIsWinding>(tileContainer,
                                                              stripBuf,
                                                              alphaBuf,
                                                              isInverse,
                                                              polyline,
                                                              maskLut,
                                                              wides,
                                                              viewportWidth,
                                                              viewportHeight
#if defined(GPU_TEST_UTILS)
                                                              , observer
#endif
            );
        });
    }

    template <uint16_t TileWidth, uint16_t TileHeight>
    static void MakeStripsMsaaSimd(const Tiles<TileWidth, TileHeight>& tileContainer,
                                   SkTDArray<Strip>& stripBuf,
                                   SkTDArray<uint8_t>& alphaBuf,
                                   SkPathFillType fillType,
                                   const Polyline& polyline,
                                   const SkTDArray<uint8_t>& maskLut,
                                   WideTiles<TileHeight>& wides,
                                   uint16_t viewportWidth,
                                   uint16_t viewportHeight
#if defined(GPU_TEST_UTILS)
                                   , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            MakeStripsSimdImpl<TileWidth, TileHeight, kIsWinding>(tileContainer,
                                                                  stripBuf,
                                                                  alphaBuf,
                                                                  isInverse,
                                                                  polyline,
                                                                  maskLut,
                                                                  wides,
                                                                  viewportWidth,
                                                                  viewportHeight
#if defined(GPU_TEST_UTILS)
                                                                  , observer
#endif
            );
        });
    }

private:
#if defined(GPU_TEST_UTILS)
    template <uint16_t, uint16_t> friend class ClipTestRunner;
    template <uint16_t, uint16_t> friend class CoverageTestRunner;
    template <uint16_t, uint16_t> friend class CoverageBenchSimd;
#endif
    template <uint16_t, uint16_t> friend class StripGenerator;

    template <typename F> static void Dispatch(SkPathFillType fillType, F&& f) {
        switch (fillType) {
            case SkPathFillType::kWinding:
                f(std::bool_constant<true>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseWinding:
                f(std::bool_constant<true>{}, /*isInverse=*/true);
                return;
            case SkPathFillType::kEvenOdd:
                f(std::bool_constant<false>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseEvenOdd:
                f(std::bool_constant<false>{}, /*isInverse=*/true);
                return;
        }
        SkUNREACHABLE;
    }

    template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding>
    static void MakeStripsImpl(const Tiles<TileWidth, TileHeight>& tileContainer,
                               SkTDArray<Strip>& stripBuf,
                               SkTDArray<uint8_t>& alphaBuf,
                               bool isInverse,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>& maskLut,
                               WideTiles<TileHeight>& wides,
                               uint16_t viewportWidth,
                               uint16_t viewportHeight
#if defined(GPU_TEST_UTILS)
                               , MsaaExactMaskObserver observer = nullptr
#endif
    );

    template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding>
    static void MakeStripsSimdImpl(const Tiles<TileWidth, TileHeight>& tileContainer,
                                   SkTDArray<Strip>& stripBuf,
                                   SkTDArray<uint8_t>& alphaBuf,
                                   bool isInverse,
                                   const Polyline& polyline,
                                   const SkTDArray<uint8_t>& maskLut,
                                   WideTiles<TileHeight>& wides,
                                   uint16_t viewportWidth,
                                   uint16_t viewportHeight
#if defined(GPU_TEST_UTILS)
                                   , MsaaExactMaskObserver observer = nullptr
#endif
    );

    static constexpr uint32_t FILL_SHIFT = 31;

    static constexpr int kNumSubSamples = 8;
    static constexpr float kPackingScale = 0.5f;
    static constexpr int kMaskWidth = 64;
    static constexpr int kMaskHeight = 64;
    static constexpr float kMaskWidthF = static_cast<float>(kMaskWidth);
    static constexpr float kMaskHeightF = static_cast<float>(kMaskHeight);
    static constexpr int kMaskWidthExcl = kMaskWidth - 1;
    static constexpr int kMaskHeightExcl = kMaskHeight - 1;

    static constexpr float kStripEpsilon = 1e-5f;

    // Note: SIMD not worth for here!
    static SK_ALWAYS_INLINE std::array<SkPoint, 2> clipToTile(
            const Line& line,
            const std::array<float, 4>& bounds,
            const std::array<float, 4>& derivatives,
            uint32_t intersectionData,
            bool cannonicalXDir,
            bool cannonicalYDir) {
        SkPoint pEntry = line.p0;
        SkPoint pExit = line.p1;

        const float tileMinX = bounds[0];
        const float tileMinY = bounds[1];
        const float tileMaxX = bounds[2];
        const float tileMaxY = bounds[3];

        const float dx = derivatives[0];
        const float dy = derivatives[1];

        uint32_t maskVIn, maskVOut;
        float boundVIn, boundVOut;
        if (cannonicalXDir) {
            maskVIn = L;
            boundVIn = tileMinX;
            maskVOut = R;
            boundVOut = tileMaxX;
        } else {
            maskVIn = R;
            boundVIn = tileMaxX;
            maskVOut = L;
            boundVOut = tileMinX;
        }

        uint32_t maskHIn, maskHOut;
        float boundHIn, boundHOut;
        if (cannonicalYDir) {
            maskHIn = T;
            boundHIn = tileMinY;
            maskHOut = B;
            boundHOut = tileMaxY;
        } else {
            maskHIn = B;
            boundHIn = tileMaxY;
            maskHOut = T;
            boundHOut = tileMinY;
        }

        const float idx = derivatives[2];
        const float idy = derivatives[3];

        const uint32_t entryHits = intersectionData & (maskVIn | maskHIn);
        if (entryHits != 0) {
            const bool useH = (intersectionData & maskHIn) != 0;

            const float bound = useH ? boundHIn : boundVIn;
            const float start = useH ? line.p0.fY : line.p0.fX;
            const float invD = useH ? idy : idx;

            const float t = (bound - start) * invD;

            pEntry.fX = line.p0.fX + t * dx;
            pEntry.fY = line.p0.fY + t * dy;

            if (useH) {
                pEntry.fY = bound;
            } else {
                pEntry.fX = bound;
            }
        }

        const uint32_t exitHits = intersectionData & (maskVOut | maskHOut);
        if (exitHits != 0) {
            const bool useH = (intersectionData & maskHOut) != 0;

            const float bound = useH ? boundHOut : boundVOut;
            const float start = useH ? line.p0.fY : line.p0.fX;
            const float invD = useH ? idy : idx;

            const float t = (bound - start) * invD;

            pExit.fX = line.p0.fX + t * dx;
            pExit.fY = line.p0.fY + t * dy;

            if (useH) {
                pExit.fY = bound;
            } else {
                pExit.fX = bound;
            }
        }

        std::array<SkPoint, 2> result;
        if (pExit.fY >= pEntry.fY) {
            result = {pEntry, pExit};
        } else {
            result = {pExit, pEntry};
        }

        result[0].fX -= tileMinX;
        result[0].fY -= tileMinY;
        result[1].fX -= tileMinX;
        result[1].fY -= tileMinY;

        // Clamping serves three purposes here:
        // 1) Points which are slightly outside the tile due to floating point error are coerced
        //    inside.
        // 2) More subtly, perfectly horizontal or vertical lines have their reciprocal derivatives
        //    set to 0 in the upstream strip logic. This causes the intersection calculation to
        //    return the original coordinate. While the coordinate fixed to the tile edge is
        //    explicitly set (and guaranteed valid), clamping forces the coordinate along that edge
        //    to be in bounds and watertight.
        // 3) In the case that there is only a single edge crossing (either from a corner touch
        //    tiebroken to a single dimension or an inclusive touch on the vertical edge of a tile),
        //    the opposing point must be somewhere in the direction normal to the edge. Clamping
        //    forces the segment to snap onto the tile edge. This creates zero-width geometry which
        //    gets discarded by downstream pixel filling logic.
        const float width = tileMaxX - tileMinX;
        const float height = tileMaxY - tileMinY;

        result[0].fX = std::fmin(std::fmax(result[0].fX, 0.0f), width);
        result[0].fY = std::fmin(std::fmax(result[0].fY, 0.0f), height);
        result[1].fX = std::fmin(std::fmax(result[1].fX, 0.0f), width);
        result[1].fY = std::fmin(std::fmax(result[1].fY, 0.0f), height);

        return result;
    }

    uint16_t x;
    uint16_t y;
    uint32_t packedAlphaIdx;
};

template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding>
void Strip::MakeStripsImpl(const Tiles<TileWidth, TileHeight>& tileContainer,
                           SkTDArray<Strip>& stripBuf,
                           SkTDArray<uint8_t>& alphaBuf,
                           bool isInverse,
                           const Polyline& polyline,
                           const SkTDArray<uint8_t>& maskLut,
                           WideTiles<TileHeight>& wides,
                           uint16_t viewportWidth,
                           uint16_t viewportHeight
#if defined(GPU_TEST_UTILS)
                           , MsaaExactMaskObserver observer
#endif
) {
    auto emitBackground = [&](uint16_t start, uint16_t end, uint16_t width) {
        for (uint16_t row = start; row < end; row += TileHeight) {
            wides.addTile(0, row, width);
        }
    };

    const auto& tiles = tileContainer.getTiles();
    if (tiles.empty()) {
        if (isInverse) {
            emitBackground(0, viewportHeight + TileHeight, viewportWidth);
        }
        return;
    }

    int16_t maskBuffer[TileHeight][TileWidth][kNumSubSamples];

    auto clearMask = [&](int16_t value) {
        for (int32_t y = 0; y < TileHeight; ++y) {
            for (int32_t x = 0; x < TileWidth; ++x) {
                for (int32_t k = 0; k < kNumSubSamples; ++k) {
                    maskBuffer[y][x][k] = value;
                }
            }
        }
    };
    clearMask(0);

    auto shouldFill = [](int32_t w) {
        if constexpr (kIsWinding) {
            return w != 0;
        } else {
            return (w & 1) != 0;
        }
    };

    int32_t windingDelta = 0;
    constexpr int32_t kTileAlphaCount = TileWidth * TileHeight;
    int32_t localAlphaIdx = alphaBuf.size();
    size_t megaGuess = tiles.size() * kTileAlphaCount * 2;
    alphaBuf.reserve(localAlphaIdx + megaGuess);

    size_t totalCount = tiles.size();
    Tile<TileWidth, TileHeight> prevTile = tiles[0];

    if (isInverse) {
        emitBackground(0, prevTile.y * TileHeight, viewportWidth);
        // If the first tile isn't flush left, we need a 0-width strip to start filling
        if (prevTile.x != 0) {
            stripBuf.push_back(Strip(0, prevTile.y * TileHeight, localAlphaIdx, false));
        }
    }

    Strip currentStrip(
            prevTile.x * TileWidth, prevTile.y * TileHeight, localAlphaIdx, false ^ isInverse);

    float tileMinX = static_cast<float>(prevTile.x * TileWidth);
    float tileMinY = static_cast<float>(prevTile.y * TileHeight);
    float tileMaxX = tileMinX + static_cast<float>(TileWidth);
    float tileMaxY = tileMinY + static_cast<float>(TileHeight);

    for (size_t i = 0; i <= totalCount; ++i) {
        bool isSentinel = (i == totalCount);
        Tile<TileWidth, TileHeight> tile;

        if (isSentinel) {
            tile = Tile<TileWidth, TileHeight>(0xFFFF, 0xFFFF, 0, 0);  // Sentinel
        } else {
            tile = tiles[i];
        }

        bool tileStart = (tile.x != prevTile.x || tile.y != prevTile.y);
        bool rowStart = (tile.y != prevTile.y);
        bool segStart = tileStart && !((tile.y == prevTile.y) && (tile.x == prevTile.x + 1));

        if (tileStart) {
            if (localAlphaIdx + kTileAlphaCount > alphaBuf.capacity()) {
                constexpr size_t kChunkSize = 4 * kTileAlphaCount;
                alphaBuf.reserve(alphaBuf.capacity() + kChunkSize);
            }
            uint8_t* alphaBase = alphaBuf.begin();

            for (int32_t y = 0; y < TileHeight; ++y) {
                for (int32_t x = 0; x < TileWidth; ++x) {
                    int32_t activeSamples = 0;

                    for (int32_t k = 0; k < kNumSubSamples; ++k) {
                        if (shouldFill(maskBuffer[y][x][k])) {
                            activeSamples++;
                        }
                    }
                    if (isInverse) {
                        activeSamples = kNumSubSamples - activeSamples;
                    }

#if defined(GPU_TEST_UTILS)
                    if (observer) {
                        uint8_t exactMask = 0;
                        for (int32_t k = 0; k < kNumSubSamples; ++k) {
                            if (shouldFill(maskBuffer[y][x][k])) {
                                exactMask |= (1 << k);
                            }
                        }
                        if (isInverse) {
                            exactMask = ~exactMask & ((1 << kNumSubSamples) - 1);
                        }
                        observer(exactMask);
                    }
#endif

                    uint8_t alpha = static_cast<uint8_t>((activeSamples * 255 + 4) / 8);
                    alphaBase[localAlphaIdx++] = alpha;
                }
            }

            if (!rowStart) {
                clearMask(static_cast<int16_t>(windingDelta));
            }
        }

        if (segStart) {
            stripBuf.push_back(currentStrip);
            if (rowStart) {
                if (windingDelta != 0 || isInverse || isSentinel) {
                    stripBuf.push_back(Strip(0xFFFF,
                                             prevTile.y * TileHeight,
                                             localAlphaIdx,
                                             shouldFill(windingDelta) ^ isInverse));
                }
                windingDelta = 0;
                clearMask(0);

                if (isInverse && !isSentinel) {
                    emitBackground(
                            (prevTile.y + 1) * TileHeight, tile.y * TileHeight, viewportWidth);
                    // Start the new row with a 0-width strip if it doesn't flush left
                    if (tile.x != 0) {
                        stripBuf.push_back(Strip(0, tile.y * TileHeight, localAlphaIdx, false));
                    }
                }
            }

            if (!isSentinel) {
                currentStrip = Strip(tile.x * TileWidth,
                                     tile.y * TileHeight,
                                     localAlphaIdx,
                                     shouldFill(windingDelta) ^ isInverse);
            }
        }

        if (isSentinel) {
            break;
        }

        prevTile = tile;

        Line line = polyline.getLine(tile.packedWindingLineIdx >> INT_MASK_SHIFT);
        bool canonicalXDir = line.p1.fX >= line.p0.fX;
        bool canonicalYDir = line.p1.fY >= line.p0.fY;

        uint32_t windingBit = (tile.packedWindingLineIdx & W) ? 1 : 0;
        if constexpr (kIsWinding) {
            windingDelta += (canonicalYDir ? 1 : -1) * static_cast<int32_t>(windingBit);
        } else {
            windingDelta ^= static_cast<int32_t>(windingBit);
        }

        float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
        if (rightEdge < 0.0f) {
            continue;
        }

        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;
        float idx = (std::abs(dx) <= kStripEpsilon) ? 0.0f : 1.0f / dx;
        float idy = (std::abs(dy) <= kStripEpsilon) ? 0.0f : 1.0f / dy;
        float dxdy = dx * idy;

        if (tileStart) {
            tileMinX = static_cast<float>(tile.x * TileWidth);
            tileMinY = static_cast<float>(tile.y * TileHeight);
            tileMaxX = tileMinX + static_cast<float>(TileWidth);
            tileMaxY = tileMinY + static_cast<float>(TileHeight);
        }

        std::array<float, 4> bounds = {tileMinX, tileMinY, tileMaxX, tileMaxY};
        std::array<float, 4> derivs = {dx, dy, idx, idy};

        auto clipped = clipToTile(line,
                                  bounds,
                                  derivs,
                                  tile.packedWindingLineIdx & INTERSECTION_MASK,
                                  canonicalXDir,
                                  canonicalYDir);
        SkPoint pTop = clipped[0];
        SkPoint pBot = clipped[1];

        if (tile.packedWindingLineIdx & L) {
            // If the line is perfectly vertical we should choose the bottom edge.
            float yEdge = pTop.fX < pBot.fX ? pTop.fY : pBot.fY;
            int32_t startY = static_cast<int32_t>(std::ceil(yEdge));

            int32_t val;
            if constexpr (kIsWinding) {
                val = canonicalXDir ? -1 : 1;
            } else {
                val = 1;
            }

            for (int32_t y = startY; y < TileHeight; ++y) {
                for (int32_t x = 0; x < TileWidth; ++x) {
                    for (int32_t k = 0; k < kNumSubSamples; ++k) {
                        if constexpr (kIsWinding) {
                            maskBuffer[y][x][k] += val;
                        } else {
                            maskBuffer[y][x][k] ^= val;
                        }
                    }
                }
            }
        }

        if (std::abs(dy) < kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            continue;
        }

        int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
        int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));

        std::array<SkPoint, TileHeight + 1> rows;
        rows.fill(SkPoint::Make(std::numeric_limits<float>::quiet_NaN(),
                                std::numeric_limits<float>::quiet_NaN()));
        rows[startY] = pTop;
        for (int32_t y = startY + 1; y < endY; ++y) {
            float gy = static_cast<float>(y);
            float gx = pTop.fX + (gy - pTop.fY) * dxdy;
            rows[y] = {gx, gy};
        }
        rows[endY] = pBot;

        // LUT indexing setup
        float nx = dy;
        float ny = -dx;
        if (nx < 0.0f) {
            nx = -nx;
            ny = -ny;
        }
        float D = nx + std::abs(ny);
        float invD = (D < kStripEpsilon) ? 0.0f : 1.0f / D;
        bool isPos = ny <= 0.0f;
        float c_local = nx * pTop.fX + ny * pTop.fY;
        float s = std::abs(ny) * invD;
        int v_mod = std::clamp(
                static_cast<int>(std::floor(s * (kMaskHeight / 2))), 0, (kMaskHeight / 2) - 1);
        int v = isPos ? (v_mod + kMaskHeight / 2) : v_mod;
        float stepX = nx * invD;
        float stepY = ny * invD;
        float t_base = ((isPos ? nx : D) - c_local) * invD;

        bool xDir = pTop.fX <= pBot.fX;
        for (int32_t y = startY; y < endY; ++y) {
            float p_top_x = rows[y].fX;
            float p_top_y = rows[y].fY;
            float p_bot_x = rows[y + 1].fX;
            float p_bot_y = rows[y + 1].fY;

            if (std::isnan(p_top_x) || std::isnan(p_bot_x)) {
                continue;
            }

            float xMin = std::fmin(p_top_x, p_bot_x);
            float xMax = std::fmax(p_top_x, p_bot_x);

            int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(xMin)), 0, TileWidth - 1);
            int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, TileWidth - 1);

            float t_val =
                    t_base + stepY * static_cast<float>(y) + stepX * static_cast<float>(xStart);

            // EndCap and bump setup
            bool is_startY = (y == startY);
            bool is_endY_minus_1 = (y == endY - 1);
            bool pixelTopTouch = (p_top_y != std::floor(p_top_y));
            bool default_bumped = pixelTopTouch && xDir;

            int canonical_start_x = xDir ? xStart : xEnd;
            int canonical_end_x = xDir ? xEnd : xStart;

            uint8_t start_mask_val = 0xff;
            bool start_bumped = default_bumped;
            if (is_startY) {
                start_bumped = (p_top_x == 0.0f && pixelTopTouch);
                if (!start_bumped) {
                    uint32_t shift = static_cast<uint32_t>(
                            std::round(8.0f * (p_top_y - static_cast<float>(y))));
                    start_mask_val = static_cast<uint8_t>(0xff << shift);
                }
            }

            uint8_t end_mask_val = 0xff;
            if (is_endY_minus_1 && p_bot_x != 0.0f) {
                uint32_t shift =
                        static_cast<uint32_t>(std::round(8.0f * (p_bot_y - static_cast<float>(y))));
                end_mask_val = static_cast<uint8_t>(~(0xff << shift));
            }

            if (!is_startY) start_mask_val = 0xff;
            if (!is_endY_minus_1) end_mask_val = 0xff;

            for (int32_t x = xStart; x <= xEnd; ++x) {
                int u = std::clamp(
                        static_cast<int>(std::floor(t_val * kMaskWidthF)), 0, kMaskWidthExcl);
                SkASSERT(v * kMaskWidth + u < maskLut.size());
                uint8_t maskVal = maskLut[v * kMaskWidth + u];

                uint8_t edgeMask = 0xff;
                if (x == canonical_start_x) edgeMask &= start_mask_val;
                if (x == canonical_end_x) edgeMask &= end_mask_val;
                maskVal &= edgeMask;

                bool bumped = (is_startY && x == canonical_start_x) ? start_bumped : default_bumped;
                for (int32_t k = 0; k < kNumSubSamples; ++k) {
                    int32_t maskBit = (maskVal & (1 << k)) ? 1 : 0;
                    if constexpr (kIsWinding) {
                        maskBit -= bumped;
                        if (maskBit != 0) {
                            if (canonicalYDir) {
                                maskBuffer[y][x][k] += maskBit;
                            } else {
                                maskBuffer[y][x][k] -= maskBit;
                            }
                        }
                    } else {
                        maskBuffer[y][x][k] ^= (maskBit ^ bumped);
                    }
                }

                t_val += stepX;
            }

            bool crossedTop = (y > startY) || (p_top_y == std::floor(p_top_y));
            if (crossedTop) {
                int32_t val;
                if constexpr (kIsWinding) {
                    val = canonicalYDir ? 1 : -1;
                } else {
                    val = 1;
                }
                for (int32_t x = xEnd + 1; x < TileWidth; ++x) {
                    for (int32_t k = 0; k < kNumSubSamples; ++k) {
                        if constexpr (kIsWinding) {
                            maskBuffer[y][x][k] += val;
                        } else {
                            maskBuffer[y][x][k] ^= val;
                        }
                    }
                }
            }
        }
    }

    alphaBuf.resize(localAlphaIdx);

    if (isInverse) {
        emitBackground((prevTile.y + 1) * TileHeight, viewportHeight + TileHeight, viewportWidth);
    }
}

template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding>
void Strip::MakeStripsSimdImpl(const Tiles<TileWidth, TileHeight>& tileContainer,
                               SkTDArray<Strip>& stripBuf,
                               SkTDArray<uint8_t>& alphaBuffer,
                               bool isInverse,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>& maskLut,
                               WideTiles<TileHeight>& wides,
                               uint16_t viewportWidth,
                               uint16_t viewportHeight
#if defined(GPU_TEST_UTILS)
                               , MsaaExactMaskObserver observer
#endif
) {
    auto emitBackground = [&](uint16_t start, uint16_t end, uint16_t width) {
        for (uint16_t row = start; row < end; row += TileHeight) {
            wides.addTile(0, row, width);
        }
    };

    const auto& tiles = tileContainer.getTiles();
    if (tiles.empty()) {
        if (isInverse) {
            emitBackground(0, viewportHeight + TileHeight, viewportWidth);
        }
        return;
    }

    auto shouldFill = [](int32_t w) {
        if constexpr (kIsWinding) {
            return w != 0;
        } else {
            return (w & 1) != 0;
        }
    };

    size_t totalCount = tiles.size();
    Tile<TileWidth, TileHeight> prevTile = tiles[0];

    constexpr int32_t kTileAlphaCount = TileWidth * TileHeight;
    int32_t localAlphaIdx = alphaBuffer.size();
    size_t megaGuess = tiles.size() * kTileAlphaCount * 2;
    alphaBuffer.reserve(localAlphaIdx + megaGuess);

    using swar_pixel = skvx::Vec<2, uint32_t>;
    using pixel_bytes = skvx::Vec<8, uint8_t>;
    swar_pixel mask[TileHeight][TileWidth];

    auto resetMask = [&](uint32_t val) {
        if (val == 0) {
            std::memset(mask, 0, sizeof(mask));
        } else {
            swar_pixel* flat_mask = &mask[0][0];
            constexpr int totalPixels = TileWidth * TileHeight;

            if constexpr (TileWidth % 8 == 0) {
                skvx::Vec<16, uint32_t> v_val = {val, val, val, val, val, val, val, val,
                                                 val, val, val, val, val, val, val, val};
                for (int i = 0; i < totalPixels; i += 8) {
                    v_val.store(flat_mask + i);
                }
            } else if constexpr (TileWidth % 4 == 0) {
                skvx::Vec<8, uint32_t> v_val = {val, val, val, val, val, val, val, val};
                for (int i = 0; i < totalPixels; i += 4) {
                    v_val.store(flat_mask + i);
                }
            } else {
                swar_pixel p = {val, val};
                for (int i = 0; i < totalPixels; ++i) {
                    flat_mask[i] = p;
                }
            }
        }
    };

    const uint32_t kInitialMaskVal = kIsWinding ? 0x80808080u : 0u;
    resetMask(kInitialMaskVal);

    int32_t windingDelta = 0;

    // Handle Inverse Fill Initial State
    if (isInverse) {
        emitBackground(0, prevTile.y * TileHeight, viewportWidth);
        // If the first tile isn't flush left, we need a 0-width strip to start filling
        if (prevTile.x != 0) {
            stripBuf.push_back(Strip(0, prevTile.y * TileHeight, localAlphaIdx, false));
        }
    }

    Strip currentStrip(
            prevTile.x * TileWidth, prevTile.y * TileHeight, localAlphaIdx, false ^ isInverse);

    float tileMinX = (float)(prevTile.x * TileWidth);
    float tileMinY = (float)(prevTile.y * TileHeight);
    float tileMaxX = tileMinX + (float)TileWidth;
    float tileMaxY = tileMinY + (float)TileHeight;

    for (size_t i = 0; i <= totalCount; ++i) {
        bool isSentinel = (i == totalCount);
        Tile<TileWidth, TileHeight> tile;

        if (isSentinel) {
            tile = Tile<TileWidth, TileHeight>(0xFFFF, 0xFFFF, 0, 0);
        } else {
            tile = tiles[i];
        }

        bool tileStart = (tile.x != prevTile.x || tile.y != prevTile.y);
        bool rowStart = (tile.y != prevTile.y);
        bool segStart = tileStart && !((tile.y == prevTile.y) && (tile.x == prevTile.x + 1));

        if (tileStart) {
            if (localAlphaIdx + kTileAlphaCount > alphaBuffer.capacity()) {
                constexpr size_t kChunkSize = 4 * kTileAlphaCount;
                alphaBuffer.reserve(alphaBuffer.capacity() + kChunkSize);
            }
            uint8_t* alphaBase = alphaBuffer.begin();

            // Use skvx::Vec to perform true hardware SIMD byte comparisons. We cast the vector into
            // 8-bit byte lanes. In winding mode, the winding sum is biased by +128 (0x80), so a
            // byte has active coverage if it does NOT equal 0x80.
            //
            // The comparison yields a hardware mask (0xFF where true, 0x00 where false). We isolate
            // the lowest bit of each byte (0x01) and use the multiply-and-shift trick to
            // horizontally sum the 4 byte-lanes into the top byte of each 32-bit word.
            auto get_counts_wide = [&](auto v) {
                using VecT = decltype(v);
                if constexpr (kIsWinding) {
                    using Vec8 = skvx::Vec<sizeof(VecT), uint8_t>;
                    Vec8 v_bytes = sk_bit_cast<Vec8>(v);
                    Vec8 bias_bytes(0x80);
                    Vec8 cmp = (v_bytes != bias_bytes);
                    VecT active_bytes = sk_bit_cast<VecT>(cmp) & VecT(0x01010101u);
                    return (active_bytes * VecT(0x01010101u)) >> 24;
                } else {
                    VecT ones(0x01010101u);
                    VecT active_bytes = v & ones;
                    return (active_bytes * ones) >> 24;
                }
            };

            auto process_chunk = [&](auto size_tag, int y, int x) {
                constexpr int ChunkSize = decltype(size_tag)::value;

                auto v_mask = skvx::Vec<ChunkSize * 2, uint32_t>::Load(&mask[y][x]);

                auto counts = get_counts_wide(v_mask);

                skvx::Vec<ChunkSize, uint32_t> activeSamples;
                if constexpr (ChunkSize == 8) {
                    auto counts_lo = skvx::shuffle<0, 2, 4, 6, 8, 10, 12, 14>(counts);
                    auto counts_hi = skvx::shuffle<1, 3, 5, 7, 9, 11, 13, 15>(counts);
                    activeSamples = counts_lo + counts_hi;
                } else {
                    auto counts_lo = skvx::shuffle<0, 2, 4, 6>(counts);
                    auto counts_hi = skvx::shuffle<1, 3, 5, 7>(counts);
                    activeSamples = counts_lo + counts_hi;
                }

                if (isInverse) {
                    activeSamples = skvx::Vec<ChunkSize, uint32_t>(kNumSubSamples) - activeSamples;
                }

                skvx::Vec<ChunkSize, uint32_t> alpha32 = (activeSamples * 255 + 4) >> 3;
                skvx::cast<uint8_t>(alpha32).store(alphaBase + localAlphaIdx);
                localAlphaIdx += ChunkSize;

#if defined(GPU_TEST_UTILS)
                if (observer) {
                    for (int xx = x; xx < x + ChunkSize; ++xx) {
                        swar_pixel v = mask[y][xx];

                        uint8_t exactMask = 0;
                        uint32_t lo = v[0];
                        uint32_t hi = v[1];
                        for (int s = 0; s < 4; ++s) {
                            int8_t s_lo = (int8_t)(lo & 0xFF);
                            int8_t s_hi = (int8_t)(hi & 0xFF);
                            if constexpr (kIsWinding) {
                                s_lo = (int8_t)((uint8_t)s_lo - 0x80);
                                s_hi = (int8_t)((uint8_t)s_hi - 0x80);
                            }
                            if (shouldFill(s_lo)) exactMask |= (1 << s);
                            if (shouldFill(s_hi)) exactMask |= (1 << (s + 4));
                            lo >>= 8;
                            hi >>= 8;
                        }
                        if (isInverse) {
                            exactMask = ~exactMask & ((1 << kNumSubSamples) - 1);
                        }
                        observer(exactMask);
                    }
                }
#endif
            };

            for (int y = 0; y < TileHeight; ++y) {
                if constexpr (TileWidth % 8 == 0) {
                    for (int x = 0; x < TileWidth; x += 8) {
                        process_chunk(std::integral_constant<int, 8>{}, y, x);
                    }
                } else if constexpr (TileWidth % 4 == 0) {
                    for (int x = 0; x < TileWidth; x += 4) {
                        process_chunk(std::integral_constant<int, 4>{}, y, x);
                    }
                }
            }

            if (!rowStart) {
                uint8_t w_byte;
                if constexpr (kIsWinding) {
                    w_byte = 0x80 + (uint8_t)(int8_t)windingDelta;
                } else {
                    w_byte = (windingDelta & 1) ? 1 : 0;
                }

                uint32_t w_val = w_byte * 0x01010101u;
                resetMask(w_val);
            }
        }

        if (segStart) {
            stripBuf.push_back(currentStrip);

            if (rowStart) {
                if (windingDelta != 0 || isInverse || isSentinel) {
                    stripBuf.push_back(Strip(0xFFFF,
                                             prevTile.y * TileHeight,
                                             localAlphaIdx,
                                             shouldFill(windingDelta) ^ isInverse));
                }

                windingDelta = 0;
                resetMask(kInitialMaskVal);

                if (isInverse && !isSentinel) {
                    emitBackground(
                            (prevTile.y + 1) * TileHeight, tile.y * TileHeight, viewportWidth);
                    // Start the new row with a 0-width strip if it doesn't flush left
                    if (tile.x != 0) {
                        stripBuf.push_back(Strip(0, tile.y * TileHeight, localAlphaIdx, false));
                    }
                }
            }

            if (!isSentinel) {
                currentStrip = Strip(tile.x * TileWidth,
                                     tile.y * TileHeight,
                                     localAlphaIdx,
                                     shouldFill(windingDelta) ^ isInverse);
            }
        }

        if (isSentinel) {
            break;
        }

        prevTile = tile;  // Ensures prevTile is a real tile for inverse filling

        Line line = polyline.getLine(tile.packedWindingLineIdx >> INT_MASK_SHIFT);
        float p0_x = line.p0.fX;
        float p0_y = line.p0.fY;
        float p1_x = line.p1.fX;
        float p1_y = line.p1.fY;

        bool canonicalXDir = p1_x >= p0_x;
        bool canonicalYDir = p1_y >= p0_y;

        uint32_t windingBit = (tile.packedWindingLineIdx & W) ? 1 : 0;
        if constexpr (kIsWinding) {
            windingDelta += (canonicalYDir ? 1 : -1) * static_cast<int32_t>(windingBit);
        } else {
            windingDelta ^= static_cast<int32_t>(windingBit);
        }

        float rightEdge = canonicalXDir ? p1_x : p0_x;
        if (rightEdge < 0.0f) {
            continue;
        }

        float dx = p1_x - p0_x;
        float dy = p1_y - p0_y;
        float idx = (std::abs(dx) <= kStripEpsilon) ? 0.0f : 1.0f / dx;
        float idy = (std::abs(dy) <= kStripEpsilon) ? 0.0f : 1.0f / dy;
        float dxdy = dx * idy;

        if (tileStart) {
            tileMinX = (float)(tile.x * TileWidth);
            tileMinY = (float)(tile.y * TileHeight);
            tileMaxX = tileMinX + (float)TileWidth;
            tileMaxY = tileMinY + (float)TileHeight;
        }

        std::array<float, 4> bounds = {tileMinX, tileMinY, tileMaxX, tileMaxY};
        std::array<float, 4> derivs = {dx, dy, idx, idy};

        auto clipped = clipToTile(line,
                                  bounds,
                                  derivs,
                                  tile.packedWindingLineIdx & INTERSECTION_MASK,
                                  canonicalXDir,
                                  canonicalYDir);
        SkPoint pTop = clipped[0];
        SkPoint pBot = clipped[1];

        // --- Wide SIMD L Boundary Fill ---
        if (tile.packedWindingLineIdx & L) {
            float yEdge = (pTop.fX < pBot.fX) ? pTop.fY : pBot.fY;
            uint8_t fill_byte;
            if constexpr (kIsWinding) {
                fill_byte = canonicalXDir ? 0xFF : 1;
            } else {
                fill_byte = 1;
            }

            uint32_t fill_32 = fill_byte * 0x01010101u;
            for (int y = (int)std::ceil(yEdge); y < TileHeight; ++y) {
                if constexpr (TileWidth % 8 == 0) {
                    skvx::Vec<16, uint32_t> v_fill16_32 = {
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32,
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32 };
                    for (int x = 0; x < TileWidth; x += 8) {
                        auto v_mask = skvx::Vec<16, uint32_t>::Load(&mask[y][x]);
                        if constexpr (kIsWinding) {
                            auto m_b = sk_bit_cast<skvx::Vec<64, uint8_t>>(v_mask);
                            auto f_b = sk_bit_cast<skvx::Vec<64, uint8_t>>(v_fill16_32);
                            v_mask = sk_bit_cast<skvx::Vec<16, uint32_t>>(m_b + f_b);
                        } else {
                            v_mask ^= v_fill16_32;
                        }
                        v_mask.store(&mask[y][x]);
                    }
                } else if constexpr (TileWidth % 4 == 0) {
                    skvx::Vec<8, uint32_t> v_fill8_32 = {
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32 };
                    for (int x = 0; x < TileWidth; x += 4) {
                        auto v_mask = skvx::Vec<8, uint32_t>::Load(&mask[y][x]);
                        if constexpr (kIsWinding) {
                            auto m_b = sk_bit_cast<skvx::Vec<32, uint8_t>>(v_mask);
                            auto f_b = sk_bit_cast<skvx::Vec<32, uint8_t>>(v_fill8_32);
                            v_mask = sk_bit_cast<skvx::Vec<8, uint32_t>>(m_b + f_b);
                        } else {
                            v_mask ^= v_fill8_32;
                        }
                        v_mask.store(&mask[y][x]);
                    }
                }
            }
        }

        if (std::abs(dy) < kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            continue;
        }

        int startY = (int)std::floor(pTop.fY);
        int endY = (int)std::ceil(pBot.fY);

        constexpr int kPaddedHeight = ((TileHeight + 1 + 3) / 4) * 4;
        float edgeX[kPaddedHeight];
        float edgeY[kPaddedHeight];
        for (int j = 0; j < kPaddedHeight; ++j) {
            edgeY[j] = static_cast<float>(j);
        }
        {
            skvx::float4 v_pTopX(pTop.fX);
            skvx::float4 v_pTopY(pTop.fY);
            skvx::float4 v_dxdy(dxdy);

            int height;
            if constexpr (TileHeight > 4) {
                height = (endY + 3) / 4 * 4;
            } else {
                height = 4;
            }
            for (int k = 0; k < height; k += 4) {
                skvx::float4 v_gridY = skvx::float4::Load(edgeY + k);
                skvx::float4 v_gridX = v_pTopX + (v_gridY - v_pTopY) * v_dxdy;
                v_gridX.store(edgeX + k);
            }

            edgeX[startY] = pTop.fX;
            edgeY[startY] = pTop.fY;
            edgeX[endY] = pBot.fX;
            edgeY[endY] = pBot.fY;
        }

        // LUT indexing setup
        float nx = dy;
        float ny = -dx;
        if (nx < 0.0f) {
            nx = -nx;
            ny = -ny;
        }
        float D = nx + std::abs(ny);
        float invD = (D < kStripEpsilon) ? 0.0f : 1.0f / D;
        bool isPos = ny <= 0.0f;
        float c_local = nx * pTop.fX + ny * pTop.fY;
        float s = std::abs(ny) * invD;
        int v_mod = std::clamp(
                static_cast<int>(std::floor(s * (kMaskHeight / 2))), 0, (kMaskHeight / 2) - 1);
        int v = isPos ? (v_mod + kMaskHeight / 2) : v_mod;
        float stepX = nx * invD;
        float stepY = ny * invD;
        float t_base = ((isPos ? nx : D) - c_local) * invD;

        bool xDir = pTop.fX <= pBot.fX;
        for (int y = startY; y < endY; ++y) {
            float p_top_x = edgeX[y];
            float p_top_y = edgeY[y];
            float p_bot_x = edgeX[y + 1];
            float p_bot_y = edgeY[y + 1];

            SkASSERT(!std::isnan(p_top_x) && !std::isnan(p_top_y) && !std::isnan(p_bot_x) &&
                     !std::isnan(p_bot_y));

            float xMin = std::fmin(p_top_x, p_bot_x);
            float xMax = std::fmax(p_top_x, p_bot_x);
            int xStart = std::clamp(static_cast<int>(std::floor(xMin)), 0, TileWidth - 1);
            int xEnd = std::clamp(static_cast<int>(std::floor(xMax)), 0, TileWidth - 1);

            // Base for the row and intnorm precision.
            float t_val =
                    t_base + stepY * static_cast<float>(y) + stepX * static_cast<float>(xStart);
            int32_t t_fixed = static_cast<int32_t>(t_val * 64.0f * 65536.0f);
            int32_t stepX_fixed = static_cast<int32_t>(stepX * 64.0f * 65536.0f);
            int lut_row_offset = v * kMaskWidth;

            // Endcap Bump setup
            bool is_startY = (y == startY);
            bool is_endY_minus_1 = (y == endY - 1);
            bool pixelTopTouch = (p_top_y != std::floor(p_top_y));
            bool default_bumped = pixelTopTouch && xDir;

            uint8_t start_mask_val = 0xff;
            bool start_bumped = default_bumped;
            if (is_startY) {
                start_bumped = (p_top_x == 0.0f && pixelTopTouch);
                if (!start_bumped) {
                    uint32_t shift = static_cast<uint32_t>(
                            std::round(8.0f * (p_top_y - static_cast<float>(y))));
                    start_mask_val = static_cast<uint8_t>(0xff << shift);
                }
            }

            uint8_t end_mask_val = 0xff;
            if (is_endY_minus_1 && p_bot_x != 0.0f) {
                uint32_t shift =
                        static_cast<uint32_t>(std::round(8.0f * (p_bot_y - static_cast<float>(y))));
                end_mask_val = static_cast<uint8_t>(~(0xff << shift));
            }
            if (!is_startY) start_mask_val = 0xff;
            if (!is_endY_minus_1) end_mask_val = 0xff;

            auto processPixel = [&](int px, uint8_t edgeMask, uint32_t bumpVal) {
                int u = std::clamp(t_fixed >> 16, 0, kMaskWidthExcl);
                SkASSERT(v * kMaskWidth + u < maskLut.size());
                uint8_t maskVal = maskLut[lut_row_offset + u] & edgeMask;

                uint32_t mask_lo = ((maskVal & 0x0F) * 0x00204081u) & 0x01010101u;
                uint32_t mask_hi = (((maskVal >> 4) & 0x0F) * 0x00204081u) & 0x01010101u;

                pixel_bytes p_res = sk_bit_cast<pixel_bytes>(swar_pixel(mask_lo, mask_hi));
                pixel_bytes p_mask = sk_bit_cast<pixel_bytes>(mask[y][px]);

                pixel_bytes p_bump(bumpVal);

                if constexpr (kIsWinding) {
                    p_res -= p_bump;
                    if (canonicalYDir) {
                        p_mask += p_res;
                    } else {
                        p_mask -= p_res;
                    }
                } else {
                    p_res ^= p_bump;
                    p_mask ^= p_res;
                }

                mask[y][px] = sk_bit_cast<swar_pixel>(p_mask);
                t_fixed += stepX_fixed;
            };

            if (xStart == xEnd) {
                uint8_t combinedMask = start_mask_val & end_mask_val;
                processPixel(xStart, combinedMask, start_bumped);
            } else {
                uint8_t leftMask = xDir ? start_mask_val : end_mask_val;
                processPixel(xStart, leftMask, xDir ? start_bumped : default_bumped);

                uint32_t midBumped = default_bumped ? 1 : 0;
                for (int x = xStart + 1; x < xEnd; ++x) {
                    processPixel(x, 0xff, midBumped);
                }

                uint8_t rightMask = xDir ? end_mask_val : start_mask_val;
                processPixel(xEnd, rightMask, xDir ? default_bumped : start_bumped);
            }

            bool crossedTop = (y > startY) || (p_top_y == std::floor(p_top_y));
            if (crossedTop) {
                uint8_t fill_byte;
                if constexpr (kIsWinding) {
                    fill_byte = canonicalYDir ? 1 : 0xFF;
                } else {
                    fill_byte = 1;
                }

                int x = xEnd + 1;
                uint32_t fill_32 = fill_byte * 0x01010101u;
                if constexpr (TileWidth >= 8) {
                    skvx::Vec<16, uint32_t> v_fill16_32 = {
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32,
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32 };
                    while (x + 8 <= TileWidth) {
                        auto v_mask = skvx::Vec<16, uint32_t>::Load(&mask[y][x]);
                        if constexpr (kIsWinding) {
                            auto m_b = sk_bit_cast<skvx::Vec<64, uint8_t>>(v_mask);
                            auto f_b = sk_bit_cast<skvx::Vec<64, uint8_t>>(v_fill16_32);
                            v_mask = sk_bit_cast<skvx::Vec<16, uint32_t>>(m_b + f_b);
                        } else {
                            v_mask ^= v_fill16_32;
                        }
                        v_mask.store(&mask[y][x]);
                        x += 8;
                    }
                } else if constexpr (TileWidth >= 4) {
                    skvx::Vec<8, uint32_t> v_fill8_32 = {
                        fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32 };
                    while (x + 4 <= TileWidth) {
                        auto v_mask = skvx::Vec<8, uint32_t>::Load(&mask[y][x]);
                        if constexpr (kIsWinding) {
                            auto m_b = sk_bit_cast<skvx::Vec<32, uint8_t>>(v_mask);
                            auto f_b = sk_bit_cast<skvx::Vec<32, uint8_t>>(v_fill8_32);
                            v_mask = sk_bit_cast<skvx::Vec<8, uint32_t>>(m_b + f_b);
                        } else {
                            v_mask ^= v_fill8_32;
                        }
                        v_mask.store(&mask[y][x]);
                        x += 4;
                    }
                }

                // Remainder scalar loop
                const pixel_bytes p_fill(fill_byte);
                for (; x < TileWidth; ++x) {
                    pixel_bytes p_mask = sk_bit_cast<pixel_bytes>(mask[y][x]);
                    if constexpr (kIsWinding) {
                        p_mask += p_fill;
                    } else {
                        p_mask ^= p_fill;
                    }
                    mask[y][x] = sk_bit_cast<swar_pixel>(p_mask);
                }
            }
        }
    }

    alphaBuffer.resize(localAlphaIdx);

    if (isInverse) {
        emitBackground((prevTile.y + 1) * TileHeight, viewportHeight + TileHeight, viewportWidth);
    }
}

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_Strip_DEFINED
