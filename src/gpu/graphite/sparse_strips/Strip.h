/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_Strip_DEFINED
#define skgpu_graphite_sparse_strips_Strip_DEFINED

#include "include/core/SkPoint.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace skgpu::graphite {

/**
 * At this point in the sparse strips pipeline, the path has been stroked, flattened into a
 * polyline, tiled, and sorted. Now, the tiles are consumed by `makeStrips*` to produce "strips,"
 * a run-length encoded representation of the alpha of a path. Two things are required here:
 *
 *  1) Coverage Resolution: Multiple line segments often intersect the exact same spatial tile.
 *     Because the input tiles are generated per-line-segment, the individual winding contributions
 *     must be combined to produce the final coverage mask for that location:
 *
 *     Line 1 (\)       Line 2 (/)        Combined Mask (V)
 *     +----------+     +----------+         +----------+
 *     | \        |     |        / |         | \      / |
 *     |  \       |  +  |       /  |    =    |  \    /  |
 *     |███\      |     |      /███|         |███\  /███|
 *     |████\     |     |     /████|         |████\/████|
 *     +----------+     +----------+         +----------+
 *
 *  2) Strip Generation (Run-Length Encoding): Because the incoming tiles are now sorted by y, then
 *     x, the difference in coordinates between consecutive tiles is used to identify contiguous
 *     interior regions which can be rendered trivially without calculating per-pixel coverage, and
 *     do not require storing a full coverage mask (as they have a solid fill).
 *
 *     Tile Grid X:
 *     0           1           2           3           4           5
 *     +-----------+-----------+-----------+-----------+-----------+-----------+
 *     |           |     /     |███████████|███████████|     \     |           |
 *     |  Outside  |   /       |██ Solid ██|██ Solid ██|       \   |  Outside  |
 *     | (No Fill) | /  Edge   |██ Fill  ██|██ Fill  ██|  Edge   \ | (No Fill) |
 *     |           |    Tile   |███████████|███████████|  Tile     |           |
 *     +-----------+-----------+-----------+-----------+-----------+-----------+
 *                             ^                       ^
 *                             | <-- Implicit Gap -->  |
 *
 * A strip consists of three primary components:
 *  1) Bottom-Left Coordinates (x, y): the location at the bottom left corner, of the left-most tile,
 *     in a run of alpha tiles.
 *
 *  2) Alpha Index (alphaIdx): The offset into the buffer that stores the final per-pixel coverage
 *     values. This is always a multiple of the tile size. The difference in alpha index between two
 *     successive strips is used to determine the number of alpha tiles prior to a possible fill.
 *
 *  3) Fill State (shouldFill): A boolean flag packed into the highest bit of the `alphaIdx`. It
 *     dictates whether the empty space between *this* strip and the *next* strip is inside the
 *     shape. If true, the renderer solid-fills the implicit gap starting from the right edge of
 *     this strip's final alpha tile up to the left edge of the next strip's starting tile.
 *
 * -------------------------------------------------------------------------------------------------
 * Example Row : Single-Tile Edges (16 bytes each)
 * -------------------------------------------------------------------------------------------------
 *
 *   0           1           2           3           4           5
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *   |           |     /     |███████████|███████████|     \     |           |
 *   |  Outside  |   /       |██ Solid ██|██ Solid ██|       \   |  Outside  |
 *   | (No Fill) | /  Alpha  |██ Fill  ██|██ Fill  ██|  Alpha  \ | (No Fill) |
 *   |           |    1 Tile |███████████|███████████|    1 Tile |           |
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *               ^                                   ^
 *               |                                   |
 *               [Strip A]                           [Strip B]
 *               x: 1                                x: 4
 *               alphaIdx: 0                         alphaIdx: 16
 *               shouldFill: 1 (true)                shouldFill: 0 (false)
 *
 * -------------------------------------------------------------------------------------------------
 * Example Row: Boundary-Crossing Edge (Left edge spans 2 tiles / 32 bytes)
 * -------------------------------------------------------------------------------------------------
 *
 *   0           1           2           3           4           5
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *   |           |           |  /        |███████████|███████████|     \     |
 *   |  Outside  |           |/          |██ Solid ██|██ Solid ██|       \   |
 *   | (No Fill) |  Alpha  / |           |██ Fill  ██|██ Fill  ██|  Alpha  \ |
 *   |           |       /   |  2 Tiles  |███████████|███████████|    1 Tile |
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *               ^                                               ^
 *               |                                               |
 *               [Strip C]                                       [Strip D]
 *               x: 1                                            x: 5
 *               alphaIdx: 32                                    alphaIdx: 64
 *               shouldFill: 1 (true)                            shouldFill: 0 (false)
 */
struct Strip : public IntersectionBits {
    static constexpr uint32_t FILL_SHIFT = 31;

    static constexpr int kNumSubSamples  = 8;
    static constexpr float kPackingScale = 0.5f;
    static constexpr int kMaskWidth      = 64;
    static constexpr int kMaskHeight     = 64;
    static constexpr float kMaskWidthF   = static_cast<float>(kMaskWidth);
    static constexpr float kMaskHeightF  = static_cast<float>(kMaskHeight);
    static constexpr int kMaskWidthExcl  = kMaskWidth - 1;
    static constexpr int kMaskHeightExcl = kMaskHeight - 1;

    static constexpr float kStripEpsilon = 1e-5f;

    Strip() = default;
    Strip(uint16_t x, uint16_t y, uint32_t alphaIdx, bool shouldFill)
            : x(x)
            , y(y)
            , fFillAlphaIdx(alphaIdx | (static_cast<uint32_t>(shouldFill) << FILL_SHIFT)) {}

    uint16_t x;
    uint16_t y;
    uint32_t fFillAlphaIdx;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_Strip_DEFINED
