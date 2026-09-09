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
        , fNextPageRowCount(SparseStripConfig::kInitialAtlasRows)
        , fWritingBytes(0) {}

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

    sk_sp<TextureProxy> proxy = nullptr;
    if (fRecorder) {
        const Caps* caps = fRecorder->priv().caps();
        TextureInfo info = caps->getDefaultSampledTextureInfo(SparseStripConfig::kColorType,
                                                              Mipmapped::kNo,
                                                              fRecorder->priv().isProtected(),
                                                              Renderable::kNo);

        proxy = TextureProxy::Make(caps,
                                   fRecorder->priv().resourceProvider(),
                                   SkISize::Make(SparseStripConfig::kAtlasWidth, rows),
                                   info,
                                   "AlphaAtlas",
                                   Budgeted::kYes);

        if (!proxy) {
            return false;
        }
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

uint8_t* AlphaAtlasManager::requestAlphaSpaceSlow(int32_t numBytes) {
    SkASSERT(numBytes > 0 && numBytes <= SparseStripConfig::kMaxCapBytes);

    // If we are already in null caps mode, append to the staging buffer
    if (fInNullCaps) {
        uint8_t* ptr = fNullAlphasBuffer.append(numBytes);
        fWritingBytes += numBytes;
        return ptr;
    }

    // 1. Ensure current active slot has a valid page allocated
    if (!fPages[fActiveSlot].isValid()) {
        if (!this->createPageInSlot(fActiveSlot, numBytes)) {
            return nullptr;
        }
    }

    TexturePage* activePage = &fPages[fActiveSlot];

    // 2. Check if the active slot has enough room for this tile
    if (activePage->fUsedBytes + fWritingBytes + numBytes <= activePage->fCapacityBytes) {
        uint8_t* ptr = activePage->fAlphaBuffer.append(numBytes);
        fWritingBytes += numBytes;
        return ptr;
    }

    // 3. Current active slot cannot accommodate the in-progress run.
    // Try to flip-flop to the other slot: nextSlot = fActiveSlot ^ 1
    int nextSlot = fActiveSlot ^ 1;
    if (!fPages[nextSlot].isValid()) {
        if (!this->createPageInSlot(nextSlot, fWritingBytes + numBytes)) {
            return nullptr;
        }

        TexturePage* nextPage = &fPages[nextSlot];
        uint8_t* dst = nextPage->fAlphaBuffer.append(fWritingBytes + numBytes);

        // If part of the run was already written into activePage, copy it over to nextPage
        // and rewind activePage back to its committed fUsedBytes.
        if (fWritingBytes > 0) {
            std::memcpy(dst, activePage->fAlphaBuffer.data() + activePage->fUsedBytes, fWritingBytes);
            activePage->fAlphaBuffer.resize(activePage->fUsedBytes);
        }

        fActiveSlot = nextSlot;
        fWritingBytes += numBytes;
        return dst + (fWritingBytes - numBytes);
    }

    // 4. Both slots are in use! Enter null caps staging mode!
    fInNullCaps = true;
    uint8_t* dst = fNullAlphasBuffer.append(fWritingBytes + numBytes);

    // Relocate in-flight run to null caps staging buffer and rewind activePage
    if (fWritingBytes > 0) {
        std::memcpy(dst, activePage->fAlphaBuffer.data() + activePage->fUsedBytes, fWritingBytes);
        activePage->fAlphaBuffer.resize(activePage->fUsedBytes);
    }

    fWritingBytes += numBytes;
    return dst + (fWritingBytes - numBytes);
}

std::optional<AlphaAtlasManager::AlphaAllocation> AlphaAtlasManager::finalizeRun(int32_t numBytes) {
    if (numBytes != -1) {
        if (numBytes <= 0 || numBytes > SparseStripConfig::kMaxCapBytes) {
            this->abortRun();
            return std::nullopt;
        }
        SkASSERT(fWritingBytes == numBytes);
    } else {
        if (fWritingBytes <= 0 || fWritingBytes > SparseStripConfig::kMaxCapBytes) {
            this->abortRun();
            return std::nullopt;
        }
        numBytes = fWritingBytes;
    }

    if (fInNullCaps) {
        int32_t alphaIdx = fNullAlphasUsedBytes;
        fNullAlphasUsedBytes += numBytes;
        uint8_t* ptr = fNullAlphasBuffer.data() + alphaIdx;
        fWritingBytes = 0;
        return AlphaAllocation{ptr, alphaIdx, SparseStripConfig::kInvalidTexPage};
    }

    SkASSERT(fPages[fActiveSlot].isValid());
    TexturePage* activePage = &fPages[fActiveSlot];
    SkASSERT(activePage->fUsedBytes + numBytes <= activePage->fCapacityBytes);

    int32_t alphaIdx = activePage->fUsedBytes;
    activePage->fUsedBytes += numBytes;
    uint8_t* ptr = activePage->fAlphaBuffer.data() + alphaIdx;
    fWritingBytes = 0;

    return AlphaAllocation{ptr, alphaIdx, static_cast<uint16_t>(fActiveSlot)};
}

void AlphaAtlasManager::abortRun() {
    if (fWritingBytes > 0) {
        if (fInNullCaps) {
            fNullAlphasBuffer.resize(fNullAlphasUsedBytes);
        } else if (fPages[fActiveSlot].isValid()) {
            fPages[fActiveSlot].fAlphaBuffer.resize(fPages[fActiveSlot].fUsedBytes);
        }
        fWritingBytes = 0;
    }
}

bool AlphaAtlasManager::resolveNullCaps(EndCaps* ends, DrawContext* dc) {
    if (!ends->hasNullCaps()) {
        return true;
    }
    int firstNull = ends->firstNullCapIndex();
    if (firstNull == EndCaps::kInvalidIndex) {
        return true;
    }
    int oldSlot = fActiveSlot ^ 1;
    // Verify no earlier caps in ends reference oldSlot before repurposing it
    for (int i = 0; i < firstNull; ++i) {
        if (ends->caps()[i].fTexPage == oldSlot) {
            this->resetNullCaps();
            return false;
        }
    }
    if (fPages[oldSlot].isValid()) {
        fPages[oldSlot].reset();
    }
    if (!this->createPageInSlot(oldSlot, fNullAlphasUsedBytes)) {
        this->resetNullCaps();
        return false;
    }
    TexturePage* newPage = &fPages[oldSlot];
    newPage->fAlphaBuffer = std::move(fNullAlphasBuffer);
    newPage->fUsedBytes = fNullAlphasUsedBytes;
    fNullAlphasBuffer.clear();
    fNullAlphasUsedBytes = 0;
    fInNullCaps = false;
    fActiveSlot = oldSlot;

    auto& caps = ends->caps();
    for (int i = firstNull; i < caps.size(); ++i) {
        caps[i].fTexPage = static_cast<uint16_t>(oldSlot);
    }
    ends->resetNullCaps();

    this->populateProxies(ends);

    if (dc) {
        this->uploadPage(dc, oldSlot);
    }
    return true;
}

void AlphaAtlasManager::uploadPage(DrawContext* dc, int slot) {
    if (slot < 0 || slot >= SparseStripConfig::kMaxTexturePages) {
        return;
    }
    TexturePage& page = fPages[slot];
    if (page.isValid() && !page.fAlphaBuffer.empty()) {
        if (page.fTexture) {
            // Compute how many rows are in the pending buffer and pad buffer to full rows
            int32_t pendingRows =
                    (page.fAlphaBuffer.size() + SparseStripConfig::kAtlasWidthBytes - 1) /
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
        page.fAlphaBuffer.clear();
    }
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    for (int i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        this->uploadPage(dc, i);
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
    fWritingBytes = 0;
    fInNullCaps = false;
    fNullAlphasBuffer.clear();
    fNullAlphasUsedBytes = 0;
}

}  // namespace skgpu::graphite
