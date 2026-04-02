/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_MakeStrip_impl_DEFINED
#define skgpu_graphite_MakeStrip_impl_DEFINED

#include "include/core/SkRect.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <type_traits>

namespace skgpu::graphite {

#if defined(GPU_TEST_UTILS)
template <typename MSAAType>
using MsaaExactMaskObserver = std::function<void(MSAAType exactMask)>;
#endif

class MakeStripImpl {
public:
    static constexpr int32_t  kMaskWidth        = SparseStripsConfig::kLutWidth;
    static constexpr float    kMaskWidthF       = static_cast<float>(kMaskWidth);
    static constexpr int32_t  kMaskWidthExcl    = kMaskWidth - 1;
    static constexpr int32_t  kMaskHeight       = SparseStripsConfig::kLutHeight;

    static constexpr uint32_t W                 = SparseStripsConstants::W;
    static constexpr uint32_t T                 = SparseStripsConstants::T;
    static constexpr uint32_t B                 = SparseStripsConstants::B;
    static constexpr uint32_t L                 = SparseStripsConstants::L;
    static constexpr uint32_t R                 = SparseStripsConstants::R;
    static constexpr uint32_t INT_MASK_SHIFT    = SparseStripsConstants::INT_MASK_SHIFT;
    static constexpr uint32_t INTERSECTION_MASK = SparseStripsConstants::INTERSECTION_MASK;

    static constexpr float kStripEpsilon        = SparseStripsConstants::kStripEpsilon;

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

        const float width = tileMaxX - tileMinX;
        const float height = tileMaxY - tileMinY;

        result[0].fX = std::fmin(std::fmax(result[0].fX, 0.0f), width);
        result[0].fY = std::fmin(std::fmax(result[0].fY, 0.0f), height);
        result[1].fX = std::fmin(std::fmax(result[1].fX, 0.0f), width);
        result[1].fY = std::fmin(std::fmax(result[1].fY, 0.0f), height);

        return result;
    }

    template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding, typename MSAAType>
    static void MakeStripsImpl(Tiles<TileWidth, TileHeight>& tileContainer,
                               SkTDArray<Strip>& stripBuf,
                               SkTDArray<MSAAType>& alphaBuf,
                               bool isInverse,
                               const Polyline& polyline,
                               const SkTDArray<MSAAType>& maskLut,
                               WideTiles<TileHeight>& wides,
                               bool isCulled,
                               const WindingHistogram& windingHistogram,
                               const SkIRect& clipRect
#if defined(GPU_TEST_UTILS)
                               , MsaaExactMaskObserver<MSAAType> observer = nullptr
#endif
    );

    template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding, typename MSAAType>
    static void MakeStripsSimdImpl(Tiles<TileWidth, TileHeight>& tileContainer,
                                   SkTDArray<Strip>& stripBuf,
                                   SkTDArray<MSAAType>& alphaBuf,
                                   bool isInverse,
                                   const Polyline& polyline,
                                   const SkTDArray<MSAAType>& maskLut,
                                   WideTiles<TileHeight>& wides,
                                   bool isCulled,
                                   const WindingHistogram& windingHistogram,
                                   const SkIRect& clipRect
#if defined(GPU_TEST_UTILS)
                                   , MsaaExactMaskObserver<MSAAType> observer = nullptr
#endif
    );

    template <uint16_t TileWidth, uint16_t TileHeight, uint32_t kWorkGroupSize, bool kIsWinding, typename MSAAType>
    static uint32_t MakeStripsGpuImpl(Tiles<TileWidth, TileHeight, GpuTile<TileWidth, TileHeight>>& tileContainer,
                                      SkTDArray<Strip>& stripBuf,
                                      bool isInverse,
                                      SkPathFillType fillType,
                                      const Polyline& polyline,
                                      WideTiles<TileHeight>& wides,
                                      bool isCulled,
                                      const WindingHistogram& windingHistogram,
                                      const SkIRect& clipRect,
                                      int32_t tileOffset,
                                      int32_t alphaOffset);

private:
    // Core tile traversal state machine. This template completely deduplicates the complex outer
    // looping, tile starting, strip pushing, and culling behaviors without any performance overhead.
    template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding, typename TileContainer,
              typename F_AlphaIdx, typename F_Init, typename F_OnTileStart, typename F_OnRowStart, typename F_OnLine>
    static void TraverseStrips(TileContainer& tileContainer,
                               SkTDArray<Strip>& stripBuf,
                               bool isInverse,
                               bool isCulled,
                               const WindingHistogram& windingHistogram,
                               WideTiles<TileHeight>& wides,
                               const SkIRect& clipRect,
                               const Polyline& polyline,
                               int32_t tileOffset,
                               F_AlphaIdx&& getAlphaIdx,
                               F_Init&& init,
                               F_OnTileStart&& onTileStart,
                               F_OnRowStart&& onRowStart,
                               F_OnLine&& onLine) {
        auto shouldFill = [](int32_t w) {
            if constexpr (kIsWinding)
                return w != 0;
            else
                return (w & 1) != 0;
        };

        auto processEmptyTileRows = [&](uint16_t startTileY, uint16_t endTileY) {
            if (startTileY >= endTileY) return;

            uint16_t y = startTileY;
            uint16_t startX = static_cast<uint16_t>(clipRect.left());
            uint16_t width = static_cast<uint16_t>(clipRect.width());

            if (isCulled && isInverse) {
                const auto& activeRows = windingHistogram.activeRows();
                while (y < endTileY) {
                    int wordIdx = y >> 5;
                    uint16_t wordEnd = std::min<uint16_t>(endTileY, (wordIdx + 1) << 5);
                    uint32_t mask = activeRows[wordIdx] & (~0u << (y & 31));
                    uint32_t endShift = wordEnd & 31;
                    if (endShift != 0) mask &= (1u << endShift) - 1;

                    while (mask != 0) {
                        int bit = std::countr_zero(mask);
                        uint16_t nextActiveY = (wordIdx << 5) + bit;
                        for (; y < nextActiveY; ++y) wides.addTile(startX, y * TileHeight, width);
                        if (!shouldFill(windingHistogram[y]))
                            wides.addTile(startX, y * TileHeight, width);
                        y++;
                        mask &= mask - 1;
                    }
                    for (; y < wordEnd; ++y) wides.addTile(startX, y * TileHeight, width);
                }
            } else if (isCulled) {
                const auto& activeRows = windingHistogram.activeRows();
                while (y < endTileY) {
                    int wordIdx = y >> 5;
                    uint16_t wordEnd = std::min<uint16_t>(endTileY, (wordIdx + 1) << 5);
                    uint32_t mask = activeRows[wordIdx] & (~0u << (y & 31));
                    uint32_t endShift = wordEnd & 31;
                    if (endShift != 0) mask &= (1u << endShift) - 1;

                    while (mask != 0) {
                        int bit = std::countr_zero(mask);
                        y = (wordIdx << 5) + bit;
                        if (shouldFill(windingHistogram[y]))
                            wides.addTile(startX, y * TileHeight, width);
                        mask &= mask - 1;
                    }
                    y = wordEnd;
                }
            } else if (isInverse) {
                for (; y < endTileY; ++y) wides.addTile(startX, y * TileHeight, width);
            }
        };

        auto& tiles = tileContainer.getTiles();
        if (tiles.size() <= static_cast<size_t>(tileOffset)) {
            processEmptyTileRows(0, windingHistogram.size());
            return;
        }

        size_t totalCount = tiles.size();
        using TileType = std::remove_reference_t<decltype(tiles[0])>;
        TileType prevTile = tiles[tileOffset];
        TileType sentinel = TileType(0xFFFF, 0xFFFF, 0, 0);

        int32_t windingDelta = windingHistogram[prevTile.y];
        init(prevTile, windingDelta);

        processEmptyTileRows(0, prevTile.y);
        bool fillLeftStart = shouldFill(windingDelta) ^ isInverse;
        if (isInverse || isCulled) {
            if (fillLeftStart) {
                stripBuf.push_back(Strip(static_cast<uint16_t>(clipRect.left()),
                                         prevTile.y * TileHeight,
                                         getAlphaIdx(),
                                         false));
            }
        }

        Strip currentStrip(
                prevTile.x * TileWidth, prevTile.y * TileHeight, getAlphaIdx(), fillLeftStart);

        for (size_t i = static_cast<size_t>(tileOffset); i <= totalCount; ++i) {
            bool isSentinel = (i == totalCount);
            TileType& tile = isSentinel ? sentinel : tiles[i];

            bool tileStart = (i == tileOffset) || (tile.x != prevTile.x || tile.y != prevTile.y);
            bool rowStart = (i == tileOffset) || (tile.y != prevTile.y);
            bool segStart = tileStart && !((tile.y == prevTile.y) && (tile.x == prevTile.x + 1));

            if (tileStart) {
                onTileStart(tile, rowStart, i, windingDelta);
            }

            if (segStart) {
                stripBuf.push_back(currentStrip);

                bool fillLeftCaptive = false;
                if (rowStart) {
                    if (windingDelta != 0 || isInverse || isSentinel) {
                        stripBuf.push_back(Strip(0xFFFF,
                                                 prevTile.y * TileHeight,
                                                 getAlphaIdx(),
                                                 shouldFill(windingDelta) ^ isInverse));
                    }

                    if (!isSentinel) {
                        processEmptyTileRows(prevTile.y + 1, tile.y);

                        windingDelta = windingHistogram[tile.y];
                        onRowStart(windingDelta);
                        fillLeftCaptive = shouldFill(windingDelta) ^ isInverse;

                        if (fillLeftCaptive) {
                            stripBuf.push_back(Strip(static_cast<uint16_t>(clipRect.left()),
                                                     tile.y * TileHeight,
                                                     getAlphaIdx(),
                                                     false));
                        }
                    }
                } else {
                    fillLeftCaptive = shouldFill(windingDelta) ^ isInverse;
                }

                if (!isSentinel) {
                    currentStrip = Strip(tile.x * TileWidth,
                                         tile.y * TileHeight,
                                         getAlphaIdx(),
                                         fillLeftCaptive);
                }
            }

            if (isSentinel) break;

            prevTile = tile;

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

            onLine(tile, line, canonicalXDir, canonicalYDir, i);
        }

        processEmptyTileRows(prevTile.y + 1, windingHistogram.size());
    }
};

template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding, typename MSAAType>
void MakeStripImpl::MakeStripsImpl(Tiles<TileWidth, TileHeight>& tileContainer,
                                   SkTDArray<Strip>& stripBuf,
                                   SkTDArray<MSAAType>& alphaBuf,
                                   bool isInverse,
                                   const Polyline& polyline,
                                   const SkTDArray<MSAAType>& maskLut,
                                   WideTiles<TileHeight>& wides,
                                   bool isCulled,
                                   const WindingHistogram& windingHistogram,
                                   const SkIRect& clipRect
#if defined(GPU_TEST_UTILS)
                                   , MsaaExactMaskObserver<MSAAType> observer
#endif
) {
    constexpr int32_t  kNumSubSamples  = sizeof(MSAAType) * 8;
    constexpr float    kNumSubSamplesF = static_cast<float>(kNumSubSamples);
    constexpr MSAAType kSubSampleMask  = static_cast<MSAAType>((1 << kNumSubSamples) - 1);

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

    auto shouldFill = [](int32_t w) {
        if constexpr (kIsWinding) return w != 0;
        else return (w & 1) != 0;
    };

    constexpr int32_t kTileAlphaCount = TileWidth * TileHeight;
    int32_t localAlphaIdx = alphaBuf.size();
    size_t megaGuess = tileContainer.getTiles().size() * kTileAlphaCount * 2;
    alphaBuf.reserve(localAlphaIdx + megaGuess);

    float tileMinX = 0, tileMinY = 0, tileMaxX = 0, tileMaxY = 0;

    TraverseStrips<TileWidth, TileHeight, kIsWinding>(
        tileContainer, stripBuf, isInverse, isCulled, windingHistogram, wides, clipRect, polyline, 0,
        /* getAlphaIdx */ [&]() { return localAlphaIdx; },
        /* init */ [&](const auto& pt, int32_t wd) {
            clearMask(wd);
            tileMinX = static_cast<float>(pt.x * TileWidth);
            tileMinY = static_cast<float>(pt.y * TileHeight);
            tileMaxX = tileMinX + static_cast<float>(TileWidth);
            tileMaxY = tileMinY + static_cast<float>(TileHeight);
        },
        /* onTileStart */ [&](const auto& tile, bool rowStart, size_t i, int32_t currentWd) {
            if (localAlphaIdx + kTileAlphaCount > alphaBuf.capacity()) {
                constexpr size_t kChunkSize = 4 * kTileAlphaCount;
                alphaBuf.reserve(alphaBuf.capacity() + kChunkSize);
            }
            MSAAType* alphaBase = alphaBuf.begin();

            for (int32_t y = 0; y < TileHeight; ++y) {
                for (int32_t x = 0; x < TileWidth; ++x) {
                    int32_t activeSamples = 0;

                    for (int32_t k = 0; k < kNumSubSamples; ++k) {
                        if (shouldFill(maskBuffer[y][x][k])) activeSamples++;
                    }
                    if (isInverse) activeSamples = kNumSubSamples - activeSamples;

#if defined(GPU_TEST_UTILS)
                    if (observer) {
                        MSAAType exactMask = 0;
                        for (int32_t k = 0; k < kNumSubSamples; ++k) {
                            if (shouldFill(maskBuffer[y][x][k])) exactMask |= (1 << k);
                        }
                        if (isInverse) exactMask = ~exactMask & kSubSampleMask;
                        observer(exactMask);
                    }
#endif

                    MSAAType alpha = static_cast<MSAAType>((activeSamples * kSubSampleMask + (kNumSubSamples / 2)) / kNumSubSamples);
                    alphaBase[localAlphaIdx++] = alpha;
                }
            }

            if (!rowStart) clearMask(currentWd);

            tileMinX = static_cast<float>(tile.x * TileWidth);
            tileMinY = static_cast<float>(tile.y * TileHeight);
            tileMaxX = tileMinX + static_cast<float>(TileWidth);
            tileMaxY = tileMinY + static_cast<float>(TileHeight);
        },
        /* onRowStart */ [&](int32_t wd) {
            clearMask(wd);
        },
        /* onLine */ [&](const auto& tile, Line line, bool canonicalXDir, bool canonicalYDir, size_t i) {
            float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
            if (rightEdge < 0.0f) return;

            float dx = line.p1.fX - line.p0.fX;
            float dy = line.p1.fY - line.p0.fY;
            float idx = (std::abs(dx) <= kStripEpsilon) ? 0.0f : 1.0f / dx;
            float idy = (std::abs(dy) <= kStripEpsilon) ? 0.0f : 1.0f / dy;
            float dxdy = dx * idy;

            std::array<float, 4> bounds = {tileMinX, tileMinY, tileMaxX, tileMaxY};
            std::array<float, 4> derivs = {dx, dy, idx, idy};

            auto clipped = clipToTile(line, bounds, derivs, tile.packedWindingLineIdx & INTERSECTION_MASK, canonicalXDir, canonicalYDir);
            SkPoint pTop = clipped[0];
            SkPoint pBot = clipped[1];

            if (tile.packedWindingLineIdx & L) {
                float yEdge = pTop.fX < pBot.fX ? pTop.fY : pBot.fY;
                int32_t startY = static_cast<int32_t>(std::ceil(yEdge));

                int32_t val;
                if constexpr (kIsWinding) val = canonicalXDir ? -1 : 1;
                else val = 1;

                for (int32_t y = startY; y < TileHeight; ++y) {
                    for (int32_t x = 0; x < TileWidth; ++x) {
                        for (int32_t k = 0; k < kNumSubSamples; ++k) {
                            if constexpr (kIsWinding) maskBuffer[y][x][k] += val;
                            else maskBuffer[y][x][k] ^= val;
                        }
                    }
                }
            }

            if (std::abs(dy) < kStripEpsilon && pTop.fY == std::floor(pTop.fY)) return;

            int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
            int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));

            std::array<SkPoint, TileHeight + 1> rows;
            rows.fill(SkPoint::Make(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()));
            rows[startY] = pTop;
            for (int32_t y = startY + 1; y < endY; ++y) {
                float gy = static_cast<float>(y);
                float gx = pTop.fX + (gy - pTop.fY) * dxdy;
                rows[y] = {gx, gy};
            }
            rows[endY] = pBot;

            float nx = dy;
            float ny = -dx;
            if (nx < 0.0f) { nx = -nx; ny = -ny; }
            float D = nx + std::abs(ny);
            float invD = (D < kStripEpsilon) ? 0.0f : 1.0f / D;
            bool isPos = ny <= 0.0f;
            float c_local = nx * pTop.fX + ny * pTop.fY;
            float s = std::abs(ny) * invD;
            int v_mod = std::clamp(static_cast<int>(std::floor(s * (kMaskHeight / 2))), 0, (kMaskHeight / 2) - 1);
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

                if (std::isnan(p_top_x) || std::isnan(p_bot_x)) continue;

                float xMin = std::fmin(p_top_x, p_bot_x);
                float xMax = std::fmax(p_top_x, p_bot_x);

                int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(xMin)), 0, TileWidth - 1);
                int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, TileWidth - 1);

                float t_val = t_base + stepY * static_cast<float>(y) + stepX * static_cast<float>(xStart);

                bool is_startY = (y == startY);
                bool is_endY_minus_1 = (y == endY - 1);
                bool pixelTopTouch = (p_top_y != std::floor(p_top_y));
                bool default_bumped = pixelTopTouch && xDir;

                int canonical_start_x = xDir ? xStart : xEnd;
                int canonical_end_x = xDir ? xEnd : xStart;

                MSAAType start_mask_val = kSubSampleMask;
                bool start_bumped = default_bumped;
                if (is_startY) {
                    start_bumped = (p_top_x == 0.0f && pixelTopTouch);
                    if (!start_bumped) {
                        uint32_t shift = static_cast<uint32_t>(std::round(kNumSubSamplesF * (p_top_y - static_cast<float>(y))));
                        start_mask_val = static_cast<MSAAType>(kSubSampleMask << shift);
                    }
                }

                MSAAType end_mask_val = kSubSampleMask;
                if (is_endY_minus_1 && p_bot_x != 0.0f) {
                    uint32_t shift = static_cast<uint32_t>(std::round(kNumSubSamplesF * (p_bot_y - static_cast<float>(y))));
                    end_mask_val = static_cast<MSAAType>(~(kSubSampleMask << shift));
                }

                if (!is_startY) start_mask_val = kSubSampleMask;
                if (!is_endY_minus_1) end_mask_val = kSubSampleMask;

                for (int32_t x = xStart; x <= xEnd; ++x) {
                    int u = std::clamp(static_cast<int>(std::floor(t_val * kMaskWidthF)), 0, kMaskWidthExcl);
                    SkASSERT(v * kMaskWidth + u < maskLut.size());
                    MSAAType maskVal = maskLut[v * kMaskWidth + u];

                    MSAAType edgeMask = kSubSampleMask;
                    if (x == canonical_start_x) edgeMask &= start_mask_val;
                    if (x == canonical_end_x) edgeMask &= end_mask_val;
                    maskVal &= edgeMask;

                    bool bumped = (is_startY && x == canonical_start_x) ? start_bumped : default_bumped;
                    for (int32_t k = 0; k < kNumSubSamples; ++k) {
                        int32_t maskBit = (maskVal & (1 << k)) ? 1 : 0;
                        if constexpr (kIsWinding) {
                            maskBit -= bumped;
                            if (maskBit != 0) {
                                if (canonicalYDir) maskBuffer[y][x][k] += maskBit;
                                else maskBuffer[y][x][k] -= maskBit;
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
                    if constexpr (kIsWinding) val = canonicalYDir ? 1 : -1;
                    else val = 1;
                    for (int32_t x = xEnd + 1; x < TileWidth; ++x) {
                        for (int32_t k = 0; k < kNumSubSamples; ++k) {
                            if constexpr (kIsWinding) maskBuffer[y][x][k] += val;
                            else maskBuffer[y][x][k] ^= val;
                        }
                    }
                }
            }
        }
    );

    alphaBuf.resize(localAlphaIdx);
}

template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding, typename MSAAType>
void MakeStripImpl::MakeStripsSimdImpl(Tiles<TileWidth, TileHeight>& tileContainer,
                                       SkTDArray<Strip>& stripBuf,
                                       SkTDArray<MSAAType>& alphaBuffer,
                                       bool isInverse,
                                       const Polyline& polyline,
                                       const SkTDArray<MSAAType>& maskLut,
                                       WideTiles<TileHeight>& wides,
                                       bool isCulled,
                                       const WindingHistogram& windingHistogram,
                                       const SkIRect& clipRect
#if defined(GPU_TEST_UTILS)
                                       , MsaaExactMaskObserver<MSAAType> observer
#endif
) {
    constexpr int32_t  kNumSubSamples  = sizeof(MSAAType) * 8;
    constexpr float    kNumSubSamplesF = static_cast<float>(kNumSubSamples);
    constexpr MSAAType kSubSampleMask  = static_cast<MSAAType>((1 << kNumSubSamples) - 1);
    constexpr MSAAType kMSAASubAsAdd   = static_cast<MSAAType>(-1);
    constexpr MSAAType kMSAABias       = static_cast<MSAAType>(1 << (kNumSubSamples - 1));

    auto deltaToMask = [&](int32_t w) {
        MSAAType w_byte;
        if constexpr (kIsWinding) w_byte = kMSAABias + static_cast<MSAAType>(static_cast<int8_t>(w));
        else w_byte = static_cast<MSAAType>(w & 1);
        return static_cast<uint32_t>(w_byte) * 0x01010101u;
    };

    constexpr int32_t kTileAlphaCount = TileWidth * TileHeight;
    int32_t localAlphaIdx = alphaBuffer.size();
    size_t megaGuess = tileContainer.getTiles().size() * kTileAlphaCount * 2;
    alphaBuffer.reserve(localAlphaIdx + megaGuess);

    using swar_pixel = skvx::Vec<2, uint32_t>;
    using pixel_bytes = skvx::Vec<8, MSAAType>;
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
                for (int i = 0; i < totalPixels; i += 8) v_val.store(flat_mask + i);
            } else if constexpr (TileWidth % 4 == 0) {
                skvx::Vec<8, uint32_t> v_val = {val, val, val, val, val, val, val, val};
                for (int i = 0; i < totalPixels; i += 4) v_val.store(flat_mask + i);
            } else {
                swar_pixel p = {val, val};
                for (int i = 0; i < totalPixels; ++i) flat_mask[i] = p;
            }
        }
    };

    float tileMinX = 0, tileMinY = 0, tileMaxX = 0, tileMaxY = 0;

    TraverseStrips<TileWidth, TileHeight, kIsWinding>(
        tileContainer, stripBuf, isInverse, isCulled, windingHistogram, wides, clipRect, polyline, 0,
        /* getAlphaIdx */ [&]() { return localAlphaIdx; },
        /* init */ [&](const auto& pt, int32_t wd) {
            resetMask(deltaToMask(wd));
            tileMinX = static_cast<float>(pt.x * TileWidth);
            tileMinY = static_cast<float>(pt.y * TileHeight);
            tileMaxX = tileMinX + static_cast<float>(TileWidth);
            tileMaxY = tileMinY + static_cast<float>(TileHeight);
        },
        /* onTileStart */ [&](const auto& tile, bool rowStart, size_t i, int32_t currentWd) {
            if (localAlphaIdx + kTileAlphaCount > alphaBuffer.capacity()) {
                constexpr size_t kChunkSize = 4 * kTileAlphaCount;
                alphaBuffer.reserve(alphaBuffer.capacity() + kChunkSize);
            }
            MSAAType* alphaBase = alphaBuffer.begin();

            auto get_counts_wide = [&](auto v) {
                using VecT = decltype(v);
                if constexpr (kIsWinding) {
                    using Vec8 = skvx::Vec<sizeof(VecT), MSAAType>;
                    Vec8 v_bytes = sk_bit_cast<Vec8>(v);
                    Vec8 bias_bytes(kMSAABias);
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

                if (isInverse) activeSamples = skvx::Vec<ChunkSize, uint32_t>(kNumSubSamples) - activeSamples;

                skvx::Vec<ChunkSize, uint32_t> alpha32 = (activeSamples * kSubSampleMask + (kNumSubSamples / 2)) / kNumSubSamples;
                skvx::cast<MSAAType>(alpha32).store(alphaBase + localAlphaIdx);
                localAlphaIdx += ChunkSize;

#if defined(GPU_TEST_UTILS)
                if (observer) {
                    auto shouldFillObs = [](int32_t w) {
                        if constexpr (kIsWinding) return w != 0;
                        else return (w & 1) != 0;
                    };
                    for (int xx = x; xx < x + ChunkSize; ++xx) {
                        swar_pixel v = mask[y][xx];
                        MSAAType exactMask = 0;
                        uint32_t lo = v[0];
                        uint32_t hi = v[1];
                        for (int s = 0; s < 4; ++s) {
                            int8_t s_lo = (int8_t)(lo & kSubSampleMask);
                            int8_t s_hi = (int8_t)(hi & kSubSampleMask);
                            if constexpr (kIsWinding) {
                                s_lo = (int8_t)((MSAAType)s_lo - kMSAABias);
                                s_hi = (int8_t)((MSAAType)s_hi - kMSAABias);
                            }
                            if (shouldFillObs(s_lo)) exactMask |= (1 << s);
                            if (shouldFillObs(s_hi)) exactMask |= (1 << (s + 4));
                            lo >>= 8;
                            hi >>= 8;
                        }
                        if (isInverse) exactMask = ~exactMask & kSubSampleMask;
                        observer(exactMask);
                    }
                }
#endif
            };

            for (int y = 0; y < TileHeight; ++y) {
                if constexpr (TileWidth % 8 == 0) {
                    for (int x = 0; x < TileWidth; x += 8) process_chunk(std::integral_constant<int, 8>{}, y, x);
                } else if constexpr (TileWidth % 4 == 0) {
                    for (int x = 0; x < TileWidth; x += 4) process_chunk(std::integral_constant<int, 4>{}, y, x);
                }
            }

            if (!rowStart) resetMask(deltaToMask(currentWd));

            tileMinX = (float)(tile.x * TileWidth);
            tileMinY = (float)(tile.y * TileHeight);
            tileMaxX = tileMinX + (float)TileWidth;
            tileMaxY = tileMinY + (float)TileHeight;
        },
        /* onRowStart */ [&](int32_t wd) {
            resetMask(deltaToMask(wd));
        },
        /* onLine */ [&](const auto& tile, Line line, bool canonicalXDir, bool canonicalYDir, size_t i) {
            float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
            if (rightEdge < 0.0f) return;

            float dx = line.p1.fX - line.p0.fX;
            float dy = line.p1.fY - line.p0.fY;
            float idx = (std::abs(dx) <= kStripEpsilon) ? 0.0f : 1.0f / dx;
            float idy = (std::abs(dy) <= kStripEpsilon) ? 0.0f : 1.0f / dy;
            float dxdy = dx * idy;

            std::array<float, 4> bounds = {tileMinX, tileMinY, tileMaxX, tileMaxY};
            std::array<float, 4> derivs = {dx, dy, idx, idy};

            auto clipped = clipToTile(line, bounds, derivs, tile.packedWindingLineIdx & INTERSECTION_MASK, canonicalXDir, canonicalYDir);
            SkPoint pTop = clipped[0];
            SkPoint pBot = clipped[1];

            // --- Wide SIMD L Boundary Fill ---
            if (tile.packedWindingLineIdx & L) {
                float yEdge = (pTop.fX < pBot.fX) ? pTop.fY : pBot.fY;
                MSAAType fill_byte;
                if constexpr (kIsWinding) fill_byte = canonicalXDir ? kMSAASubAsAdd : 1;
                else fill_byte = 1;

                uint32_t fill_32 = fill_byte * 0x01010101u;
                for (int y = (int)std::ceil(yEdge); y < TileHeight; ++y) {
                    if constexpr (TileWidth % 8 == 0) {
                        skvx::Vec<16, uint32_t> v_fill16_32 = {
                            fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32,
                            fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32 };
                        for (int x = 0; x < TileWidth; x += 8) {
                            auto v_mask = skvx::Vec<16, uint32_t>::Load(&mask[y][x]);
                            if constexpr (kIsWinding) {
                                auto m_b = sk_bit_cast<skvx::Vec<64, MSAAType>>(v_mask);
                                auto f_b = sk_bit_cast<skvx::Vec<64, MSAAType>>(v_fill16_32);
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
                                auto m_b = sk_bit_cast<skvx::Vec<32, MSAAType>>(v_mask);
                                auto f_b = sk_bit_cast<skvx::Vec<32, MSAAType>>(v_fill8_32);
                                v_mask = sk_bit_cast<skvx::Vec<8, uint32_t>>(m_b + f_b);
                            } else {
                                v_mask ^= v_fill8_32;
                            }
                            v_mask.store(&mask[y][x]);
                        }
                    }
                }
            }

            if (std::abs(dy) < kStripEpsilon && pTop.fY == std::floor(pTop.fY)) return;

            int startY = (int)std::floor(pTop.fY);
            int endY = (int)std::ceil(pBot.fY);

            constexpr int kPaddedHeight = ((TileHeight + 1 + 3) / 4) * 4;
            float edgeX[kPaddedHeight];
            float edgeY[kPaddedHeight];
            for (int j = 0; j < kPaddedHeight; ++j) edgeY[j] = static_cast<float>(j);
            {
                skvx::float4 v_pTopX(pTop.fX);
                skvx::float4 v_pTopY(pTop.fY);
                skvx::float4 v_dxdy(dxdy);

                int height;
                if constexpr (TileHeight > 4) height = (endY + 3) / 4 * 4;
                else height = 4;

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

            float nx = dy;
            float ny = -dx;
            if (nx < 0.0f) { nx = -nx; ny = -ny; }
            float D = nx + std::abs(ny);
            float invD = (D < kStripEpsilon) ? 0.0f : 1.0f / D;
            bool isPos = ny <= 0.0f;
            float c_local = nx * pTop.fX + ny * pTop.fY;
            float s = std::abs(ny) * invD;
            int v_mod = std::clamp(static_cast<int>(std::floor(s * (kMaskHeight / 2))), 0, (kMaskHeight / 2) - 1);
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

                SkASSERT(!std::isnan(p_top_x) && !std::isnan(p_top_y) && !std::isnan(p_bot_x) && !std::isnan(p_bot_y));

                float xMin = std::fmin(p_top_x, p_bot_x);
                float xMax = std::fmax(p_top_x, p_bot_x);
                int xStart = std::clamp(static_cast<int>(std::floor(xMin)), 0, TileWidth - 1);
                int xEnd = std::clamp(static_cast<int>(std::floor(xMax)), 0, TileWidth - 1);

                float t_val = t_base + stepY * static_cast<float>(y) + stepX * static_cast<float>(xStart);
                int32_t t_fixed = static_cast<int32_t>(t_val * 64.0f * 65536.0f);
                int32_t stepX_fixed = static_cast<int32_t>(stepX * 64.0f * 65536.0f);
                int lut_row_offset = v * kMaskWidth;

                bool is_startY = (y == startY);
                bool is_endY_minus_1 = (y == endY - 1);
                bool pixelTopTouch = (p_top_y != std::floor(p_top_y));
                bool default_bumped = pixelTopTouch && xDir;

                MSAAType start_mask_val = kSubSampleMask;
                bool start_bumped = default_bumped;
                if (is_startY) {
                    start_bumped = (p_top_x == 0.0f && pixelTopTouch);
                    if (!start_bumped) {
                        uint32_t shift = static_cast<uint32_t>(std::round(kNumSubSamplesF * (p_top_y - static_cast<float>(y))));
                        start_mask_val = static_cast<MSAAType>(kSubSampleMask << shift);
                    }
                }

                MSAAType end_mask_val = kSubSampleMask;
                if (is_endY_minus_1 && p_bot_x != 0.0f) {
                    uint32_t shift = static_cast<uint32_t>(std::round(kNumSubSamplesF * (p_bot_y - static_cast<float>(y))));
                    end_mask_val = static_cast<MSAAType>(~(kSubSampleMask << shift));
                }
                if (!is_startY) start_mask_val = kSubSampleMask;
                if (!is_endY_minus_1) end_mask_val = kSubSampleMask;

                auto processPixel = [&](int px, MSAAType edgeMask, uint32_t bumpVal) {
                    int u = std::clamp(t_fixed >> 16, 0, kMaskWidthExcl);
                    SkASSERT(v * kMaskWidth + u < maskLut.size());
                    MSAAType maskVal = maskLut[lut_row_offset + u] & edgeMask;

                    uint32_t mask_lo = ((maskVal & 0x0F) * 0x00204081u) & 0x01010101u;
                    uint32_t mask_hi = (((maskVal >> 4) & 0x0F) * 0x00204081u) & 0x01010101u;

                    pixel_bytes p_res = sk_bit_cast<pixel_bytes>(swar_pixel(mask_lo, mask_hi));
                    pixel_bytes p_mask = sk_bit_cast<pixel_bytes>(mask[y][px]);
                    pixel_bytes p_bump(bumpVal);

                    if constexpr (kIsWinding) {
                        p_res -= p_bump;
                        if (canonicalYDir) p_mask += p_res;
                        else p_mask -= p_res;
                    } else {
                        p_res ^= p_bump;
                        p_mask ^= p_res;
                    }

                    mask[y][px] = sk_bit_cast<swar_pixel>(p_mask);
                    t_fixed += stepX_fixed;
                };

                if (xStart == xEnd) {
                    MSAAType combinedMask = start_mask_val & end_mask_val;
                    processPixel(xStart, combinedMask, start_bumped);
                } else {
                    MSAAType leftMask = xDir ? start_mask_val : end_mask_val;
                    processPixel(xStart, leftMask, xDir ? start_bumped : default_bumped);

                    uint32_t midBumped = default_bumped ? 1 : 0;
                    for (int x = xStart + 1; x < xEnd; ++x) processPixel(x, kSubSampleMask, midBumped);

                    MSAAType rightMask = xDir ? end_mask_val : start_mask_val;
                    processPixel(xEnd, rightMask, xDir ? default_bumped : start_bumped);
                }

                bool crossedTop = (y > startY) || (p_top_y == std::floor(p_top_y));
                if (crossedTop) {
                    MSAAType fill_byte;
                    if constexpr (kIsWinding) fill_byte = canonicalYDir ? 1 : kMSAASubAsAdd;
                    else fill_byte = 1;

                    int x = xEnd + 1;
                    uint32_t fill_32 = fill_byte * 0x01010101u;
                    if constexpr (TileWidth >= 8) {
                        skvx::Vec<16, uint32_t> v_fill16_32 = {
                            fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32,
                            fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32, fill_32 };
                        while (x + 8 <= TileWidth) {
                            auto v_mask = skvx::Vec<16, uint32_t>::Load(&mask[y][x]);
                            if constexpr (kIsWinding) {
                                auto m_b = sk_bit_cast<skvx::Vec<64, MSAAType>>(v_mask);
                                auto f_b = sk_bit_cast<skvx::Vec<64, MSAAType>>(v_fill16_32);
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
                                auto m_b = sk_bit_cast<skvx::Vec<32, MSAAType>>(v_mask);
                                auto f_b = sk_bit_cast<skvx::Vec<32, MSAAType>>(v_fill8_32);
                                v_mask = sk_bit_cast<skvx::Vec<8, uint32_t>>(m_b + f_b);
                            } else {
                                v_mask ^= v_fill8_32;
                            }
                            v_mask.store(&mask[y][x]);
                            x += 4;
                        }
                    }

                    const pixel_bytes p_fill(fill_byte);
                    for (; x < TileWidth; ++x) {
                        pixel_bytes p_mask = sk_bit_cast<pixel_bytes>(mask[y][x]);
                        if constexpr (kIsWinding) p_mask += p_fill;
                        else p_mask ^= p_fill;
                        mask[y][x] = sk_bit_cast<swar_pixel>(p_mask);
                    }
                }
            }
        }
    );

    alphaBuffer.resize(localAlphaIdx);
}

template <uint16_t TileWidth, uint16_t TileHeight, uint32_t kWorkGroupSize, bool kIsWinding, typename MSAAType>
uint32_t MakeStripImpl::MakeStripsGpuImpl(
        Tiles<TileWidth, TileHeight, GpuTile<TileWidth, TileHeight>>& tileContainer,
        SkTDArray<Strip>& stripBuf,
        bool isInverse,
        SkPathFillType fillType,
        const Polyline& polyline,
        WideTiles<TileHeight>& wides,
        bool isCulled,
        const WindingHistogram& windingHistogram,
        const SkIRect& clipRect,
        int32_t tileOffset,
        int32_t alphaOffset) {
    static_assert(kWorkGroupSize != 0);

    constexpr int32_t kTileAlphaCount = TileWidth * TileHeight;
    int32_t tileStartId = tileOffset;
    int32_t leftEdgeWindingDelta = 0;
    uint32_t localAlphaIdx = 0;


    auto& tiles = tileContainer.getTiles();
    size_t totalCount = tiles.size();

    TraverseStrips<TileWidth, TileHeight, kIsWinding>(
        tileContainer, stripBuf, isInverse, isCulled, windingHistogram, wides, clipRect, polyline, tileOffset,
        /* getAlphaIdx */ [&]() { return localAlphaIdx; },
        /* init */ [&](const auto& pt, int32_t wd) { leftEdgeWindingDelta = wd; },
        /* onTileStart */ [&](const auto& tile, bool rowStart, size_t i, int32_t currentWd) {
            localAlphaIdx += kTileAlphaCount;
            tileStartId = i;
            leftEdgeWindingDelta = currentWd;
        },
        /* onRowStart */ [&](int32_t wd) {
            // Capture the correct left-edge winding for the new row
            leftEdgeWindingDelta = wd;
        },
        /* onLine */ [&](auto& tile, Line line, bool canonicalXDir, bool canonicalYDir, size_t i) {
            bool isEndTile = (i == totalCount - 1) || (tiles[i].x != tiles[i + 1].x || tiles[i].y != tiles[i + 1].y);
            constexpr int32_t kPartSize = static_cast<int32_t>(kWorkGroupSize / TileHeight);
            int32_t blockAligned = i / kPartSize * kPartSize;
            SkASSERT(tileStartId - blockAligned < kPartSize);
            int32_t blockLocalTileStartId = tileStartId < blockAligned ?
                                            SparseStripsConstants::INVALID_START_TILE :
                                            tileStartId - blockAligned;
            uint32_t fillRule = static_cast<uint32_t>(fillType);
            SkASSERT(fillRule <= SparseStripsConstants::FILL_RULE_MASK);
            tile.alphaIdx = localAlphaIdx + alphaOffset;
            tile.packed = ((static_cast<uint32_t>(leftEdgeWindingDelta) & 0xff) << SparseStripsConstants::SCANNED_WINDING_SHIFT) |
                          (fillRule << SparseStripsConstants::FILL_RULE_SHIFT) |
                          (isEndTile << SparseStripsConstants::IS_END_TILE_SHIFT) |
                          (static_cast<uint32_t>(blockLocalTileStartId));
        }
    );

    return localAlphaIdx;
}

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_MakeStrip_impl_DEFINED
