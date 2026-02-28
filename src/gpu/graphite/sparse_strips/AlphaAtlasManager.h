/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_AlphaAtlasManager_DEFINED
#define skgpu_graphite_AlphaAtlasManager_DEFINED

#include "src/gpu/graphite/TextureProxy.h"
#include <vector>
#include <cstdint>

namespace skgpu::graphite {

class DrawContext;
class Recorder;

class AlphaAtlasManager {
public:
    static constexpr int kAtlasWidth = 4096;
    static constexpr int kAtlasHeight = 4096;
    static constexpr uint32_t kFull = 0xFFFFFFFF;

    AlphaAtlasManager(Recorder* recorder);

    // 1) Registers the buffer and initializes the texture
    bool registerBuffer(const std::vector<uint8_t>* alphas);

    // 2) Track usage. Returns the global index for the shader, or kFull if we run out of space.
    uint32_t allocate(size_t size);

    void recordUploads(DrawContext* dc);
    void freeGpuResources();

    sk_sp<TextureProxy> getProxy() const { return fTexture; }

private:
    bool initializeTexture();

    Recorder* fRecorder;
    sk_sp<TextureProxy> fTexture;

    const std::vector<uint8_t>* fAlphaBuffer = nullptr;
    uint32_t fCurrentOffset = 0;
    uint32_t fTotalAllocated = 0;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_AlphaAtlasManager_DEFINED
