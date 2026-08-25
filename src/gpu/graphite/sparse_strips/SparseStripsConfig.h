/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SparseStripConfig_DEFINED
#define skgpu_graphite_SparseStripConfig_DEFINED

#include "include/core/SkColorType.h"

#include <cstdint>

namespace skgpu::graphite {

struct SparseStripConfig {
    static constexpr uint16_t kTileWidth  = 8;
    static constexpr uint16_t kTileHeight = 8;
    static_assert(kTileWidth == kTileHeight);

    // The tile dimensions MUST match the hard coded constant in the fragment shader. If that value
    // changes, this must be updated.
    static_assert(kTileWidth == 8);

    static constexpr int32_t kAtlasWidth          = 8192;
    static constexpr int32_t kAtlasBytesPerTexel  = 4;
    static constexpr int32_t kAtlasWidthBytes     = kAtlasWidth * kAtlasBytesPerTexel;
    static constexpr int32_t kInitialAtlasRows    = 1;
    static constexpr int32_t kMaxAtlasRows        = 8192;
    static constexpr int32_t kMaxCapBytes         = kAtlasWidth * kMaxAtlasRows *
                                                    kAtlasBytesPerTexel;
    static constexpr int32_t kMaxTexturePages     = 2;
    static constexpr uint16_t kInvalidTexPage     = 0xFFFF;
    static constexpr SkColorType kColorType       = SkColorType::kRGBA_8888_SkColorType;

    static constexpr int32_t   kNumSubSamples     = 8;
    static_assert(!((kNumSubSamples & (kNumSubSamples - 1))));

    static constexpr int32_t kLutMaskWidth        = 64;
    static constexpr int32_t kLutMaskHeight       = 64;
    static constexpr int32_t kLutMaskWidthExcl    = kLutMaskWidth - 1;
    static constexpr int32_t kLutMaskHeightExcl   = kLutMaskHeight - 1;
    static constexpr float   kLutMaskWidthF       = static_cast<float>(kLutMaskWidth);
    static constexpr float   kLutMaskHeightF      = static_cast<float>(kLutMaskHeight);

    static constexpr float kStripEpsilon          = 1e-5f;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_SparseStripConfig_DEFINED
