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

AlphaAtlasManager::AlphaAtlasManager(Recorder* recorder) : fRecorder(recorder) {
    fTextures.push({});
}

bool AlphaAtlasManager::initializeTexture() {
    SkASSERT(!fTextures.empty());
    if (fTextures.back().fTexture) {
        return true;
    }

    const Caps* caps = fRecorder->priv().caps();
    TextureInfo info = caps->getDefaultSampledTextureInfo(
            kColorType, Mipmapped::kNo, fRecorder->priv().isProtected(), Renderable::kNo);

    fTextures.back().fTexture = TextureProxy::Make(caps,
                                                   fRecorder->priv().resourceProvider(),
                                                   SkISize::Make(kAtlasTextureDims,
                                                                 kAtlasTextureDims),
                                                   info,
                                                   "AlphaAtlas",
                                                   Budgeted::kYes);

    return fTextures.back().fTexture != nullptr;
}

void AlphaAtlasManager::getAssociatedProxies(
        SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>>& allocations) {
    SkASSERT(fMarked);
    SkDEBUGCODE(fMarked = false;)
    SkASSERT(!fTextures.empty());
    TextureState& origTex = fTextures.back();
    int32_t currentSize = origTex.fAlphaBuffer.size();

    // Nothing appended
    if (currentSize == fMarkedStart) {
        return;
    }

    // Calculate how much room is actually left in the active texture
    int32_t firstPageSize = kMaxCapBytes - origTex.fUploadSize;

    // If we're within the remaining space, we're done
    if (currentSize <= firstPageSize) {
        allocations.push_back({currentSize, origTex.fTexture});
        return;
    }

    // Else handle overflow: point the first allocation to the remaining space in the original texture
    allocations.push_back({firstPageSize, origTex.fTexture});

    // Paginate the overflow, in almost all cases this will be a single page
    int32_t processed = firstPageSize;
    while (processed < currentSize) {
        int32_t chunk = std::min(kMaxCapBytes, currentSize - processed);
        fTextures.push({});
        initializeTexture();
        fTextures.back().fAlphaBuffer.append(chunk, origTex.fAlphaBuffer.begin() + processed);
        processed += chunk;

        // Return the index relative to the current batch's total buffer size
        allocations.push_back({processed, fTextures.back().fTexture});
    }

    // Truncate the original texture's buffer to what actually fit
    origTex.fAlphaBuffer.resize(firstPageSize);
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    while (!fTextures.empty()) {
        TextureState& tex = fTextures.front();

        if (!tex.fAlphaBuffer.empty()) {
            // The texture better not have been dropped here.
            SkASSERT(tex.fTexture);

            int height = (tex.fAlphaBuffer.size() + kAtlasDimsBytes - 1) / kAtlasDimsBytes;
            int paddedSize = height * kAtlasDimsBytes;
            if (tex.fAlphaBuffer.size() != paddedSize) {
                tex.fAlphaBuffer.resize(paddedSize);
            }

            MipLevel level;
            level.fPixels = tex.fAlphaBuffer.data();
            level.fRowBytes = kAtlasDimsBytes;
            int startRow = tex.fUploadSize / kAtlasDimsBytes;
            SkIRect dstRect = SkIRect::MakeXYWH(0, startRow, kAtlasTextureDims, height);

            Swizzle readSwizzle = ReadSwizzleForColorType(
                    kColorType, TextureInfoPriv::ViewFormat(tex.fTexture->textureInfo()));
            TextureProxyView proxyView(tex.fTexture, readSwizzle);

            SkColorInfo srcColorInfo(kColorType, kPremul_SkAlphaType, nullptr);
            SkColorInfo dstColorInfo(kColorType, kPremul_SkAlphaType, nullptr);
            UploadSource source = UploadSource::Make(fRecorder->priv().caps(),
                                                     proxyView,
                                                     srcColorInfo,
                                                     dstColorInfo,
                                                     SkSpan<const MipLevel>(&level, 1),
                                                     dstRect);

            dc->recordUpload(fRecorder, source, nullptr);

            tex.fUploadSize += tex.fAlphaBuffer.size();
            tex.fAlphaBuffer.clear();
        }

        if (fTextures.size() > 1) {
            fTextures.pop();
        } else {
            break;
        }
    }
}

void AlphaAtlasManager::freeGpuResources() {
    while (!fTextures.empty()) {
        fTextures.pop();
    }
}

}  // namespace skgpu::graphite
