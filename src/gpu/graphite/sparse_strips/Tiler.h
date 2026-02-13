/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Tiler_DEFINED
#define skgpu_graphite_Tiler_DEFINED

#include "include/private/base/SkAssert.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

namespace skgpu::graphite {

struct IntersectionBits {
    static constexpr uint32_t T = 0b00001;
    static constexpr uint32_t B = 0b00010;
    static constexpr uint32_t L = 0b00100;
    static constexpr uint32_t R = 0b01000;
    static constexpr uint32_t W = 0b10000;

    static constexpr uint32_t BOT_SHIFT        = 1;
    static constexpr uint32_t LEFT_SHIFT       = 2;
    static constexpr uint32_t RIGHT_SHIFT      = 3;
    static constexpr uint32_t WINDING_SHIFT    = 4;
    static constexpr uint32_t INT_MASK_SHIFT   = 5;

    static constexpr uint32_t INTERSECTION_MASK  = W | R | L | B | T;
    static constexpr uint32_t MAX_LINES_PER_PATH = 1 << (32 - INT_MASK_SHIFT);

    static std::string maskToString(uint32_t mask) {
        if (mask == 0) return "0";

        std::vector<std::string> parts;
        if (mask & W) parts.push_back("W");
        if (mask & T) parts.push_back("T");
        if (mask & B) parts.push_back("B");
        if (mask & L) parts.push_back("L");
        if (mask & R) parts.push_back("R");

        std::string s;
        for (size_t i = 0; i < parts.size(); ++i) {
            s += parts[i];
            if (i < parts.size() - 1) {
                s += " | ";
            }
        }
        return s;
    }
};

template<uint16_t WIDTH, uint16_t HEIGHT>
struct Tile : public IntersectionBits {
    uint16_t x;
    uint16_t y;
    uint32_t packedWindingLineIdx;

    Tile() : x(0), y(0), packedWindingLineIdx(0) {}

    Tile(uint16_t x, uint16_t y, uint32_t lineIdx, uint32_t intersectionMask)
        : x(x), y(y)
        , packedWindingLineIdx((lineIdx << INT_MASK_SHIFT) | intersectionMask) {}

    static Tile newClamped(uint16_t x, uint16_t y, uint32_t lineIdx, uint32_t intersectionMask) {
        uint16_t clampedX = std::min(x, (uint16_t)(0xFFFF / WIDTH));
        uint16_t clampedY = std::min(y, (uint16_t)(0xFFFF / HEIGHT));
        return Tile(clampedX, clampedY, lineIdx, intersectionMask);
    }

    uint64_t toBits() const {
        return (static_cast<uint64_t>(y) << 48) | (static_cast<uint64_t>(x) << 32) |
                static_cast<uint64_t>(packedWindingLineIdx);
    }

    bool operator<(const Tile& other) const {
        return toBits() < other.toBits();
    }

    bool operator==(const Tile& other) const {
        return toBits() == other.toBits();
    }
};

template<uint16_t WIDTH, uint16_t HEIGHT>
class Tiles : public IntersectionBits {
public:
    using TileType = Tile<WIDTH, HEIGHT>;

    Tiles() {}

    void reset() {
        fTileBuf.clear();
        SkDEBUGCODE(fSorted = false;)
    }

    void sortTiles() {
        std::sort(fTileBuf.begin(), fTileBuf.end());
        SkDEBUGCODE(fSorted = true;)
    }

    const std::vector<TileType>& getTiles() const {
        return fTileBuf;
    }

    void makeTilesMSAA(const std::vector<Line>& lines, uint16_t width, uint16_t height) {
        SkASSERT(lines.size() <= MAX_LINES_PER_PATH); // Can now access directly
        this->reset();

        if (width == 0 || height == 0) {
            return;
        }

        uint16_t tileColumns = divCeil(width, WIDTH);
        uint16_t tileRows = divCeil(height, HEIGHT);
        for (uint32_t lineIdx = 0; lineIdx < lines.size(); ++lineIdx) {
            const Line& line = lines[lineIdx];

            float p0X = line.p0.x / static_cast<float>(WIDTH);
            float p0Y = line.p0.y / static_cast<float>(HEIGHT);
            float p1X = line.p1.x / static_cast<float>(WIDTH);
            float p1Y = line.p1.y / static_cast<float>(HEIGHT);

            float lineLeftX, lineRightX;
            if (p0X < p1X) {
                lineLeftX = p0X;
                lineRightX = p1X;
            } else {
                lineLeftX = p1X;
                lineRightX = p0X;
            }

            if (lineLeftX > static_cast<float>(tileColumns)) {
                continue;
            }

            float lineTopY, lineTopX, lineBottomY, lineBottomX;
            if (p0Y < p1Y) {
                lineTopY = p0Y;
                lineTopX = p0X;
                lineBottomY = p1Y;
                lineBottomX = p1X;
            } else {
                lineTopY = p1Y;
                lineTopX = p1X;
                lineBottomY = p0Y;
                lineBottomX = p0X;
            }

            uint16_t yTopTiles = std::min(f32ToU16Sat(lineTopY), tileRows);
            float lineBottomYCeil = std::ceil(lineBottomY);
            uint16_t yBottomTiles = std::min(f32ToU16Sat(lineBottomYCeil), tileRows);

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
            if (notSameTile) {
                if (lineLeftX == lineRightX) {
                    uint16_t x =
                            std::min(f32ToU16Sat(lineLeftX),
                                     static_cast<uint16_t>(tileColumns > 0 ? tileColumns - 1 : 0));

                    bool isStartCulled = (lineTopY < 0.0f);
                    if (!isStartCulled) {
                        uint32_t winding = (static_cast<float>(yTopTiles) >= lineTopY) ? W : 0;
                        uint32_t intersectionMask = B | winding;
                        fTileBuf.push_back(
                                TileType::newClamped(x, yTopTiles, lineIdx, intersectionMask));
                    }

                    uint16_t yStart = isStartCulled ? yTopTiles : (yTopTiles + 1);
                    float lineBottomFloor = std::floor(lineBottomY);
                    uint16_t yEndIdx = std::min(f32ToU16Sat(lineBottomFloor), tileRows);

                    if (yStart < yEndIdx) {
                        uint16_t yLast = yEndIdx - 1;
                        for (uint16_t yIdx = yStart; yIdx < yLast; ++yIdx) {
                            uint32_t intersectionMask = W | B | T;
                            fTileBuf.push_back(
                                    TileType::newClamped(x, yIdx, lineIdx, intersectionMask));
                        }

                        {
                            bool isEndTile = ((int32_t)yLast == p1TileY);
                            uint32_t intersectionMask = W | T | (isEndTile ? 0 : B);
                            fTileBuf.push_back(
                                    TileType::newClamped(x, yLast, lineIdx, intersectionMask));
                        }
                    }

                    if (lineBottomY != lineBottomFloor && yEndIdx < tileRows) {
                        uint32_t intersectionMask = W | T;
                        fTileBuf.push_back(
                                TileType::newClamped(x, yEndIdx, lineIdx, intersectionMask));
                    }
                } else {
                    float dx = p1X - p0X;
                    float dy = p1Y - p0Y;
                    float xSlope = dx / dy;

                    uint32_t dxDir = static_cast<uint32_t>(lineBottomX >= lineTopX);
                    uint32_t notDxDir = dxDir ^ 1;

                    uint32_t wStartBase = dxDir ? W : 0;
                    uint32_t wEndBase = notDxDir ? W : 0;

                    LineContext ctx {
                        lineIdx,
                        p0X, p0Y,
                        xSlope,
                        lineLeftX, lineRightX,
                        dxDir, notDxDir,
                        wStartBase, wEndBase,
                        p0TileX, p0TileY,
                        p1TileX, p1TileY,
                        tileColumns
                    };

                    float minX = std::min(p0X, p1X);
                    float maxX = std::max(p0X, p1X);
                    bool needsClamping = (minX < lineLeftX) || (maxX >= lineRightX);

                    if (needsClamping) {
                        runLoops<true>(ctx, lineTopY, lineBottomY, yTopTiles, tileRows);
                    } else {
                        runLoops<false>(ctx, lineTopY, lineBottomY, yTopTiles, tileRows);
                    }
                }

            } else {
                uint16_t xClamped =
                        std::min(f32ToU16Sat(lineLeftX), static_cast<uint16_t>(tileColumns + 1));
                uint32_t winding = (static_cast<float>(yTopTiles) >= lineTopY) ? W : 0;

                fTileBuf.push_back(TileType::newClamped(xClamped, yTopTiles, lineIdx, winding));
            }
        }
    }

private:
    std::vector<TileType> fTileBuf;
    SkDEBUGCODE(bool fSorted;)

    struct LineContext {
        uint32_t lineIdx;
        float    p0X, p0Y;
        float    xSlope;
        float    lineLeftX, lineRightX;
        uint32_t dxDir;
        uint32_t notDxDir;
        uint32_t wStartBase;
        uint32_t wEndBase;
        int32_t  p0TileX, p0TileY;
        int32_t  p1TileX, p1TileY;
        uint16_t tileColumns;
    };

    static SK_ALWAYS_INLINE uint16_t divCeil(uint16_t a, uint16_t b) {
        return (a + b - 1) / b;
    }

    static SK_ALWAYS_INLINE uint16_t f32ToU16Sat(float v) {
        if (v < 0.0f) return 0;
        if (v > 65535.0f) return 65535;
        return static_cast<uint16_t>(v);
    }

    template <bool kCheckStart, bool kCheckEnd>
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

        uint32_t canonicalRowStart = (ctx.dxDir & uncRowStart) | (ctx.notDxDir & uncRowEnd);
        uint32_t canonicalRowEnd = (ctx.notDxDir & uncRowStart) | (ctx.dxDir & uncRowEnd);

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
        // Direct access to LEFT_SHIFT and BOT_SHIFT via inheritance
        mask |= canonicalRowStart & notStartTile;
        mask |= (1 ^ canonicalRowStart) << ctx.notDxDir << LEFT_SHIFT;
        mask |= (canonicalRowEnd & notEndTile) << BOT_SHIFT;
        mask |= (1 ^ canonicalRowEnd) << ctx.dxDir << LEFT_SHIFT;

        float xLeftF  = static_cast<float>(xIdx);
        float xRightF = static_cast<float>(xIdx + 1);

        uint32_t trc = static_cast<uint32_t>(rowTopX == xRightF)    & notStartTile;
        uint32_t tlc = static_cast<uint32_t>(rowTopX == xLeftF)     & notStartTile;
        uint32_t brc = static_cast<uint32_t>(rowBottomX == xRightF) & notEndTile;
        uint32_t blc = static_cast<uint32_t>(rowBottomX == xLeftF)  & notEndTile;

        uint32_t tieBreak = tlc & (canonicalRowStart ^ 1);

        // Direct access to L, R, T, B via inheritance
        if (tieBreak | blc) mask |= L;
        if (trc | brc)      mask |= R;
        if (tieBreak | trc) mask &= ~T;
        if (blc | brc)      mask &= ~B;

        fTileBuf.push_back(TileType(xIdx, y, ctx.lineIdx, mask));
    }

    template <bool kNeedsClamping, bool kCheckStart, bool kCheckEnd>
    SK_ALWAYS_INLINE void processRow(const LineContext& ctx,
                                     uint16_t yIdx,
                                     float rowTopY,
                                     float rowBottomY,
                                     uint32_t wMask) {
        float rowTopX = ctx.p0X + (rowTopY - ctx.p0Y) * ctx.xSlope;
        float rowBottomX = ctx.p0X + (rowBottomY - ctx.p0Y) * ctx.xSlope;

        float lx, rx;
        uint16_t xEndVal;

        if constexpr (kNeedsClamping) {
            float minRowX = std::fmin(rowTopX, rowBottomX);
            float maxRowX = std::fmax(rowTopX, rowBottomX);
            lx = std::fmax(minRowX, ctx.lineLeftX);
            rx = std::fmin(maxRowX, ctx.lineRightX);
            xEndVal = std::min(f32ToU16Sat(rx),
                               static_cast<uint16_t>(ctx.tileColumns > 0 ? ctx.tileColumns - 1 : 0));
        } else {
            lx = std::fmin(rowTopX, rowBottomX);
            rx = std::fmax(rowTopX, rowBottomX);
            xEndVal = f32ToU16Sat(rx);
        }

        int32_t canonicalXStart = static_cast<int32_t>(std::floor(lx));
        uint16_t canonicalXEnd = f32ToU16Sat(rx);
        uint16_t xStart = f32ToU16Sat(lx);

        if (xStart <= xEndVal) {
            uint32_t isSingle = (xStart == xEndVal) ? 1 : 0;
            // Direct access to W
            uint32_t wLeft = (ctx.wStartBase | (isSingle ? W : 0)) & wMask;

            pushEdge<kCheckStart, kCheckEnd>(ctx,
                                             xStart,
                                             yIdx,
                                             rowTopX,
                                             rowBottomX,
                                             canonicalXStart,
                                             canonicalXEnd,
                                             wLeft);

            for (uint16_t xK = xStart + 1; xK < xEndVal; ++xK) {
                // Direct access to R and L
                fTileBuf.push_back(TileType(xK, yIdx, ctx.lineIdx, R | L));
            }

            if (xStart < xEndVal) {
                uint32_t wRight = ctx.wEndBase & wMask;
                pushEdge<kCheckStart, kCheckEnd>(ctx,
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

    template <bool kNeedsClamping>
    SK_ALWAYS_INLINE void runLoops(const LineContext& ctx,
                                   float lineTopY,
                                   float lineBottomY,
                                   uint16_t yTopTiles,
                                   uint16_t tileRows) {
        bool isStartCulled = (lineTopY < 0.0f);
        if (!isStartCulled) {
            float y = static_cast<float>(yTopTiles);
            float rowBottomY = std::min(y + 1.0f, lineBottomY);
            uint32_t mask = (y >= lineTopY) ? W : 0;
            processRow<kNeedsClamping, true, true>(ctx, yTopTiles, lineTopY, rowBottomY, mask);
        }

        uint16_t yStartMiddle = isStartCulled ? yTopTiles : (yTopTiles + 1);
        float lineBottomFloor = std::floor(lineBottomY);
        uint16_t yEndMiddle = std::min(f32ToU16Sat(lineBottomFloor), tileRows);

        bool hasSeparateBottomRow = (lineBottomY != lineBottomFloor) &&
                                    (yEndMiddle < tileRows) &&
                                    (isStartCulled || yEndMiddle != yTopTiles);

        if (yStartMiddle < yEndMiddle) {
            uint16_t middleLimit = yEndMiddle;
            if (!hasSeparateBottomRow && middleLimit > yStartMiddle) {
                middleLimit--;
            }

            for (uint16_t yIdx = yStartMiddle; yIdx < middleLimit; ++yIdx) {
                float y = static_cast<float>(yIdx);
                processRow<kNeedsClamping, false, false>(ctx, yIdx, y, y + 1.0f, 0xffffffff);
            }

            if (!hasSeparateBottomRow && yEndMiddle > yStartMiddle) {
                uint16_t yIdx = yEndMiddle - 1;
                float y = static_cast<float>(yIdx);
                float rowBottomY = std::min(y + 1.0f, lineBottomY);
                processRow<kNeedsClamping, false, true>(ctx, yIdx, y, rowBottomY, 0xffffffff);
            }
        }

        if (hasSeparateBottomRow) {
            uint16_t yIdx = yEndMiddle;
            float y = static_cast<float>(yIdx);
            processRow<kNeedsClamping, false, true>(ctx, yIdx, y, lineBottomY, 0xffffffff);
        }
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Tiler_DEFINED
