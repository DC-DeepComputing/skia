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

#include <algorithm>
#include <cstring>

namespace skgpu::graphite {

namespace {

int32_t rows_for_bytes(int64_t bytes) {
    return static_cast<int32_t>((bytes + SparseStripConfig::kAtlasWidthBytes - 1) /
                                SparseStripConfig::kAtlasWidthBytes);
}

}  // anonymous namespace

AlphaAtlasManager::AlphaAtlasManager(Recorder* recorder)
        : fRecorder(recorder)
        , fOverflowStagingUsedBytes(0)
        , fOverflowDemandBytes(0)
        , fOverflowReplacementBytes(0)
        , fInFlightDraws(0)
        , fNextPageID(kInvalidPageID + 1)
        , fActiveSlot(0)
        , fLastGpuSlot(0)
        , fNextPageRowCount(SparseStripConfig::kInitialAtlasRows)
        , fNextOverflowRowCount(SparseStripConfig::kInitialAtlasRows) {}

uint32_t AlphaAtlasManager::newPageID() {
    uint32_t id = fNextPageID++;
    if (fNextPageID == kInvalidPageID) {
        fNextPageID = kInvalidPageID + 1;
    }
    return id;
}

skgpu::Token AlphaAtlasManager::nextFlushToken() const {
    return fRecorder->priv().tokenTracker()->nextFlushToken();
}

bool AlphaAtlasManager::initPage(TexturePage* page, int32_t rows, std::string_view label) {
    SkASSERT(!page->isValid());
    SkASSERT(rows > 0 && rows <= SparseStripConfig::kMaxAtlasRows);

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
                               Budgeted::kYes,
                               label);

    if (!proxy) {
        return false;
    }

    page->fTexture = std::move(proxy);
    page->fRowCount = rows;
    page->fCapacityBytes = rows * SparseStripConfig::kAtlasWidthBytes;
    page->fUsedBytes = 0;
    page->fUploadedRows = 0;
    page->fAlphaBuffer.clear();
    page->fAlphaBuffer.reserve(page->fCapacityBytes);
    page->fID = this->newPageID();
    page->fLastUseToken = skgpu::Token::InvalidToken();
    return true;
}

bool AlphaAtlasManager::createPageInSlot(int32_t slot, int32_t minRequiredBytes) {
    SkASSERT(slot >= 0 && slot < SparseStripConfig::kMaxTexturePages);
    SkASSERT(!fPages[slot].isValid());

    int32_t rows = fNextPageRowCount;
    if (minRequiredBytes > 0) {
        int32_t neededRows = rows_for_bytes(minRequiredBytes);
        if (neededRows > SparseStripConfig::kMaxAtlasRows) {
            return false;
        }
        rows = std::max(rows, neededRows);
    }
    // Absorb the overflow seen since the last page was created, so that sustained overflow grows
    // the regular pages instead of repeatedly spilling into the overflow page.
    if (fOverflowDemandBytes > 0) {
        rows = std::max(rows,
                        rows_for_bytes(std::min<int64_t>(fOverflowDemandBytes,
                                                         SparseStripConfig::kMaxAtlasBytes)));
    }
    rows = std::min(rows, SparseStripConfig::kMaxAtlasRows);

    if (!this->initPage(&fPages[slot], rows, "AlphaAtlas")) {
        return false;
    }
    fOverflowDemandBytes = 0;

    // Double row count for the next created page, clamped to max atlas rows
    fNextPageRowCount = std::min(SparseStripConfig::kMaxAtlasRows, rows * 2);
    return true;
}

uint8_t* AlphaAtlasManager::requestAlphaSpace(int32_t numBytes) {
    if (numBytes <= 0 || numBytes > SparseStripConfig::kMaxAtlasBytes) {
        return nullptr;
    }

    // 0. If already staging overflow, append directly to it.
    if (fActiveSlot == kOverflowSlot) {
        return fOverflowStaging.append(numBytes);
    }

    // 1. Ensure current active slot has a valid page allocated
    SkASSERT(fActiveSlot >= 0 && fActiveSlot < SparseStripConfig::kMaxTexturePages);
    if (!fPages[fActiveSlot].isValid()) {
        if (!this->createPageInSlot(fActiveSlot, numBytes)) {
            return nullptr;
        }
    }

    // 2. If the active page is already full, proactively flip to the other slot
    if (fPages[fActiveSlot].fUsedBytes >= fPages[fActiveSlot].fCapacityBytes) {
        int32_t nextSlot = fActiveSlot ^ 1;
        // Both pages are in use, transition to the overflow staging buffer.
        if (fPages[nextSlot].isValid()) {
            fActiveSlot = kOverflowSlot;
            return fOverflowStaging.append(numBytes);
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
    // Case 0: We are already staging overflow
    if (fActiveSlot == kOverflowSlot) {
        SkASSERT(fOverflowStaging.size() > fOverflowStagingUsedBytes);
        int32_t runStartIdx = fOverflowStagingUsedBytes;
        fOverflowStagingUsedBytes = fOverflowStaging.size();
        return std::make_pair(runStartIdx, kOverflowSlot);
    }

    TexturePage& activePage = fPages[fActiveSlot];
    if (!activePage.isValid()) {
        return std::nullopt;
    }

    SkASSERT(activePage.fAlphaBuffer.size() > activePage.fUsedBytes);
    int32_t runBytes = activePage.fAlphaBuffer.size() - activePage.fUsedBytes;

    // Case 1: Run completely fits in active page (no straddle)
    int32_t runStartIdx = activePage.fUsedBytes;
    if (activePage.fAlphaBuffer.size() <= activePage.fCapacityBytes) {
        activePage.fUsedBytes = activePage.fAlphaBuffer.size();
        return std::make_pair(runStartIdx, static_cast<uint16_t>(fActiveSlot));
    }

    // Case 2: Run straddled the page boundary!
    int32_t nextSlot = fActiveSlot ^ 1;
    if (fPages[nextSlot].isValid()) {
        // Both regular pages are full! Move the run into the overflow staging buffer.
        SkASSERT(fOverflowStaging.size() == fOverflowStagingUsedBytes);
        int32_t stagingStartIdx = fOverflowStagingUsedBytes;
        uint8_t* dst = fOverflowStaging.append(runBytes);
        std::memcpy(dst,
                    activePage.fAlphaBuffer.data() + runStartIdx,
                    runBytes);

        // Trim overrun from activePage back to its committed size
        activePage.fAlphaBuffer.resize(activePage.fUsedBytes);

        // Switch active slot to kOverflowSlot
        fActiveSlot = kOverflowSlot;
        fOverflowStagingUsedBytes = fOverflowStaging.size();

        return std::make_pair(stagingStartIdx, kOverflowSlot);
    }

    if (!this->createPageInSlot(nextSlot, runBytes)) {
        return std::nullopt;
    }

    TexturePage& newPage = fPages[nextSlot];

    // Append runBytes to newPage and copy the overrun data
    uint8_t* dst = newPage.fAlphaBuffer.append(runBytes);
    std::memcpy(dst,
                activePage.fAlphaBuffer.data() + runStartIdx,
                runBytes);

    // Trim overrun from activePage back to its committed size
    activePage.fAlphaBuffer.resize(activePage.fUsedBytes);

    // Switch active slot to nextSlot
    fActiveSlot = nextSlot;
    newPage.fUsedBytes = runBytes;

    return std::make_pair(0, static_cast<uint16_t>(fActiveSlot));
}

void AlphaAtlasManager::beginDraw() {
    // A draw that failed mid-run can leave an unfinished run in the active page's buffer.
    if (fActiveSlot < SparseStripConfig::kMaxTexturePages && fPages[fActiveSlot].isValid()) {
        TexturePage& activePage = fPages[fActiveSlot];
        if (activePage.fAlphaBuffer.size() > activePage.fUsedBytes) {
            activePage.fAlphaBuffer.resize(activePage.fUsedBytes);
        }
    }
    fOverflowStaging.clear();
    fOverflowStagingUsedBytes = 0;
}

void AlphaAtlasManager::bindPages(EndCaps* ends) const {
    if (ends->empty()) {
        return;
    }

    // Every binding must be valid, so bindings without a page reuse another page as a placeholder.
    // The overflow slot is only bound for real by commitOverflow().
    sk_sp<TextureProxy> placeholder;
    for (int32_t i = 0; i < SparseStripConfig::kMaxTexturePages && !placeholder; ++i) {
        placeholder = fPages[i].fTexture;
    }
    for (int32_t i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        ends->setProxy(i, fPages[i].isValid() ? fPages[i].fTexture : placeholder);
    }
    ends->setProxy(kOverflowSlot, placeholder);
}

bool AlphaAtlasManager::canOverwrite(const TexturePage& page) const {
    // A page's contents may only be overwritten once every draw that sampled them has been flushed,
    // i.e. a full flush (which issues a new flush token) happened since the page was last used.
    // In-flight draws aren't covered by that, since they haven't been recorded yet, so any in-flight
    // draw other than the one performing the overwrite blocks it. Committed rows that were never
    // uploaded are also still needed, regardless of the flush token.
    const bool hasPendingUploads = rows_for_bytes(page.fUsedBytes) > page.fUploadedRows;
    return !hasPendingUploads &&
           fInFlightDraws <= 1 &&
           page.fLastUseToken < this->nextFlushToken();
}

int32_t AlphaAtlasManager::overflowRowsFor(int32_t numBytes) const {
    // Overflow pages are at least as large as the next regular page, and double with every
    // allocation so that repeated replacements within a flush epoch converge quickly.
    int32_t rows = std::max({rows_for_bytes(numBytes), fNextOverflowRowCount, fNextPageRowCount});
    return std::min(rows, SparseStripConfig::kMaxAtlasRows);
}

bool AlphaAtlasManager::overflowCommitNeedsFlush() const {
    const int32_t numBytes = fOverflowStagingUsedBytes;
    if (numBytes == 0) {
        return false;
    }
    // A flush can't make the overflow page overwritable while another draw is in flight.
    if (fInFlightDraws > 1) {
        return false;
    }
    const TexturePage& page = fOverflowPage;
    if (!page.isValid() || this->canOverwrite(page)) {
        // Committing can already overwrite the page in place (or create the first one).
        return false;
    }
    if (page.fUsedBytes + numBytes <= page.fCapacityBytes) {
        // Committing can append to the page.
        return false;
    }
    if (numBytes > page.fCapacityBytes) {
        // Even after a flush the page would be too small, so it has to be replaced either way.
        return false;
    }
    const int64_t replacementBytes =
            static_cast<int64_t>(this->overflowRowsFor(numBytes)) *
            SparseStripConfig::kAtlasWidthBytes;
    return fOverflowReplacementBytes + replacementBytes > SparseStripConfig::kOverflowSoftCapBytes;
}

bool AlphaAtlasManager::reserveOverflow(int32_t numBytes) {
    SkASSERT(numBytes > 0);
    if (numBytes > SparseStripConfig::kMaxAtlasBytes) {
        return false;
    }

    TexturePage& page = fOverflowPage;
    bool replacingPageInUse = false;
    if (page.isValid() && this->canOverwrite(page)) {
        // Every draw that sampled the page's current contents has been flushed.
        fOverflowReplacementBytes = 0;
        if (numBytes <= page.fCapacityBytes) {
            // Overwrite it in place.
            page.fUsedBytes = 0;
            page.fUploadedRows = 0;
            page.fAlphaBuffer.clear();
            page.fID = this->newPageID();
            return true;
        }
        // It's too small, and nothing still depends on it, so simply replace it.
        page.reset();
    } else if (page.isValid()) {
        if (page.fUsedBytes + numBytes <= page.fCapacityBytes) {
            // Append after the contents that are still in use.
            return true;
        }
        // Still in use but too small. Park it so that its pending rows are still uploaded, and
        // replace it with a larger page. Recorded draws keep its texture alive.
        fParkedOverflowPages.push_back(std::move(page));
        page = TexturePage();
        replacingPageInUse = true;
    }

    const int32_t rows = this->overflowRowsFor(numBytes);
    if (!this->initPage(&page, rows, "AlphaAtlasOverflow")) {
        return false;
    }
    fNextOverflowRowCount = std::min(SparseStripConfig::kMaxAtlasRows, rows * 2);
    if (replacingPageInUse) {
        fOverflowReplacementBytes += page.fCapacityBytes;
    }
    return true;
}

bool AlphaAtlasManager::commitOverflow(EndCaps* ends) {
    SkASSERT(ends);
    if (!ends->hasPendingOverflow()) {
        return true;
    }

    auto& caps = ends->caps();
    const int32_t firstOverflow = ends->firstOverflowCapIndex();
    SkASSERT(firstOverflow >= 0 && firstOverflow < caps.size());

    // The draw's overflow caps form a contiguous suffix referencing the end of the staging buffer.
    const int32_t stagingStart = caps[firstOverflow].fAlphaIndex;
    const int32_t numBytes = fOverflowStagingUsedBytes - stagingStart;
    SkASSERT(stagingStart >= 0 && numBytes > 0);

    if (!this->reserveOverflow(numBytes)) {
        return false;
    }

    TexturePage& page = fOverflowPage;
    SkASSERT(page.isValid() && page.fAlphaBuffer.size() == page.fUsedBytes);
    const int32_t base = page.fUsedBytes;
    uint8_t* dst = page.fAlphaBuffer.append(numBytes);
    std::memcpy(dst, fOverflowStaging.data() + stagingStart, numBytes);
    page.fUsedBytes += numBytes;
    page.fLastUseToken = this->nextFlushToken();

    const int32_t delta = base - stagingStart;
    for (int i = firstOverflow; i < caps.size(); ++i) {
        SkASSERT(caps[i].fTexPage == kOverflowSlot);
        caps[i].fAlphaIndex += delta;
    }

    // Bind the overflow page, and use it as the placeholder for any regular binding without a page.
    for (int32_t i = 0; i < SparseStripConfig::kNumAlphaBindings; ++i) {
        if (i == kOverflowSlot || !ends->proxies()[i]) {
            ends->setProxy(i, page.fTexture);
        }
    }
    ends->markOverflowCommitted();

    fOverflowDemandBytes += numBytes;
    fOverflowStaging.clear();
    fOverflowStagingUsedBytes = 0;
    return true;
}

void AlphaAtlasManager::markPagesUsed(const EndCaps& ends) {
    if (ends.empty()) {
        return;
    }
    const skgpu::Token token = this->nextFlushToken();
    auto markUsed = [&](TexturePage* page) {
        if (!page->isValid()) {
            return;
        }
        for (const sk_sp<TextureProxy>& proxy : ends.proxies()) {
            if (proxy == page->fTexture) {
                page->fLastUseToken = token;
                return;
            }
        }
    };
    for (TexturePage& page : fPages) {
        markUsed(&page);
    }
    markUsed(&fOverflowPage);
}

AlphaAtlasManager::DrawScope::DrawScope(AlphaAtlasManager* manager, const EndCaps* ends)
        : fManager(manager), fEnds(ends) {
    SkASSERT(fManager && fEnds);
    fManager->fInFlightDraws++;
}

AlphaAtlasManager::DrawScope::~DrawScope() {
    SkASSERT(fManager->fInFlightDraws > 0);
    fManager->markPagesUsed(*fEnds);
    fManager->fInFlightDraws--;
}

void AlphaAtlasManager::uploadPage(TexturePage* page, DrawContext* dc) {
    SkASSERT(page->isValid());
    int32_t startRow = page->fUploadedRows;
    int32_t totalUsedRows = rows_for_bytes(page->fUsedBytes);
    int32_t pendingRows = totalUsedRows - startRow;
    if (pendingRows <= 0) {
        return;
    }

    int32_t paddedSize = totalUsedRows * SparseStripConfig::kAtlasWidthBytes;
    if (page->fAlphaBuffer.size() < paddedSize) {
        int32_t diff = paddedSize - page->fAlphaBuffer.size();
        uint8_t* ptr = page->fAlphaBuffer.append(diff);
        std::memset(ptr, 0, diff);
    }

    MipLevel level;
    level.fPixels = page->fAlphaBuffer.data() + (startRow * SparseStripConfig::kAtlasWidthBytes);
    level.fRowBytes = SparseStripConfig::kAtlasWidthBytes;
    SkIRect dstRect = SkIRect::MakeXYWH(0, startRow, SparseStripConfig::kAtlasWidth, pendingRows);

    Swizzle readSwizzle = ReadSwizzleForColorType(
            SparseStripConfig::kColorType,
            TextureInfoPriv::ViewFormat(page->fTexture->textureInfo()));
    TextureProxyView proxyView(page->fTexture, readSwizzle);

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

    page->fUploadedRows = totalUsedRows;
    page->fUsedBytes = paddedSize;
}

void AlphaAtlasManager::recordUploads(DrawContext* dc) {
    // Replaced overflow pages may still hold committed rows that recorded draws depend on. Upload
    // them before dropping them; the recorded draws keep their textures alive.
    for (TexturePage& page : fParkedOverflowPages) {
        this->uploadPage(&page, dc);
    }
    fParkedOverflowPages.clear();

    for (TexturePage& page : fPages) {
        if (page.isValid()) {
            this->uploadPage(&page, dc);
        }
    }
    if (fOverflowPage.isValid()) {
        this->uploadPage(&fOverflowPage, dc);
    }

    // At flush time, retire the older GPU slot if both were valid.
    if (fPages[0].isValid() && fPages[1].isValid()) {
        // If we're on the overflow slot, the active slot is instead the last gpu slot.
        int32_t activeGpuSlot =
                (fActiveSlot < SparseStripConfig::kMaxTexturePages) ? fActiveSlot : fLastGpuSlot;
        int32_t oldSlot = activeGpuSlot ^ 1;
        SkASSERT(fPages[oldSlot].isValid());
        fPages[oldSlot].reset();
        // If the regular pages were exhausted, resume allocating from the freed slot.
        if (fActiveSlot == kOverflowSlot) {
            fActiveSlot = oldSlot;
        }
    }
}

void AlphaAtlasManager::freeGpuResources() {
    for (TexturePage& page : fPages) {
        page.reset();
    }
    fOverflowPage.reset();
    fParkedOverflowPages.clear();
    fOverflowStaging.clear();
    fOverflowStagingUsedBytes = 0;
    fOverflowDemandBytes = 0;
    fOverflowReplacementBytes = 0;
    fActiveSlot = 0;
    fLastGpuSlot = 0;
    fNextPageRowCount = SparseStripConfig::kInitialAtlasRows;
    fNextOverflowRowCount = SparseStripConfig::kInitialAtlasRows;
    fStripCache.reset();
}

void AlphaAtlasManager::evictAtlases() {
    for (TexturePage& page : fPages) {
        if (page.isValid()) {
            page.fID = this->newPageID();
        }
    }
    if (fOverflowPage.isValid()) {
        fOverflowPage.fID = this->newPageID();
    }
    // The new page IDs already invalidate every cached strip, so free them right away.
    fStripCache.reset();
}

uint32_t AlphaAtlasManager::pageID(uint16_t slot) const {
    const TexturePage* page = this->pageForSlot(slot);
    return page && page->isValid() ? page->fID : kInvalidPageID;
}

std::optional<uint16_t> AlphaAtlasManager::findResidentSlot(uint32_t pageID) const {
    if (pageID == kInvalidPageID) {
        return std::nullopt;
    }
    for (int32_t i = 0; i < SparseStripConfig::kMaxTexturePages; ++i) {
        if (fPages[i].isValid() && fPages[i].fID == pageID) {
            return static_cast<uint16_t>(i);
        }
    }
    return std::nullopt;
}

skgpu::Token AlphaAtlasManager::lastUseToken(uint16_t slot) const {
    const TexturePage* page = this->pageForSlot(slot);
    return page && page->isValid() ? page->fLastUseToken : skgpu::Token::InvalidToken();
}

bool AlphaAtlasManager::findCachedStrips(const StripCache::Key& key,
                                         SkIPoint origin,
                                         EndCaps* ends,
                                         WideTiles* wides) {
    SkASSERT(ends && ends->empty() && !ends->hasOverflowCaps());
    SkASSERT(wides && wides->empty());
    const StripCache::Entry* entry = fStripCache.find(key, this->nextFlushToken().value());
    if (!entry) {
        return false;
    }

    // Regular pages are append-only and never change slots, so the cached alphas are still in place
    // as long as each slot they were written to still holds the same page. Their rows may not have
    // been uploaded yet, but any pending rows are uploaded before the next render pass executes.
    for (int32_t slot = 0; slot < SparseStripConfig::kMaxTexturePages; ++slot) {
        const uint32_t id = entry->fPageIDs[slot];
        if (id != kInvalidPageID && id != this->pageID(slot)) {
            fStripCache.remove(key);
            return false;
        }
    }

    // The translated strips must remain within the range of their uint16 coordinates.
    const int64_t dx = static_cast<int64_t>(origin.fX) - entry->fOrigin.fX;
    const int64_t dy = static_cast<int64_t>(origin.fY) - entry->fOrigin.fY;
    const SkIRect& bounds = entry->fBounds;
    if (bounds.fLeft + dx < 0 || bounds.fTop + dy < 0 ||
        bounds.fRight + dx > UINT16_MAX || bounds.fBottom + dy > UINT16_MAX) {
        return false;
    }

    // EndCaps sample their alphas relative to their own top-left corner, so they don't need to be
    // tile aligned and can be translated by any integer amount.
    ends->caps() = entry->fCaps;
    wides->tiles() = entry->fWides;
    if (dx != 0 || dy != 0) {
        for (EndCaps::EndCap& cap : ends->caps()) {
            cap.fX = static_cast<uint16_t>(cap.fX + dx);
            cap.fY = static_cast<uint16_t>(cap.fY + dy);
        }
        for (WideTiles::WideTile& wide : wides->tiles()) {
            wide.fX = static_cast<uint16_t>(wide.fX + dx);
            wide.fY = static_cast<uint16_t>(wide.fY + dy);
        }
    }
#if defined(GPU_TEST_UTILS)
    fStripCacheHits++;
#endif
    return true;
}

void AlphaAtlasManager::cacheStrips(const StripCache::Key& key,
                                    SkIPoint origin,
                                    const EndCaps& ends,
                                    const WideTiles& wides) {
    // The overflow page is overwritten in place, so alphas committed to it never stay resident.
    SkASSERT(!ends.hasOverflowCaps());

    StripCache::PageIDs pageIDs;
    pageIDs.fill(kInvalidPageID);
    for (const EndCaps::EndCap& cap : ends.caps()) {
        SkASSERT(cap.fTexPage < SparseStripConfig::kMaxTexturePages);
        SkASSERT(fPages[cap.fTexPage].isValid());
        pageIDs[cap.fTexPage] = fPages[cap.fTexPage].fID;
    }
    fStripCache.add(
            key, this->nextFlushToken().value(), origin, pageIDs, ends.caps(), wides.tiles());
}

}  // namespace skgpu::graphite
