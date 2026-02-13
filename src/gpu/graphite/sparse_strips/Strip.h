/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_Strip_DEFINED
#define skgpu_graphite_Strip_DEFINED

#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"  // For Tiles class

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

namespace skgpu::graphite {

enum class FillRule { kNonZero, kEvenOdd };

class Strip : IntersectionBits {
public:
    Strip(uint16_t x, uint16_t y, uint32_t alphaIdx, bool shouldFill)
            : x(x)
            , y(y)
            , packedAlphaIdx(alphaIdx | (static_cast<uint32_t>(shouldFill) << FILL_SHIFT)) {}

    template <uint16_t TileWidth, uint16_t TileHeight>
    static void MakeStripsMsaa(
            const Tiles<TileWidth, TileHeight>& tiles,
            std::vector<Strip>& stripBuf,
            std::vector<uint8_t>& alphaBuf,
            FillRule fillRule,
            const std::vector<Line>& lines,
            const std::vector<uint8_t>& maskLut);

    static SK_ALWAYS_INLINE std::array<Point, 2> clipToTile(const Line& line,
                                                            const std::array<float, 4>& bounds,
                                                            const std::array<float, 4>& derivatives,
                                                            uint32_t intersectionData,
                                                            bool cannonicalXDir,
                                                            bool cannonicalYDir) {
        Point pEntry = line.p0;
        Point pExit = line.p1;

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
            const float start = useH ? line.p0.y : line.p0.x;
            const float invD = useH ? idy : idx;

            const float t = (bound - start) * invD;

            pEntry.x = line.p0.x + t * dx;
            pEntry.y = line.p0.y + t * dy;

            // Explicitly snap the specific axis to the bound
            if (useH) {
                pEntry.y = bound;
            } else {
                pEntry.x = bound;
            }
        }

        const uint32_t exitHits = intersectionData & (maskVOut | maskHOut);
        if (exitHits != 0) {
            const bool useH = (intersectionData & maskHOut) != 0;

            const float bound = useH ? boundHOut : boundVOut;
            const float start = useH ? line.p0.y : line.p0.x;
            const float invD = useH ? idy : idx;

            const float t = (bound - start) * invD;

            pExit.x = line.p0.x + t * dx;
            pExit.y = line.p0.y + t * dy;

            // Explicitly snap the specific axis to the bound
            if (useH) {
                pExit.y = bound;
            } else {
                pExit.x = bound;
            }
        }

        std::array<Point, 2> result;
        if (pExit.y >= pEntry.y) {
            result = {pEntry, pExit};
        } else {
            result = {pExit, pEntry};
        }

        result[0].x -= tileMinX;
        result[0].y -= tileMinY;
        result[1].x -= tileMinX;
        result[1].y -= tileMinY;

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

        result[0].x = std::clamp(result[0].x, 0.0f, width);
        result[0].y = std::clamp(result[0].y, 0.0f, height);
        result[1].x = std::clamp(result[1].x, 0.0f, width);
        result[1].y = std::clamp(result[1].y, 0.0f, height);

        return result;
    }

private:
    static constexpr uint32_t FILL_SHIFT = 31;

    static constexpr int kNumSubSamples = 8;
    static constexpr float kPackingScale = 0.5f;
    static constexpr int kMaskWidth = 64;
    static constexpr int kMaskHeight = 64;
    static constexpr float kMaskWidthF = static_cast<float>(kMaskWidth);
    static constexpr float kMaskHeightF = static_cast<float>(kMaskHeight);
    static constexpr int kMaskWidthExcl = kMaskWidth - 1;
    static constexpr int kMaskHeightExcl = kMaskWidth - 1;

    uint16_t x;
    uint16_t y;
    uint32_t packedAlphaIdx;
};

template <uint16_t TileWidth, uint16_t TileHeight>
void Strip::MakeStripsMsaa(const Tiles<TileWidth, TileHeight>& tileContainer,
                           std::vector<Strip>& stripBuf,
                           std::vector<uint8_t>& alphaBuf,
                           FillRule fillRule,
                           const std::vector<Line>& lines,
                           const std::vector<uint8_t>& maskLut) {
    const auto& tiles = tileContainer.getTiles();
    if (tiles.empty()) {
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

    auto shouldFill = [fillRule](int32_t w) {
        return (fillRule == FillRule::kNonZero) ? (w != 0) : ((w % 2) != 0);
    };

    int32_t windingDelta = 0;

    size_t totalCount = tiles.size();
    Tile<TileWidth, TileHeight> prevTile = tiles[0];
    Strip currentStrip(prevTile.x * TileWidth,
                       prevTile.y * TileHeight,
                       static_cast<uint32_t>(alphaBuf.size()),
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
            for (int x = 0; x < TileWidth; ++x) {
                for (int y = 0; y < TileHeight; ++y) {
                    int activeSamples = 0;
                    for (int k = 0; k < kNumSubSamples; ++k) {
                        if (shouldFill(maskBuffer[y][x][k])) {
                            activeSamples++;
                        }
                    }
                    uint8_t alpha = static_cast<uint8_t>((activeSamples * 255 + 4) / 8);
                    alphaBuf.push_back(alpha);
                }
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
                            static_cast<uint32_t>(alphaBuf.size()),
                            shouldFill(windingDelta)));
                }
                windingDelta = 0;
                clearMask(0);
            }

            if (isSentinel) break;
            currentStrip = Strip(tile.x * TileWidth,
                                 tile.y * TileHeight,
                                 static_cast<uint32_t>(alphaBuf.size()),
                                 shouldFill(windingDelta));
        }

        prevTile = tile;
        if (isSentinel) break;
        const Line& line = lines[tile.packedWindingLineIdx >> INT_MASK_SHIFT];

        bool canonicalXDir = line.p1.x >= line.p0.x;
        bool canonicalYDir = line.p1.y >= line.p0.y;

        int32_t sign = canonicalYDir ? 1 : -1;
        uint32_t windingBit = (tile.packedWindingLineIdx & W) ? 1 : 0;
        windingDelta += sign * (int32_t)windingBit;

        float rightEdge = canonicalXDir ? line.p1.x : line.p0.x;
        if (rightEdge < 0.0f) continue;

        float dx = line.p1.x - line.p0.x;
        float dy = line.p1.y - line.p0.y;
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

        auto clipped = ClipToTile(line,
                                  bounds,
                                  derivs,
                                  tile.packedWindingLineIdx & INTERSECTION_MASK,
                                  canonicalXDir,
                                  canonicalYDir);
        Point pTop = clipped[0];
        Point pBot = clipped[1];

        if (tile.packedWindingLineIdx & L) {
            float yEdge = (pTop.x <= pBot.x) ? pTop.y : pBot.y;
            int startY = (int)std::ceil(yEdge);
            int val = canonicalXDir ? -1 : 1;

            for (int y = startY; y < TileHeight; ++y) {
                for (int x = 0; x < TileWidth; ++x) {
                    for (int k = 0; k < kNumSubSamples; ++k) maskBuffer[y][x][k] += val;
                }
            }
        }

        if (std::abs(dy) < 1e-5f && pTop.y == std::floor(pTop.y)) continue;

        int startY = (int)std::floor(pTop.y);
        int endY = (int)std::ceil(pBot.y);

        Point rows[TileHeight + 1];
        rows[startY] = pTop;
        for (int y = startY + 1; y < endY; ++y) {
            float gy = (float)y;
            float gx = pTop.x + (gy - pTop.y) * dxdy;
            rows[y] = {gx, gy};
        }
        rows[endY] = pBot;

        float deltaSq = dx * dx + dy * dy;
        float invLen = (deltaSq < 1e-12f) ? 0.0f : (1.0f / std::sqrt(deltaSq));
        float nx = dy * invLen;
        float ny = -dx * invLen;
        float cAnchor = pTop.x * nx + pTop.y * ny - 0.5f * (nx + ny);

        for (int y = startY; y < endY; ++y) {
            Point pt = rows[y];
            Point pb = rows[y + 1];

            if (std::isnan(pt.x) || std::isnan(pb.x)) continue;

            float xMin = std::min(pt.x, pb.x);
            float xMax = std::max(pt.x, pb.x);

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

                uint8_t maskVal = maskLut[v * kMaskWidth + u];

                // TODO(thomsmit) hoist
                if (y == startY) {
                    float localDy = pt.y - py;
                    int shift = (int)std::round(8.0f * localDy);
                    if (shift < 8 && shift > 0)
                        maskVal &= (0xFF << shift);
                    else if (shift >= 8)
                        maskVal = 0;
                }
                if (y == endY - 1) {
                    float localDy = pb.y - py;
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

            bool crossedTop = (y > startY) || (pt.y == std::floor(pt.y));
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
