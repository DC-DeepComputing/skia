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

namespace {

int32_t aligned_width(int32_t bytes) {
    constexpr int32_t kMask = SparseStripConfig::kAtlasWidthBytes - 1;
    return (bytes + kMask) & ~kMask;
}

void allocate_aligned(SkTDArray<uint8_t>* buffer, int32_t targetBytes) {
    int32_t alignedSize = aligned_width(targetBytes);
    if (buffer->size() < alignedSize) {
        buffer->resize(alignedSize);
    }
}

}  // namespace

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
        int32_t neededRows = aligned_width(minRequiredBytes) / SparseStripConfig::kAtlasWidthBytes;
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
        bufferOffset = (otherPage.fBufferOffset == 0) ? otherPage.fCapacityBytes : 0;
    } else {
        bufferOffset = 0;
    }

    int32_t requiredTotalBytes = bufferOffset + capacityBytes;
    allocate_aligned(&fAlphaBuffer, requiredTotalBytes);

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

uint8_t* AlphaAtlasManager::requestAlphaSpace(int32_t numBytes) {
    SkASSERT(numBytes > 0 && numBytes <= SparseStripConfig::kMaxCapBytes);

    auto appendBytes = [&](int32_t baseOffset) -> uint8_t* {
        int32_t offset = baseOffset + fWritingBytes;
        allocate_aligned(&fAlphaBuffer, offset + numBytes);
        fWritingBytes += numBytes;
        return fAlphaBuffer.data() + offset;
    };

#if defined(GPU_TEST_UTILS)
    if (!fRecorder) {
        return appendBytes(fNullAlphasBytes);
    }
#endif

    if (fNullAlphasBytes > 0) {
        return appendBytes(fNullAlphasOffset + fNullAlphasBytes);
    }

    if (!fPages[fActiveSlot].isValid()) {
        if (!this->createPageInSlot(fActiveSlot, numBytes)) {
            return nullptr;
        }
    }

    TexturePage* activePage = &fPages[fActiveSlot];
    return appendBytes(activePage->fBufferOffset + activePage->fUsedBytes);
}

// TODO (thomsmit): Currently we memmove runs so that straddling runs always start cleanly on the
// memory allocation for the next page. This seems a little messy, but performance is quite good, as
// the movment happens rarely. The vast majority of allocations occur within the page.
std::optional<AlphaAtlasManager::AlphaAllocation> AlphaAtlasManager::finalizeRun(int32_t numBytes) {
    if (numBytes <= 0 || numBytes > SparseStripConfig::kMaxCapBytes) {
        fWritingBytes = 0;
        return std::nullopt;
    }
    SkASSERT(fWritingBytes == numBytes);

    auto advance = [&](int32_t bufferOffset, int32_t& usedBytes, uint16_t texPage) {
        int32_t alphaIdx = usedBytes;
        usedBytes += numBytes;
        fWritingBytes = 0;
        return AlphaAllocation{fAlphaBuffer.data() + bufferOffset + alphaIdx, alphaIdx, texPage};
    };

#if defined(GPU_TEST_UTILS)
    if (!fRecorder) {
        return advance(/*bufferOffset=*/0, fNullAlphasBytes, /*texPage=*/0);
    }
#endif

    if (fNullAlphasBytes > 0) {
        return advance(fNullAlphasOffset, fNullAlphasBytes, SparseStripConfig::kInvalidTexPage);
    }

    SkASSERT(fPages[fActiveSlot].isValid());
    TexturePage* activePage = &fPages[fActiveSlot];
    int32_t runStartOffset = activePage->fBufferOffset + activePage->fUsedBytes;

    if (activePage->fUsedBytes + numBytes <= activePage->fCapacityBytes) {
        return advance(activePage->fBufferOffset, activePage->fUsedBytes,
                       static_cast<uint16_t>(fActiveSlot));
    }

    // Run does not fit in activePage!
    int nextSlot = fActiveSlot ^ 1;
    if (!fPages[nextSlot].isValid()) {
        if (!this->createPageInSlot(nextSlot, numBytes)) {
            fWritingBytes = 0;
            return std::nullopt;
        }
        fActiveSlot = nextSlot;
        activePage = &fPages[fActiveSlot];
        std::memmove(fAlphaBuffer.data() + activePage->fBufferOffset,
                     fAlphaBuffer.data() + runStartOffset,
                     numBytes);
        return advance(activePage->fBufferOffset, activePage->fUsedBytes,
                       static_cast<uint16_t>(fActiveSlot));
    }

    // Both slots in use! Enter null caps!
    fNullAlphasOffset = aligned_width(fAlphaBuffer.size());
    allocate_aligned(&fAlphaBuffer, fNullAlphasOffset + numBytes);
    std::memmove(fAlphaBuffer.data() + fNullAlphasOffset,
                 fAlphaBuffer.data() + runStartOffset,
                 numBytes);
    return advance(fNullAlphasOffset, fNullAlphasBytes, SparseStripConfig::kInvalidTexPage);
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    for (int i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        TexturePage& page = fPages[i];
        if (page.isValid() &&
            page.fUsedBytes > page.fUploadedRows * SparseStripConfig::kAtlasWidthBytes) {
            SkASSERT(page.fTexture);

            int32_t uploadedBytes = page.fUploadedRows * SparseStripConfig::kAtlasWidthBytes;
            int32_t pendingBytes = page.fUsedBytes - uploadedBytes;
            int32_t pendingRows = aligned_width(pendingBytes) / SparseStripConfig::kAtlasWidthBytes;
            int32_t paddedSize =
                    (page.fUploadedRows + pendingRows) * SparseStripConfig::kAtlasWidthBytes;

            // Zero-pad trailing bytes in the last row in fAlphaBuffer
            int32_t currentEndOffset = page.fBufferOffset + page.fUsedBytes;
            int32_t targetEndOffset = page.fBufferOffset + paddedSize;

            SkASSERT(fAlphaBuffer.size() >= targetEndOffset);
            SkASSERT(targetEndOffset == aligned_width(targetEndOffset));

            int32_t diff = targetEndOffset - currentEndOffset;
            if (diff > 0) {
                std::memset(fAlphaBuffer.data() + currentEndOffset, 0, diff);
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
    fWritingBytes = 0;
}

}  // namespace skgpu::graphite
