/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_CompressedRowTiles_DEFINED
#define skgpu_graphite_CompressedRowTiles_DEFINED

#include "src/base/SkVx.h"
#include "include/core/SkRect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsSorts.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace skgpu::graphite {

struct CompressedRowTile {
    uint32_t packedXRunLengthStartMask;
    uint32_t packedLineIdxEndMask;

    CompressedRowTile() : packedXRunLengthStartMask(0), packedLineIdxEndMask(0) {}

    CompressedRowTile(uint16_t x, uint16_t runLength, uint32_t lineIdx,
                      uint32_t startMask, uint32_t endMask = 0) {

        // 1. Enforce the spatial guarantees that make the 13-bit packing safe
        static_assert(SparseStripsConfig::kTileWidth == 8 &&
                      SparseStripsConfig::kTileHeight == 8,
                      "Tile dimensions must be 8x8 to fit x and runLength into 13 bits");

        // 2. Ensure the mask doesn't overflow our remaining bits (32 - 13 - 13 = 6)
        static_assert(SparseStripsConstants::INT_MASK_SHIFT <= 6,
                      "Mask requires too many bits to fit in the 32-bit word alongside X and runLength");

        SkASSERT(x < (1 << 13));
        SkASSERT(runLength < (1 << 13));

        // Pack Word 0: [ X (13 bits) | runLength (13 bits) | startMask (6 bits) ]
        packedXRunLengthStartMask = (static_cast<uint32_t>(x) << (13 + SparseStripsConstants::INT_MASK_SHIFT)) |
                                    (static_cast<uint32_t>(runLength) << SparseStripsConstants::INT_MASK_SHIFT) |
                                    (startMask & ((1 << SparseStripsConstants::INT_MASK_SHIFT) - 1));

        // Pack Word 1: [ lineIdx (26 bits) | endMask (6 bits) ] - Using existing Graphite convention
        packedLineIdxEndMask = (lineIdx << SparseStripsConstants::INT_MASK_SHIFT) |
                               (endMask & ((1 << SparseStripsConstants::INT_MASK_SHIFT) - 1));
    }

    SK_ALWAYS_INLINE uint16_t x() const {
        return packedXRunLengthStartMask >> (13 + SparseStripsConstants::INT_MASK_SHIFT);
    }

    SK_ALWAYS_INLINE uint16_t runLength() const {
        return (packedXRunLengthStartMask >> SparseStripsConstants::INT_MASK_SHIFT) & 0x1FFF; // 0x1FFF = 13 bits
    }

    SK_ALWAYS_INLINE uint32_t startMask() const {
        return packedXRunLengthStartMask & ((1 << SparseStripsConstants::INT_MASK_SHIFT) - 1);
    }

    SK_ALWAYS_INLINE uint32_t lineIdx() const {
        return packedLineIdxEndMask >> SparseStripsConstants::INT_MASK_SHIFT;
    }

    SK_ALWAYS_INLINE uint32_t endMask() const {
        return packedLineIdxEndMask & ((1 << SparseStripsConstants::INT_MASK_SHIFT) - 1);
    }

    // Sort by X ascending, then by runLength descending (for the traversal skip optimization)
    // We decode the values here so the comparison logic is clean and readable,
    // the bitwise shifts are basically free on modern CPUs.
    bool operator<(const CompressedRowTile& other) const {
        uint16_t thisX = this->x();
        uint16_t otherX = other.x();
        if (thisX != otherX) {
            return thisX < otherX;
        }
        return this->runLength() > other.runLength();
    }
};

template <typename TileType = CompressedRowTile>
class CompressedRowTiles {
public:
    using value_type = TileType;

    CompressedRowTiles() {}

    void reset() {
        for (auto& row : fTileRows) {
            row.clear();
        }
    }

    void sortTiles() {
        for (auto& row : fTileRows) {
            if (!row.empty()) {
                std::stable_sort(row.begin(), row.end());
            }
        }
    }

    const skia_private::TArray<SkTDArray<TileType>>& getRows() const { return fTileRows; }
    skia_private::TArray<SkTDArray<TileType>>& getRows() { return fTileRows; }

    bool makeCompressedTilesMSAA(const Polyline& polyline, const SkIRect& clip,
                                 WindingHistogram& histogram, int32_t startingLineOffset) {
        SkASSERT(polyline.count() <= SparseStripsConstants::MAX_LINES_PER_PATH);

        bool cullingEventOccurred = false;
        if (clip.isEmpty()) {
            return cullingEventOccurred;
        }

        constexpr float invW = 1.0f / static_cast<float>(SparseStripsConfig::kTileWidth);
        constexpr float invH = 1.0f / static_cast<float>(SparseStripsConfig::kTileHeight);

        uint16_t tileColumns = divCeil(clip.right(), SparseStripsConfig::kTileWidth);
        uint16_t tileRows = divCeil(clip.bottom(), SparseStripsConfig::kTileHeight);

        if (fTileRows.size() < tileRows + 1) {
            fTileRows.resize(tileRows + 1);
        }

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
                        fTileRows[yTopTiles].push_back(TileType(x, 1, lineIdx, intersectionMask));
                    }

                    uint16_t yStart = isStartCulled ? yTopTiles : (yTopTiles + 1);
                    float lineBottomFloor = std::floor(lineBottomY);
                    uint16_t yEndIdx = std::min(f32ToU16Sat(lineBottomFloor), tileRows);

                    if (yStart < yEndIdx) {
                        uint16_t yLast = yEndIdx - 1;
                        for (uint16_t yIdx = yStart; yIdx < yLast; ++yIdx) {
                            uint32_t intersectionMask = W | B | T;
                            fTileRows[yIdx].push_back(TileType(x, 1, lineIdx, intersectionMask));
                        }

                        {
                            bool isEndTile = ((int32_t)yLast == p1TileY);
                            uint32_t intersectionMask = W | T | (isEndTile ? 0 : B);
                            fTileRows[yLast].push_back(TileType(x, 1, lineIdx, intersectionMask));
                        }
                    }

                    if (lineBottomY != lineBottomFloor && yEndIdx < tileRows) {
                        uint32_t intersectionMask = W | T;
                        fTileRows[yEndIdx].push_back(TileType(x, 1, lineIdx, intersectionMask));
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
                uint16_t xClamped = std::min(f32ToU16Sat(lineLeftX), static_cast<uint16_t>(tileColumns + 1));
                xClamped = std::max(minTileX, xClamped);
                uint32_t winding = static_cast<float>(yTopTiles) >= lineTopY ? W : 0;
                fTileRows[yTopTiles].push_back(TileType(xClamped, 1, lineIdx, winding));
            }
        }

        return cullingEventOccurred;
    }

    bool makeCompressedTilesMSAA(const Polyline& polyline, const SkIRect& clip, WindingHistogram& histogram) {
        return this->makeCompressedTilesMSAA(polyline, clip, histogram, /*startingLineOffset=*/0);
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

    skia_private::TArray<SkTDArray<TileType>> fTileRows;

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

    // 2-wide SIMD Edge Mask Calculator using skvx
    template <bool kXDir, bool kCheckStart, bool kCheckEnd>
    static SK_ALWAYS_INLINE skvx::uint2 computeEdgeMasks2(const LineContext& ctx,
                                                          skvx::uint2 xIdx,
                                                          uint16_t yIdx,
                                                          float rowTopX,
                                                          float rowBottomX,
                                                          int32_t canonicalStart,
                                                          uint16_t canonicalEnd,
                                                          skvx::uint2 windingInput) {
        skvx::int2 xIdxInt = skvx::cast<int32_t>(xIdx);

        // Vectorized Equality: Returns ~0 (true) or 0 (false). AND with 1 to get 1 or 0.
        skvx::uint2 uncRowStart = skvx::cast<uint32_t>(xIdxInt == canonicalStart) & 1;
        skvx::uint2 uncRowEnd   = skvx::cast<uint32_t>(xIdx == canonicalEnd) & 1;

        skvx::uint2 canonicalRowStart = kXDir ? uncRowStart : uncRowEnd;
        skvx::uint2 canonicalRowEnd   = kXDir ? uncRowEnd : uncRowStart;

        skvx::uint2 notStartTile(1);
        if constexpr (kCheckStart) {
            if (yIdx == ctx.p0TileY) { // Only do the X-math if we are on the start row
                notStartTile ^= (skvx::cast<uint32_t>(xIdxInt == ctx.p0TileX) & 1);
            }
        }

        skvx::uint2 notEndTile(1);
        if constexpr (kCheckEnd) {
            if (yIdx == ctx.p1TileY) { // Only do the X-math if we are on the end row
                notEndTile ^= (skvx::cast<uint32_t>(xIdxInt == ctx.p1TileX) & 1);
            }
        }

        skvx::uint2 mask = windingInput;
        mask |= canonicalRowStart & notStartTile;

        if constexpr (kXDir) {
            mask |= (skvx::uint2(1) ^ canonicalRowStart) << LEFT_SHIFT;
            mask |= (canonicalRowEnd & notEndTile) << BOT_SHIFT;
            mask |= (skvx::uint2(1) ^ canonicalRowEnd) << RIGHT_SHIFT;
        } else {
            mask |= (skvx::uint2(1) ^ canonicalRowStart) << RIGHT_SHIFT;
            mask |= (canonicalRowEnd & notEndTile) << BOT_SHIFT;
            mask |= (skvx::uint2(1) ^ canonicalRowEnd) << LEFT_SHIFT;
        }

        skvx::float2 xLeftF = skvx::cast<float>(xIdx);
        skvx::float2 xRightF = xLeftF + 1.0f; // Adding 1 to the float vector directly

        skvx::uint2 trc = (skvx::cast<uint32_t>(rowTopX == xRightF) & 1) & notStartTile;
        skvx::uint2 tlc = (skvx::cast<uint32_t>(rowTopX == xLeftF) & 1) & notStartTile;
        skvx::uint2 brc = (skvx::cast<uint32_t>(rowBottomX == xRightF) & 1) & notEndTile;
        skvx::uint2 blc = (skvx::cast<uint32_t>(rowBottomX == xLeftF) & 1) & notEndTile;

        skvx::uint2 tieBreak = tlc & (canonicalRowStart ^ 1);

        mask |= (tieBreak | blc) * L;
        mask |= (trc | brc) * R;
        mask &= ~((tieBreak | trc) * T);
        mask &= ~((blc | brc) * B);

        return mask;
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
                    wMask &= ~W;
                }
            }

            lx = std::fmax(std::fmin(rowTopX, rowBottomX), ctx.lineLeftX);
            rx = std::fmin(std::fmax(rowTopX, rowBottomX), ctx.lineRightX);

            xEndVal = std::min(f32ToU16Sat(rx),
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
            uint16_t runLength = xEndVal - xStart + 1;
            uint32_t isSingle = (runLength == 1) ? 1 : 0;

            uint32_t wLeft = ((kXDir ? W : 0) | (isSingle ? W : 0)) & wMask;
            uint32_t wRight = (kXDir ? 0 : W) & wMask;

            // Pack our inputs into 2-wide vectors
            skvx::uint2 xIdx2(xStart, xEndVal);
            skvx::uint2 wInput2(wLeft, wRight);

            // Compute both edge masks simultaneously without branching!
            skvx::uint2 edgeMasks = computeEdgeMasks2<kXDir, kCheckStart, kCheckEnd>(
                    ctx, xIdx2, yIdx, rowTopX, rowBottomX, canonicalXStart, canonicalXEnd, wInput2);

            // Unpack. (If runLength == 1, edgeMasks[1] was computed but is safely discarded here).
            uint32_t startMask = edgeMasks[0];
            uint32_t endMask = (runLength > 1) ? edgeMasks[1] : 0;

            fTileRows[yIdx].push_back(TileType(xStart, runLength, ctx.lineIdx, startMask, endMask));
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

#endif  // skgpu_graphite_CompressedRowTiles_DEFINED
