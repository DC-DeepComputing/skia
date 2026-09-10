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
        , fActiveSlot(0)
        , fNextPageRowCount(SparseStripConfig::kInitialAtlasRows) {}

bool AlphaAtlasManager::createPageInSlot(int slot, int32_t minRequiredBytes) {
    SkASSERT(slot < SparseStripConfig::kMaxTexturePages);
    SkASSERT(!fPages[slot].isValid());

    int32_t rows = fNextPageRowCount;
    if (minRequiredBytes > 0) {
        int32_t neededRows = (minRequiredBytes + SparseStripConfig::kAtlasWidthBytes - 1) /
                             SparseStripConfig::kAtlasWidthBytes;
        rows = std::max(rows, neededRows);
    }
    rows = std::min(rows, SparseStripConfig::kMaxAtlasRows);

    const Caps* caps = fRecorder->priv().caps();
    TextureInfo info = caps->getDefaultSampledTextureInfo(SparseStripConfig::kColorType,
                                                          Mipmapped::kNo,
                                                          fRecorder->priv().isProtected(),
                                                          Renderable::kNo);

    sk_sp<TextureProxy> proxy =
            TextureProxy::Make(caps,
                               fRecorder->priv().resourceProvider(),
                               SkISize::Make(SparseStripConfig::kAtlasWidth, rows),
                               info,
                               "AlphaAtlas",
                               Budgeted::kYes);

    if (!proxy) {
        return false;
    }

    TexturePage& page = fPages[slot];
    page.fTexture = std::move(proxy);
    page.fRowCount = rows;
    page.fCapacityBytes = rows * SparseStripConfig::kAtlasWidthBytes;
    page.fUsedBytes = 0;
    page.fUploadedRows = 0;
    page.fAlphaBuffer.clear();
    page.fAlphaBuffer.reserve(page.fCapacityBytes);

    // Double row count for the next created page, clamped to max atlas rows
    fNextPageRowCount = std::min(SparseStripConfig::kMaxAtlasRows, rows * 2);
    return true;
}

uint8_t* AlphaAtlasManager::getAlphaPointer(int32_t numBytes) {
    if (numBytes <= 0 || numBytes > SparseStripConfig::kMaxCapBytes) {
        return nullptr;
    }

    // 0. If already on the nullbuffer slot, append directly to it.
    if (fActiveSlot == kNullSlot) {
        return fPages[kNullSlot].fAlphaBuffer.append(numBytes);
    }

    // 1. Ensure current active slot has a valid page allocated
    if (!fPages[fActiveSlot].isValid()) {
        if (!this->createPageInSlot(fActiveSlot, numBytes)) {
            return nullptr;
        }
    }

    // 2. If the active page is already full, proactively flip to the other slot
    if (fPages[fActiveSlot].fUsedBytes == fPages[fActiveSlot].fCapacityBytes) {
        int nextSlot = fActiveSlot ^ 1;
        if (fPages[nextSlot].isValid()) {
            // Both GPU pages are full! Transition to nullbuffer.
            fActiveSlot = kNullSlot;
            fPages[kNullSlot].fCapacityBytes = SparseStripConfig::kMaxCapBytes;
            return fPages[kNullSlot].fAlphaBuffer.append(numBytes);
        }
        if (!this->createPageInSlot(nextSlot, numBytes)) {
            return nullptr;
        }
        fActiveSlot = nextSlot;
    }

    TexturePage& activePage = fPages[fActiveSlot];
    fLastGpuSlot = fActiveSlot;
    return activePage.fAlphaBuffer.append(numBytes);
}

std::optional<std::pair<int32_t, uint16_t>> AlphaAtlasManager::finalizeRun() {
    TexturePage& activePage = fPages[fActiveSlot];

    // Case 0: We are already on the nullbuffer
    if (fActiveSlot == kNullSlot) {
        int32_t runBytes = activePage.fAlphaBuffer.size() - activePage.fUsedBytes;
        if (runBytes <= 0) {
            return std::nullopt;
        }
        int32_t runStartIdx = activePage.fUsedBytes;
        activePage.fUsedBytes = activePage.fAlphaBuffer.size();
        return std::make_pair(runStartIdx, static_cast<uint16_t>(kNullSlot));
    }

    if (!activePage.isValid()) {
        return std::nullopt;
    }

    int32_t runBytes = activePage.fAlphaBuffer.size() - activePage.fUsedBytes;
    if (runBytes <= 0) {
        return std::nullopt;
    }

    int32_t runStartIdx = activePage.fUsedBytes;

    // Case 1: Run completely fits in active page (no straddle)
    if (activePage.fAlphaBuffer.size() <= activePage.fCapacityBytes) {
        activePage.fUsedBytes = activePage.fAlphaBuffer.size();
        return std::make_pair(runStartIdx, static_cast<uint16_t>(fActiveSlot));
    }

    // Case 2: Run straddled the page boundary!
    int nextSlot = fActiveSlot ^ 1;
    if (fPages[nextSlot].isValid()) {
        // Both GPU slots are full! Straddle to the nullbuffer.
        TexturePage& nullPage = fPages[kNullSlot];
        nullPage.fCapacityBytes = SparseStripConfig::kMaxCapBytes;

        uint8_t* dst = nullPage.fAlphaBuffer.append(runBytes);
        std::memmove(dst,
                     activePage.fAlphaBuffer.data() + runStartIdx,
                     runBytes);

        // Trim overrun from activePage back to its committed size
        activePage.fAlphaBuffer.resize(activePage.fUsedBytes);

        // Switch active slot to kNullSlot
        fActiveSlot = kNullSlot;
        nullPage.fUsedBytes = runBytes;

        return std::make_pair(0, static_cast<uint16_t>(kNullSlot));
    }

    if (!this->createPageInSlot(nextSlot, runBytes)) {
        return std::nullopt;
    }

    TexturePage& newPage = fPages[nextSlot];

    // Append runBytes to newPage and memmove the overrun data
    uint8_t* dst = newPage.fAlphaBuffer.append(runBytes);
    std::memmove(dst,
                 activePage.fAlphaBuffer.data() + runStartIdx,
                 runBytes);

    // Trim overrun from activePage back to its committed size
    activePage.fAlphaBuffer.resize(activePage.fUsedBytes);

    // Switch active slot to nextSlot
    fActiveSlot = nextSlot;
    newPage.fUsedBytes = runBytes;

    return std::make_pair(0, static_cast<uint16_t>(nextSlot));
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    for (int i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        TexturePage& page = fPages[i];
        if (!page.isValid()) {
            continue;
        }
        int32_t startRow = page.fUploadedRows;
        int32_t totalUsedRows =
                (page.fUsedBytes + SparseStripConfig::kAtlasWidthBytes - 1) /
                SparseStripConfig::kAtlasWidthBytes;
        int32_t pendingRows = totalUsedRows - startRow;
        if (pendingRows > 0) {
            SkASSERT(page.fTexture);

            int32_t paddedSize = totalUsedRows * SparseStripConfig::kAtlasWidthBytes;
            if (page.fAlphaBuffer.size() < paddedSize) {
                int32_t diff = paddedSize - page.fAlphaBuffer.size();
                uint8_t* ptr = page.fAlphaBuffer.append(diff);
                std::memset(ptr, 0, diff);
            }

            MipLevel level;
            level.fPixels = page.fAlphaBuffer.data() + (startRow * SparseStripConfig::kAtlasWidthBytes);
            level.fRowBytes = SparseStripConfig::kAtlasWidthBytes;
            SkIRect dstRect =
                    SkIRect::MakeXYWH(0, startRow, SparseStripConfig::kAtlasWidth, pendingRows);

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

            if (dc) {
                dc->recordUpload(fRecorder, source, nullptr);
            }

            page.fUploadedRows = totalUsedRows;
            page.fUsedBytes = paddedSize;
        }
    }

    // At flush time, retire the older GPU slot if both were valid.
    if (fPages[0].isValid() && fPages[1].isValid()) {
        int activeGpuSlot = (fActiveSlot < SparseStripConfig::kMaxTexturePages) ? fActiveSlot : fLastGpuSlot;
        int oldSlot = activeGpuSlot ^ 1;
        if (fPages[oldSlot].isValid()) {
            fPages[oldSlot].reset();
        }
    }
}

bool AlphaAtlasManager::resolveNullCaps(EndCaps* ends) {
    if (!ends || !ends->hasNullCaps()) {
        return true;
    }

    TexturePage& nullPage = fPages[kNullSlot];
    int32_t nullBytes = nullPage.fUsedBytes;
    if (nullBytes <= 0) {
        ends->clearNullCaps();
        return true;
    }

    // Find the free GPU slot (the one that was retired by recordUploads)
    int freeSlot = -1;
    for (int i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        if (!fPages[i].isValid()) {
            freeSlot = i;
            break;
        }
    }

    if (freeSlot == -1) {
        return false;
    }

    if (!this->createPageInSlot(freeSlot, nullBytes)) {
        return false;
    }

    TexturePage& newPage = fPages[freeSlot];
    uint8_t* dst = newPage.fAlphaBuffer.append(nullBytes);
    std::memcpy(dst, nullPage.fAlphaBuffer.data(), nullBytes);
    newPage.fUsedBytes = nullBytes;

    sk_sp<TextureProxy> proxy = newPage.fTexture;
    int firstNull = ends->firstNullCapIndex();
    auto& caps = ends->caps();
    for (int i = firstNull; i < caps.size(); ++i) {
        caps[i].fTexPage = static_cast<uint16_t>(freeSlot);
        caps[i].fProxy = proxy;
    }

    ends->setProxy(freeSlot, proxy);
    ends->clearNullCaps();

    nullPage.reset();
    fActiveSlot = freeSlot;
    fLastGpuSlot = freeSlot;
    return true;
}

void AlphaAtlasManager::freeGpuResources() {
    for (int i = 0; i < kMaxSlots; ++i) {
        fPages[i].reset();
    }
    fActiveSlot = 0;
    fLastGpuSlot = 0;
    fNextPageRowCount = SparseStripConfig::kInitialAtlasRows;
}

}  // namespace skgpu::graphite
