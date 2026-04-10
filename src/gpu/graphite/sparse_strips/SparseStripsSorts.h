/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_sparse_strips_SparseStripsSorts_DEFINED
#define skgpu_graphite_sparse_strips_SparseStripsSorts_DEFINED

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkVx.h"

#include <algorithm>

namespace skgpu::graphite {

class SparseStripsSorts {
public:
    template <typename TileType>
    static SK_ALWAYS_INLINE void SortRow(SkTDArray<TileType>& row) {
        int count = row.size();
        if (count <= 1) {
            return;
        }

        if (count <= 8) {
            Sort8(row.data(), count);
        } else {
            std::stable_sort(row.begin(), row.end());
        }
    }

private:
    template <typename TileType>
    static SK_ALWAYS_INLINE void Sort8(TileType* elements, int count) {
        // We build a 32-bit key: (X << 16) | lane_idx.
        // Packing the lane_idx in the lower bits guarantees stability and prevents ties,
        // so we can use a strict '<' comparison to find the exact target rank.
        // Unused lanes get X = 0xFFFF so they safely sort to the back of the pack.
        auto getKey = [&](int i) -> uint32_t {
            if (i < count) {
                return (static_cast<uint32_t>(elements[i].x) << 16) | static_cast<uint32_t>(i);
            }
            return (0xFFFFu << 16) | static_cast<uint32_t>(i);
        };

        // FIX: Use curly braces {} to invoke the std::initializer_list constructor
        skvx::uint8 keys = {
            getKey(0), getKey(1), getKey(2), getKey(3),
            getKey(4), getKey(5), getKey(6), getKey(7)
        };

        skvx::uint8 ranks(0u);
        skvx::uint8 rot = keys;

        // rot < keys returns a mask of ~0 (0xFFFFFFFF) or 0.
        // Bitwise AND with 1 extracts exactly 1 or 0 for the rank accumulator.
        #define ROTATE_AND_RANK \
            rot = skvx::shuffle<1, 2, 3, 4, 5, 6, 7, 0>(rot); \
            ranks += (rot < keys) & 1;

        ROTATE_AND_RANK
        ROTATE_AND_RANK
        ROTATE_AND_RANK
        ROTATE_AND_RANK
        ROTATE_AND_RANK
        ROTATE_AND_RANK
        ROTATE_AND_RANK

        #undef ROTATE_AND_RANK

        // Scatter based on computed ranks.
        TileType temp[8];
        for (int i = 0; i < count; ++i) {
            temp[ranks[i]] = elements[i];
        }

        // Copy back to original array.
        for (int i = 0; i < count; ++i) {
            elements[i] = temp[i];
        }
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_SparseStripsSorts_DEFINED
