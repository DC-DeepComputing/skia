/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_AlphaAtlasManager_DEFINED
#define skgpu_graphite_AlphaAtlasManager_DEFINED

#include "include/core/SkColorType.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/TextureProxy.h"

#include <cstdint>
#include <queue>
#include <vector>

namespace skgpu::graphite {

class DrawContext;
class Recorder;

class AlphaAtlasManager {
public:
    static constexpr int32_t kAtlasWidth = 4096;
    static constexpr int32_t kAtlasHeight = 4096;
    static constexpr int32_t kAtlasWidthBytes = kAtlasWidth * 4;
    static constexpr int32_t kMaxCapBytes = kAtlasWidth * kAtlasHeight * 4;
    static constexpr SkColorType kColorType = SkColorType::kRGBA_8888_SkColorType;

    AlphaAtlasManager(Recorder* recorder);

    SkTDArray<uint8_t>& getAlphaBuffer() {
        SkASSERT(!fMarked);
        SkDEBUGCODE(fMarked = true;)
        initializeTexture();
        fMarkedStart = fTextures.back().fAlphaBuffer.size();
        return fTextures.back().fAlphaBuffer;
    }

    int32_t getUploadOffset() const {
        SkASSERT(!fTextures.empty() && fTextures.back().fTexture);
        return fTextures.back().fUploadSize;
    }

    void getAssociatedProxies(SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>>& allocations);

    bool initializeTexture();
    void recordUploads(DrawContext* dc);
    void freeGpuResources();

private:
#if defined(GPU_TEST_UTILS)
    template <uint16_t, uint16_t> friend class CoverageTestRunner;
    void resetBuffer() { fTextures.back().fAlphaBuffer.clear(); }
    uint8_t getAlpha(int32_t index) const { return fTextures.back().fAlphaBuffer[index]; }
#endif

    struct TextureState {
        int32_t fUploadSize = 0;
        sk_sp<TextureProxy> fTexture;
        SkTDArray<uint8_t> fAlphaBuffer;
    };

    Recorder* fRecorder;
    int32_t fMarkedStart = 0;
    int32_t fConsumedBytes = 0;
    std::queue<TextureState> fTextures;
    SkDEBUGCODE(bool fMarked = false;)
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_AlphaAtlasManager_DEFINED
