/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_Strip_DEFINED
#define skgpu_graphite_Strip_DEFINED

#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include <cstdint>

namespace skgpu::graphite {

struct Strip {
    Strip(uint16_t x, uint16_t y, uint32_t alphaIdx, bool shouldFill)
            : fX(x)
            , fY(y)
            , fPackedAlphaIdx(alphaIdx | (static_cast<uint32_t>(shouldFill) << SparseStripsConstants::FILL_SHIFT)) {}

    uint16_t x() const { return fX; }
    uint16_t y() const { return fY; }

    int32_t alphaIdx() const {
        return static_cast<int32_t>(fPackedAlphaIdx & ~(1u << SparseStripsConstants::FILL_SHIFT));
    }

    bool shouldFill() const {
        return static_cast<bool>(fPackedAlphaIdx & (1u << SparseStripsConstants::FILL_SHIFT));
    }

private:
    uint16_t fX;
    uint16_t fY;
    uint32_t fPackedAlphaIdx;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_Strip_DEFINED
