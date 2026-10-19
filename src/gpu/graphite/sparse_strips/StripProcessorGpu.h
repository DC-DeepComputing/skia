/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripProcessorGpu_DEFINED
#define skgpu_graphite_sparse_strips_StripProcessorGpu_DEFINED

#include "include/core/SkPoint.h"
#include "src/gpu/graphite/sparse_strips/GpuAlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/PreMergeTile.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <array>
#include <cstdint>

namespace skgpu::graphite {

// The CPU half of GPU sparse strips coverage. It plugs into MakeStrips::TraverseCPU exactly like
// StripProcessorScalar and StripProcessorSimd, so the EndCaps and WideTiles it produces are
// identical. But instead of rasterizing each tile's subsample winding, it packages each `Tile` into
// a PreMergeTile, which the coverage compute shader rasterizes and merges into the alpha atlas.
//
// Only the coarse winding is still tracked on the CPU, because it drives WideTile generation and
// provides the backdrop of each spatial tile.
template <uint16_t kTileWidth, uint16_t kTileHeight, bool kIsWinding>
class StripProcessorGpu {
public:
    StripProcessorGpu(bool isInverse, const Polyline& polyline, GpuAlphaAtlasManager* atlasManager)
            : fCoarseWinding(0)
            , fFlags((kIsWinding ? 0 : PreMergeTile::kEvenOdd) |
                     (isInverse ? PreMergeTile::kInverse : 0))
            , fTileStartPending(true)
            , fPolyline(polyline)
            , fAtlasManager(atlasManager) {}

    SK_ALWAYS_INLINE static bool ShouldFill(int32_t w) {
        if constexpr (kIsWinding) {
            return w != 0;
        } else {
            return (w & 1) != 0;
        }
    }

    SK_ALWAYS_INLINE int32_t coarseWinding() const { return fCoarseWinding; }
    SK_ALWAYS_INLINE void setCoarseWinding(int32_t val) { fCoarseWinding = val; }

    // TraverseCPU calls this whenever a new spatial tile begins, which is where the CPU processors
    // seed the tile's subsample winding with the coarse winding. Here, the next PreMergeTile is
    // flagged as the tile's first instead, and carries the coarse winding to the shader as the
    // tile's backdrop.
    SK_ALWAYS_INLINE void clearWithCoarseWinding() { fTileStartPending = true; }

    SK_ALWAYS_INLINE void rasterizeLineToTile(const Tile& tile,
                                              std::array<SkPoint, 2> /*tileBounds*/) {
        const Line line = fPolyline.getLine(tile.lineIdx());
        const uint32_t flags = fFlags | (fTileStartPending ? PreMergeTile::kTileStart : 0);
        *fAtlasManager->stagePmt() = PreMergeTile::Make(tile, line, fCoarseWinding, flags);
        fTileStartPending = false;

        tile.accumulateCoarseWinding<kIsWinding>(/*canonicalYDir=*/line.p1.fY >= line.p0.fY,
                                                 &fCoarseWinding);
    }

    // Called once all of a spatial tile's PreMergeTiles have been staged, with the byte offset of
    // the tile's alphas from the start of its run. The tile's last PreMergeTile is flagged as its
    // end tile, which is where the shader stores the tile's merged coverage.
    SK_ALWAYS_INLINE void endTile(int32_t runOffset) {
        PreMergeTile& last = fAtlasManager->lastStagedPmt();
        // Rebased to the run's offset within its page by GpuAlphaAtlasManager::finalizeRun().
        last.fAlphaIndex = static_cast<uint32_t>(runOffset);
        last.fPackedScannedWinding |= PreMergeTile::kEndTile;
    }

private:
    int32_t fCoarseWinding;
    const uint32_t fFlags;
    bool fTileStartPending;
    const Polyline& fPolyline;
    GpuAlphaAtlasManager* fAtlasManager;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_StripProcessorGpu_DEFINED
