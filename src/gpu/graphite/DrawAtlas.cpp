/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawAtlas.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkSwizzlePriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/MaskFormat.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/task/UploadTask.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <tuple>

enum SkColorType : int;

using namespace skia_private;

namespace skgpu::graphite {

#if defined(DUMP_ATLAS_DATA)
static const constexpr bool kDumpAtlasData = true;
#else
static const constexpr bool kDumpAtlasData = false;
#endif

DrawAtlas::Plot* DrawAtlas::findPlot(PlotID id) const {
    if (auto** p = fPlotMap.find(id)) {
        return *p;
    }
    return nullptr;
}

DrawAtlas::Plot* DrawAtlas::findPlot(Record record) const {
    auto plot = this->findPlot(record.fPlotID);
    if (plot) {
        SkASSERT(plot->entryAtlasRect(record.fEntryID));
    }
    return plot;
}

void DrawAtlas::internalSetLastUseToken(Plot* plot, Token token) {
    this->makeMRU(plot->plotCoord().fPageIdx, plot->indexInPageMru());
    plot->setLastUseToken(token);
}

#ifdef SK_DEBUG
void DrawAtlas::validate() const {
    SkASSERT(fNumActivePages >= 0 && fNumActivePages <= fMaxPages);
    auto ppp = this->plotsPerPage();
    SkASSERT(fPlotMap.count() == ppp * fNumActivePages);

    for (int pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        const auto& page = fPages[pageIndex];
        SkASSERT(fProxies[pageIndex]);
        SkASSERT(page.fPlotArray);
        std::vector foundCoord(size_t(ppp), false);
        Plot* prevPlot = nullptr;
        for (int plotIndex = 0; plotIndex < ppp; ++plotIndex) {
            auto* plot = page.fPlotArray[plotIndex].get();
            SkASSERT(plot);
            auto coord = plot->plotCoord();
            int coordIndex = coord.fY * fPlotGridDimensions.width() + coord.fX;
            SkASSERT(!foundCoord[coordIndex]);
            foundCoord[coordIndex] = true;
            SkASSERT(plot->plotCoord() == coord);
            SkASSERT(plot->plotID() != PlotID::kInvalid);
            auto* mapEntry = fPlotMap.find(plot->plotID());
            SkASSERT(mapEntry && *mapEntry == plot);
            if (prevPlot) {
                SkASSERT(plot->lastUseToken() <= prevPlot->lastUseToken());
            }
            prevPlot = plot;
        }
        SkASSERT(std::all_of(foundCoord.begin(), foundCoord.end(), [](bool v) { return v; }));
    }
}
#endif

std::unique_ptr<DrawAtlas> DrawAtlas::Make(MaskFormat maskFormat,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           AllowMultitexturing allowMultitexturing,
                                           UseStorageTextures useStorageTextures,
                                           EvictionCallback* evictor,
                                           std::string_view label) {
    std::unique_ptr<DrawAtlas> atlas(new DrawAtlas(maskFormat,
                                                   width, height,
                                                   plotWidth, plotHeight,

                                                   allowMultitexturing,
                                                   useStorageTextures,
                                                   label));

    if (evictor != nullptr) {
        atlas->fEvictionCallbacks.emplace_back(evictor);
    }
    SkDEBUGCODE(atlas->validate());
    return atlas;
}

///////////////////////////////////////////////////////////////////////////////

DrawAtlas::DrawAtlas(MaskFormat maskFormat,
                     int width, int height,
                     int plotWidth, int plotHeight,
                     AllowMultitexturing allowMultitexturing,
                     UseStorageTextures useStorageTextures,
                     std::string_view label)
        : fMaskFormat(maskFormat)
        , fTextureWidth(width)
        , fTextureHeight(height)
        , fPlotWidth(plotWidth)
        , fPlotHeight(plotHeight)
        , fUseStorageTextures(useStorageTextures)
        , fLabel(label)
        , fPrevFlushToken(Token::InvalidToken())
        , fFlushesSinceLastUse(0)
        , fMaxPages(allowMultitexturing == AllowMultitexturing::kYes ? kMaxMultitexturePages : 1)
        , fNumActivePages(0) {
    fPlotGridDimensions = {width / plotWidth, height / plotHeight};
    SkASSERTF(fPlotWidth * fPlotGridDimensions.width() == fTextureWidth,
              "Invalid DrawAtlas. Plot width: %d, texture width %d",
              fPlotWidth,
              fTextureWidth);
    SkASSERTF(fPlotHeight * fPlotGridDimensions.height() == fTextureHeight,
              "Invalid DrawAtlas. Plot height: %d, texture height %d",
              fPlotHeight,
              fTextureHeight);
}

inline void DrawAtlas::processEvictionAndResetRects(Plot* plot, EvictionType type) {
    SkASSERT(plot);
    // Process evictions
    if (!plot->isEmpty()) {
        const RecordVisitor visitor{plot};
        for (EvictionCallback* evictor : fEvictionCallbacks) {
            evictor->evict(visitor);
        }
        fEvictionCounter = NextEvictionCounter(fEvictionCounter);
    }
    fPlotMap.remove(plot->plotID());
    switch (type) {
        case EvictionType::kWillBeRemoved:
            break;
        case EvictionType::kReuse_FreeData:
            plot->recycle(/*freeData=*/true);
            fPlotMap[plot->plotID()] = plot;
            break;
        case EvictionType::kReuse_KeepData:
            plot->recycle(/*freeData=*/false);
            fPlotMap[plot->plotID()] = plot;
            break;
    }
}

std::optional<DrawAtlas::AddResult> DrawAtlas::addRectToPage(int pageIdx,
                                                             SkISize size,
                                                             const std::byte* image) {
    SkASSERT(fProxies[pageIdx]);

    // look through all allocated plots for one we can share, in Most Recently Refed order
    int numPlots = this->plotsPerPage();
    for (int i = 0; i < numPlots; ++i) {
        auto* plot = fPages[pageIdx].fPlotArray[i].get();
        if (auto addRectResult = plot->addRect(size, image)) {
            this->makeMRU(pageIdx, i);
            return AddResult{.fRecord   = {plot->plotID(), addRectResult->fEntryID},
                             .fPageIdx  = pageIdx,
                             .fPosition = addRectResult->fPositionInAtlas};
        }
    }
    return {};
}

inline void DrawAtlas::makeMRU(int pageIdx, int plotIndex) {
    SkASSERT(pageIdx < fNumActivePages);
    SkASSERT(plotIndex < this->plotsPerPage());
    if (plotIndex == 0) {
        // Already MRU.
        return;
    }
    std::unique_ptr<Plot>* plotArray = fPages[pageIdx].fPlotArray.get();
    std::unique_ptr<Plot> mruPlot = std::move(plotArray[plotIndex]);
    for (int i = plotIndex; i > 0; --i) {
        plotArray[i] = std::move(plotArray[i - 1]);
        plotArray[i]->updateIndexInPageMru(i);
    }
    plotArray[0] = std::move(mruPlot);
    plotArray[0]->updateIndexInPageMru(0);

    // No MRU update for pages -- since we will always try to add from
    // the front and remove from the back there is no need for MRU.
}

bool DrawAtlas::recordUploads(DrawContext* dc, Recorder* recorder) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    int numPlots = this->plotsPerPage();
    for (int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        auto* plots = fPages[pageIdx].fPlotArray.get();
        for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
            Plot* plot = plots[plotIndex].get();
            if (plot->needsUpload()) {
                TextureProxy* proxy = fProxies[pageIdx].get();
                SkASSERT(proxy);

                const void* dataPtr;
                SkIRect dstRect;
                std::tie(dataPtr, dstRect) = plot->prepareForUpload();
                if (dstRect.isEmpty()) {
                    continue;
                }

                std::vector<MipLevel> levels;
                levels.push_back({dataPtr, plot->rowBytes()});

                // Src and dst colorInfo are the same
                SkColorInfo colorInfo(MaskFormatToColorType(fMaskFormat),
                                      kUnknown_SkAlphaType,
                                      nullptr);
                const UploadSource uploadSource = UploadSource::Make(
                        recorder->priv().caps(), *proxy, colorInfo, colorInfo, levels, dstRect);
                if (!uploadSource.isValid()) {
                    return false;
                }
                if (!dc->recordUpload(recorder,
                                      sk_ref_sp(proxy),
                                      colorInfo,
                                      colorInfo,
                                      uploadSource,
                                      dstRect,
                                      /*ConditionalUploadContext=*/nullptr)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool DrawAtlas::hasRecord(Record record) const {
    auto* plot = findPlot(record.fPlotID);
    if (!plot) {
        return false;
    }
    // Since our eviction is always a full plot it should not be possible to have
    // an AtlasRecord with a valid plot ID but an invalid entry ID for that plot.
    SkASSERT(plot->entryAtlasRect(record.fEntryID).has_value());
    return true;
}

void DrawAtlas::setLastUseTokenBulk(const BulkRecordUseUpdater& updater, Token token) {
    updater.foreach([&](PlotID pid) {
        // it's possible we've added a plot to the updater and subsequently the plot's page
        // was deleted -- so we check to prevent a crash
        if (auto* plot = findPlot(pid)) {
            this->internalSetLastUseToken(plot, token);
        }
    });
}

// Number of atlas-related flushes beyond which we consider a plot to no longer be in use.
//
// This value is somewhat arbitrary -- the idea is to keep it low enough that
// a page with unused plots will get removed reasonably quickly, but allow it
// to hang around for a bit in case it's needed. The assumption is that flushes
// are rare; i.e., we are not continually refreshing the frame.
static constexpr auto kPlotRecentlyUsedCount = 32;
// Number of flushes before we'll try to evict a plot from a sparsely used page.
static constexpr auto kPlotUsedCountBeforeEvict = 8;
// Number of flushes beyond which we'll consider the atlas no longer in use.
static constexpr auto kAtlasRecentlyUsedCount = 128;
DrawAtlas::ErrorCode DrawAtlas::addRect(Recorder* recorder,
                                        SkISize size,
                                        AddResult* result,
                                        const std::byte* image) {
    if (size.width() > fPlotWidth || size.height() > fPlotHeight || size.isEmpty()) {
        return ErrorCode::kError;
    }

    // We permit zero-sized rects to allow inverse fills in the PathAtlases to work,
    // but we don't want to enter them in the Rectanizer. So we handle this special case here.
    // For text this should be caught at a higher level, but if not the only end result
    // will be rendering a degenerate quad.
    if (size.area() == 0) {
        if (fNumActivePages == 0) {
            // Make sure we have a Page for the AtlasLocator to refer to
            this->activateNewPage(recorder);
        }
        const auto& plot = fPages[0].fPlotArray[0];
        // TODO: Special Record value for emtpy?
        result->fRecord.fEntryID = EntryID::kInvalid;
        result->fRecord.fPlotID = plot->plotID();
        result->fPosition = {0, 0};
        return ErrorCode::kSucceeded;
    }

    // Look through each page to see if we can upload without having to flush
    // We prioritize this upload to the first pages, not the most recently used, to make it easier
    // to remove unused pages in reverse page order.
    for (int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        if (auto addRectResult = this->addRectToPage(pageIdx, size, image)) {
            *result = *addRectResult;
            return ErrorCode::kSucceeded;
        }
    }

    // If the above fails, then see if the least recently used plot per page has already been
    // queued for upload if we're at max page allocation, or if the plot has aged out otherwise.
    // We wait until we've grown to the full number of pages to begin evicting already queued
    // plots so that we can maximize the opportunity for reuse.
    // As before we prioritize this upload to the first pages, not the most recently used.
    if (fNumActivePages == this->maxPages()) {
        auto lruPlotIndex = this->plotsPerPage() - 1;
        for (int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
            auto& plot = fPages[pageIdx].fPlotArray[lruPlotIndex];
            if (plot->lastUseToken() < recorder->priv().tokenTracker()->nextFlushToken()) {
                this->processEvictionAndResetRects(plot.get(), EvictionType::kReuse_KeepData);
                auto addRectResult = plot->addRect(size, image);
                SkASSERT(addRectResult);
                result->fRecord = {plot->plotID(), addRectResult->fEntryID};
                result->fPageIdx = pageIdx;
                result->fPosition = addRectResult->fPositionInAtlas;
                // Do this last since it will shuffle the array plot is referring into.
                this->makeMRU(pageIdx, lruPlotIndex);
                return ErrorCode::kSucceeded;
            }
        }
        // All plots are currently in use by the current set of draws, so we need to fail. This
        // gives the Device a chance to snap the current set of uploads and draws, advance the draw
        // token, and call back into this function. The subsequent call will have plots available
        // for fresh uploads.
        return ErrorCode::kTryAgain;
    }

    // If we haven't activated all the available pages, try to create a new one and add to it
    if (!this->activateNewPage(recorder)) {
        return ErrorCode::kError;
    }

    if (auto addRectResult = this->addRectToPage(fNumActivePages - 1, size, image)) {
        *result = *addRectResult;
        return ErrorCode::kSucceeded;
    }
    // If we fail to upload to a newly activated page then something has gone terribly
    // wrong - return an error
    return ErrorCode::kError;
}

std::optional<SkPixmap> DrawAtlas::entryPixmap(Record record,
                                               int padding,
                                               std::optional<SkColor> color) {
    if (auto* plot = findPlot(record.fPlotID)) {
        return plot->entryPixmap(record.fEntryID, padding, color);
    }
    return {};
}

std::optional<std::tuple<int, SkIRect>> DrawAtlas::entryLocation(Record record) const {
    if (auto* plot = findPlot(record.fPlotID)) {
        auto rect = plot->entryAtlasRect(record.fEntryID);
        // Purging is at the plot level. So it shouldn't be possible to have a record that
        // has a valid PlotID for an existing Plot with an EntryID that isn't still valid.
        SkASSERT(rect);
        auto pageIdx = static_cast<int>(plot->plotCoord().fPageIdx);
        return std::make_tuple(pageIdx, *rect);
    }
    return {};
}

void DrawAtlas::compact(Token startTokenForNextFlush) {
    if (fNumActivePages < 1) {
        fPrevFlushToken = startTokenForNextFlush;
        return;
    }

    SkDEBUGCODE(this->validate());

    // For all plots, reset number of flushes since used if used this frame.
    bool atlasUsedThisFlush = false;
    auto numPlots = this->plotsPerPage();
    for (int pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        auto* plots = fPages[pageIndex].fPlotArray.get();
        for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
            auto& plot = plots[plotIndex];
            if (plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                plot->resetFlushesSinceLastUsed();
                atlasUsedThisFlush = true;
            }
        }
    }

    if (atlasUsedThisFlush) {
        fFlushesSinceLastUse = 0;
    } else {
        ++fFlushesSinceLastUse;
    }

    // We only try to compact if the atlas was used in the recently completed flush or
    // hasn't been used in a long time.
    // This is to handle the case where a lot of text or path rendering has occurred but then just
    // a blinking cursor is drawn.
    if (atlasUsedThisFlush || fFlushesSinceLastUse > kAtlasRecentlyUsedCount) {
        TArray<Plot*> availablePlots;
        int lastPageIndex = fNumActivePages - 1;

        // For all pages but the last one, update number of flushes since used, and check to see
        // if there are any in the first pages that the last page can safely upload to.
        for (int pageIndex = 0; pageIndex < lastPageIndex; ++pageIndex) {
            if constexpr (kDumpAtlasData) {
                SkDebugf("page %d: ", pageIndex);
            }

            auto* plots = fPages[pageIndex].fPlotArray.get();
            for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
                auto& plot = plots[plotIndex];
                // Update number of flushes since plot was last used
                // We only increment the 'sinceLastUsed' count for flushes where the atlas was used
                // to avoid deleting everything when we return to text drawing in the blinking
                // cursor case
                if (!plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                    plot->incFlushesSinceLastUsed();
                }

                if constexpr (kDumpAtlasData) {
                    SkDebugf("%d ", plot->flushesSinceLastUsed());
                }

                // Count plots we can potentially upload to in all pages except the last one
                // (the potential compactee).
                if (plot->flushesSinceLastUsed() > kPlotRecentlyUsedCount) {
                    availablePlots.push_back(plot.get());
                }
            }

            if constexpr (kDumpAtlasData) {
                SkDebugf("\n");
            }
        }

        // Count recently used plots in the last page and evict any that are no longer in use.
        // Since we prioritize uploading to the first pages, this will eventually
        // clear out usage of this page unless we have a large need.
        int usedPlots = 0;
        if constexpr (kDumpAtlasData) {
            SkDebugf("page %d: ", lastPageIndex);
        }
        auto* lastPagePlots = fPages[lastPageIndex].fPlotArray.get();
        for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
            auto& plot = lastPagePlots[plotIndex];
            // Update number of flushes since plot was last used
            if (!plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                plot->incFlushesSinceLastUsed();
            }

            if constexpr (kDumpAtlasData) {
                SkDebugf("%d ", plot->flushesSinceLastUsed());
            }

            // If this plot was used recently
            if (plot->flushesSinceLastUsed() <= kPlotRecentlyUsedCount) {
                usedPlots++;
            } else if (plot->lastUseToken() != Token::InvalidToken()) {
                // otherwise if aged out just evict it.
                this->processEvictionAndResetRects(plot.get(), EvictionType::kReuse_KeepData);
            }
        }

        if constexpr (kDumpAtlasData) {
            SkDebugf("\n");
        }

        // If recently used plots in the last page are using less than a quarter of the page, try
        // to evict them if there's available space in lower index pages. Since we prioritize
        // uploading to the first pages, this will eventually clear out usage of this page unless
        // we have a large need.
        if (!availablePlots.empty() && usedPlots && usedPlots <= this->plotsPerPage() / 4) {
            for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
                auto& plot = lastPagePlots[plotIndex];
                // If this plot was used recently
                int plotFlushes = plot->flushesSinceLastUsed();
                if (kPlotUsedCountBeforeEvict <= plotFlushes &&
                    plotFlushes <= kPlotRecentlyUsedCount) {
                    // See if there's room in an lower index page and if so evict.
                    // We need to be somewhat harsh here so that a handful of plots that are
                    // consistently in use don't end up locking the page in memory.
                    if (!availablePlots.empty()) {
                        // The thinking here is that availablePlot will be available to be populated
                        // by cache misses that occur because we evicted plot.
                        this->processEvictionAndResetRects(plot.get(),
                                                           EvictionType::kReuse_FreeData);
                        this->processEvictionAndResetRects(availablePlots.back(),
                                                           EvictionType::kReuse_KeepData);
                        availablePlots.pop_back();
                        --usedPlots;
                    }
                    if (usedPlots == 0 || availablePlots.empty()) {
                        break;
                    }
                }
            }
        }

        // If none of the plots in the last page have been used recently, delete it.
        if (usedPlots == 0) {
            if constexpr (kDumpAtlasData) {
                SkDebugf("delete %d\n", fNumActivePages - 1);
            }

            this->deactivateLastPage();
            fFlushesSinceLastUse = 0;
        }
    }

    fPrevFlushToken = startTokenForNextFlush;
    SkDEBUGCODE(this->validate());
}

bool DrawAtlas::activateNewPage(Recorder* recorder) {
    SkASSERT(fNumActivePages < this->maxPages());
    SkASSERT(!fProxies[fNumActivePages]);

    auto ct = MaskFormatToColorType(fMaskFormat);

    const Caps* caps = recorder->priv().caps();
    auto textureInfo = fUseStorageTextures == UseStorageTextures::kYes
                               ? caps->getDefaultStorageTextureInfo(ct)
                               : caps->getDefaultSampledTextureInfo(ct,
                                                                    Mipmapped::kNo,
                                                                    recorder->priv().isProtected(),
                                                                    Renderable::kNo);
    fProxies[fNumActivePages] = TextureProxy::Make(caps,
                                                   recorder->priv().resourceProvider(),
                                                   {fTextureWidth, fTextureHeight},
                                                   textureInfo,
                                                   fLabel,
                                                   skgpu::Budgeted::kYes);
    if (!fProxies[fNumActivePages]) {
        return false;
    }

    // set up allocated plots
    auto numPlots = this->plotsPerPage();
    fPages[fNumActivePages].fPlotArray = std::make_unique<std::unique_ptr<Plot>[]>(numPlots);

    for (int y = 0; y < fPlotGridDimensions.height(); ++y) {
        for (int x = 0; x < fPlotGridDimensions.width(); ++x) {
            PlotCoord coord {
                .fPageIdx = SkToU8(fNumActivePages),
                .fX       = SkToU8(x),
                .fY       = SkToU8(y)
            };

            // The initial MRU plot will have coord 0, 0.
            int mruIndex = y * fPlotGridDimensions.width() + x;
            auto& plotUP = fPages[fNumActivePages].fPlotArray[mruIndex];
            plotUP = Plot::Make(coord, mruIndex, SkISize{fPlotWidth, fPlotHeight}, fMaskFormat);

            fPlotMap[plotUP->plotID()] = plotUP.get();
        }
    }

    if constexpr (kDumpAtlasData) {
        SkDebugf("activated page#: %d\n", fNumActivePages);
    }

    ++fNumActivePages;
    SkDEBUGCODE(this->validate());
    return true;
}

inline void DrawAtlas::deactivateLastPage() {
    SkASSERT(fNumActivePages);

    int lastPageIndex = fNumActivePages - 1;

    auto numPlots = this->plotsPerPage();

    auto* plots = fPages[lastPageIndex].fPlotArray.get();
    for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
        this->processEvictionAndResetRects(plots[plotIndex].get(), EvictionType::kWillBeRemoved);
    }
    fPages[lastPageIndex].fPlotArray = {};

    // remove ref to the texture proxy
    fProxies[lastPageIndex].reset();
    --fNumActivePages;
}

void DrawAtlas::markUsedPlotsAsFull() {
    auto numPlots = this->plotsPerPage();
    for (int pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        auto* plots = fPages[pageIndex].fPlotArray.get();
        for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
            plots[plotIndex]->markFullIfUsed();
        }
    }
}

void DrawAtlas::freeGpuResources(Token token) {
    auto numPlots = this->plotsPerPage();
    for (int pageIndex = fNumActivePages - 1; pageIndex >= 0; --pageIndex) {
        auto* plots = fPages[pageIndex].fPlotArray.get();
        for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
            if (plots[plotIndex]->lastUseToken().inInterval(fPrevFlushToken, token)) {
                // This page is in use, and we can only deactivate pages from high index
                // to low index, so bail.
                return;
            }
        }
        this->deactivateLastPage();
    }
}

void DrawAtlas::evictAllPlots() {
    auto numPlots = this->plotsPerPage();
    for (int pageIndex = fNumActivePages - 1; pageIndex >= 0; --pageIndex) {
        auto* plots = fPages[pageIndex].fPlotArray.get();
        for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
            this->processEvictionAndResetRects(plots[plotIndex].get(),
                                               EvictionType::kReuse_FreeData);
        }
    }
}

#if defined(GPU_TEST_UTILS)
int DrawAtlas::numAllocatedPlots() const {
    return this->iteratePlots([](const Plot* plot) {
        return plot->hasAllocation();
    });
}

int DrawAtlas::numNonEmptyPlots() const {
    return this->iteratePlots([](const Plot* plot) {
        return !plot->isEmpty();
    });
}
#endif

inline DrawAtlas::PlotID DrawAtlas::Plot::NextPlotID() {
    static std::atomic<std::underlying_type_t<PlotID>> gID;
    auto next = gID.fetch_add(1, std::memory_order_relaxed) + 1;
    return static_cast<PlotID>(next);
}

DrawAtlas::Plot::Plot(PlotCoord coord,
                      int indexInPageMru,
                      SkISize plotDimensions,
                      MaskFormat maskFormat)
        : fRectanizer{plotDimensions.width(), plotDimensions.height()}
        , fPlotDimensions{plotDimensions}
        , fPlotCoord{coord}
        , fIndexInPageMru{indexInPageMru}
        , fMaskFormat{maskFormat} {
    // We expect the allocated dimensions to be a multiple of 4 bytes
    SkASSERT((this->rowBytes() & 0x3) == 0);
}

DrawAtlas::Plot::~Plot() = default;

inline DrawAtlas::EntryID DrawAtlas::NextEntryID(EntryID e) {
    auto v = static_cast<std::underlying_type_t<EntryID>>(e);
    return static_cast<EntryID>(v + 1);
}

inline std::optional<std::tuple<DrawAtlas::EntryID, SkIPoint>> DrawAtlas::Plot::makeEntry(
        SkISize size) {
    SkIPoint16 loc;
    if (!fRectanizer.addRect(size.width(), size.height(), &loc)) {
        return {};
    }
    fPrevEntryID = NextEntryID(fPrevEntryID);
    SkASSERT(!fEntries.find(fPrevEntryID));
    auto rect = SkIRect::MakePtSize({loc.fX, loc.fY}, size);
    fEntries[fPrevEntryID] = rect;
    return std::make_tuple(fPrevEntryID, SkIPoint{loc.fX, loc.fY});
}

std::optional<DrawAtlas::Plot::AddResult> DrawAtlas::Plot::addRect(SkISize size,
                                                                   const std::byte* image) {
    auto makeResult = this->makeEntry(size);
    if (!makeResult) {
        return {};
    }
    const auto& [entry, point] = *makeResult;

    AddResult result{entry, point + this->topLeftInAtlas()};

    if (image) {
        auto   dataPtr       = this->dataAt(point);
        auto   [w, h]        = size;
        auto   bpp           = this->bpp();
        size_t imageRowBytes = w * bpp;
        size_t plotRowBytes  = this->rowBytes();

        // copy into the data buffer, swizzling as we go if this is ARGB data
        constexpr bool kBGRAIsNative = kN32_SkColorType == kBGRA_8888_SkColorType;
        if (4 == bpp && kBGRAIsNative) {
            for (int i = 0; i < h; ++i, dataPtr += plotRowBytes, image += imageRowBytes) {
                SkOpts::RGBA_to_BGRA(reinterpret_cast<uint32_t*>(dataPtr),
                                     reinterpret_cast<const uint32_t*>(image),
                                     w);
            }
        } else {
            for (int i = 0; i < h; ++i, dataPtr += plotRowBytes, image += imageRowBytes) {
                memcpy(dataPtr, image, imageRowBytes);
            }
        }
        this->dirty(SkIRect::MakePtSize(point, size));
    }
    return result;
}

std::optional<SkIRect> DrawAtlas::Plot::entryAtlasRect(DrawAtlas::EntryID id) const {
    auto* rect16 = fEntries.find(id);
    if (!rect16) {
        return {};
    }
    return SkIRect(*rect16).makeOffset(this->topLeftInAtlas());
}

std::optional<SkPixmap> DrawAtlas::Plot::entryPixmap(EntryID id,
                                                     int padding,
                                                     std::optional<SkColor> initialColor) {
    SkASSERT(padding >= 0);
    auto* rect16 = fEntries.find(id);
    if (!rect16) {
        return {};
    }
    auto rect = SkIRect(*rect16);
    this->dirty(rect);

    auto info = SkImageInfo::Make(rect.size(),
                                  MaskFormatToColorType(fMaskFormat),
                                  kOpaque_SkAlphaType);
    SkPixmap outerPM{info, this->dataAt(rect.topLeft()), this->rowBytes()};
    if (initialColor) {
#if defined(SK_DEBUG)
        if (*initialColor == 0) {
            SkDebugf("Plot Data: potential redundant clear of Plot to zero.");
        }
#endif
        outerPM.erase(*initialColor);
    }
    SkPixmap innerPM;
    SkIRect innerRect = SkIRect::MakeSize(rect.size()).makeInset(padding, padding);
    SkAssertResult(outerPM.extractSubset(&innerPM, innerRect));
    return innerPM;
}

std::byte* DrawAtlas::Plot::dataAt(SkIPoint plotPoint) {
    auto rb = this->rowBytes();
    if (!fData) {
        // make_unique will iniit to 0.
        fData = std::make_unique<std::byte[]>(rb * fPlotDimensions.height());
    }

    SkASSERT(plotPoint.fX >= 0 && plotPoint.fX < fPlotDimensions.width());
    SkASSERT(plotPoint.fY >= 0 && plotPoint.fY < fPlotDimensions.height());

    size_t offset = rb * plotPoint.y() + this->bpp() * plotPoint.x();

    return fData.get() + offset;
}

std::pair<const std::byte*, SkIRect> DrawAtlas::Plot::prepareForUpload() {
    // We should only be issuing uploads if we are dirty
    SkASSERT(!fDirtyRect.isEmpty());
    if (!fData) {
        return {nullptr, {}};
    }
    SkIRect dirtyRect = this->alignedDirtyRect();
    auto* dataPtr = this->dataAt(dirtyRect.topLeft());

    fDirtyRect = {};
    fIsFull = false;

    return {dataPtr, dirtyRect.makeOffset(this->topLeftInAtlas())};
}

void DrawAtlas::Plot::recycle(bool freeData) {
    fRectanizer.reset();
    fPlotID = NextPlotID();
    fLastUse = Token::InvalidToken();

    if (freeData) {
        fData = {};
    } else if (fData) {
        sk_bzero(fData.get(), this->rowBytes() * fPlotDimensions.height());
    }

    fEntries.reset();
    fPrevEntryID = EntryID::kInvalid;

    fDirtyRect = {};
    fIsFull = false;
}

}  // namespace skgpu::graphite
