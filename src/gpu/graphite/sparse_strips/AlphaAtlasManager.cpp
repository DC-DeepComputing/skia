/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/task/UploadTask.h"

namespace skgpu::graphite {

AlphaAtlasManager::AlphaAtlasManager(Recorder* recorder)
        : fRecorder(recorder) {}

bool AlphaAtlasManager::initializeTexture() {
    if (fTexture) {
        return true;
    }

    const Caps* caps = fRecorder->priv().caps();
    SkColorType atlasColor =
            caps->isStorage(caps->getDefaultStorageTextureInfo(kAlpha_8_SkColorType))
            ? kAlpha_8_SkColorType : kRGBA_8888_SkColorType;

    TextureInfo info = caps->getDefaultSampledTextureInfo(atlasColor,
                                                          Mipmapped::kNo,
                                                          fRecorder->priv().isProtected(),
                                                          Renderable::kNo);

    fTexture = TextureProxy::Make(caps,
                                  fRecorder->priv().resourceProvider(),
                                  SkISize::Make(kAtlasWidth, kAtlasHeight),
                                  info,
                                  "AlphaAtlas",
                                  Budgeted::kYes);
    return fTexture != nullptr;
}

uint32_t AlphaAtlasManager::addAlphas(const uint8_t* alphas, size_t size) {
    SkASSERT(fTexture);

    uint32_t allocatedIndex = fAlphaBuffer.size();

    if (allocatedIndex + size > kAtlasWidth * kAtlasHeight) {
        printf("warning overflow\n");
        return kFull; // Overflow guard: Too full, skip upload
    }

    fAlphaBuffer.insert(fAlphaBuffer.end(), alphas, alphas + size);

    return allocatedIndex;
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    if (fAlphaBuffer.empty()) {
        return;
    }

    if (!this->initializeTexture()) {
        return;
    }

    int height = (fAlphaBuffer.size() + kAtlasWidth - 1) / kAtlasWidth;
    int paddedSize = height * kAtlasWidth;

    if (fAlphaBuffer.size() < (size_t)paddedSize) {
        fAlphaBuffer.resize(paddedSize, 0);
    }

    const Caps* caps = fRecorder->priv().caps();
    SkColorType atlasColor = caps->isStorage(caps->getDefaultStorageTextureInfo(kAlpha_8_SkColorType))
            ? kAlpha_8_SkColorType : kRGBA_8888_SkColorType;

    SkColorInfo srcColorInfo(kAlpha_8_SkColorType, kPremul_SkAlphaType, nullptr);
    SkColorInfo dstColorInfo(atlasColor, kPremul_SkAlphaType, nullptr);

    SkIRect dstRect = SkIRect::MakeWH(kAtlasWidth, height);

    MipLevel level;
    level.fPixels = fAlphaBuffer.data();
    level.fRowBytes = kAtlasWidth;

    UploadSource source = UploadSource::Make(
            caps,
            *fTexture,
            srcColorInfo,
            dstColorInfo,
            SkSpan<const MipLevel>(&level, 1),
            dstRect);

    dc->recordUpload(fRecorder,
                     fTexture,
                     srcColorInfo,
                     dstColorInfo,
                     source,
                     dstRect,
                     nullptr);

    fAlphaBuffer.clear();
}

void AlphaAtlasManager::freeGpuResources() {
    fTexture.reset();
    fAlphaBuffer.clear();
}

}  // namespace skgpu::graphite
