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
    SkColorType atlasColor = caps->isStorage(caps->getDefaultStorageTextureInfo(kAlpha_8_SkColorType))
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

bool AlphaAtlasManager::registerBuffer(const std::vector<uint8_t>* alphas) {
    if (!this->initializeTexture()) {
        return false;
    }
    fAlphaBuffer = alphas;
    // We don't reset fCurrentOffset here if you are registering multiple times
    // per frame, but if it's a 1:1 relationship per flush, resetting is fine.
    fCurrentOffset = 0;
    fTotalAllocated = 0;
    return true;
}

uint32_t AlphaAtlasManager::allocate(size_t size) {
    if (!fAlphaBuffer || fCurrentOffset + size > kAtlasWidth * kAtlasHeight) {
        return kFull; // Too full, skip upload
    }

    uint32_t allocatedIndex = fCurrentOffset;
    fCurrentOffset += size;
    fTotalAllocated = fCurrentOffset;

    return allocatedIndex;
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    if (!fAlphaBuffer || fTotalAllocated == 0 || !fTexture) {
        return;
    }

    const Caps* caps = fRecorder->priv().caps();
    SkColorType atlasColor = caps->isStorage(caps->getDefaultStorageTextureInfo(kAlpha_8_SkColorType))
            ? kAlpha_8_SkColorType : kRGBA_8888_SkColorType;

    SkColorInfo srcColorInfo(kAlpha_8_SkColorType, kPremul_SkAlphaType, nullptr);
    SkColorInfo dstColorInfo(atlasColor, kPremul_SkAlphaType, nullptr);

    int height = (fTotalAllocated + kAtlasWidth - 1) / kAtlasWidth;

    // SAFETY CHECK: Ensure StripGenerator padded the buffer so we don't segfault on the last row
    SkASSERT(fAlphaBuffer->size() >= (size_t)(height * kAtlasWidth));

    SkIRect dstRect = SkIRect::MakeWH(kAtlasWidth, height);

    MipLevel level;
    level.fPixels = fAlphaBuffer->data();
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
}

void AlphaAtlasManager::freeGpuResources() {
    fTexture.reset();
    fAlphaBuffer = nullptr;
}

}  // namespace skgpu::graphite
