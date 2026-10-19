/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_PreMergeTile_DEFINED
#define skgpu_graphite_sparse_strips_PreMergeTile_DEFINED

#include "include/core/SkSpan.h"
#include "include/private/SkAssert.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace skgpu::graphite {

/*
 * When sparse strips coverage is rasterized on the GPU (MakeStrips::MsaaGpu), the CPU still tiles
 * the polyline and traverses the sorted tiles to generate the EndCaps and WideTiles, but each
 * `Tile`, i.e. a single line's intersection with a single spatial tile, is packaged into a
 * PreMergeTile (PMT) instead of being rasterized.
 *
 * The PMTs of a spatial tile are contiguous, from its first PMT (S) to its last PMT (E). At flush,
 * each spatial tile becomes a TileWork, and the coverage compute shader (SparseStripsCoverageStep)
 * rasterizes all of the tile's PMTs on top of its backdrop, and stores the merged coverage at the
 * end tile's `fAlphaIndex` in the alpha atlas, using the same layout as the CPU path.
 *
 * The CPU still carries the coarse winding across each row, so the first PMT of each spatial tile
 * also carries the tile's backdrop, the coarse winding entering the tile.
 *
 * MUST EXACTLY MATCH how the coverage compute shader reads PreMergeTiles.
 */
struct PreMergeTile {
    // Only meaningful on end tiles (kEndTile): the byte offset of the spatial tile's alphas within
    // its atlas page, in the same units and layout as EndCaps::EndCap::fAlphaIndex.
    uint32_t fAlphaIndex;

    /// The parent Tile's line index and intersection mask, copied verbatim from
    /// Tile::fPackedLineIdxIntersectionMask.
    /// MSB                                                            LSB
    /// 31------------------------------------------------------5|4|3|2|1|0|
    /// |            Parent Line Index (27 bits)                 |W|R|L|B|T|
    uint32_t fPackedLineIdxIntersectionMask;

    /// Contains the Scanned Winding, and Flags.
    /// MSB                                                     LSB
    /// 31----------------------16|15--------8|7-------4|3|2|1|0|
    /// |         Unused          | Scan Wind |  Unused |S|I|F|E|
    ///
    /// - Scan Wind: i8, the coarse winding before this PMT (an exclusive prefix sum). For the
    ///   first PMT of a spatial tile, this is the tile's backdrop.
    /// - S: First PMT of its spatial tile.
    /// - I: Inverse fill.
    /// - F: Even-odd fill.
    /// - E: Last PMT of its spatial tile.
    uint32_t fPackedScannedWinding;

    /// The parent Tile's coordinates, in tiles.
    /// MSB                  LSB
    /// 31-------16|15--------0|
    /// |  Tile Y  |  Tile X   |
    uint32_t fPackedXY;

    // The parent line, in device space.
    Line fLine;

    static constexpr uint32_t kEndTile   = 1 << 0;
    static constexpr uint32_t kEvenOdd   = 1 << 1;
    static constexpr uint32_t kInverse   = 1 << 2;
    static constexpr uint32_t kTileStart = 1 << 3;
    static constexpr uint32_t kFlagsMask = kEndTile | kEvenOdd | kInverse | kTileStart;

    static constexpr uint32_t kScannedWindingShift = 8;

    static PreMergeTile Make(const Tile& tile,
                             const Line& line,
                             int32_t scannedWinding,
                             uint32_t flags) {
        SkASSERT((flags & ~kFlagsMask) == 0);
        // Truncating to an i8 wraps exactly like the CPU processors' SWAR subsample winding, and
        // preserves the winding's parity for even-odd fills.
        const uint32_t packedWinding = static_cast<uint32_t>(static_cast<uint8_t>(scannedWinding));
        return {/*fAlphaIndex=*/0,
                tile.fPackedLineIdxIntersectionMask,
                (packedWinding << kScannedWindingShift) | flags,
                (static_cast<uint32_t>(tile.y) << 16) | tile.x,
                line};
    }

    bool isTileStart() const { return (fPackedScannedWinding & kTileStart) != 0; }
    bool isEndTile() const { return (fPackedScannedWinding & kEndTile) != 0; }

    int8_t scannedWinding() const {
        return static_cast<int8_t>((fPackedScannedWinding >> kScannedWindingShift) & 0xFF);
    }
};
static_assert(sizeof(PreMergeTile) == 32);
// The shader reads a PreMergeTile as two uint4s, the second being fLine.
static_assert(offsetof(PreMergeTile, fLine) == 16);
static_assert(std::is_trivially_copyable_v<PreMergeTile>);

/*
 * A spatial tile for the coverage compute shader: the PreMergeTiles in [fPmtStart, fPmtStart +
 * count) are rasterized on top of the tile's backdrop, and the merged coverage is stored at the
 * tile's alpha index. A TileWork with a count of 0 is padding, and stores nothing.
 *
 * MUST EXACTLY MATCH how the coverage compute shader reads TileWorks.
 */
struct TileWork {
    uint32_t fPmtStart;
    // [23:0] The number of PreMergeTiles. [31:24] The backdrop, an i8.
    uint32_t fCountBackdrop;
    // The byte offset of the tile's alphas within its atlas page, which is 64-aligned, so the fill
    // rule is in the low bits.
    uint32_t fAlphaFlags;
    // The tile's coordinates, in tiles, like PreMergeTile::fPackedXY.
    uint32_t fPackedXY;

    static constexpr uint32_t kMaxCount     = (1 << 24) - 1;
    static constexpr uint32_t kBackdropShift = 24;
    static constexpr uint32_t kEvenOdd      = 1 << 0;
    static constexpr uint32_t kInverse      = 1 << 1;
};
static_assert(sizeof(TileWork) == 16);

// Returns the number of spatial tiles of `pmts`, a page's batch of PreMergeTiles.
inline uint32_t CountTiles(SkSpan<const PreMergeTile> pmts) {
    uint32_t count = 0;
    for (const PreMergeTile& pmt : pmts) {
        count += pmt.isEndTile() ? 1 : 0;
    }
    return count;
}

// Calls `fn(const TileWork&)` for each spatial tile of `pmts`, in order.
template <typename Fn>
void ForEachTileWork(SkSpan<const PreMergeTile> pmts, Fn&& fn) {
    SkASSERT(pmts.empty() || pmts[0].isTileStart());
    uint32_t tileStart = 0;
    for (uint32_t i = 0; i < pmts.size(); ++i) {
        const PreMergeTile& pmt = pmts[i];
        if (pmt.isTileStart()) {
            tileStart = i;
        }
        if (pmt.isEndTile()) {
            const PreMergeTile& first = pmts[tileStart];
            const uint32_t count = i - tileStart + 1;
            SkASSERT(count <= TileWork::kMaxCount);
            SkASSERT((pmt.fAlphaIndex & 63) == 0);
            const uint32_t backdrop = static_cast<uint8_t>(first.scannedWinding());
            const uint32_t flags =
                    ((first.fPackedScannedWinding & PreMergeTile::kEvenOdd) ? TileWork::kEvenOdd
                                                                            : 0) |
                    ((first.fPackedScannedWinding & PreMergeTile::kInverse) ? TileWork::kInverse
                                                                            : 0);
            fn(TileWork{tileStart,
                        count | (backdrop << TileWork::kBackdropShift),
                        pmt.fAlphaIndex | flags,
                        first.fPackedXY});
        }
    }
}

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_PreMergeTile_DEFINED
