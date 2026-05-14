/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_Strip_DEFINED
#define skgpu_graphite_sparse_strips_Strip_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
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
                               const SkTDArray<uint8_t>& maskLut
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
                                                              maskLut
#if defined(GPU_TEST_UTILS)
                                                              ,
                                                              observer
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
                                   const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                                   , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        // STUB
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
                               const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                               , MsaaExactMaskObserver observer = nullptr
#endif
    );

    static constexpr uint32_t FILL_SHIFT = 31;

    static constexpr int kNumSubSamples  = 8;
    static constexpr float kPackingScale = 0.5f;
    static constexpr int kMaskWidth      = 64;
    static constexpr int kMaskHeight     = 64;
    static constexpr float kMaskWidthF   = static_cast<float>(kMaskWidth);
    static constexpr float kMaskHeightF  = static_cast<float>(kMaskHeight);
    static constexpr int kMaskWidthExcl  = kMaskWidth - 1;
    static constexpr int kMaskHeightExcl = kMaskHeight - 1;

    static constexpr float kStripEpsilon = 1e-5f;

    // Responsible for consuming the tile coordinates and intersection mask produced by the Tiler
    // and producing the exact points at which the parent line intersects the edges of the tile.
    // It also safely handles when there is only one edge intersection, either from the line ending
    // inside the tile body, or a horizontal edge touch.
    //
    // This function provides the following guarantees:
    //  1) Y-Sorting: The resulting points are always sorted top-to-bottom.
    //  2) Interior Endpoints: If a line ends inside the tile, its exact endpoint is preserved and
    //     returned.
    //  3) Zero-Width Degenerates: A single-point intersection (e.g., a horizontal graze or corner
    //     touch) produces a second point along that exact same edge, yielding safe, zero-width
    //     geometry.
    //  4) Boundary Snapping: Edge intersections are explicitly snapped to the exact bounding
    //     coordinate of the tile.
    //
    // (Note: Guarantees 2 and 4 ensure downstream logic can safely rely on floating-point equality
    // checks).
    //
    // Note, this function is tightly coupled to Tiler and MakeStrips. If the incoming tile
    // coordinates are incorrect, the results from this function are meaningless. It assumes that:
    //  1) Derivatives resulting from division by zero (e.g. perfectly vertical or horizontal lines)
    //     will be set to zero.
    //  2) Only two bits are ever set in the mask. (Expects that perfect corner touches are tie
    //     broken by the Tiler.)
    static SK_ALWAYS_INLINE std::array<SkPoint, 2> ClipToTile(
        const Line& line,
        const std::array<float, 4>& bounds,
        const std::array<float, 4>& derivatives,
        uint32_t intersectionMask,
        bool canonicalXDir,
        bool canonicalYDir) {
        SkPoint pEntry = line.p0;
        SkPoint pExit  = line.p1;

        const float tileMinX = bounds[0];
        const float tileMinY = bounds[1];
        const float tileMaxX = bounds[2];
        const float tileMaxY = bounds[3];

        const float dx = derivatives[0];
        const float dy = derivatives[1];

        // A line's direction dictates which edges it can cross from the outside-in (entry) versus
        // inside-out (exit). For example, if dx > 0 (canonicalXDir = true), the vector is monotonic
        // in the positive X direction. It is impossible to intersect the right edge as an entry
        // point, or the left edge as an exit point. This reduces the number of edges which must be
        // checked from four to two candidate entry and two candidate exit edges.
        uint32_t maskVIn, maskVOut;
        float boundVIn, boundVOut;
        if (canonicalXDir) {
            maskVIn  = L;
            boundVIn = tileMinX;
            maskVOut = R;
            boundVOut= tileMaxX;
        } else {
            maskVIn  = R;
            boundVIn = tileMaxX;
            maskVOut = L;
            boundVOut= tileMinX;
        }

        uint32_t maskHIn, maskHOut;
        float boundHIn, boundHOut;
        if (canonicalYDir) {
            maskHIn  = T;
            boundHIn = tileMinY;
            maskHOut = B;
            boundHOut= tileMaxY;
        } else {
            maskHIn  = B;
            boundHIn = tileMaxY;
            maskHOut = T;
            boundHOut= tileMinY;
        }

        const float idx = derivatives[2];
        const float idy = derivatives[3];

        // Check the candidate edges against the intersection mask
        const uint32_t entryHits = intersectionMask & (maskVIn | maskHIn);
        if (entryHits != 0) {
            // Since the Tiler guarantees strict tie-breaking upstream (never outputting conflicting
            // entry bits for a perfect corner hit), we can safely isolate the specific entry axis.
            const bool useH = (intersectionMask & maskHIn) != 0;

            const float bound = useH ? boundHIn   : boundVIn;
            const float start = useH ? line.p0.fY : line.p0.fX;
            const float invD  = useH ? idy        : idx;

            // Evaluate the parametric equation t = (bound - start) * (1 / delta)
            const float t = (bound - start) * invD;

            pEntry.fX = line.p0.fX + t * dx;
            pEntry.fY = line.p0.fY + t * dy;

            // Explicitly snap the specific axis to the bound to eliminate floating point drift
            if (useH) {
                pEntry.fY = bound;
            } else {
                pEntry.fX = bound;
            }
        }

        // Repeat for exit
        const uint32_t exitHits = intersectionMask & (maskVOut | maskHOut);
        if (exitHits != 0) {
            const bool useH = (intersectionMask & maskHOut) != 0;

            const float bound = useH ? boundHOut   : boundVOut;
            const float start = useH ? line.p0.fY  : line.p0.fX;
            const float invD  = useH ? idy         : idx;

            const float t = (bound - start) * invD;

            pExit.fX = line.p0.fX + t * dx;
            pExit.fY = line.p0.fY + t * dy;

            // Explicitly snap the specific axis to the bound
            if (useH) {
                pExit.fY = bound;
            } else {
                pExit.fX = bound;
            }
        }

        // Guarantee predictable winding order for downstream rasterization stages.
        std::array<SkPoint, 2> result;
        if (pExit.fY >= pEntry.fY) {
            result = {pEntry, pExit};
        } else {
            result = {pExit, pEntry};
        }

        // Shift from global viewport coordinates into tile-local space.
        result[0].fX -= tileMinX;
        result[0].fY -= tileMinY;
        result[1].fX -= tileMinX;
        result[1].fY -= tileMinY;

        // Clamping serves three functions here:
        //  1) Floating-Point Correction: Points which are slightly outside the tile due to floating
        //     point error are coerced inside.
        //  2) Confinement: Upstream logic sets reciprocal derivatives to 0 for perfectly horizontal
        //     or vertical lines. This causes the parametric math above to return the original
        //     unclipped coordinate. Clamping confines this "free" axis to the tile boundaries.
        //  3) Zero-Width Degenerates: If there is only a single edge crossing (e.g., a tie-broken
        //     corner hit or an inclusive touch), the opposing uncalculated endpoint is forced onto
        //     that same edge. This flattens the segment into zero-width geometry that the
        //     downstream coverage calculation will safely ignore.
        const float width  = tileMaxX - tileMinX;
        const float height = tileMaxY - tileMinY;

        result[0].fX = std::clamp(result[0].fX, 0.0f, width);
        result[0].fY = std::clamp(result[0].fY, 0.0f, height);
        result[1].fX = std::clamp(result[1].fX, 0.0f, width);
        result[1].fY = std::clamp(result[1].fY, 0.0f, height);

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
                           const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                           , MsaaExactMaskObserver observer
#endif
) {
    const auto& tiles = tileContainer.getTiles();
    if (tiles.empty()) {
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
    Tile prevTile = tiles[0];
    Strip currentStrip(prevTile.x * TileWidth, prevTile.y * TileHeight, localAlphaIdx, false);

    float tileMinX = static_cast<float>(prevTile.x * TileWidth);
    float tileMinY = static_cast<float>(prevTile.y * TileHeight);
    float tileMaxX = tileMinX + static_cast<float>(TileWidth);
    float tileMaxY = tileMinY + static_cast<float>(TileHeight);

    for (size_t i = 0; i <= totalCount; ++i) {
        bool isSentinel = (i == totalCount);
        Tile tile;

        if (isSentinel) {
            tile = Tile(0xFFFF, 0xFFFF, 0, 0);  // Sentinel
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
                if (windingDelta != 0 || isSentinel) {
                    stripBuf.push_back(Strip(0xFFFF,
                                             prevTile.y * TileHeight,
                                             localAlphaIdx,
                                             shouldFill(windingDelta)));
                }
                windingDelta = 0;
                clearMask(0);
            }

            if (isSentinel) {
                break;
            }
            currentStrip = Strip(tile.x * TileWidth,
                                 tile.y * TileHeight,
                                 localAlphaIdx,
                                 shouldFill(windingDelta));
        }

        prevTile = tile;
        if (isSentinel) {
            break;
        }

        Line line = polyline.getLine(tile.lineIdx());
        bool canonicalXDir = line.p1.fX >= line.p0.fX;
        bool canonicalYDir = line.p1.fY >= line.p0.fY;

        uint32_t windingBit = tile.coarseWinding() ? 1 : 0;
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

        auto clipped = ClipToTile(line,
                                  bounds,
                                  derivs,
                                  tile.intersectionMask(),
                                  canonicalXDir,
                                  canonicalYDir);
        SkPoint pTop = clipped[0];
        SkPoint pBot = clipped[1];

        if (tile.left()) {
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
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_Strip_DEFINED
