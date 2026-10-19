/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_SparseStripsCoverageStep_DEFINED
#define skgpu_graphite_sparse_strips_SparseStripsCoverageStep_DEFINED

#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/compute/ComputeStep.h"

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>

namespace skgpu::graphite {

class Caps;

/**
 * Rasterizes the coverage of sparse strips (PathRendererStrategy::kGPUSparseStripsMSAA8) directly
 * into an alpha atlas page. It is the GPU counterpart of StripProcessorSimd, whose rules it follows
 * exactly, and of the coverage resolve of MakeStrips::TraverseCPU.
 *
 * Each spatial tile (a TileWork) is rasterized by kLanesPerTile invocations of one subgroup, one
 * per row of the tile. They loop over the tile's PreMergeTiles, accumulate the winding of each of
 * their pixels' samples in registers, and finally store the tile's alphas at its alpha index, in the
 * same layout as the CPU path. Within a PreMergeTile, the coverage of pixels the line doesn't cross
 * only depends on the row, so each invocation computes it for its own row. The pixels the line
 * crosses need the MSAA mask LUT. A steep line crosses at most 2 pixels per row, but a shallow one
 * can cross the whole row, so for shallow lines the invocations take a column each instead, and the
 * masks are transposed between the invocations with subgroup shuffles.
 *
 * Requires subgroups of at least kLanesPerTile invocations.
 */
class SparseStripsCoverageStep final : public ComputeStep {
public:
    // The shared slots of the step's resources, which are assigned by the dispatch.
    enum Slot : int {
        kTilesSlot = 0,   // TileWork[], padded to whole workgroups with empty TileWorks.
        kPmtsSlot,        // PreMergeTile[]
        kMaskLUTSlot,     // ComputeProvider::sparseStripsMaskLUT()
        kAtlasSlot,       // The atlas page, as a write-only storage texture.
    };

    static constexpr uint32_t kLanesPerTile = 8;
    static constexpr uint32_t kTilesPerWorkgroup = 8;
    static constexpr uint32_t kWorkgroupSize = kLanesPerTile * kTilesPerWorkgroup;

    // The most workgroups that are dispatched along one dimension, WebGPU's minimum
    // maxComputeWorkgroupsPerDimension.
    static constexpr uint32_t kMaxWorkgroupsPerDimension = 65535;

    SparseStripsCoverageStep();
    ~SparseStripsCoverageStep() override;

    // Returns the workgroup count that rasterizes `tileCount` tiles. The TileWorks must be padded
    // with empty ones to DispatchedTiles() of it. Large dispatches use a second dimension.
    static WorkgroupSize DispatchSize(uint32_t tileCount);
    static uint32_t DispatchedTiles(WorkgroupSize dispatchSize) {
        return dispatchSize.fWidth * dispatchSize.fHeight * kTilesPerWorkgroup;
    }

    std::string computeSkSL() const override;

    std::tuple<SkISize, SkColorType> calculateTextureParameters(
            int resourceIndex, const ResourceDesc&) const override;

    std::optional<uint32_t> preferredSubgroupSize(const Caps&) const override;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_SparseStripsCoverageStep_DEFINED
