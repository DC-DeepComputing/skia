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
    SkASSERT(slot == 0 || slot == 1);
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

    int otherSlot = slot ^ 1;
    const TexturePage& otherPage = fPages[otherSlot];
    int32_t bufferOffset = 0;
    int32_t capacityBytes = rows * SparseStripConfig::kAtlasWidthBytes;

    if (otherPage.isValid()) {
        if (otherPage.fBufferOffset == 0) {
            bufferOffset = otherPage.fCapacityBytes;
        } else {
            if (capacityBytes <= otherPage.fBufferOffset) {
                bufferOffset = 0;
            } else {
                bufferOffset = otherPage.fBufferOffset + otherPage.fCapacityBytes;
            }
        }
    } else {
        bufferOffset = 0;
    }

    int32_t requiredTotalBytes = bufferOffset + capacityBytes;
    if (fAlphaBuffer.size() < requiredTotalBytes) {
        fAlphaBuffer.resize(requiredTotalBytes);
    }

    TexturePage& page = fPages[slot];
    page.fTexture = std::move(proxy);
    page.fRowCount = rows;
    page.fCapacityBytes = capacityBytes;
    page.fUsedBytes = 0;
    page.fUploadedRows = 0;
    page.fBufferOffset = bufferOffset;

    // Double row count for the next created page, clamped to max atlas rows
    fNextPageRowCount = std::min(SparseStripConfig::kMaxAtlasRows, rows * 2);
    return true;
}

std::optional<AlphaAtlasManager::AlphaAllocation> AlphaAtlasManager::requestAlphaSpace(
        int32_t numBytes) {
    if (numBytes <= 0 || numBytes > SparseStripConfig::kMaxCapBytes) {
        return std::nullopt;
    }

    // If we have already entered the null caps state in the current geometry traversal,
    // continue allocating in the overflow area.
    // If we have already entered the null caps state in the current geometry traversal,
    // continue allocating in the overflow area.
    if (fNullAlphasBytes > 0) {
        int32_t currentSize = fAlphaBuffer.size();
        fAlphaBuffer.resize(currentSize + numBytes);
        uint8_t* ptr = fAlphaBuffer.data() + currentSize;
        int32_t nullAlphaIdx = fNullAlphasBytes;
        fNullAlphasBytes += numBytes;
        return AlphaAllocation{ptr, nullAlphaIdx, SparseStripConfig::kInvalidTexPage};
    }

    // 1. Ensure current active slot has a valid page allocated
    if (!fPages[fActiveSlot].isValid()) {
        if (!this->createPageInSlot(fActiveSlot, numBytes)) {
            return std::nullopt;
        }
    }

    TexturePage* activePage = &fPages[fActiveSlot];

    // 2. Check if the active slot has enough remaining room.
    // An endcap's alphas must be atomic to a single texture page (never split across pages).
    if (activePage->fUsedBytes + numBytes <= activePage->fCapacityBytes) {
        int32_t alphaIdx = activePage->fUsedBytes;
        activePage->fUsedBytes += numBytes;
        uint8_t* ptr = fAlphaBuffer.data() + activePage->fBufferOffset + alphaIdx;
        return AlphaAllocation{ptr, alphaIdx, static_cast<uint16_t>(fActiveSlot)};
    }

    // 3. Current active slot does not have enough room.
    // Flip-flop to the other slot: nextSlot = fActiveSlot ^ 1
    int nextSlot = fActiveSlot ^ 1;

    // If nextSlot is NOT in use, flip to it immediately!
    if (!fPages[nextSlot].isValid()) {
        if (!this->createPageInSlot(nextSlot, numBytes)) {
            return std::nullopt;
        }
        fActiveSlot = nextSlot;
        activePage = &fPages[fActiveSlot];
        int32_t alphaIdx = 0;
        activePage->fUsedBytes = numBytes;
        uint8_t* ptr = fAlphaBuffer.data() + activePage->fBufferOffset;
        return AlphaAllocation{ptr, alphaIdx, static_cast<uint16_t>(fActiveSlot)};
    }

    // 4. Both slots are in use! We have run out of room on the proxies.
    // MakeStrips must always complete. Store alphas in the overflow area of fAlphaBuffer,
    // and assign kInvalidTexPage. Device.cpp will flush and call cleanupNullCaps().
    fNullAlphasOffset = fAlphaBuffer.size();
    fAlphaBuffer.resize(fNullAlphasOffset + numBytes);
    uint8_t* ptr = fAlphaBuffer.data() + fNullAlphasOffset;
    int32_t nullAlphaIdx = 0;
    fNullAlphasBytes = numBytes;
    return AlphaAllocation{ptr, nullAlphaIdx, SparseStripConfig::kInvalidTexPage};
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    for (int i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        TexturePage& page = fPages[i];
        if (page.isValid() &&
            page.fUsedBytes > page.fUploadedRows * SparseStripConfig::kAtlasWidthBytes) {
            SkASSERT(page.fTexture);

            int32_t uploadedBytes = page.fUploadedRows * SparseStripConfig::kAtlasWidthBytes;
            int32_t pendingBytes = page.fUsedBytes - uploadedBytes;
            int32_t pendingRows =
                    (pendingBytes + SparseStripConfig::kAtlasWidthBytes - 1) /
                    SparseStripConfig::kAtlasWidthBytes;
            int32_t paddedSize =
                    (page.fUploadedRows + pendingRows) * SparseStripConfig::kAtlasWidthBytes;

            // Zero-pad trailing bytes in the last row in fAlphaBuffer
            int32_t currentEndOffset = page.fBufferOffset + page.fUsedBytes;
            int32_t targetEndOffset = page.fBufferOffset + paddedSize;
            if (targetEndOffset > currentEndOffset) {
                std::memset(fAlphaBuffer.data() + currentEndOffset, 0, targetEndOffset - currentEndOffset);
            }

            MipLevel level;
            level.fPixels = fAlphaBuffer.data() + page.fBufferOffset + uploadedBytes;
            level.fRowBytes = SparseStripConfig::kAtlasWidthBytes;
            int32_t startRow = page.fUploadedRows;
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

            page.fUploadedRows += pendingRows;
            page.fUsedBytes = page.fUploadedRows * SparseStripConfig::kAtlasWidthBytes;
        }
    }

    // At flush time, retire the older slot (fActiveSlot ^ 1).
    // Graphite's resource management keeps it alive for any pending GPU tasks.
    int oldSlot = fActiveSlot ^ 1;
    if (fPages[oldSlot].isValid()) {
        fPages[oldSlot].reset();
    }
}

void AlphaAtlasManager::freeGpuResources() {
    fPages[0].reset();
    fPages[1].reset();
    fActiveSlot = 0;
    fNextPageRowCount = SparseStripConfig::kInitialAtlasRows;
    fAlphaBuffer.clear();
    fNullAlphasOffset = 0;
    fNullAlphasBytes = 0;
}

}  // namespace skgpu::graphite
