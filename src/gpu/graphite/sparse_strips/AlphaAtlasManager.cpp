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
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/task/UploadTask.h"

namespace skgpu::graphite {

AlphaAtlasManager::AlphaAtlasManager(Recorder* recorder)
        : fRecorder(recorder)
        , fActivePageIndex(-1)
        , fNextPageRowCount(SparseStripConfig::kInitialAtlasRows) {}

bool AlphaAtlasManager::createNewPage(int32_t minRequiredBytes) {
    int32_t rows = fNextPageRowCount;
    if (minRequiredBytes > 0) {
        int32_t neededRows = (minRequiredBytes + SparseStripConfig::kAtlasWidthBytes - 1) /
                             SparseStripConfig::kAtlasWidthBytes;
        rows = std::max(rows, neededRows);
    }
    rows = std::min(rows, SparseStripConfig::kMaxAtlasRows);

    const Caps* caps = fRecorder->priv().caps();
    TextureInfo info = caps->getDefaultSampledTextureInfo(
            SparseStripConfig::kColorType,
            Mipmapped::kNo,
            fRecorder->priv().isProtected(),
            Renderable::kNo);

    sk_sp<TextureProxy> proxy = TextureProxy::Make(
            caps,
            fRecorder->priv().resourceProvider(),
            SkISize::Make(SparseStripConfig::kAtlasWidth, rows),
            info,
            "AlphaAtlas",
            Budgeted::kYes);

    if (!proxy) {
        return false;
    }

    TexturePage page;
    page.fTexture = std::move(proxy);
    page.fRowCount = rows;
    page.fCapacityBytes = rows * SparseStripConfig::kAtlasWidthBytes;
    page.fUsedBytes = 0;

    fPages.push_back(std::move(page));
    fActivePageIndex = static_cast<int32_t>(fPages.size()) - 1;

    // Double row count for the next created page, clamped to max atlas rows
    fNextPageRowCount = std::min(SparseStripConfig::kMaxAtlasRows, rows * 2);
    return true;
}

AlphaAtlasManager::AlphaAllocation AlphaAtlasManager::requestAlphaSpace(
        skia_private::TArray<sk_sp<TextureProxy>>& proxies, int32_t numBytes) {
    if (numBytes <= 0) {
        return {nullptr, 0, 0, true};
    }
    if (numBytes > SparseStripConfig::kMaxCapBytes) {
        return {nullptr, 0, 0, false};
    }

    // 1. If no active page exists yet, create the initial page (1 row)
    if (fActivePageIndex < 0 || fPages.empty()) {
        if (!this->createNewPage(numBytes)) {
            return {nullptr, 0, 0, false};
        }
    }

    // 2. Helper to find or add the page's proxy into the draw's proxies array
    auto getOrAddTexPage = [&](const sk_sp<TextureProxy>& proxy) -> std::pair<uint16_t, bool> {
        for (int i = 0; i < proxies.size(); ++i) {
            if (proxies[i] == proxy) {
                return {static_cast<uint16_t>(i), true};
            }
        }
        if (proxies.size() < SparseStripConfig::kMaxTexturePages) {
            proxies.push_back(proxy);
            return {static_cast<uint16_t>(proxies.size() - 1), true};
        }
        return {0, false}; // Exceeded max texture pages per draw
    };

    TexturePage* activePage = &fPages[fActivePageIndex];

    // 3. Check if the active page has enough remaining room
    if (activePage->fUsedBytes + numBytes <= activePage->fCapacityBytes) {
        auto [texPage, ok] = getOrAddTexPage(activePage->fTexture);
        if (ok) {
            int32_t alphaIdx = activePage->fUsedBytes;
            activePage->fUsedBytes += numBytes;
            uint8_t* ptr = activePage->fAlphaBuffer.append(numBytes);
            return {ptr, alphaIdx, texPage, true};
        }
    }

    // 4. Current page is full or cannot bind to this draw. We must allocate a new doubled page.
    // First check if this draw already has kMaxTexturePages bound (and the new page would be a 3rd).
    if (proxies.size() >= SparseStripConfig::kMaxTexturePages) {
        // Cannot bind a 3rd texture to this draw, fail gracefully.
        return {nullptr, 0, 0, false};
    }

    if (!this->createNewPage(numBytes)) {
        return {nullptr, 0, 0, false};
    }

    activePage = &fPages[fActivePageIndex];
    auto [texPage, ok] = getOrAddTexPage(activePage->fTexture);
    if (!ok) {
        return {nullptr, 0, 0, false};
    }

    int32_t alphaIdx = 0;
    activePage->fUsedBytes = numBytes;
    uint8_t* ptr = activePage->fAlphaBuffer.append(numBytes);
    return {ptr, alphaIdx, texPage, true};
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    for (auto& page : fPages) {
        if (!page.fAlphaBuffer.empty()) {
            SkASSERT(page.fTexture);

            // Compute how many rows are in the pending buffer and pad buffer to full rows
            int32_t pendingRows = (page.fAlphaBuffer.size() + SparseStripConfig::kAtlasWidthBytes - 1) /
                                  SparseStripConfig::kAtlasWidthBytes;
            int32_t paddedSize = pendingRows * SparseStripConfig::kAtlasWidthBytes;
            if (page.fAlphaBuffer.size() < paddedSize) {
                int32_t diff = paddedSize - page.fAlphaBuffer.size();
                uint8_t* ptr = page.fAlphaBuffer.append(diff);
                std::memset(ptr, 0, diff);
            }

            MipLevel level;
            level.fPixels = page.fAlphaBuffer.data();
            level.fRowBytes = SparseStripConfig::kAtlasWidthBytes;
            int32_t startRow = page.fUploadedRows;
            SkIRect dstRect = SkIRect::MakeXYWH(0, startRow, SparseStripConfig::kAtlasWidth, pendingRows);

            Swizzle readSwizzle = ReadSwizzleForColorType(
                    SparseStripConfig::kColorType,
                    TextureInfoPriv::ViewFormat(page.fTexture->textureInfo()));
            TextureProxyView proxyView(page.fTexture, readSwizzle);

            SkColorInfo srcColorInfo(SparseStripConfig::kColorType, kPremul_SkAlphaType, nullptr);
            SkColorInfo dstColorInfo(SparseStripConfig::kColorType, kPremul_SkAlphaType, nullptr);
            UploadSource source = UploadSource::Make(fRecorder->priv().caps(),
                                                     proxyView,
                                                     srcColorInfo,
                                                     dstColorInfo,
                                                     SkSpan<const MipLevel>(&level, 1),
                                                     dstRect);

            dc->recordUpload(fRecorder, source, nullptr);

            page.fUploadedRows += pendingRows;
            page.fUsedBytes = page.fUploadedRows * SparseStripConfig::kAtlasWidthBytes;
            page.fAlphaBuffer.clear();
        }
    }
}

void AlphaAtlasManager::freeGpuResources() {
    fPages.clear();
    fActivePageIndex = -1;
    fNextPageRowCount = SparseStripConfig::kInitialAtlasRows;
}

}  // namespace skgpu::graphite
