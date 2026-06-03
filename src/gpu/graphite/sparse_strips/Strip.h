/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_Strip_DEFINED
#define skgpu_graphite_sparse_strips_Strip_DEFINED

#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"

namespace skgpu::graphite {

// A difference encoded representation of a path's alpha coverage.
//
// A Strip defines the start coordinate (x, y) and indexing details for a run of rasterized alpha
// tiles, along with a packed flag indicating whether the renderer should solid-fill any succeeding
// gap.
//
// See MakeStrips.h for a detailed breakdown of coverage resolution, difference encoding, and row
// layout/rendering examples.

// TODO (thomsmit): should probably move into SparseStripTypes?
struct Strip : public IntersectionBits {
    static constexpr uint32_t FILL_SHIFT = 31;

    static constexpr int   kNumSubSamples     = 8;
    static constexpr int   kLutMaskWidth      = 64;
    static constexpr int   kLutMaskHeight     = 64;
    static constexpr int   kLutMaskWidthExcl  = kLutMaskWidth - 1;
    static constexpr int   kLutMaskHeightExcl = kLutMaskHeight - 1;
    static constexpr float kLutMaskWidthF     = static_cast<float>(kLutMaskWidth);
    static constexpr float kLutMaskHeightF    = static_cast<float>(kLutMaskHeight);

    static constexpr float kStripEpsilon = 1e-5f;

    // Should be a power of two
    static_assert(!((kNumSubSamples & (kNumSubSamples - 1))));

    Strip() = default;
    Strip(uint16_t x, uint16_t y, uint32_t alphaIdx, bool shouldFill)
            : x(x)
            , y(y)
            , fFillAlphaIdx(alphaIdx | (static_cast<uint32_t>(shouldFill) << FILL_SHIFT)) {}

    uint32_t alphaIndex() const {
        return fFillAlphaIdx & ~(1u << FILL_SHIFT);
    }

    bool shouldFill() const {
        return static_cast<int32_t>(fFillAlphaIdx) < 0;
    }

    uint16_t x;
    uint16_t y;
    uint32_t fFillAlphaIdx;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_Strip_DEFINED
