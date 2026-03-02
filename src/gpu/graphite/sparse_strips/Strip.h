/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_Strip_DEFINED
#define skgpu_graphite_Strip_DEFINED

#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

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
                               std::vector<Strip>& stripBuf,
                               AlphaAtlasManager* alphaAtlasManager,
                               SkPathFillType fillType,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>* maskLut
#if defined(GPU_TEST_UTILS)
                               , MsaaExactMaskObserver observer = nullptr
#endif
                               );

private:
#if defined(GPU_TEST_UTILS)
    template <uint16_t, uint16_t> friend class ClipTestRunner;
    template <uint16_t, uint16_t> friend class CoverageTestRunner;
#endif
    friend class StripGenerator;

    static constexpr uint32_t FILL_SHIFT = 31;

    static constexpr int kNumSubSamples = 8;
    static constexpr float kPackingScale = 0.5f;
    static constexpr int kMaskWidth = 64;
    static constexpr int kMaskHeight = 64;
    static constexpr float kMaskWidthF = static_cast<float>(kMaskWidth);
    static constexpr float kMaskHeightF = static_cast<float>(kMaskHeight);
    static constexpr int kMaskWidthExcl = kMaskWidth - 1;
    static constexpr int kMaskHeightExcl = kMaskWidth - 1;

    static SK_ALWAYS_INLINE std::array<SkPoint, 2> clipToTile(
        const Line& line,
        const std::array<float, 4>& bounds,
        const std::array<float, 4>& derivatives,
        uint32_t intersectionData,
        bool cannonicalXDir,
        bool cannonicalYDir
    ) {
        SkPoint pEntry = line.p0;
        SkPoint pExit  = line.p1;

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
            const float start = useH ? line.p0.fY  : line.p0.fX;
            const float invD  = useH ? idy        : idx;

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
            const float start = useH ? line.p0.fY   : line.p0.fX;
            const float invD  = useH ? idy         : idx;

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

template <uint16_t TileWidth, uint16_t TileHeight>
void Strip::MakeStripsMsaa(const Tiles<TileWidth, TileHeight>& tileContainer,
                           std::vector<Strip>& stripBuf,
                           AlphaAtlasManager* alphaAtlasManager,
                           SkPathFillType fillType,
                           const Polyline& polyline,
                           const SkTDArray<uint8_t>* maskLut
#if defined(GPU_TEST_UTILS)
                           , MsaaExactMaskObserver observer
#endif
                           ) {
    const auto& tiles = tileContainer.getTiles();
    if (tiles.empty()) {
        return;
    }
    if (!alphaAtlasManager->initializeTexture()) {
        return;
    }

    int16_t maskBuffer[TileHeight][TileWidth][kNumSubSamples];

    auto clearMask = [&](int16_t value) {
        for (int y = 0; y < TileHeight; ++y) {
            for (int x = 0; x < TileWidth; ++x) {
                for (int k = 0; k < kNumSubSamples; ++k) {
                    maskBuffer[y][x][k] = value;
                }
            }
        }
    };

    // Initialize with 0
    clearMask(0);

    auto shouldFill = [fillType](int32_t w) {
        return (fillType == SkPathFillType::kWinding) ? (w != 0) : ((w % 2) != 0);
    };

    int32_t windingDelta = 0;

    size_t totalCount = tiles.size();
    Tile<TileWidth, TileHeight> prevTile = tiles[0];
    Strip currentStrip(prevTile.x * TileWidth,
                       prevTile.y * TileHeight,
                       alphaAtlasManager->currentOffset(),
                       false);

    float tileMinX = (float)(prevTile.x * TileWidth);
    float tileMinY = (float)(prevTile.y * TileHeight);
    float tileMaxX = tileMinX + (float)TileWidth;
    float tileMaxY = tileMinY + (float)TileHeight;

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
            std::array<uint8_t, TileWidth * TileHeight> tileAlphas;
            int alphaIdx = 0;

            for (int x = 0; x < TileWidth; ++x) {
                for (int y = 0; y < TileHeight; ++y) {
                    int activeSamples = 0;

                    for (int k = 0; k < kNumSubSamples; ++k) {
                        if (shouldFill(maskBuffer[y][x][k])) {
                            activeSamples++;
                        }
                    }

                    tileAlphas[alphaIdx++] = static_cast<uint8_t>((activeSamples * 255 + 4) / 8);

#if defined(GPU_TEST_UTILS)
                    if (observer) {
                        uint8_t exactMask = 0;
                        for (int k = 0; k < kNumSubSamples; ++k) {
                            if (shouldFill(maskBuffer[y][x][k])) {
                                exactMask |= (1 << k);
                            }
                        }
                        observer(exactMask);
                    }
#endif
                }
            }

            uint32_t globalIdx = alphaAtlasManager->addAlphas(tileAlphas.data(), tileAlphas.size());
            if (globalIdx == AlphaAtlasManager::kFull) {
                return;
            }

            if (!rowStart) {
                clearMask((int16_t)windingDelta);
            }
        }

        if (segStart) {
            stripBuf.push_back(currentStrip);
            if (rowStart) {
                if (windingDelta != 0 || isSentinel) {
                    stripBuf.push_back(Strip(
                            0xFFFF,
                            prevTile.y * TileHeight,
                            alphaAtlasManager->currentOffset(),
                            shouldFill(windingDelta)));
                }
                windingDelta = 0;
                clearMask(0);
            }

            if (isSentinel) break;
            currentStrip = Strip(tile.x * TileWidth,
                                 tile.y * TileHeight,
                                 alphaAtlasManager->currentOffset(),
                                 shouldFill(windingDelta));
        }

        prevTile = tile;
        if (isSentinel) break;

        // Grab the line using the newly added getter
        Line line = polyline.getLine(tile.packedWindingLineIdx >> INT_MASK_SHIFT);

        bool canonicalXDir = line.p1.fX >= line.p0.fX;
        bool canonicalYDir = line.p1.fY >= line.p0.fY;

        int32_t sign = canonicalYDir ? 1 : -1;
        uint32_t windingBit = (tile.packedWindingLineIdx & W) ? 1 : 0;
        windingDelta += sign * (int32_t)windingBit;

        float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
        if (rightEdge < 0.0f) continue;

        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;
        float idx = (std::abs(dx) <= 1e-5f) ? 0.0f : 1.0f / dx;
        float idy = (std::abs(dy) <= 1e-5f) ? 0.0f : 1.0f / dy;
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

        if (tile.packedWindingLineIdx & L) {
            float yEdge = (pTop.fX <= pBot.fX) ? pTop.fY : pBot.fY;
            int startY = (int)std::ceil(yEdge);
            int val = canonicalXDir ? -1 : 1;

            for (int y = startY; y < TileHeight; ++y) {
                for (int x = 0; x < TileWidth; ++x) {
                    for (int k = 0; k < kNumSubSamples; ++k) maskBuffer[y][x][k] += val;
                }
            }
        }

        if (std::abs(dy) < 1e-5f && pTop.fY == std::floor(pTop.fY)) continue;

        int startY = (int)std::floor(pTop.fY);
        int endY = (int)std::ceil(pBot.fY);

        SkPoint rows[TileHeight + 1];
        rows[startY] = pTop;
        for (int y = startY + 1; y < endY; ++y) {
            float gy = (float)y;
            float gx = pTop.fX + (gy - pTop.fY) * dxdy;
            rows[y] = {gx, gy};
        }
        rows[endY] = pBot;

        float deltaSq = dx * dx + dy * dy;
        float invLen = (deltaSq < 1e-12f) ? 0.0f : (1.0f / std::sqrt(deltaSq));
        float nx = dy * invLen;
        float ny = -dx * invLen;
        float cAnchor = pTop.fX * nx + pTop.fY * ny - 0.5f * (nx + ny);

        for (int y = startY; y < endY; ++y) {
            SkPoint pt = rows[y];
            SkPoint pb = rows[y + 1];

            if (std::isnan(pt.fX) || std::isnan(pb.fX)) continue;

            float xMin = std::min(pt.fX, pb.fX);
            float xMax = std::max(pt.fX, pb.fX);

            int xStart = std::max(0, std::min((int)std::floor(xMin), TileWidth - 1));
            int xEnd = std::max(0, std::min((int)std::floor(xMax), TileWidth - 1));

            for (int x = xStart; x <= xEnd; ++x) {
                float px = (float)x;
                float py = (float)y;

                float dist = cAnchor - (nx * px + ny * py);
                float signDist = (dist >= 0.0f) ? 1.0f : -1.0f;
                float distAbs = dist * signDist;

                float c2 = std::max(0.0f, 1.0f - distAbs * kPackingScale);
                float uvScale = c2 * signDist * 0.5f;

                float uf = (uvScale * nx + 0.5f) * kMaskWidthF;
                float vf = (uvScale * ny + 0.5f) * kMaskHeightF;

                uint32_t u = std::min((uint32_t)std::max(0.0f, uf), (uint32_t)kMaskWidthExcl);
                uint32_t v = std::min((uint32_t)std::max(0.0f, vf), (uint32_t)kMaskHeightExcl);

                SkASSERT(v * kMaskWidth + u < static_cast<uint32_t>((*maskLut).size()));
                uint8_t maskVal = (*maskLut)[v * kMaskWidth + u];

                // TODO(thomsmit) hoist
                if (y == startY) {
                    float localDy = pt.fY - py;
                    int shift = (int)std::round(8.0f * localDy);
                    if (shift < 8 && shift > 0)
                        maskVal &= (0xFF << shift);
                    else if (shift >= 8)
                        maskVal = 0;
                }
                if (y == endY - 1) {
                    float localDy = pb.fY - py;
                    int shift = (int)std::round(8.0f * localDy);
                    if (shift < 8 && shift > 0)
                        maskVal &= ~(0xFF << shift);
                    else if (shift <= 0)
                        maskVal = 0;
                }

                for (int k = 0; k < kNumSubSamples; ++k) {
                    if (maskVal & (1 << k)) {
                        if (canonicalYDir)
                            maskBuffer[y][x][k]++;
                        else
                            maskBuffer[y][x][k]--;
                    }
                }
            }

            bool crossedTop = (y > startY) || (pt.fY == std::floor(pt.fY));
            if (crossedTop) {
                int fillVal = canonicalYDir ? 1 : -1;
                for (int x = xEnd + 1; x < TileWidth; ++x) {
                    for (int k = 0; k < kNumSubSamples; ++k) {
                        maskBuffer[y][x][k] += fillVal;
                    }
                }
            }
        }
    }
}

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_Strip_DEFINED
