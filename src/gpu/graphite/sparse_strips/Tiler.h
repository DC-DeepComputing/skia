/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_sparse_strips_Tiler_DEFINED
#define skgpu_graphite_sparse_strips_Tiler_DEFINED

#include "include/core/SkRect.h"
#include "include/private/SkAssert.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <tuple>
#include <vector>

namespace skgpu::graphite {

// A sparse strips tile, contains:
//  1) The top left corner of the tile in device space, in tile sized units.
//  2) The coarse winding (W) of the tile
//  3) The intersection mask (R | L | T | B) of the tile
//  4) The index of their parent line in the array backing polyline
//
// TODO (thomsmit): Should move onto SparseStripsTypes?
struct Tile : public IntersectionBits {
    uint16_t x;
    uint16_t y;
    /// Contains the parent line's index and the intersection/winding mask.
    /// MSB                                                            LSB
    /// 31------------------------------------------------------5|4|3|2|1|0|
    /// |            Parent Line Index (27 bits)                 |W|R|L|B|T|
    uint32_t fPackedLineIdxIntersectionMask;

    Tile() : x(0), y(0), fPackedLineIdxIntersectionMask(0) {}

    Tile(uint16_t x, uint16_t y, uint32_t lineIdx, uint32_t intersectionMask)
            : x(x)
            , y(y)
            , fPackedLineIdxIntersectionMask((lineIdx << IntersectionBits::INT_MASK_SHIFT) |
                                   intersectionMask) {
        SkASSERT(intersectionMask < (1 << IntersectionBits::INT_MASK_SHIFT));
    }

    // When sorting we place y in the highest bits followed by x, so that tiles are grouped by row,
    // left to right. Note, lineIdx is placed in the lower bits of the sorting key so that tiles
    // from the same parent line at the same geometric location are adjacent and have better cache
    // locality when processed in makeStrips.
    //
    // TODO (thomsmit): try holding as u64 and converting for member access instead of converting
    // for sorting.
    SK_ALWAYS_INLINE uint64_t toBits() const {
        return (static_cast<uint64_t>(y) << 48) | (static_cast<uint64_t>(x) << 32) |
               static_cast<uint64_t>(fPackedLineIdxIntersectionMask);
    }

    bool operator<(const Tile& other) const { return toBits() < other.toBits(); }

    bool operator==(const Tile& other) const { return toBits() == other.toBits(); }

    uint32_t lineIdx() const {
        return fPackedLineIdxIntersectionMask >> IntersectionBits::INT_MASK_SHIFT;
    }
    uint32_t intersectionMask() const {
        return fPackedLineIdxIntersectionMask & IntersectionBits::INTERSECTION_MASK;
    }
    bool coarseWinding() const {
        return (fPackedLineIdxIntersectionMask & IntersectionBits::W) != 0;
    }
    bool hasLeftIntersection() const {
        return (fPackedLineIdxIntersectionMask & IntersectionBits::L) != 0;
    }

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
    template <uint16_t kWidth, uint16_t kHeight>
    static SK_ALWAYS_INLINE std::tuple<Line, bool, bool> ClipToTile(
            const Line& line,
            const std::array<SkPoint, 2>& tileBounds,
            const std::array<float, 4>& derivatives,
            uint32_t intersectionMask,
            bool canonicalXDir,
            bool canonicalYDir) {
        constexpr float width = static_cast<float>(kWidth);
        constexpr float height = static_cast<float>(kHeight);

        const SkPoint topLeft = tileBounds[0];
        const float tileMinX = tileBounds[0].fX;
        const float tileMinY = tileBounds[0].fY;
        const float tileMaxX = tileBounds[1].fX;
        const float tileMaxY = tileBounds[1].fY;

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
            maskVIn   = L;
            boundVIn  = tileMinX;
            maskVOut  = R;
            boundVOut = tileMaxX;
        } else {
            maskVIn   = R;
            boundVIn  = tileMaxX;
            maskVOut  = L;
            boundVOut = tileMinX;
        }

        uint32_t maskHIn, maskHOut;
        float boundHIn, boundHOut;
        if (canonicalYDir) {
            maskHIn   = T;
            boundHIn  = tileMinY;
            maskHOut  = B;
            boundHOut = tileMaxY;
        } else {
            maskHIn   = B;
            boundHIn  = tileMaxY;
            maskHOut  = T;
            boundHOut = tileMinY;
        }

        const float invDx = derivatives[2];
        const float invDy = derivatives[3];

        // Check the candidate edges against the intersection mask
        const uint32_t entryHits = intersectionMask & (maskVIn | maskHIn);
        const uint32_t exitHits = intersectionMask & (maskVOut | maskHOut);

        auto clipPt = [&](SkPoint p, uint32_t hits, uint32_t maskH, float boundH, float boundV)
                -> std::pair<SkPoint, bool> {
            bool isLeft = false;
            if (hits != 0) {
                const bool useH = (intersectionMask & maskH) != 0;
                const float bound = useH ? boundH : boundV;
                const float start = useH ? line.p0.fY : line.p0.fX;
                const float invD = useH ? invDy : invDx;

                const float t = (bound - start) * invD;

                p.fX = line.p0.fX + t * dx;
                p.fY = line.p0.fY + t * dy;

                if (useH) {
                    p.fY = bound;
                    p.fX -= topLeft.fX;
                    p.fY -= topLeft.fY;
                    p.fX = std::clamp(p.fX, 0.0f, width);
                } else {
                    p.fX = bound;
                    p.fX -= topLeft.fX;
                    p.fY -= topLeft.fY;
                    p.fY = std::clamp(p.fY, 0.0f, height);
                    isLeft = (bound == tileMinX);
                }
            } else {
                p.fX -= topLeft.fX;
                p.fY -= topLeft.fY;
                p.fX = std::clamp(p.fX, 0.0f, width);
                p.fY = std::clamp(p.fY, 0.0f, height);
            }
            return {p, isLeft};
        };

        auto [pEntry, entryLeft] = clipPt(line.p0, entryHits, maskHIn, boundHIn, boundVIn);
        auto [pExit, exitLeft] = clipPt(line.p1, exitHits, maskHOut, boundHOut, boundVOut);

        // TODO (thomsmit): add a section to the function preamble describing the what these bools
        // guarantee. Explain, how this works. (Non-Obvious why entry/exit maps to top/bottom.)
        bool topIsOnLeftEdge = canonicalYDir ? entryLeft : exitLeft;
        bool botIsOnLeftEdge = canonicalYDir ? exitLeft : entryLeft;

        // Guarantee predictable winding order for downstream rasterization stages.
        if (canonicalYDir) {
            return {{pEntry, pExit}, topIsOnLeftEdge, botIsOnLeftEdge};
        } else {
            return {{pExit, pEntry}, topIsOnLeftEdge, botIsOnLeftEdge};
        }
    }
};

// Make sure the struct is a size convenient to move around, as tiles will be sorted.
static_assert(sizeof(Tile) == 8);

// Grouped per-row tiles ("PolyTiles", Tiles<..., /*kRows=*/true, /*kGrouped=*/true>) hold one key
// per run of consecutive segments of the polyline inside one tile, rather than one key per segment
// and tile. y is implied by the row:
//
//  MSB                                                                            LSB
//  |63--------48|47---------------------21|20--------16|15-------11|10----------0|
//  |     x      | first line index (27)   | first mask | last mask | extra (11)  |
//
//  - The 32 bits under x are the run's first segment's Tile::fPackedLineIdxIntersectionMask, so a
//    run of one segment is a plain tile.
//  - extra: the number of segments after the first, which follow it in the polyline.
//  - last mask: the intersection mask of the last segment, if extra > 0. The segments in between
//    have mask 0: a segment is only appended while the run's last segment is its first, or has
//    mask 0 (CanAppend()).
// So every segment keeps its own mask, and expanding the runs (ForEachSegment()) gives MakeStrips
// exactly the (segment, mask, tile) triples of plain tiles. Sorting is unchanged: within a tile,
// runs are ordered by their first line index, and their segments are consecutive, so expanding
// them visits the segments in the flat layout's order.
struct GroupedTileKey : public IntersectionBits {
    static constexpr int kXShift = 48;
    static constexpr int kPackedShift = 16;
    static constexpr int kLastShift = 11;
    static constexpr uint32_t kMaxExtra = (1u << kLastShift) - 1;

    static SK_ALWAYS_INLINE uint64_t Make(uint16_t x, uint32_t packed) {
        return (uint64_t{x} << kXShift) | (uint64_t{packed} << kPackedShift);
    }
    static SK_ALWAYS_INLINE uint16_t X(uint64_t key) {
        return static_cast<uint16_t>(key >> kXShift);
    }
    // The run's first segment: for a plain tile (Extra() == 0), its fPackedLineIdxIntersectionMask.
    static SK_ALWAYS_INLINE uint32_t Packed(uint64_t key) {
        return static_cast<uint32_t>(key >> kPackedShift);
    }
    static SK_ALWAYS_INLINE uint32_t Extra(uint64_t key) {
        return static_cast<uint32_t>(key) & kMaxExtra;
    }

    // True if the run can take the next segment: it isn't full, and its last segment, which would
    // move inside the run, is its first or has mask 0.
    static SK_ALWAYS_INLINE bool CanAppend(uint64_t key) {
        // The low 16 bits are the last mask and extra; the last mask is 0 while extra is.
        return (static_cast<uint32_t>(key) & 0xFFFF) < kMaxExtra;
    }

    // Appends the next segment, with its mask in this tile.
    static SK_ALWAYS_INLINE uint64_t Append(uint64_t key, uint32_t mask) {
        SkASSERT(CanAppend(key));
        return key + ((uint64_t{mask} << kLastShift) | 1);
    }

    // The run's last segment, if Extra() > 0: what its own Tile::fPackedLineIdxIntersectionMask
    // would be.
    static SK_ALWAYS_INLINE uint32_t LastPacked(uint64_t key) {
        const uint32_t firstLine = Packed(key) & ~INTERSECTION_MASK;
        const uint32_t lastMask = (static_cast<uint32_t>(key) >> kLastShift) & INTERSECTION_MASK;
        return (firstLine + (Extra(key) << INT_MASK_SHIFT)) | lastMask;
    }

    // Calls fn(packed, inner) for each segment of the run in 'key', in polyline order, where
    // 'packed' is what the segment's own Tile::fPackedLineIdxIntersectionMask would be, and 'inner'
    // is true for the segments strictly inside the run, whose mask is 0.
    template <typename Fn> static SK_ALWAYS_INLINE void ForEachSegment(uint64_t key, Fn&& fn) {
        const uint32_t packed = Packed(key);
        const uint32_t extra = Extra(key);
        const uint32_t firstMask = packed & INTERSECTION_MASK;
        const uint32_t lastMask = (static_cast<uint32_t>(key) >> kLastShift) & INTERSECTION_MASK;
        uint32_t line = packed & ~INTERSECTION_MASK;
        for (uint32_t i = 0; i <= extra; ++i) {
            const bool inner = i != 0 && i != extra;
            fn(line | (i == 0 ? firstMask : 0u) | (i == extra ? lastMask : 0u), inner);
            line += 1u << INT_MASK_SHIFT;
        }
    }
};

// Container that holds the tiles and manages their state (sorting). Templated to expose the tile
// size as statically tunable parameter which can be tested within the same compilation unit
// Tiles are stored in one of three layouts:
//  - Flat (kRows = false): one buffer of Tiles, sorted globally by Tile::toBits().
//  - Per row (kRows = true): one buffer of 64-bit keys per tile row, x << 32 |
//    fPackedLineIdxIntersectionMask, with y implied by the row. The Tiler emits straight into the
//    rows, so sorting is one small sort per row: about 2x faster than the flat sort, since each row
//    is sorted in cache and y drops out of the compare. Walking the rows in order yields exactly
//    the flat order, and dropping y from the key frees 16 bits.
//  - Grouped per row (kRows = kGrouped = true): the same, with GroupedTileKeys, which use those 16
//    bits to hold a run of consecutive segments inside one tile in one key. The Tiler runs as for
//    plain tiles; then a segment's tile at its start point joins the previous segment's run when
//    that ended in the same cell (see mergeEnds()). There are fewer keys to sort.
template <uint16_t kTileWidth, uint16_t kTileHeight, bool kRows = false, bool kGrouped = false>
class Tiles : public IntersectionBits {
    static_assert(kRows || !kGrouped, "Grouped tiles are stored per row.");

public:
    Tiles() {}
    void reset() {
        fTileBuf.clear();
        if constexpr (kRows) {
            for (int y = fRowMin; y < fRowMax; ++y) {
                Row& row = fRows.data()[y];
                row.fKeys.clear();
                row.fSize = 0;
            }
            fRowMin = std::numeric_limits<int>::max();
            fRowMax = 0;
        }
    }

    void sortTiles(int32_t offset) {
        static_assert(!kRows, "Sort per-row tiles with sortTiles().");
        std::sort(fTileBuf.begin() + offset, fTileBuf.end());
    }

    void sortTiles() {
        if constexpr (kRows) {
            for (int y = fRowMin; y < fRowMax; ++y) {
                SkTDArray<uint64_t>& keys = fRows.data()[y].fKeys;
                std::sort(keys.begin(), keys.end());
            }
        } else {
            this->sortTiles(0);
        }
    }

    // Flat layout only.
    const SkTDArray<Tile>& getTiles() const { return fTileBuf; }

    // Per-row layouts only: the rows in [rowMin(), rowMax()) may hold tiles. Their keys are
    // GroupedTileKeys if kGrouped.
    int rowMin() const { return fRowMin; }
    int rowMax() const { return fRowMax; }
    const SkTDArray<uint64_t>& getRow(int y) const { return fRows.data()[y].fKeys; }

    // The bytes of tile storage, which reset() retains.
    size_t retainedBytes() const {
        if constexpr (kRows) {
            return sizeof(uint64_t) * fRetainedKeys + sizeof(Row) * fRows.capacity();
        } else {
            return sizeof(Tile) * static_cast<size_t>(fTileBuf.capacity());
        }
    }

    // This function has two purposes:
    //     1) The viewport is divided into tiles, whose dimensions are given by the template
    //     parameters. Tiles act as a "super coarse rasterization stage," which elides carrying a
    //     scanline for each MSAA subsample point. (Without this technique, we would require 64
    //     scanlines for an 8 tall tile with 8xMSAA). So for each line produced by
    //     flattening---contained inside Polyline---we need to find which tiles this line
    //     intersects.  Tile edge touches are vertical exclusive, horizontal inclusive. This is
    //     because:
    //          A) In the vertical case, inclusivity would cause the coarse winding to be double
    //          counted or negated by a single-point tile produced by a grazing touch.
    //          B) In the horizontal case, the inclusivity is necessary because the tile produced by
    //          the succeeding line may not consider itself left-touching (depending on its
    //          direction), so the single-point tile is necessary to carry over the left-edge
    //          winding.
    //     Using the line's direction enforces mutually exclusive ownership of boundary
    //     intersections between consecutive segments *of the same direction*, ensuring that the
    //     left-edge winding is neither lost nor double-counted:
    //
    //                                  Left edge behavior:
    //                      +----------------------+----------+------------+
    //                      | X Direction          | Endpoint | L Bit Set? |
    //                      +----------------------+----------+------------+
    //                      | Left-to-Right (L->R) | Start    | No         |
    //                      | Left-to-Right (L->R) | End      | Yes        |
    //                      | Right-to-Left (R->L) | Start    | Yes        |
    //                      | Right-to-Left (R->L) | End      | No         |
    //                      +----------------------+----------+------------+
    //
    //     2) To enable parallel rasterization, we need to establish a source of truth for the line
    //     intersection points on tiles, such that adjacent tiles agree where intersections occur.
    //     While calculating exact intersection coordinates here is feasible, it is (relatively)
    //     computationally expensive, so instead, we defer the heavy math to the GPU/rasterizer and
    //     produce a lightweight intersection bitmask. This mask unambiguously defines which edges
    //     of a tile a line segment touches:
    //
    //     Bit representation:
    //     Bit: 4 | 3 | 2 | 1 | 0
    //     Val: W | R | L | B | T
    //
    //     - W (Winding): Tracks whether the line touched the top edge of the tile.
    //     - R/L/B/T: Right, Left, Bottom, and Top edge intersections.
    void makeTilesMSAA(const Polyline& polyline, uint16_t viewportWidth, uint16_t viewportHeight) {
        int32_t numRows = (viewportHeight + kTileHeight - 1) / kTileHeight;
        WindingHistogram culled(numRows);
        culled.resize(numRows);
        this->makeTilesMSAA(polyline, SkIRect::MakeWH(viewportWidth, viewportHeight), &culled);
    }

    bool makeTilesMSAA(const Polyline& polyline, SkIRect clip, WindingHistogram* histogram) {
        SkASSERT(polyline.count() <= MAX_LINES_PER_PATH);

        bool cullingEventOccurred = false;
        if (clip.isEmpty()) {
            return cullingEventOccurred;
        }

        constexpr float invW = 1.0f / static_cast<float>(kTileWidth);
        constexpr float invH = 1.0f / static_cast<float>(kTileHeight);

        uint16_t minTileX = f32ToU16Sat(clip.left() * invW);
        uint16_t minTileY = f32ToU16Sat(clip.top() * invH);
        uint16_t maxTileX = DivCeil(clip.right(), kTileWidth);
        uint16_t maxTileY = DivCeil(clip.bottom(), kTileHeight);
        if constexpr (kRows) {
            if (fRows.size() < static_cast<size_t>(maxTileY)) {
                fRows.resize(maxTileY);
            }
        }

        float clipLeftTile = static_cast<float>(minTileX);
        float clipTopTile = static_cast<float>(minTileY);
        float clipRightTile = clip.right() * invW;

        // The run of the previous segment's tile at its end point (kGrouped only).
        [[maybe_unused]] Run run;

        for (auto it = polyline.begin(); it != polyline.end(); ++it) {
            auto [line, lineIdx] = *it;

            // map line into tile units
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

            // If the leftmost point of this line is right of the clip bounds, cull it. Although we
            // cull path verbs right of the viewport in the flattening stage, a right edge crossing
            // path verb may still be flattened into lines, some of which may be completely outside
            // of the viewport/clip.
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

            uint16_t yTopTiles = std::max(minTileY, std::min(f32ToU16Sat(lineTopY), maxTileY));
            float lineBottomYCeil = std::ceil(lineBottomY);
            uint16_t yBottomTiles =
                    std::max(minTileY, std::min(f32ToU16Sat(lineBottomYCeil), maxTileY));

            // If yTopTiles == yBottomTiles, then the line is either completely above or below the
            // clip OR it is perfectly horizontal and aligned to the tile grid, contributing no
            // winding. In either case, it should be culled.
            if (yTopTiles >= yBottomTiles) {
                // Technically, the `>` part of the `>=` is unnecessary due to clamping, but this
                // gives stronger signal.
                continue;
            }
            if constexpr (kRows) {
                fRowMin = std::min(fRowMin, static_cast<int>(yTopTiles));
                fRowMax = std::max(fRowMax, static_cast<int>(yBottomTiles));
            }

            int32_t p0TileX = f32ToI32Sat(std::floor(lineTopX));
            int32_t p0TileY = f32ToI32Sat(std::floor(lineTopY));
            int32_t p1TileX = f32ToI32Sat(std::floor(lineBottomX));
            int32_t p1TileY;
            if (lineBottomY == lineBottomYCeil) {
                p1TileY = f32ToI32Sat(lineBottomY) - 1;
            } else {
                p1TileY = f32ToI32Sat(std::floor(lineBottomY));
            }

            // Each line processed falls into 1 of four categories:
            //  1) The line is completely to the left of the clip (culled into WindingHistogram).
            //  2) The line is perfectly vertical.
            //  3) The line produces a single tile.
            //  4) The line is general (sloped across tiles, potentially crossing clip edges).
            bool notSameTile = (p0TileY != p1TileY) || (p0TileX != p1TileX);

            // Left-edge culling: Lines completely to the left of the clip bounds do not generate
            // any tiles in the visible grid. However, their vertical span contributes winding to
            // all scanlines to their right. Record this winding directly into the WindingHistogram
            // rather than allocating tiles into fTileBuf.
            if (lineRightX < clipLeftTile) {
                cullingEventOccurred = true;
                bool isStartCulled = (lineTopY < clipTopTile);
                if (!isStartCulled) {
                    if (static_cast<float>(yTopTiles) >= lineTopY) {
                        histogram->addWinding(yTopTiles, windingDir);
                    }
                }

                uint16_t yStart = isStartCulled ? yTopTiles : (yTopTiles + 1);
                float lineBottomFloor = std::floor(lineBottomY);
                uint16_t yEndIdx = std::min(f32ToU16Sat(lineBottomFloor), maxTileY);
                histogram->addWindingRange(yStart, yEndIdx, windingDir);

                if ((p0TileY != p1TileY) && lineBottomY != lineBottomFloor && yEndIdx < maxTileY) {
                    histogram->addWinding(yEndIdx, windingDir);
                }
                continue;
            }

            if (notSameTile) {
                // Where the line's tiles at its top and bottom points went (kGrouped only).
                EndTiles ends;
                if (lineLeftX == lineRightX) { // Vertical line case
                    uint16_t x = std::min(f32ToU16Sat(lineLeftX),
                                          static_cast<uint16_t>(maxTileX > 0 ? maxTileX - 1 : 0));
                    x = std::max(minTileX, x);

                    // Process the Top Row (if visible on screen)
                    bool isStartCulled = (lineTopY < clipTopTile);
                    if (!isStartCulled) {
                        uint32_t winding = (static_cast<float>(yTopTiles) >= lineTopY) ? W : 0;
                        uint32_t intersectionMask = B | winding;
                        ends.top = this->emit(Tile(x, yTopTiles, lineIdx, intersectionMask));
                    }

                    // Process all "fully crossed" tiles (W | T | B).
                    uint16_t yStart = isStartCulled ? yTopTiles : (yTopTiles + 1);
                    int32_t yEndIdx = std::min(p1TileY, static_cast<int32_t>(maxTileY));
                    for (int32_t yIdx = yStart; yIdx < yEndIdx; ++yIdx) {
                        uint32_t intersectionMask = W | T | B;
                        this->emit(Tile(x, static_cast<uint16_t>(yIdx), lineIdx, intersectionMask));
                    }

                    // Process the terminal tile (W | T), if it exists. We only emit this if the
                    // line actually terminates on the screen, and if we haven't already
                    // processed/culled it via `yStart`.
                    if (p1TileY >= yStart && p1TileY < static_cast<int32_t>(maxTileY)) {
                        uint32_t intersectionMask = W | T;
                        ends.bottom = this->emit(
                                Tile(x, static_cast<uint16_t>(p1TileY), lineIdx, intersectionMask));
                    }
                    if constexpr (kGrouped) {
                        const uint32_t topCell = Cell(x, yTopTiles);
                        const uint32_t bottomCell = Cell(x, static_cast<uint32_t>(p1TileY));
                        this->mergeEnds(ends,
                                        static_cast<uint32_t>(lineIdx),
                                        windingDir > 0,
                                        topCell,
                                        bottomCell,
                                        &run);
                    }
                } else { // General case
                    float dx = p1X - p0X;
                    float dy = p1Y - p0Y;
                    float xSlope = dx / dy;

                    // Package for the helper functions, with inlining this should be zero cost.
                    // Changing to a class member regresses perf by ~10% on TilerSortBench
                    LineContext ctx {
                        static_cast<uint32_t>(lineIdx),
                        lineTopX, lineTopY,
                        lineBottomX, lineBottomY,
                        xSlope,
                        lineLeftX, lineRightX,
                        p0TileX, p0TileY,
                        p1TileX, p1TileY,
                        maxTileX,
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
                        cullingEventOccurred |= leftCrossing;
                        if (kXDir) {
                            runLoops</*kXDir=*/true, /*kCrossesEdge=*/true>(ctx,
                                                                            lineTopY,
                                                                            lineBottomY,
                                                                            yTopTiles,
                                                                            maxTileY,
                                                                            histogram,
                                                                            &ends);
                        } else {
                            runLoops</*kXDir=*/false, /*kCrossesEdge=*/true>(ctx,
                                                                             lineTopY,
                                                                             lineBottomY,
                                                                             yTopTiles,
                                                                             maxTileY,
                                                                             histogram,
                                                                             &ends);
                        }
                    } else {
                        if (kXDir) {
                            runLoops</*kXDir=*/true, /*kCrossesEdge=*/false>(ctx,
                                                                             lineTopY,
                                                                             lineBottomY,
                                                                             yTopTiles,
                                                                             maxTileY,
                                                                             histogram,
                                                                             &ends);
                        } else {
                            runLoops</*kXDir=*/false, /*kCrossesEdge=*/false>(ctx,
                                                                              lineTopY,
                                                                              lineBottomY,
                                                                              yTopTiles,
                                                                              maxTileY,
                                                                              histogram,
                                                                              &ends);
                        }
                    }
                    if constexpr (kGrouped) {
                        // The top and bottom tiles are in the cells of the line's top and bottom
                        // points.
                        const uint32_t topCell = Cell(static_cast<uint32_t>(p0TileX),
                                                      static_cast<uint32_t>(p0TileY));
                        const uint32_t bottomCell = Cell(static_cast<uint32_t>(p1TileX),
                                                         static_cast<uint32_t>(p1TileY));
                        this->mergeEnds(ends,
                                        static_cast<uint32_t>(lineIdx),
                                        windingDir > 0,
                                        topCell,
                                        bottomCell,
                                        &run);
                    }
                }
            } else { // Single tile case
                uint16_t xClamped =
                        std::min(f32ToU16Sat(lineLeftX),
                                 static_cast<uint16_t>(maxTileX > 0 ? maxTileX - 1 : 0));
                xClamped = std::max(minTileX, xClamped);
                uint32_t winding = static_cast<float>(yTopTiles) >= lineTopY ? W : 0;
                const Tile tile(xClamped, yTopTiles, lineIdx, winding);
                if constexpr (kGrouped) {
                    this->mergeSingle(tile, &run);
                } else {
                    this->emit(tile);
                }
            }
        }

        if constexpr (kRows) {
            // Sync each touched row's size and count its growth for retainedBytes().
            for (int y = fRowMin; y < fRowMax; ++y) {
                Row& row = fRows.data()[y];
                row.fKeys.resize(row.fSize);
                fRetainedKeys += static_cast<size_t>(row.fKeys.capacity() - row.fAccountedCapacity);
                row.fAccountedCapacity = row.fKeys.capacity();
            }
        }
        return cullingEventOccurred;
    }

private:
    // For now, only support square tiles.
    static_assert(kTileWidth == kTileHeight);

    struct LineContext {
        uint32_t lineIdx;
        float    topX, topY;
        float    bottomX, bottomY;
        float    xSlope;
        float    lineLeftX, lineRightX;
        int32_t  p0TileX, p0TileY;
        int32_t  p1TileX, p1TileY;
        uint16_t maxTileX;
        int16_t  windingDir;
        float    clipLeftTile;
        float    clipTopTile;
        uint16_t minTileX;
    };

    SkTDArray<Tile> fTileBuf;

    // Per-row layout. During makeTilesMSAA(), keys are written directly through fKeys.data()
    // with capacity reserved on demand and a single resize(fSize) per touched row at the end.
    struct Row {
        SkTDArray<uint64_t> fKeys;
        int fSize = 0;
        int fAccountedCapacity = 0;  // fKeys' capacity, as counted in fRetainedKeys

        SK_ALWAYS_INLINE void ensureSpace(int count) {
            if (fKeys.capacity() - fSize < count) {
                fKeys.reserve(fSize + count);
            }
        }

        SK_ALWAYS_INLINE int push(uint64_t key) {
            this->ensureSpace(1);
            const int idx = fSize++;
            fKeys.data()[idx] = key;
            return idx;
        }

        SK_ALWAYS_INLINE uint64_t* append(int count) {
            this->ensureSpace(count);
            uint64_t* dst = fKeys.data() + fSize;
            fSize += count;
            return dst;
        }

        // Writes 'key' at index idx <= fSize, appending it if idx == fSize without branching.
        SK_ALWAYS_INLINE void setOrAppend(int idx, uint64_t key) {
            SkASSERT(idx >= 0 && idx <= fSize);
            this->ensureSpace(1);
            fKeys.data()[idx] = key;
            fSize += static_cast<int>(idx == fSize);
        }

        SK_ALWAYS_INLINE void removeShuffle(int idx) {
            SkASSERT(idx >= 0 && idx < fSize);
            uint64_t* keys = fKeys.data();
            keys[idx] = keys[--fSize];
        }
    };
    std::vector<Row> fRows;
    int fRowMin = std::numeric_limits<int>::max();
    int fRowMax = 0;
    size_t fRetainedKeys = 0;

    static SK_ALWAYS_INLINE uint64_t RowKey(const Tile& tile) {
        if constexpr (kGrouped) {
            return GroupedTileKey::Make(tile.x, tile.fPackedLineIdxIntersectionMask);
        } else {
            return (uint64_t{tile.x} << 32) | tile.fPackedLineIdxIntersectionMask;
        }
    }

    // Returns the tile's index in its row, in the per-row layouts.
    SK_ALWAYS_INLINE int emit(const Tile& tile) {
        if constexpr (kRows) {
            return fRows.data()[tile.y].push(RowKey(tile));
        } else {
            fTileBuf.push_back(tile);
            return 0;
        }
    }

    // ChainMerge (kGrouped only) -----------------------------------------------------------------
    //
    // The Tiler runs exactly as for plain tiles. Then a segment's tile at its start point joins the
    // previous segment's run if that ended in the same cell, and the run can take it
    // (GroupedTileKey::CanAppend()): its mask goes into the run's key, and its own key is dropped.
    // That's exact whatever the geometry: a segment has at most one tile per cell, and each keeps
    // its own mask. So there's no geometric test.
    //
    // The row loops don't merge; they only note where the line's top and bottom tiles went, and
    // mergeEnds() runs after them. Merging inside them, at the end point tiles, cost several times
    // more per segment, and so did keeping the run in members rather than in locals.

    static constexpr uint64_t kNoRun = ~uint64_t{0};

    static SK_ALWAYS_INLINE uint32_t Cell(uint32_t x, uint32_t y) {
        return (x & 0xFFFFu) | (y << 16);
    }

    // The segment and the cell that can join a run.
    static SK_ALWAYS_INLINE uint64_t Tag(uint32_t lineIdx, uint32_t cell) {
        return (uint64_t{lineIdx} << 32) | cell;
    }

    // The run of the previous segment's tile at its end point.
    struct Run {
        uint64_t tag = kNoRun;  // the next segment's Tag() in the run's cell, if it can join
        int idx = 0;            // the run's key's index in its row
        uint64_t key = 0;
    };

    // The indices in their rows of a segment's tiles at its top and bottom points, or -1.
    struct EndTiles {
        int top = -1;
        int bottom = -1;
    };

    // A segment inside one tile: the tile is at both its start and its end point.
    SK_ALWAYS_INLINE void mergeSingle(const Tile& tile, Run* run) {
        const uint32_t lineIdx = tile.lineIdx();
        const uint32_t cell = Cell(tile.x, tile.y);
        Row& row = fRows.data()[tile.y];
        // No branches: whether a segment joins depends on the geometry, so they would mispredict.
        const bool merge = run->tag == Tag(lineIdx, cell);
        const uint64_t key =
                merge ? GroupedTileKey::Append(run->key, tile.intersectionMask()) : RowKey(tile);
        const int idx = merge ? run->idx : row.fSize;
        row.setOrAppend(idx, key);
        run->tag = GroupedTileKey::CanAppend(key) ? Tag(lineIdx + 1, cell) : kNoRun;
        run->idx = idx;
        run->key = key;
    }

    // After a segment across several tiles, given where its top and bottom tiles went and their
    // cells: its tile at its start point (its top if it runs down) joins the run, and its tile at
    // its end point opens the next one.
    SK_ALWAYS_INLINE void mergeEnds(const EndTiles& ends,
                                    uint32_t lineIdx,
                                    bool down,
                                    uint32_t topCell,
                                    uint32_t bottomCell,
                                    Run* run) {
        const int startIdx = down ? ends.top : ends.bottom;
        const uint32_t startCell = down ? topCell : bottomCell;
        int endIdx = down ? ends.bottom : ends.top;
        const uint32_t endCell = down ? bottomCell : topCell;
        if (startIdx >= 0 && run->tag == Tag(lineIdx, startCell)) {
            Row& startRow = fRows.data()[startCell >> 16];
            SkASSERT(startIdx < startRow.fSize && run->idx >= 0 && run->idx < startRow.fSize);
            uint64_t* keys = startRow.fKeys.data();
            const uint32_t mask = GroupedTileKey::Packed(keys[startIdx]) & INTERSECTION_MASK;
            keys[run->idx] = GroupedTileKey::Append(run->key, mask);
            // Drop the start tile's key; the row's last key moves into its place. That's not the
            // run's key, which is older, but it may be the end tile's.
            const int last = startRow.fSize - 1;
            startRow.removeShuffle(startIdx);
            const bool endMoved = (endCell >> 16) == (startCell >> 16) && endIdx == last;
            endIdx = endMoved ? startIdx : endIdx;
        }
        if (endIdx >= 0) {
            Row& endRow = fRows.data()[endCell >> 16];
            SkASSERT(endIdx < endRow.fSize);
            run->tag = Tag(lineIdx + 1, endCell);  // a run of one segment can take more
            run->idx = endIdx;
            run->key = endRow.fKeys.data()[endIdx];
        }
        // Else the run's tag names this segment or an earlier one, which no later one matches.
    }

    static SK_ALWAYS_INLINE uint16_t DivCeil(uint16_t a, uint16_t b) {
        return (a + b - 1) / b;
    }

    static SK_ALWAYS_INLINE uint16_t f32ToU16Sat(float v) {
        // std::clamp will catch +/- inf here, but not NaN. However we should never get NaN here.
        SkASSERT(!std::isnan(v));
        return static_cast<uint16_t>(std::clamp(v, 0.0f, 65535.0f));
    }

    static SK_ALWAYS_INLINE int32_t f32ToI32Sat(float v) {
        // std::clamp will catch +/- inf here, but not NaN. However we should never get NaN here.
        // Clamping to [-65536.0f, 65536.0f] avoids float-to-int32_t overflow UB while staying well
        // outside the valid [0, 65535] uint16_t tile grid range.
        SkASSERT(!std::isnan(v));
        return static_cast<int32_t>(std::clamp(v, -65536.0f, 65536.0f));
    }

    template <bool kXDir>
    SK_ALWAYS_INLINE void pushEdge(const LineContext& ctx,
                                   uint16_t xIdx,
                                   uint16_t y,
                                   float rowTopX,
                                   float rowBottomX,
                                   int32_t canonicalStart,
                                   uint16_t canonicalEnd,
                                   uint32_t windingInput,
                                   bool checkStart,
                                   bool checkEnd,
                                   EndTiles* ends) {
        // Determine whether this tile is the true start or end of the line within this horizontal
        // row. We need to account for clamping because the line may start or end off-screen (e.g.,
        // X = -5), but we clamp X to the viewport.
        uint32_t uncRowStart = static_cast<uint32_t>(static_cast<int32_t>(xIdx) == canonicalStart);
        uint32_t uncRowEnd = static_cast<uint32_t>(xIdx == canonicalEnd);

        // Relativize the start/end based on line direction
        uint32_t canonicalRowStart = kXDir ? uncRowStart : uncRowEnd;
        uint32_t canonicalRowEnd = kXDir ? uncRowEnd : uncRowStart;

        // Mask out the Top/Bottom bits if this tile contains the line endpoints.
        uint32_t notStartTile = 1;
        if (checkStart) {
            notStartTile ^= static_cast<uint32_t>((static_cast<int32_t>(xIdx) == ctx.p0TileX) &&
                                                  (static_cast<int32_t>(y) == ctx.p0TileY));
        }

        uint32_t notEndTile = 1;
        if (checkEnd) {
            notEndTile ^= static_cast<uint32_t>((static_cast<int32_t>(xIdx) == ctx.p1TileX) &&
                                                (static_cast<int32_t>(y) == ctx.p1TileY));
        }

        uint32_t mask = windingInput;
        // If this tile is the start of the row, the line must have entered through the Top edge.
        // (Unless it's the line start).
        mask |= canonicalRowStart & notStartTile;
        // If this tile is the end of the row, the line must have exited through the Bottom edge.
        // (Unless it's the line end).
        mask |= (canonicalRowEnd & notEndTile) << BOT_SHIFT;

        // If a tile is NOT the start of the row, it must have been entered horizontally. If it is
        // NOT the end, it must have exited horizontally. Base L/R on the direction of the line.
        if constexpr (kXDir) {
            mask |= (1 ^ canonicalRowStart) << LEFT_SHIFT;
            mask |= (1 ^ canonicalRowEnd) << RIGHT_SHIFT;
        } else {
            mask |= (1 ^ canonicalRowStart) << RIGHT_SHIFT;
            mask |= (1 ^ canonicalRowEnd) << LEFT_SHIFT;
        }

        // Corner handling
        float xLeftF = static_cast<float>(xIdx);
        float xRightF = static_cast<float>(xIdx + 1);
        uint32_t trc = static_cast<uint32_t>(rowTopX == xRightF) & notStartTile;
        uint32_t tlc = static_cast<uint32_t>(rowTopX == xLeftF) & notStartTile;
        uint32_t brc = static_cast<uint32_t>(rowBottomX == xRightF) & notEndTile;
        uint32_t blc = static_cast<uint32_t>(rowBottomX == xLeftF) & notEndTile;

        // If the line hits the exact Top-Left corner, but it is NOT the canonical start of the row,
        // we must treat it as a Left intersection to properly bridge the mask to the adjacent tile.
        uint32_t tieBreak = tlc & (canonicalRowStart ^ 1);

        // Force corners into into purely horizontal intersections. This makes the downstream
        // intersection calculation logic simpler.
        mask |= (tieBreak | blc) << LEFT_SHIFT;
        mask |= (trc | brc) << RIGHT_SHIFT;
        mask &= ~(tieBreak | trc);
        mask &= ~((blc | brc) << BOT_SHIFT);

        const Tile tile(xIdx, y, ctx.lineIdx, mask);
        if constexpr (kGrouped) {
            // Only the top and bottom rows (checkStart, checkEnd) hold the line's tiles at its top
            // and bottom points. Note where they go for mergeEnds(), without branches.
            const int idx = this->emit(tile);
            if (checkStart) {
                ends->top = notStartTile == 0 ? idx : ends->top;
            }
            if (checkEnd) {
                ends->bottom = notEndTile == 0 ? idx : ends->bottom;
            }
        } else {
            this->emit(tile);
        }
    }

    template <bool kXDir, bool kCrossesEdge>
    SK_ALWAYS_INLINE void processRow(const LineContext& ctx,
                                     uint16_t yIdx,
                                     float rowTopX,
                                     float rowBottomX,
                                     uint32_t wMask,
                                     bool checkStart,
                                     bool checkEnd,
                                     WindingHistogram* histogram,
                                     EndTiles* ends) {
        float lx, rx;
        uint16_t xEndVal;

        if constexpr (kCrossesEdge) {
            // Clamp the row's crossings to the extent of the line, since the computed crossings
            // may drift slightly past it.
            lx = std::clamp(std::fmin(rowTopX, rowBottomX), ctx.lineLeftX, ctx.lineRightX);
            rx = std::clamp(std::fmax(rowTopX, rowBottomX), ctx.lineLeftX, ctx.lineRightX);

            // When the line crosses the clip's left boundary, the portion of the segment left of
            // the boundary contributes winding but no visible tiles. If the entire segment is left
            // of clip, early-out. Otherwise, clear the winding flag from interior tiles so winding
            // is not double-counted.
            //
            // Note: This tests the clamped crossing at the top of the row rather than `rowTopX`.
            // Otherwise, a line ending exactly on the clip's left edge could drift left of it, and
            // move its winding into the histogram even though:
            //  - It does not cross the left edge, so no culling event is reported for it, and the
            //    histogram is ignored, dropping its winding from the rest of the row.
            //  - Its leftmost tile, classified from `lx` below, still enters through the top (T)
            //    rather than the left (L), so honoring the histogram would count it twice.
            const float rowTopCrossX = kXDir ? lx : rx;
            if (rowTopCrossX < ctx.clipLeftTile) {
                if (wMask & W) {
                    histogram->addWinding(yIdx, ctx.windingDir);
                }

                if (rx < ctx.clipLeftTile) {
                    return;
                } else {
                    wMask &= ~W;
                }
            }

            xEndVal = std::min(f32ToU16Sat(rx),
                               static_cast<uint16_t>(ctx.maxTileX > 0 ? ctx.maxTileX - 1 : 0));
        } else {
            lx = std::fmin(rowTopX, rowBottomX);
            rx = std::fmax(rowTopX, rowBottomX);
            xEndVal = std::min(f32ToU16Sat(rx),
                               static_cast<uint16_t>(ctx.maxTileX > 0 ? ctx.maxTileX - 1 : 0));
        }

        // Convert floating-point boundaries into discrete integer tile indices. Note:
        // `canonicalXStart` preserves the true start (even if negative) before clamping, which is
        // required by `pushEdge` to tie-break in some cases.
        int32_t canonicalXStart = f32ToI32Sat(std::floor(lx));
        uint16_t canonicalXEnd = f32ToU16Sat(rx);
        uint16_t xStart = std::max(ctx.minTileX, f32ToU16Sat(lx));

        if (xStart <= xEndVal) {
            // Process the Leftmost Tile of the row. If this is the *only* tile in the row,
            // `isSingle`, it is both the row start and row end so the Winding (W) bit is passed
            // regardless of direction.
            bool isSingle = xStart == xEndVal;
            uint32_t wLeft = (kXDir || isSingle ? W : 0) & wMask;
            pushEdge<kXDir>(ctx,
                            xStart,
                            yIdx,
                            rowTopX,
                            rowBottomX,
                            canonicalXStart,
                            canonicalXEnd,
                            wLeft,
                            checkStart,
                            checkEnd,
                            ends);

            // Process all captive "Middle" tiles in this row. These tiles never have vertical
            // crossings, and for the purpose of the intersection mask, are identical as they always
            // recieve [R | L]. Bulk append with `inner count.`
            int innerCount = static_cast<int>(xEndVal) - static_cast<int>(xStart) - 1;
            if (innerCount > 0) {
                uint32_t innerMask = R | L;
                if constexpr (kRows) {
                    uint64_t* outKeys = fRows.data()[yIdx].append(innerCount);
                    for (int i = 0; i < innerCount; ++i) {
                        outKeys[i] = RowKey(Tile(xStart + 1 + i, yIdx, ctx.lineIdx, innerMask));
                    }
                } else {
                    Tile* outTiles = fTileBuf.append(innerCount);
                    for (int i = 0; i < innerCount; ++i) {
                        outTiles[i] = Tile(xStart + 1 + i, yIdx, ctx.lineIdx, innerMask);
                    }
                }
            }

            // Process the Rightmost Tile of the row. Emitted only if the row spans more than one
            // tile (i.e., we haven't already processed this exact tile as the Leftmost Tile).
            if (xStart < xEndVal) {
                uint32_t wRight = (kXDir ? 0 : W) & wMask;
                pushEdge<kXDir>(ctx,
                                xEndVal,
                                yIdx,
                                rowTopX,
                                rowBottomX,
                                canonicalXStart,
                                canonicalXEnd,
                                wRight,
                                checkStart,
                                checkEnd,
                                ends);
            }
        }
    }

    template <bool kXDir, bool kCrossesEdge>
    SK_ALWAYS_INLINE void runLoops(const LineContext& ctx,
                                   float lineTopY,
                                   float lineBottomY,
                                   uint16_t yTopTiles,
                                   uint16_t maxTileY,
                                   WindingHistogram* histogram,
                                   EndTiles* ends) {
        // Process the Top Row (if visible on screen)
        uint16_t yStart = yTopTiles;
        bool isStartCulled = (lineTopY < ctx.clipTopTile);
        if (!isStartCulled) {
            float y = static_cast<float>(yStart);
            float rowBottomY = std::min(y + 1.0f, lineBottomY);
            // Catch perfectly horizontal lines and/or prevent floating point drift
            float rowBottomX = (rowBottomY == ctx.bottomY)
                                       ? ctx.bottomX
                                       : ctx.topX + (rowBottomY - ctx.topY) * ctx.xSlope;
            uint32_t mask = y >= lineTopY ? W : 0;
            // The top row might ALSO be the bottom row, so checkEnd = true
            processRow<kXDir, kCrossesEdge>(ctx,
                                            yStart,
                                            ctx.topX,
                                            rowBottomX,
                                            mask,
                                            /*checkStart=*/true,
                                            /*checkEnd=*/true,
                                            histogram,
                                            ends);
            yStart++;
        }

        // Process all "Middle" fully crossed rows; the tiles cannot be the start or the end
        int32_t yEndIdx = std::min(ctx.p1TileY, static_cast<int32_t>(maxTileY));
        for (int32_t yIdx = yStart; yIdx < yEndIdx; ++yIdx) {
            float y = static_cast<float>(yIdx);
            // Although this seems like duplicate calculation, finding the intersections
            // independently allows the entire loop to auto-vectorize, and is well worth it.
            float rowTopX = ctx.topX + (y - ctx.topY) * ctx.xSlope;
            float rowBottomX = ctx.topX + (y + 1.0f - ctx.topY) * ctx.xSlope;
            processRow<kXDir, kCrossesEdge>(ctx,
                                            yIdx,
                                            rowTopX,
                                            rowBottomX,
                                            0xffffffff,
                                            /*checkStart=*/false,
                                            /*checkEnd=*/false,
                                            histogram,
                                            ends);
        }

        // Process the Terminal Row, if it exists. I.e. if it's on-screen AND wasn't already
        // processed as the Top Row.
        if (ctx.p1TileY >= yStart && ctx.p1TileY < static_cast<int32_t>(maxTileY)) {
            float y = static_cast<float>(ctx.p1TileY);
            // No guard is necessary here against horizontal lines, as a horizontal line would
            // have been processed as a starting row.
            float rowTopX = ctx.topX + (y - ctx.topY) * ctx.xSlope;
            // No need to check start (we are past it), but must check end.
            processRow<kXDir, kCrossesEdge>(ctx,
                                            ctx.p1TileY,
                                            rowTopX,
                                            ctx.bottomX,
                                            0xffffffff,
                                            /*checkStart=*/false,
                                            /*checkEnd=*/true,
                                            histogram,
                                            ends);
        }
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_Tiler_DEFINED
