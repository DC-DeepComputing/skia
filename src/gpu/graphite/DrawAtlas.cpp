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

#ifdef SK_DEBUG
void DrawAtlas::validate(const AtlasLocator& atlasLocator) const {
    // Verify that the plotIndex stored in the PlotLocator is consistent with the glyph rectangle
    int numPlotsX = fTextureWidth / fPlotWidth;
    int numPlotsY = fTextureHeight / fPlotHeight;

    int plotIndex = atlasLocator.plotIndex();
    auto topLeft = atlasLocator.topLeft();
    int plotX = topLeft.x() / fPlotWidth;
    int plotY = topLeft.y() / fPlotHeight;
    SkASSERT(plotIndex == (numPlotsY - plotY - 1) * numPlotsX + (numPlotsX - plotX - 1));
}
#endif

std::unique_ptr<DrawAtlas> DrawAtlas::Make(MaskFormat maskFormat,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           GenerationCounter* generationCounter,
                                           AllowMultitexturing allowMultitexturing,
                                           UseStorageTextures useStorageTextures,
                                           PlotEvictionCallback* evictor,
                                           std::string_view label) {
    std::unique_ptr<DrawAtlas> atlas(new DrawAtlas(maskFormat,
                                                   width, height,
                                                   plotWidth, plotHeight,
                                                   generationCounter,
                                                   allowMultitexturing,
                                                   useStorageTextures,
                                                   label));

    if (evictor != nullptr) {
        atlas->fEvictionCallbacks.emplace_back(evictor);
    }
    return atlas;
}

std::unique_ptr<DrawAtlas> DrawAtlas::Make(MaskFormat maskFormat,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           AllowMultitexturing allowMultitexturing,
                                           UseStorageTextures useStorageTextures,
                                           EvictionCallback* evictor,
                                           std::string_view label) {
    // Generate a temporary counter since we don't need it externally
    static GenerationCounter gTempCounter;
    std::unique_ptr<DrawAtlas> atlas(new DrawAtlas(maskFormat,
                                                   width, height,
                                                   plotWidth, plotHeight,
                                                   &gTempCounter,
                                                   allowMultitexturing,
                                                   useStorageTextures,
                                                   label));

    if (evictor != nullptr) {
        atlas->fRecordEvictionCallbacks.emplace_back(evictor);
    }
    return atlas;
}

///////////////////////////////////////////////////////////////////////////////
static uint32_t next_id() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidGenID);
    return id;
}
DrawAtlas::DrawAtlas(MaskFormat maskFormat,
                     int width, int height,
                     int plotWidth, int plotHeight,
                     GenerationCounter* generationCounter,
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
        , fAtlasID(next_id())
        , fGenerationCounter(generationCounter)
        , fAtlasGeneration(fGenerationCounter->next())
        , fPrevFlushToken(Token::InvalidToken())
        , fFlushesSinceLastUse(0)
        , fMaxPages(allowMultitexturing == AllowMultitexturing::kYes ? kMaxMultitexturePages : 1)
        , fNumActivePages(0) {
    int numPlotsX = width/plotWidth;
    int numPlotsY = height/plotHeight;
    SkASSERT(numPlotsX * numPlotsY <= kMaxPlots);
    SkASSERTF(fPlotWidth * numPlotsX == fTextureWidth,
             "Invalid DrawAtlas. Plot width: %d, texture width %d", fPlotWidth, fTextureWidth);
    SkASSERTF(fPlotHeight * numPlotsY == fTextureHeight,
              "Invalid DrawAtlas. Plot height: %d, texture height %d", fPlotHeight, fTextureHeight);

    fNumPlots = numPlotsX * numPlotsY;

    this->createPages(generationCounter);
}

void DrawAtlas::processEvictionAndResetRects(Plot* plot, bool freeData) {
    this->processEvictionAndResetRects(plot, freeData ? EvictionType::kReuse_FreeData
                                                     : EvictionType::kReuse_KeepData);
}

static uint32_t NextEvictionCounter(uint32_t counter) {
    if (counter == std::numeric_limits<uint32_t>::max()) {
        return 1;
    }
    return counter + 1;
}

void DrawAtlas::processEvictionAndResetRects(Plot* plot, EvictionType type) {
    if (!plot->isEmpty()) {
        if (!fEvictionCallbacks.empty()) {
            PlotLocator locator = plot->plotLocator();
            for (auto* cb : fEvictionCallbacks) {
                cb->evict(locator);
            }
        }
        if (!fRecordEvictionCallbacks.empty()) {
            RecordVisitor visitor(plot);
            for (auto* cb : fRecordEvictionCallbacks) {
                cb->evict(visitor);
            }
        }
        fEvictionCounter = NextEvictionCounter(fEvictionCounter);
        fAtlasGeneration = fGenerationCounter->next();
    }

    fPlotMap.remove(plot->plotID());
    if (type == EvictionType::kWillBeRemoved) {
        plot->recycle(/*freeData=*/true);
    } else {
        plot->recycle(type == EvictionType::kReuse_FreeData);
        fPlotMap.set(plot->plotID(), plot);
    }
}

inline void DrawAtlas::updatePlot(Plot* plot, AtlasLocator* atlasLocator) {
    int pageIdx = plot->pageIndex();
    this->makeMRU(plot, pageIdx);

    // The actual upload will be created in recordUploads().

    atlasLocator->updatePlotLocator(plot->plotLocator());
    SkDEBUGCODE(this->validate(*atlasLocator);)
}

bool DrawAtlas::addRectToPage(unsigned int pageIdx, int width, int height,
                              AtlasLocator* atlasLocator) {
    SkASSERT(fProxies[pageIdx]);

    // look through all allocated plots for one we can share, in Most Recently Refed order
    PlotList::Iter plotIter;
    plotIter.init(fPages[pageIdx].fPlotList, PlotList::Iter::kHead_IterStart);

    for (Plot* plot = plotIter.get(); plot; plot = plotIter.next()) {
        if (plot->addRect(width, height, atlasLocator)) {
            this->updatePlot(plot, atlasLocator);
            return true;
        }
    }

    return false;
}

bool DrawAtlas::recordUploads(DrawContext* dc, Recorder* recorder) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    const SkColorType maskCT = MaskFormatToColorType(fMaskFormat);
    // Src and dst colorInfo are the same
    const SkColorInfo colorInfo(maskCT, kUnknown_SkAlphaType, nullptr);
    for (uint32_t pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        PlotList::Iter plotIter;
        plotIter.init(fPages[pageIdx].fPlotList, PlotList::Iter::kHead_IterStart);

        Swizzle readSwizzle = ReadSwizzleForColorType(maskCT, fProxies[pageIdx]->format());
        TextureProxyView view{fProxies[pageIdx], readSwizzle};
        for (Plot* plot = plotIter.get(); plot; plot = plotIter.next()) {
            if (plot->needsUpload()) {
                const void* dataPtr;
                SkIRect dstRect;
                std::tie(dataPtr, dstRect) = plot->prepareForUpload();
                if (dstRect.isEmpty()) {
                    continue;
                }

                MipLevel level{dataPtr, plot->rowBytes()};
                const UploadSource uploadSource = UploadSource::Make(
                        recorder->priv().caps(), view, colorInfo, colorInfo,
                        SkSpan(&level, 1), dstRect);
                if (!dc->recordUpload(recorder, uploadSource)) {
                    return false;
                }
            }
        }
    }
    return true;
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
                                        int width, int height,
                                        AtlasLocator* atlasLocator) {
    AddResult res;
    ErrorCode ec = this->addRect(recorder, {width, height}, &res);
    if (ec == ErrorCode::kSucceeded) {
        if (width == 0 || height == 0) {
            atlasLocator->updateRect(SkIRect::MakeEmpty());
            atlasLocator->updatePlotLocator(fPages[0].fPlotList.head()->plotLocator());
            return ErrorCode::kSucceeded;
        }
        auto locOpt = this->entryLocation(res.fRecord);
        SkASSERT(locOpt);
        auto [pageIdx, rect] = *locOpt;
        atlasLocator->updateRect(rect);
        Plot* plot = this->findPlot(res.fRecord);
        atlasLocator->updatePlotLocator(plot->plotLocator());
        atlasLocator->updateRecord(res.fRecord);
    }
    return ec;
}

DrawAtlas::ErrorCode DrawAtlas::addRect(Recorder* recorder,
                                        SkISize size,
                                        AddResult* addResult,
                                        const std::byte* image) {
    if (size.width() > fPlotWidth || size.height() > fPlotHeight || size.width() < 0 || size.height() < 0) {
        return ErrorCode::kError;
    }

    if (size.isEmpty()) {
        if (fNumActivePages == 0) {
            this->activateNewPage(recorder);
        }
        addResult->fRecord = Record();
        addResult->fPageIdx = 0;
        addResult->fPosition = SkIPoint::Make(0, 0);
        return ErrorCode::kSucceeded;
    }

    for (unsigned int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        auto plotResult = this->addRectToPage(pageIdx, size, image);
        if (plotResult) {
            *addResult = *plotResult;
            return ErrorCode::kSucceeded;
        }
    }

    if (fNumActivePages == this->maxPages()) {
        for (unsigned int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
            Plot* plot = fPages[pageIdx].fPlotList.tail();
            SkASSERT(plot);
            if (plot->lastUseToken() < recorder->priv().tokenTracker()->nextFlushToken()) {
                this->processEvictionAndResetRects(plot, EvictionType::kReuse_KeepData);
                auto plotResult = plot->addRect(size, image);
                SkASSERT(plotResult);
                this->makeMRU(plot);
                addResult->fRecord = Record{plot->plotID(), plotResult->fEntryID};
                addResult->fPageIdx = pageIdx;
                addResult->fPosition = plotResult->fPositionInAtlas;
                return ErrorCode::kSucceeded;
            }
        }
    } else {
        if (!this->activateNewPage(recorder)) {
            return ErrorCode::kError;
        }
        auto plotResult = this->addRectToPage(fNumActivePages - 1, size, image);
        if (plotResult) {
            *addResult = *plotResult;
            return ErrorCode::kSucceeded;
        } else {
            return ErrorCode::kError;
        }
    }

    if (!fNumActivePages) {
        return ErrorCode::kError;
    }

    return ErrorCode::kTryAgain;
}

std::optional<DrawAtlas::AddResult> DrawAtlas::addRectToPage(int pageIdx, SkISize size, const std::byte* image) {
    SkASSERT(fProxies[pageIdx]);

    PlotList::Iter plotIter;
    plotIter.init(fPages[pageIdx].fPlotList, PlotList::Iter::kHead_IterStart);

    for (Plot* plot = plotIter.get(); plot; plot = plotIter.next()) {
        auto plotResult = plot->addRect(size, image);
        if (plotResult) {
            this->makeMRU(plot);
            return AddResult{
                Record{plot->plotID(), plotResult->fEntryID},
                pageIdx,
                plotResult->fPositionInAtlas
            };
        }
    }

    return std::nullopt;
}

DrawAtlas::ErrorCode DrawAtlas::addToAtlas(Recorder* recorder,
                                           int width, int height, const void* image,
                                           AtlasLocator* atlasLocator) {
    AddResult res;
    ErrorCode ec = this->addRect(recorder, {width, height}, &res, (const std::byte*)image);
    if (ec == ErrorCode::kSucceeded) {
        if (width == 0 || height == 0) {
            atlasLocator->updateRect(SkIRect::MakeEmpty());
            atlasLocator->updatePlotLocator(fPages[0].fPlotList.head()->plotLocator());
            return ErrorCode::kSucceeded;
        }
        auto locOpt = this->entryLocation(res.fRecord);
        SkASSERT(locOpt);
        auto [pageIdx, rect] = *locOpt;
        atlasLocator->updateRect(rect);
        Plot* plot = this->findPlot(res.fRecord);
        atlasLocator->updatePlotLocator(plot->plotLocator());
        atlasLocator->updateRecord(res.fRecord);
    }
    return ec;
}

SkPixmap DrawAtlas::prepForRender(const AtlasLocator& locator,
                                  int padding,
                                  std::optional<SkColor> initialColor) {
    auto pmOpt = this->entryPixmap(locator.record(), padding, initialColor);
    SkASSERT(pmOpt);
    return *pmOpt;
}

bool DrawAtlas::hasRecord(Record record) const {
    Plot* plot = this->findPlot(record.fPlotID);
    if (!plot) {
        return false;
    }
    return plot->entryAtlasRect(record.fEntryID).has_value();
}

std::optional<SkPixmap> DrawAtlas::entryPixmap(Record record, int padding, std::optional<SkColor> clearColor) {
    Plot* plot = this->findPlot(record.fPlotID);
    if (!plot) {
        return std::nullopt;
    }
    return plot->entryPixmap(record.fEntryID, padding, clearColor);
}

std::optional<std::tuple<int, SkIRect>> DrawAtlas::entryLocation(Record record) const {
    Plot* plot = this->findPlot(record.fPlotID);
    if (!plot) {
        return std::nullopt;
    }
    auto rectOpt = plot->entryAtlasRect(record.fEntryID);
    if (!rectOpt) {
        return std::nullopt;
    }
    return std::make_tuple(plot->plotCoord().fPageIdx, *rectOpt);
}

void DrawAtlas::compact(Token startTokenForNextFlush) {
    if (fNumActivePages < 1) {
        fPrevFlushToken = startTokenForNextFlush;
        return;
    }

    // For all plots, reset number of flushes since used if used this frame.
    PlotList::Iter plotIter;
    bool atlasUsedThisFlush = false;
    for (uint32_t pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        while (Plot* plot = plotIter.get()) {
            // Reset number of flushes since used
            if (plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                plot->resetFlushesSinceLastUsed();
                atlasUsedThisFlush = true;
            }

            plotIter.next();
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
        uint32_t lastPageIndex = fNumActivePages - 1;

        // For all pages but the last one, update number of flushes since used, and check to see
        // if there are any in the first pages that the last page can safely upload to.
        for (uint32_t pageIndex = 0; pageIndex < lastPageIndex; ++pageIndex) {
            if constexpr (kDumpAtlasData) {
                SkDebugf("page %u: ", pageIndex);
            }

            plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
            while (Plot* plot = plotIter.get()) {
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
                    availablePlots.push_back() = plot;
                }

                plotIter.next();
            }

            if constexpr (kDumpAtlasData) {
                SkDebugf("\n");
            }
        }

        // Count recently used plots in the last page and evict any that are no longer in use.
        // Since we prioritize uploading to the first pages, this will eventually
        // clear out usage of this page unless we have a large need.
        plotIter.init(fPages[lastPageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        unsigned int usedPlots = 0;
        if constexpr (kDumpAtlasData) {
            SkDebugf("page %u: ", lastPageIndex);
        }
        while (Plot* plot = plotIter.get()) {
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
                this->processEvictionAndResetRects(plot, /*freeData=*/false);
            }
            plotIter.next();
        }

        if constexpr (kDumpAtlasData) {
            SkDebugf("\n");
        }

        // If recently used plots in the last page are using less than a quarter of the page, try
        // to evict them if there's available space in lower index pages. Since we prioritize
        // uploading to the first pages, this will eventually clear out usage of this page unless
        // we have a large need.
        if (!availablePlots.empty() && usedPlots && usedPlots <= fNumPlots / 4) {
            plotIter.init(fPages[lastPageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
            while (Plot* plot = plotIter.get()) {
                // If this plot was used recently
                int plotFlushes = plot->flushesSinceLastUsed();
                if (kPlotUsedCountBeforeEvict <= plotFlushes &&
                    plotFlushes <= kPlotRecentlyUsedCount) {
                    // See if there's room in an lower index page and if so evict.
                    // We need to be somewhat harsh here so that a handful of plots that are
                    // consistently in use don't end up locking the page in memory.
                    if (!availablePlots.empty()) {
                        this->processEvictionAndResetRects(plot, /*freeData=*/true);
                        this->processEvictionAndResetRects(availablePlots.back(),
                                                           /*freeData=*/false);
                        availablePlots.pop_back();
                        --usedPlots;
                    }
                    if (usedPlots == 0 || availablePlots.empty()) {
                        break;
                    }
                }
                plotIter.next();
            }
        }

        // If none of the plots in the last page have been used recently, delete it.
        if (usedPlots == 0) {
            if constexpr (kDumpAtlasData) {
                SkDebugf("delete %u\n", fNumActivePages-1);
            }

            this->deactivateLastPage();
            fFlushesSinceLastUse = 0;
        }
    }

    fPrevFlushToken = startTokenForNextFlush;
}

bool DrawAtlas::createPages(GenerationCounter* generationCounter) {
    SkASSERT(SkIsPow2(fTextureWidth) && SkIsPow2(fTextureHeight));

    int numPlotsX = fTextureWidth/fPlotWidth;
    int numPlotsY = fTextureHeight/fPlotHeight;

    for (uint32_t i = 0; i < this->maxPages(); ++i) {
        // Proxies are uncreated at first
        fProxies[i] = nullptr;

        // set up allocated plots
        fPages[i].fPlotArray = std::make_unique<std::unique_ptr<Plot>[]>(numPlotsX * numPlotsY);

        auto* currPlot = fPages[i].fPlotArray.get();
        for (int y = numPlotsY - 1, r = 0; y >= 0; --y, ++r) {
            for (int x = numPlotsX - 1, c = 0; x >= 0; --x, ++c) {
                uint32_t plotIndex = r * numPlotsX + c;
                *currPlot = Plot::Make({(int)i, x, y},
                                       plotIndex,
                                       {fPlotWidth, fPlotHeight},
                                       fMaskFormat);

                // build LRU list
                fPages[i].fPlotList.addToHead(currPlot->get());
                fPlotMap.set((*currPlot)->plotID(), currPlot->get());
                ++currPlot;
            }
        }
    }

    return true;
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

    if constexpr (kDumpAtlasData) {
        SkDebugf("activated page#: %u\n", fNumActivePages);
    }

    // Recycle all plots on this page to start fresh and register them in fPlotMap
    for (unsigned int plotIndex = 0; plotIndex < fNumPlots; ++plotIndex) {
        Plot* plot = fPages[fNumActivePages].fPlotArray[plotIndex].get();
        plot->recycle(/*freeData=*/true);
        fPlotMap.set(plot->plotID(), plot);
    }

    ++fNumActivePages;
    return true;
}

inline void DrawAtlas::deactivateLastPage() {
    SkASSERT(fNumActivePages);

    uint32_t lastPageIndex = fNumActivePages - 1;

    int numPlotsX = fTextureWidth/fPlotWidth;
    int numPlotsY = fTextureHeight/fPlotHeight;

    for (int r = 0; r < numPlotsY; ++r) {
        for (int c = 0; c < numPlotsX; ++c) {
            uint32_t plotIndex = r * numPlotsX + c;

            Plot* currPlot = fPages[lastPageIndex].fPlotArray[plotIndex].get();
            this->processEvictionAndResetRects(currPlot, EvictionType::kWillBeRemoved);
            currPlot->resetFlushesSinceLastUsed();

            fPages[lastPageIndex].fPlotList.remove(currPlot);
            fPages[lastPageIndex].fPlotList.addToHead(currPlot);
        }
    }

    // remove ref to the texture proxy
    fProxies[lastPageIndex].reset();
    --fNumActivePages;
}

void DrawAtlas::markUsedPlotsAsFull() {
    PlotList::Iter plotIter;
    for (uint32_t pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        while (Plot* plot = plotIter.get()) {
            plot->markFullIfUsed();
            plotIter.next();
        }
    }
}

void DrawAtlas::freeGpuResources(Token token) {
    PlotList::Iter plotIter;
    for (int pageIndex = (int)(fNumActivePages)-1; pageIndex >= 0; --pageIndex) {
        const Page& currPage = fPages[pageIndex];
        plotIter.init(currPage.fPlotList, PlotList::Iter::kHead_IterStart);
        while (Plot* plot = plotIter.get()) {
            if (plot->lastUseToken().inInterval(fPrevFlushToken, token)) {
                // This page is in use and we can only deactivate pages from high index
                // to low index, so bail.
                return;
            }
            plotIter.next();
        }
        this->deactivateLastPage();
    }
}

void DrawAtlas::evictAllPlots() {
    PlotList::Iter plotIter;
    for (uint32_t pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        plotIter.init(fPages[pageIndex].fPlotList, PlotList::Iter::kHead_IterStart);
        while (Plot* plot = plotIter.get()) {
            this->processEvictionAndResetRects(plot, /*freeData=*/true);
            plotIter.next();
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

DrawAtlas::PlotID DrawAtlas::Plot::NextPlotID() {
    static std::atomic<uint32_t> gNextPlotID{1};
    uint32_t id;
    do {
        id = gNextPlotID.fetch_add(1, std::memory_order_relaxed);
    } while (id == static_cast<uint32_t>(PlotID::kInvalid));
    return static_cast<PlotID>(id);
}

DrawAtlas::EntryID DrawAtlas::Plot::NextEntryID(DrawAtlas::EntryID entryID) {
    auto value = static_cast<std::underlying_type_t<EntryID>>(entryID);
    if (value == std::numeric_limits<std::underlying_type_t<EntryID>>::max()) {
        return static_cast<EntryID>(1);
    }
    value++;
    if (value == static_cast<std::underlying_type_t<EntryID>>(EntryID::kInvalid)) {
        value++;
    }
    return static_cast<EntryID>(value);
}

DrawAtlas::Plot::Plot(PlotCoord plotCoord,
                      uint32_t plotIndex,
                      SkISize plotDimensions,
                      MaskFormat maskFormat)
        : fRectanizer(plotDimensions.width(), plotDimensions.height())
        , fPlotDimensions(plotDimensions)
        , fPlotIndex(plotIndex)
        , fPlotCoord(plotCoord)
        , fMaskFormat(maskFormat) {
    SkASSERT(((plotDimensions.width() * this->bpp()) & 0x3) == 0);
    SkASSERT(this->bpp() == 1 || this->bpp() == 2 || this->bpp() == 4);
    fDirtyRect.setEmpty();
}

DrawAtlas::Plot::~Plot() = default;

std::optional<DrawAtlas::Plot::AddResult> DrawAtlas::Plot::addRect(SkISize size, const std::byte* image) {
    auto entryOpt = this->makeEntry(size);
    if (!entryOpt) {
        return std::nullopt;
    }
    auto [entryID, pos] = *entryOpt;
    auto offset = this->topLeftInAtlas();
    SkIPoint absPos = pos + offset;

    if (image) {
        unsigned char* dataPtr = (unsigned char*)this->dataAt(absPos);
        const unsigned char* imagePtr = (const unsigned char*)image;
        auto bpp = this->bpp();
        size_t imageRB = size.width() * bpp;
        size_t plotRB = this->rowBytes();

        constexpr bool kBGRAIsNative = kN32_SkColorType == kBGRA_8888_SkColorType;
        if (bpp == 4 && kBGRAIsNative) {
            for (int i = 0; i < size.height(); ++i) {
                SkOpts::RGBA_to_BGRA((uint32_t*)dataPtr, (const uint32_t*)imagePtr, size.width());
                dataPtr += plotRB;
                imagePtr += imageRB;
            }
        } else {
            for (int i = 0; i < size.height(); ++i) {
                memcpy(dataPtr, imagePtr, imageRB);
                dataPtr += plotRB;
                imagePtr += imageRB;
            }
        }
    }

    SkIRect localRect = SkIRect::MakePtSize(pos, size);
    fDirtyRect.join(localRect);

    return AddResult{entryID, absPos};
}

std::optional<SkIRect> DrawAtlas::Plot::entryAtlasRect(EntryID entryID) const {
    const Rect16* rect = fEntries.find(entryID);
    if (!rect) {
        return std::nullopt;
    }
    SkIRect r = *rect;
    r.offset(this->topLeftInAtlas());
    return r;
}

std::optional<SkPixmap> DrawAtlas::Plot::entryPixmap(EntryID entryID,
                                                     int padding,
                                                     std::optional<SkColor> clearColor) {
    auto rOpt = this->entryAtlasRect(entryID);
    if (!rOpt) {
        return std::nullopt;
    }
    SkIRect r = *rOpt;
    SkASSERT(padding >= 0);
    auto info = SkImageInfo::Make(
            r.size(), MaskFormatToColorType(fMaskFormat), kOpaque_SkAlphaType);
    SkPixmap outerPM{info, this->dataAt(r.topLeft()), this->rowBytes()};
    if (clearColor) {
#if defined(SK_DEBUG)
        if (*clearColor == 0) {
            SkDebugf("Plot Data: potential redudant clear of Plot to zero.");
        }
#endif
        outerPM.erase(*clearColor);
    }
    SkPixmap innerPM;
    SkIRect rect = SkIRect::MakeSize(outerPM.dimensions()).makeInset(padding, padding);
    SkAssertResult(outerPM.extractSubset(&innerPM, rect));
    return innerPM;
}

bool DrawAtlas::Plot::addRect(int width, int height, AtlasLocator* atlasLocator) {
    auto res = this->addRect({width, height}, nullptr);
    if (!res) {
        return false;
    }
    auto rect = SkIRect::MakePtSize(res->fPositionInAtlas, {width, height});
    atlasLocator->updateRect(rect);
    atlasLocator->updatePlotLocator(this->plotLocator());
    atlasLocator->updateRecord(Record(fPlotID, res->fEntryID));
    return true;
}

void DrawAtlas::Plot::copySubImage(const AtlasLocator& al, const void* image) {
    const unsigned char* imagePtr = (const unsigned char*)image;
    unsigned char* dataPtr = (unsigned char*)this->dataAt(al.topLeft());
    int width = al.width();
    int height = al.height();
    auto bpp = this->bpp();
    size_t imageRB = width * bpp;
    size_t plotRB = this->rowBytes();

    // copy into the data buffer, swizzling as we go if this is ARGB data
    constexpr bool kBGRAIsNative = kN32_SkColorType == kBGRA_8888_SkColorType;
    if (bpp == 4 && kBGRAIsNative) {
        for (int i = 0; i < height; ++i) {
            SkOpts::RGBA_to_BGRA((uint32_t*)dataPtr, (const uint32_t*)imagePtr, width);
            dataPtr += plotRB;
            imagePtr += imageRB;
        }
    } else {
        for (int i = 0; i < height; ++i) {
            memcpy(dataPtr, imagePtr, imageRB);
            dataPtr += plotRB;
            imagePtr += imageRB;
        }
    }

    SkIRect localRect = SkIRect::MakePtSize(al.topLeft() - this->topLeftInAtlas(), {width, height});
    fDirtyRect.join(localRect);
}

std::byte* DrawAtlas::Plot::dataAt(SkIPoint atlasPoint) {
    if (!fData) {
        fData = std::make_unique<std::byte[]>(this->bpp() * fPlotDimensions.area());
    }

    auto localPoint = atlasPoint - this->topLeftInAtlas();
    SkASSERT(localPoint.fX >= 0 && localPoint.fX < fPlotDimensions.width());
    SkASSERT(localPoint.fY >= 0 && localPoint.fY < fPlotDimensions.height());

    size_t offset = this->bpp() * (localPoint.fY * fPlotDimensions.width() + localPoint.fX);
    return fData.get() + offset;
}

SkPixmap DrawAtlas::Plot::prepForRender(const AtlasLocator& al,
                                        int padding,
                                        std::optional<SkColor> initialColor) {
    Record r = al.record();
    if (r.fPlotID != PlotID::kInvalid && r.fEntryID != EntryID::kInvalid) {
        auto pmOpt = this->entryPixmap(r.fEntryID, padding, initialColor);
        if (pmOpt) {
            return *pmOpt;
        }
    }
    SkASSERT(padding >= 0);
    auto info = SkImageInfo::Make(
            al.dimensions(), MaskFormatToColorType(fMaskFormat), kOpaque_SkAlphaType);
    SkPixmap outerPM{info, this->dataAt(al.topLeft()), this->rowBytes()};
    if (initialColor) {
        outerPM.erase(*initialColor);
    }
    SkPixmap innerPM;
    SkIRect rect = SkIRect::MakeSize(outerPM.dimensions()).makeInset(padding, padding);
    SkAssertResult(outerPM.extractSubset(&innerPM, rect));
    return innerPM;
}

std::pair<const void*, SkIRect> DrawAtlas::Plot::prepareForUpload() {
    // We should only be issuing uploads if we are dirty
    SkASSERT(!fDirtyRect.isEmpty());
    if (!fData) {
        return {nullptr, {}};
    }
    auto aligned = this->alignedDirtyRect();

    const std::byte* dataPtr = fData.get();
    dataPtr += this->rowBytes() * aligned.fTop;
    dataPtr += this->bpp() * aligned.fLeft;

    SkIRect offsetRect = aligned.makeOffset(this->topLeftInAtlas().fX, this->topLeftInAtlas().fY);

    fDirtyRect.setEmpty();
    fIsFull = false;

    return {dataPtr, offsetRect};
}

void DrawAtlas::Plot::recycle(bool freeData) {
    fEntries.reset();
    fRectanizer.reset();
    fPlotID = NextPlotID();
    fLastUse = Token::InvalidToken();
    fFlushesSinceLastUse = 0;
    fDirtyRect.setEmpty();
    fIsFull = false;
    if (freeData) {
        fData.reset();
    } else if (fData) {
        sk_bzero(fData.get(), this->rowBytes() * fPlotDimensions.height());
    }
}

DrawAtlas::Plot* DrawAtlas::findPlot(PlotID id) const {
    Plot* const* plot = fPlotMap.find(id);
    return plot ? *plot : nullptr;
}

DrawAtlas::Plot* DrawAtlas::findPlot(Record record) const {
    return this->findPlot(record.fPlotID);
}

}  // namespace skgpu::graphite
