/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_GpuAlphaAtlasManager_DEFINED
#define skgpu_graphite_GpuAlphaAtlasManager_DEFINED

#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

#include <cstdint>

namespace skgpu::graphite {

class Recorder;

class GpuAlphaAtlasManager {
public:
    static constexpr int32_t kAtlasTextureDims = SparseStripsConfig::kTextureDims;
    static constexpr int32_t kAtlasDimsBytes = kAtlasTextureDims * 4;
    static constexpr int32_t kMaxCapBytes = kAtlasTextureDims * kAtlasDimsBytes;
    static constexpr SkColorType kColorType = SparseStripsConfig::kColorType;

    GpuAlphaAtlasManager(Recorder* recorder);

    // Returns the current global alpha index.
    int32_t getAlphaIndex() const { return fAlphaIndex; }

    // Advances the internal alpha index by the requested count and instantiates
    // any necessary TextureProxies to back the new capacity.
    void advanceAlphaIndex(int32_t alphaCount);

    // Returns the list of active texture proxies for the current batch in order (0 to n).
    // The shader can bind these sequentially and index them via `alphaIdx / kMaxCapBytes`.
    const SkTDArray<sk_sp<TextureProxy>>& getProxies() const { return fTextures; }

    // Rolls over the alpha index for the next batch and retains the last texture
    // as page 0 if there is leftover space.
    void postFlush();

    void freeGpuResources();

private:
    bool initializeTexture();

    Recorder* fRecorder;
    int32_t fAlphaIndex = 0;

    // Ordered array of textures representing pages 0 to n.
    SkTDArray<sk_sp<TextureProxy>> fTextures;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_GpuAlphaAtlasManager_DEFINED
