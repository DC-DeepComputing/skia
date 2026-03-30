/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SparseStripsConfig_DEFINED
#define skgpu_graphite_SparseStripsConfig_DEFINED

#include <cstdint>
#include <type_traits>

namespace skgpu::graphite {

// Contains the *tune-able* constants related SparseStrips generation
struct SparseStripsConfig {
    static constexpr uint16_t kTileWidth  = 8;
    static constexpr uint16_t kTileHeight = 8;
    static_assert(kTileWidth == kTileHeight);

    // Note although LUT generation supports 16xMSAA/uint16_t strip generation does not. So this
    // cant be changed currently.
    using MSAAType = uint8_t;
    static_assert(std::is_same_v<MSAAType, uint8_t>);
    static constexpr int32_t kLutWidth  = 64;
    static constexpr int32_t kLutHeight = 64;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_SparseStripsConfig_DEFINED
