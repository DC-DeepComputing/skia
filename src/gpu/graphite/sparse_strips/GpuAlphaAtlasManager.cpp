/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/sparse_strips/GpuAlphaAtlasManager.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu::graphite {

GpuAlphaAtlasManager::GpuAlphaAtlasManager(Recorder* recorder) : fRecorder(recorder) {}

void GpuAlphaAtlasManager::advanceAlphaIndex(int32_t alphaCount) {
    fAlphaIndex += alphaCount;

    // Calculate how many textures we need to cover the current fAlphaIndex based on byte capacity.
    // If we are at index 0 (e.g., beginning of recording), we ensure at least 1
    // texture exists so the shader always has a valid binding.
    int32_t requiredTextures = fAlphaIndex == 0 ? 1 : (fAlphaIndex + kMaxCapBytes - 1) / kMaxCapBytes;

    while (fTextures.size() < requiredTextures) {
        bool success = this->initializeTexture();
        SkASSERT(success);
    }
}

bool GpuAlphaAtlasManager::initializeTexture() {
    const Caps* caps = fRecorder->priv().caps();

    // We use getDefaultStorageTextureInfo here because the compute shader will
    // write directly into this texture. We strictly use kColorType (RGBA 8888)
    // to align with the byte-wise overflow management.
    TextureInfo info = caps->getDefaultStorageTextureInfo(kColorType);

    sk_sp<TextureProxy> proxy = TextureProxy::Make(caps,
                                                   fRecorder->priv().resourceProvider(),
                                                   SkISize::Make(kAtlasTextureDims, kAtlasTextureDims),
                                                   info,
                                                   "GpuAlphaAtlas",
                                                   Budgeted::kYes);

    if (proxy) {
        fTextures.push_back(std::move(proxy));
        return true;
    }
    return false;
}

void GpuAlphaAtlasManager::postFlush() {
    if (fTextures.empty()) {
        fAlphaIndex = 0;
        return;
    }

    fAlphaIndex = fAlphaIndex % kMaxCapBytes;

    if (fAlphaIndex > 0) {
        // We have leftover space on the last page. Keep it as page 0 for the next batch.
        sk_sp<TextureProxy> activeTexture = fTextures.back();
        fTextures.clear();
        fTextures.push_back(std::move(activeTexture));
    } else {
        // The pages were perfectly filled. Clean slate.
        fTextures.clear();
    }
}

void GpuAlphaAtlasManager::freeGpuResources() {
    fTextures.clear();
    fAlphaIndex = 0;
}

}  // namespace skgpu::graphite
