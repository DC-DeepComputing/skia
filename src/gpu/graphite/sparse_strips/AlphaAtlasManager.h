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

    uint32_t addAlphas(const uint8_t* alphas, size_t size);

    uint32_t addAlphas(const std::vector<uint8_t>& alphas) {
        return addAlphas(alphas.data(), alphas.size());
    }

    uint32_t currentOffset() const { return fAlphaBuffer.size(); }

    bool initializeTexture();
    void recordUploads(DrawContext* dc);
    void freeGpuResources();

    sk_sp<TextureProxy> getProxy() const { return fTexture; }

private:
#if defined(GPU_TEST_UTILS)
    template<uint16_t, uint16_t> friend class CoverageTestRunner;
    void resetBuffer() { fAlphaBuffer.clear(); }
#endif

    Recorder* fRecorder;
    sk_sp<TextureProxy> fTexture;

    std::vector<uint8_t> fAlphaBuffer;
    uint32_t fCurrentOffset = 0;
    uint32_t fTotalAllocated = 0;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_AlphaAtlasManager_DEFINED
