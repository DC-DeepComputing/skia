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

    static constexpr int32_t kAtlasWidth          = 8192;
    static constexpr int32_t kAtlasBytesPerTexel  = 4;
    static constexpr int32_t kAtlasWidthBytes     = kAtlasWidth * kAtlasBytesPerTexel; // 32,768 bytes per row
    static constexpr int32_t kInitialAtlasRows    = 1;
    static constexpr int32_t kMaxAtlasRows        = 8192;
    static constexpr int32_t kMaxCapBytes         = kAtlasWidth * kMaxAtlasRows * kAtlasBytesPerTexel;
    static constexpr int32_t kMaxTexturePages     = 2;
    static constexpr SkColorType kColorType       = SkColorType::kRGBA_8888_SkColorType;

    static constexpr int   kNumSubSamples     = 8;
    static constexpr int   kLutMaskWidth      = 64;
    static constexpr int   kLutMaskHeight     = 64;
    static constexpr int   kLutMaskWidthExcl  = kLutMaskWidth - 1;
    static constexpr int   kLutMaskHeightExcl = kLutMaskHeight - 1;
    static constexpr float kLutMaskWidthF     = static_cast<float>(kLutMaskWidth);
    static constexpr float kLutMaskHeightF    = static_cast<float>(kLutMaskHeight);
    static constexpr float kStripEpsilon      = 1e-5f;

    static_assert(!((kNumSubSamples & (kNumSubSamples - 1))));
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_SparseStripConfig_DEFINED
