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

#include "src/gpu/graphite/task/CopyTask.h"

namespace skgpu::graphite {

AlphaAtlasManager::AlphaAtlasManager(Recorder* recorder) : fRecorder(recorder) {
    fTextures.push({});
}

bool AlphaAtlasManager::initializeTexture() {
    SkASSERT(!fTextures.empty());
    if (fTextures.back().fTexture) {
        return true;
    }
    return makeTextureProxy(fTextures.back(), fTextures.back().fCurrentHeight);
}

bool AlphaAtlasManager::makeTextureProxy(TextureState& state, int32_t height) {
    const Caps* caps = fRecorder->priv().caps();
    TextureInfo info = caps->getDefaultSampledTextureInfo(
            kColorType, Mipmapped::kNo, fRecorder->priv().isProtected(), Renderable::kNo);

    state.fTexture = TextureProxy::Make(caps,
                                        fRecorder->priv().resourceProvider(),
                                        SkISize::Make(kAtlasWidth, height),
                                        info,
                                        "AlphaAtlas",
                                        Budgeted::kYes);
    state.fCurrentHeight = height;
    return state.fTexture != nullptr;
}

bool AlphaAtlasManager::getAssociatedProxies(
        SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>>& allocations) {
    SkASSERT(fMarked);
    SkDEBUGCODE(fMarked = false;)
    SkASSERT(!fTextures.empty());
    TextureState& origTex = fTextures.back();
    int32_t currentSize = origTex.fAlphaBuffer.size();

    // Nothing appended
    if (currentSize == fMarkedStart) {
        return true;
    }

    // Check if we need to grow the current texture height
    int32_t totalNeededBytes = origTex.fUploadSize + currentSize;
    int32_t maxCurrentPageCapacity = kAtlasWidth * origTex.fCurrentHeight * 4;

    if (totalNeededBytes > maxCurrentPageCapacity && origTex.fCurrentHeight < kMaxAtlasHeight) {
        int32_t newHeight = origTex.fCurrentHeight;
        while (totalNeededBytes > (kAtlasWidth * newHeight * 4) && newHeight < kMaxAtlasHeight) {
            newHeight *= 2;
        }

        sk_sp<TextureProxy> oldTexture = origTex.fTexture;
        int32_t oldUploadSize = origTex.fUploadSize;

        // Upload any pending alpha data that fits into oldTexture before replacing it
        int32_t remainingOldBytes = maxCurrentPageCapacity - oldUploadSize;
        int32_t uploadableBytes = std::min(currentSize, remainingOldBytes);
        int32_t rowsToUpload = uploadableBytes / kAtlasDimsBytes;
        int32_t bytesToUpload = rowsToUpload * kAtlasDimsBytes;

        if (bytesToUpload > 0 && oldTexture) {
            MipLevel level;
            level.fPixels = origTex.fAlphaBuffer.data();
            level.fRowBytes = kAtlasDimsBytes;
            int startRow = oldUploadSize / kAtlasDimsBytes;
            SkIRect dstRect = SkIRect::MakeXYWH(0, startRow, kAtlasWidth, rowsToUpload);

            Swizzle readSwizzle = ReadSwizzleForColorType(
                    kColorType, TextureInfoPriv::ViewFormat(oldTexture->textureInfo()));
            TextureProxyView proxyView(oldTexture, readSwizzle);

            SkColorInfo srcColorInfo(kColorType, kPremul_SkAlphaType, nullptr);
            SkColorInfo dstColorInfo(kColorType, kPremul_SkAlphaType, nullptr);
            UploadSource source = UploadSource::Make(fRecorder->priv().caps(),
                                                     proxyView,
                                                     srcColorInfo,
                                                     dstColorInfo,
                                                     SkSpan<const MipLevel>(&level, 1),
                                                     dstRect);

            UploadInstance instance = UploadInstance::Make(fRecorder, source, nullptr);
            if (instance.isValid()) {
                fRecorder->priv().add(UploadTask::Make(std::move(instance)));
            }

            origTex.fUploadSize += bytesToUpload;
            int32_t remainingInBuf = currentSize - bytesToUpload;
            if (remainingInBuf > 0) {
                memmove(origTex.fAlphaBuffer.data(),
                        origTex.fAlphaBuffer.data() + bytesToUpload,
                        remainingInBuf);
            }
            origTex.fAlphaBuffer.resize(remainingInBuf);
            currentSize = remainingInBuf;
        }

        if (makeTextureProxy(origTex, newHeight)) {
            int copyUploadSize = origTex.fUploadSize;
            if (copyUploadSize > 0 && oldTexture) {
                int oldUploadedRows = (copyUploadSize + kAtlasDimsBytes - 1) / kAtlasDimsBytes;
                SkIRect srcRect = SkIRect::MakeWH(kAtlasWidth, oldUploadedRows);
                fRecorder->priv().add(CopyTextureToTextureTask::Make(
                        std::move(oldTexture), srcRect, origTex.fTexture, SkIPoint::Make(0, 0)));
            }
        }
    }

    int32_t maxCapacity = kAtlasWidth * origTex.fCurrentHeight * 4;
    int32_t firstPageSize = maxCapacity - origTex.fUploadSize;

    if (currentSize <= firstPageSize) {
        allocations.push_back({currentSize, origTex.fTexture});
        return true;
    }

    // Handle overflow across pages
    allocations.push_back({firstPageSize, origTex.fTexture});

    int32_t processed = firstPageSize;
    while (processed < currentSize) {
        if (fTextures.size() >= static_cast<size_t>(kMaxPages)) {
            // BAILOUT! Exceeded total texture page budget!
            origTex.fAlphaBuffer.resize(fMarkedStart);
            allocations.clear();
            return false;
        }
        int32_t chunk = std::min(kMaxCapBytes, currentSize - processed);
        fTextures.push({});
        initializeTexture();
        fTextures.back().fAlphaBuffer.append(chunk, origTex.fAlphaBuffer.begin() + processed);
        processed += chunk;
        allocations.push_back({processed, fTextures.back().fTexture});
    }

    origTex.fAlphaBuffer.resize(firstPageSize);
    return true;
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    while (!fTextures.empty()) {
        TextureState& tex = fTextures.front();

        if (!tex.fAlphaBuffer.empty()) {
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
            SkIRect dstRect = SkIRect::MakeXYWH(0, startRow, kAtlasWidth, height);

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

        fTextures.pop();
    }

    // Reset initial texture state for the next frame
    fTextures.push({});
}

void AlphaAtlasManager::freeGpuResources() {
    while (!fTextures.empty()) {
        fTextures.pop();
    }
}

}  // namespace skgpu::graphite
