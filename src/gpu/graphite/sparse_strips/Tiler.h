/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Tiler_DEFINED
#define skgpu_graphite_Tiler_DEFINED

#include "include/core/SkRect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace skgpu::graphite {

template <uint16_t WIDTH, uint16_t HEIGHT> struct Tile {
    uint16_t x;
    uint16_t y;
    uint32_t packedWindingLineIdx;

    Tile() : x(0), y(0), packedWindingLineIdx(0) {}

    Tile(uint16_t x, uint16_t y, uint32_t lineIdx, uint32_t intersectionMask)
            : x(x), y(y),
            packedWindingLineIdx((lineIdx << SparseStripsConstants::INT_MASK_SHIFT) | intersectionMask) {}

    SK_ALWAYS_INLINE uint64_t toBits() const {
        static constexpr uint32_t xyShift = sizeof(packedWindingLineIdx) * 8;
        static constexpr uint32_t yShift = SparseStripsConstants::Y_SHIFT + xyShift;
        return (static_cast<uint64_t>(y) << yShift) | (static_cast<uint64_t>(x) << xyShift) |
               static_cast<uint64_t>(packedWindingLineIdx);
    }

    bool operator<(const Tile& other) const { return toBits() < other.toBits(); }

    bool operator==(const Tile& other) const { return toBits() == other.toBits(); }
};

template <uint16_t WIDTH, uint16_t HEIGHT>
struct GpuTile : public Tile<WIDTH, HEIGHT> {
    int32_t alphaIdx;   // The location in the alpha texture this will correspond to

    int32_t packed;     // PostScanned winding goes into the upper bits, so that pulling them down
                        // sign extends the value. the lower bits contain the localIdx of the
                        // parent tile.

    GpuTile() : Tile<WIDTH, HEIGHT>() {}

    GpuTile(uint16_t xGpu, uint16_t yGpu, uint32_t lineIdx, uint32_t intersectionMask)
            : Tile<WIDTH, HEIGHT>(xGpu, yGpu, lineIdx, intersectionMask) {}
};

template <uint16_t WIDTH, uint16_t HEIGHT, typename TileType = Tile<WIDTH, HEIGHT>>
class Tiles {
public:
    using value_type = TileType;

    Tiles() {}

    void reset() {
        fTileBuf.clear();
    }

    void sortTiles(int32_t offset) {
        std::stable_sort(fTileBuf.begin() + offset, fTileBuf.end());
    }

    void sortTiles() {
        this->sortTiles(0);
    }

    const std::vector<TileType>& getTiles() const { return fTileBuf; }
    std::vector<TileType>& getTiles() { return fTileBuf; }

    bool makeTilesMSAA(const Polyline& polyline, const SkIRect& clip, WindingHistogram& histogram,
                       int32_t startingLineOffset) {
        SkASSERT(polyline.count() <= SparseStripsConstants::MAX_LINES_PER_PATH);

        bool cullingEventOccurred = false;
        if (clip.isEmpty()) {
            return cullingEventOccurred;
        }

        uint16_t tileColumns = divCeil(clip.right(), WIDTH);
        uint16_t tileRows = divCeil(clip.bottom(), HEIGHT);

        constexpr float invW = 1.0f / static_cast<float>(WIDTH);
        constexpr float invH = 1.0f / static_cast<float>(HEIGHT);

        float clipLeftTile = clip.left() * invW;
        float clipTopTile = clip.top() * invH;
        float clipRightTile = clip.right() * invW;

        uint16_t minTileX = f32ToU16Sat(clipLeftTile);
        uint16_t minTileY = f32ToU16Sat(clipTopTile);

        for (auto it = polyline.beginFrom(startingLineOffset); it != polyline.end(); ++it) {
            auto [line, lineIdx] = *it;

            float p0X = line.p0.fX * invW;
            float p0Y = line.p0.fY * invH;
            float p1X = line.p1.fX * invW;
            float p1Y = line.p1.fY * invH;

            float lineLeftX, lineRightX;
            if (p0X < p1X) {
                lineLeftX = p0X;
                lineRightX = p1X;
            } else {
                lineLeftX = p1X;
                lineRightX = p0X;
            }

            if (lineLeftX > clipRightTile) {
                continue;
            }

            float lineTopY, lineTopX, lineBottomY, lineBottomX;
            int16_t windingDir;
            if (p0Y < p1Y) {
                lineTopY = p0Y;
                lineTopX = p0X;
                lineBottomY = p1Y;
                lineBottomX = p1X;
                windingDir = 1;
            } else {
                lineTopY = p1Y;
                lineTopX = p1X;
                lineBottomY = p0Y;
                lineBottomX = p0X;
                windingDir = -1;
            }

            uint16_t yTopTiles = std::max(minTileY, std::min(f32ToU16Sat(lineTopY), tileRows));
            float lineBottomYCeil = std::ceil(lineBottomY);
            uint16_t yBottomTiles = std::max(minTileY, std::min(f32ToU16Sat(lineBottomYCeil), tileRows));

            if (yTopTiles >= yBottomTiles) {
                continue;
            }

            int32_t p0TileX = static_cast<int32_t>(std::floor(lineTopX));
            int32_t p0TileY = static_cast<int32_t>(std::floor(lineTopY));
            int32_t p1TileX = static_cast<int32_t>(std::floor(lineBottomX));

            int32_t p1TileY;
            if (lineBottomY == lineBottomYCeil) {
                p1TileY = static_cast<int32_t>(lineBottomY) - 1;
            } else {
                p1TileY = static_cast<int32_t>(std::floor(lineBottomY));
            }

            bool notSameTile = (p0TileY != p1TileY) || (p0TileX != p1TileX);

            // If the line is fully left of the viewport, it is culled, and the resulting
            // winding is stored in a histogram forwarded to the strip generation.
            if (lineRightX < clipLeftTile) {
                cullingEventOccurred = true;
                bool isStartCulled = (lineTopY < clipTopTile);
                if (!isStartCulled) {
                    if (static_cast<float>(yTopTiles) >= lineTopY) {
                        histogram.addWinding(yTopTiles, windingDir);
                    }
                }

                uint16_t yStart = isStartCulled ? yTopTiles : (yTopTiles + 1);
                float lineBottomFloor = std::floor(lineBottomY);
                uint16_t yEndIdx = std::min(f32ToU16Sat(lineBottomFloor), tileRows);
                histogram.addWindingRange(yStart, yEndIdx, windingDir);

                if ((p0TileY != p1TileY) && lineBottomY != lineBottomFloor && yEndIdx < tileRows) {
                    histogram.addWinding(yEndIdx, windingDir);
                }
                continue;
            }

            if (notSameTile) {
                if (lineLeftX == lineRightX) {
                    uint16_t x = std::min(f32ToU16Sat(lineLeftX),
                                          static_cast<uint16_t>(tileColumns > 0 ? tileColumns - 1 : 0));
                    x = std::max(minTileX, x);
                    bool isStartCulled = (lineTopY < clipTopTile);
                    if (!isStartCulled) {
                        uint32_t winding = (static_cast<float>(yTopTiles) >= lineTopY) ? W : 0;
                        uint32_t intersectionMask = B | winding;
                        fTileBuf.push_back(TileType(x, yTopTiles, lineIdx, intersectionMask));
                    }

                    uint16_t yStart = isStartCulled ? yTopTiles : (yTopTiles + 1);
                    float lineBottomFloor = std::floor(lineBottomY);
                    uint16_t yEndIdx = std::min(f32ToU16Sat(lineBottomFloor), tileRows);

                    if (yStart < yEndIdx) {
                        uint16_t yLast = yEndIdx - 1;
                        for (uint16_t yIdx = yStart; yIdx < yLast; ++yIdx) {
                            uint32_t intersectionMask = W | B | T;
                            fTileBuf.push_back(TileType(x, yIdx, lineIdx, intersectionMask));
                        }

                        {
                            bool isEndTile = ((int32_t)yLast == p1TileY);
                            uint32_t intersectionMask = W | T | (isEndTile ? 0 : B);
                            fTileBuf.push_back(TileType(x, yLast, lineIdx, intersectionMask));
                        }
                    }

                    if (lineBottomY != lineBottomFloor && yEndIdx < tileRows) {
                        uint32_t intersectionMask = W | T;
                        fTileBuf.push_back(TileType(x, yEndIdx, lineIdx, intersectionMask));
                    }
                } else {
                    float dx = p1X - p0X;
                    float dy = p1Y - p0Y;
                    float xSlope = dx / dy;

                    LineContext ctx {
                        static_cast<uint32_t>(lineIdx),
                        lineTopX, lineTopY,
                        lineBottomX, lineBottomY,
                        xSlope,
                        lineLeftX, lineRightX,
                        p0TileX, p0TileY,
                        p1TileX, p1TileY,
                        tileColumns,
                        windingDir,
                        clipLeftTile,
                        clipTopTile,
                        minTileX
                    };

                    bool leftCrossing = lineLeftX < clipLeftTile;
                    bool rightCrossing = lineRightX >= clipRightTile;

                    bool kXDir = lineBottomX >= lineTopX;
                    bool crossesEdge = leftCrossing || rightCrossing;

                    if (crossesEdge) {
                        if (leftCrossing) {
                            cullingEventOccurred = true;
                        }

                        if (kXDir)
                            runLoops<true, true>(ctx, lineTopY, lineBottomY, yTopTiles, tileRows, histogram);
                        else
                            runLoops<false, true>(ctx, lineTopY, lineBottomY, yTopTiles, tileRows, histogram);
                    } else {
                        if (kXDir)
                            runLoops<true, false>(ctx, lineTopY, lineBottomY, yTopTiles, tileRows, histogram);
                        else
                            runLoops<false, false>(ctx, lineTopY, lineBottomY, yTopTiles, tileRows, histogram);
                    }
                }

            } else {
                uint16_t xClamped =
                        std::min(f32ToU16Sat(lineLeftX), static_cast<uint16_t>(tileColumns + 1));
                xClamped = std::max(minTileX, xClamped);
                uint32_t winding = static_cast<float>(yTopTiles) >= lineTopY ? W : 0;
                fTileBuf.push_back(TileType(xClamped, yTopTiles, lineIdx, winding));
            }
        }

        return cullingEventOccurred;
    }

    bool makeTilesMSAA(const Polyline& polyline, const SkIRect& clip, WindingHistogram& histogram) {
        return this->makeTilesMSAA(polyline, clip, histogram, /*startingLineOffset=*/0);
    }

private:
    static constexpr uint32_t W           = SparseStripsConstants::W;
    static constexpr uint32_t T           = SparseStripsConstants::T;
    static constexpr uint32_t B           = SparseStripsConstants::B;
    static constexpr uint32_t L           = SparseStripsConstants::L;
    static constexpr uint32_t R           = SparseStripsConstants::R;
    static constexpr uint32_t LEFT_SHIFT  = SparseStripsConstants::LEFT_SHIFT;
    static constexpr uint32_t RIGHT_SHIFT = SparseStripsConstants::RIGHT_SHIFT;
    static constexpr uint32_t BOT_SHIFT   = SparseStripsConstants::BOT_SHIFT;

    std::vector<TileType> fTileBuf;
    SkDEBUGCODE(bool fSorted;)

    struct LineContext {
        uint32_t lineIdx;
        float    topX, topY;
        float    bottomX, bottomY;
        float    xSlope;
        float    lineLeftX, lineRightX;
        int32_t  p0TileX, p0TileY;
        int32_t  p1TileX, p1TileY;
        uint16_t tileColumns;
        int16_t  windingDir;
        float    clipLeftTile;
        float    clipTopTile;
        uint16_t minTileX;
    };

    static SK_ALWAYS_INLINE uint16_t divCeil(uint16_t a, uint16_t b) {
        return (a + b - 1) / b;
    }

    static SK_ALWAYS_INLINE uint16_t f32ToU16Sat(float v) {
        return static_cast<uint16_t>(std::clamp(v, 0.0f, 65535.0f));
    }

    template <bool kXDir, bool kCheckStart, bool kCheckEnd>
    SK_ALWAYS_INLINE void pushEdge(const LineContext& ctx,
                                   uint16_t xIdx,
                                   uint16_t y,
                                   float rowTopX,
                                   float rowBottomX,
                                   int32_t canonicalStart,
                                   uint16_t canonicalEnd,
                                   uint32_t windingInput) {
        uint32_t uncRowStart = (static_cast<int32_t>(xIdx) == canonicalStart) ? 1 : 0;
        uint32_t uncRowEnd = (xIdx == canonicalEnd) ? 1 : 0;

        uint32_t canonicalRowStart = kXDir ? uncRowStart : uncRowEnd;
        uint32_t canonicalRowEnd = kXDir ? uncRowEnd : uncRowStart;

        uint32_t notStartTile = 1;
        if constexpr (kCheckStart) {
            notStartTile ^= static_cast<uint32_t>((static_cast<int32_t>(xIdx) == ctx.p0TileX) &&
                                                  (static_cast<int32_t>(y) == ctx.p0TileY));
        }

        uint32_t notEndTile = 1;
        if constexpr (kCheckEnd) {
            notEndTile ^= static_cast<uint32_t>((static_cast<int32_t>(xIdx) == ctx.p1TileX) &&
                                                (static_cast<int32_t>(y) == ctx.p1TileY));
        }

        uint32_t mask = windingInput;
        mask |= canonicalRowStart & notStartTile;

        if constexpr (kXDir) {
            mask |= (1 ^ canonicalRowStart) << LEFT_SHIFT;
            mask |= (canonicalRowEnd & notEndTile) << BOT_SHIFT;
            mask |= (1 ^ canonicalRowEnd) << RIGHT_SHIFT;
        } else {
            mask |= (1 ^ canonicalRowStart) << RIGHT_SHIFT;
            mask |= (canonicalRowEnd & notEndTile) << BOT_SHIFT;
            mask |= (1 ^ canonicalRowEnd) << LEFT_SHIFT;
        }

        float xLeftF = static_cast<float>(xIdx);
        float xRightF = static_cast<float>(xIdx + 1);

        uint32_t trc = static_cast<uint32_t>(rowTopX == xRightF) & notStartTile;
        uint32_t tlc = static_cast<uint32_t>(rowTopX == xLeftF) & notStartTile;
        uint32_t brc = static_cast<uint32_t>(rowBottomX == xRightF) & notEndTile;
        uint32_t blc = static_cast<uint32_t>(rowBottomX == xLeftF) & notEndTile;

        uint32_t tieBreak = tlc & (canonicalRowStart ^ 1);

        mask |= (tieBreak | blc) * L;
        mask |= (trc | brc) * R;
        mask &= ~((tieBreak | trc) * T);
        mask &= ~((blc | brc) * B);

        fTileBuf.push_back(TileType(xIdx, y, ctx.lineIdx, mask));
    }

    template <bool kXDir, bool kCrossesEdge, bool kCheckStart, bool kCheckEnd>
    SK_ALWAYS_INLINE void processRow(const LineContext& ctx,
                                     uint16_t yIdx,
                                     float rowTopX,
                                     float rowBottomX,
                                     uint32_t wMask,
                                     WindingHistogram& histogram) {
        float lx, rx;
        uint16_t xEndVal;

        if constexpr (kCrossesEdge) {
            if (rowTopX < ctx.clipLeftTile) {
                if (wMask & W) {
                    histogram.addWinding(yIdx, ctx.windingDir);
                }

                if (rowBottomX < ctx.clipLeftTile) {
                    return;
                } else {
                    // Strip the winding bit so the winding isn't double counted.
                    wMask &= ~W;
                }
            }

            lx = std::fmax(std::fmin(rowTopX, rowBottomX), ctx.lineLeftX);
            rx = std::fmin(std::fmax(rowTopX, rowBottomX), ctx.lineRightX);

            xEndVal =
                    std::min(f32ToU16Sat(rx),
                             static_cast<uint16_t>(ctx.tileColumns > 0 ? ctx.tileColumns - 1 : 0));
        } else {
            lx = std::fmin(rowTopX, rowBottomX);
            rx = std::fmax(rowTopX, rowBottomX);
            xEndVal = f32ToU16Sat(rx);
        }

        int32_t canonicalXStart = static_cast<int32_t>(std::floor(lx));
        uint16_t canonicalXEnd = f32ToU16Sat(rx);
        uint16_t xStart = std::max(ctx.minTileX, f32ToU16Sat(lx));

        if (xStart <= xEndVal) {
            uint32_t isSingle = (xStart == xEndVal) ? 1 : 0;

            uint32_t wLeft = ((kXDir ? W : 0) | (isSingle ? W : 0)) & wMask;
            pushEdge<kXDir, kCheckStart, kCheckEnd>(
                    ctx, xStart, yIdx, rowTopX, rowBottomX, canonicalXStart, canonicalXEnd, wLeft);

            int innerCount = static_cast<int>(xEndVal) - static_cast<int>(xStart) - 1;
            if (innerCount > 0) {
                size_t oldSize = fTileBuf.size();
                fTileBuf.resize(oldSize + innerCount);
                TileType* outTiles = fTileBuf.data() + oldSize;
                uint32_t innerMask = R | L;
                uint32_t lineIdx = ctx.lineIdx;
                for (int i = 0; i < innerCount; ++i) {
                    outTiles[i] = TileType(xStart + 1 + i, yIdx, lineIdx, innerMask);
                }
            }

            if (xStart < xEndVal) {
                uint32_t wRight = (kXDir ? 0 : W) & wMask;
                pushEdge<kXDir, kCheckStart, kCheckEnd>(ctx,
                                                        xEndVal,
                                                        yIdx,
                                                        rowTopX,
                                                        rowBottomX,
                                                        canonicalXStart,
                                                        canonicalXEnd,
                                                        wRight);
            }
        }
    }

    template <bool kXDir, bool kCrossesEdge>
    SK_ALWAYS_INLINE void runLoops(const LineContext& ctx,
                                   float lineTopY,
                                   float lineBottomY,
                                   uint16_t yTopTiles,
                                   uint16_t tileRows,
                                   WindingHistogram& histogram) {
        bool isStartCulled = (lineTopY < ctx.clipTopTile);
        if (!isStartCulled) {
            float y = static_cast<float>(yTopTiles);
            float rowBottomY = std::min(y + 1.0f, lineBottomY);

            float rowTopX = ctx.topX;
            float rowBottomX = (rowBottomY == ctx.bottomY)
                                       ? ctx.bottomX
                                       : ctx.topX + (rowBottomY - ctx.topY) * ctx.xSlope;

            uint32_t mask = (y >= lineTopY) ? W : 0;
            processRow<kXDir, kCrossesEdge, true, true>(
                    ctx, yTopTiles, rowTopX, rowBottomX, mask, histogram);
        }

        uint16_t yStartMiddle = isStartCulled ? yTopTiles : (yTopTiles + 1);
        float lineBottomFloor = std::floor(lineBottomY);
        uint16_t yEndMiddle = std::min(f32ToU16Sat(lineBottomFloor), tileRows);

        bool hasSeparateBottomRow = (lineBottomY != lineBottomFloor) && (yEndMiddle < tileRows) &&
                                    (isStartCulled || yEndMiddle != yTopTiles);

        if (yStartMiddle < yEndMiddle) {
            uint16_t middleLimit = yEndMiddle;
            if (!hasSeparateBottomRow && middleLimit > yStartMiddle) {
                middleLimit--;
            }

            for (uint16_t yIdx = yStartMiddle; yIdx < middleLimit; ++yIdx) {
                float y = static_cast<float>(yIdx);
                float rowTopX = ctx.topX + (y - ctx.topY) * ctx.xSlope;
                float rowBottomX = ctx.topX + (y + 1.0f - ctx.topY) * ctx.xSlope;
                processRow<kXDir, kCrossesEdge, false, false>(
                        ctx, yIdx, rowTopX, rowBottomX, 0xffffffff, histogram);
            }

            if (!hasSeparateBottomRow && yEndMiddle > yStartMiddle) {
                uint16_t yIdx = yEndMiddle - 1;
                float y = static_cast<float>(yIdx);
                float rowBottomY = std::min(y + 1.0f, lineBottomY);

                float rowTopX = ctx.topX + (y - ctx.topY) * ctx.xSlope;
                float rowBottomX = (rowBottomY == ctx.bottomY)
                                           ? ctx.bottomX
                                           : ctx.topX + (rowBottomY - ctx.topY) * ctx.xSlope;
                processRow<kXDir, kCrossesEdge, false, true>(
                        ctx, yIdx, rowTopX, rowBottomX, 0xffffffff, histogram);
            }
        }

        if (hasSeparateBottomRow) {
            uint16_t yIdx = yEndMiddle;
            float y = static_cast<float>(yIdx);

            float rowTopX = ctx.topX + (y - ctx.topY) * ctx.xSlope;
            float rowBottomX = ctx.bottomX;
            processRow<kXDir, kCrossesEdge, false, true>(
                    ctx, yIdx, rowTopX, rowBottomX, 0xffffffff, histogram);
        }
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_Tiler_DEFINED
