/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SparseStripConfig_DEFINED
#define skgpu_graphite_SparseStripConfig_DEFINED

#include "include/core/SkColorType.h"

namespace skgpu::graphite {

struct SparseStripConfig {
    static constexpr uint16_t kTileWidth  = 8;
    static constexpr uint16_t kTileHeight = 8;
    static_assert(kTileWidth == kTileHeight);

    static constexpr int32_t kTextureDims     = 4096;
    static constexpr int32_t kMaxTexturePages = 2;
    static constexpr SkColorType kColorType   = SkColorType::kRGBA_8888_SkColorType;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_SparseStripConfig_DEFINED
