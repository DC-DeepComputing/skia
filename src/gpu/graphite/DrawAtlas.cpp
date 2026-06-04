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
        SkASSERT(record.fEntryID == EntryID::kEmpty || plot->entryAtlasRect(record.fEntryID));
    }
    return plot;
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
        for (unsigned int plotIndex = 0; plotIndex < ppp; ++plotIndex) {
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
        }
        SkASSERT(std::all_of(foundCoord.begin(), foundCoord.end(), [](bool v) { return v; }));
        int n = 0;
        for (Plot* prev = nullptr; Plot* plot : page.fMRUList) {
            SkASSERT(!prev || plot->lastUseToken() <= prev->lastUseToken());
            prev = plot;
            ++n;
        }
        SkASSERT(n == static_cast<int>(ppp));
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

// Legacy Make overload
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
                                                   allowMultitexturing,
                                                   useStorageTextures,
                                                   label));
    if (evictor != nullptr) {
        atlas->fPlotEvictionCallbacks.emplace_back(evictor);
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
        const PlotLocator& plotLocator = plot->plotLocator();
        for (PlotEvictionCallback* evictor : fPlotEvictionCallbacks) {
            evictor->evict(plotLocator);
        }

        const RecordVisitor visitor{plot};
        for (EvictionCallback* evictor : fEvictionCallbacks) {
            evictor->evict(visitor);
        }
        fEvictionCounter = NextEvictionCounter(fEvictionCounter);
    }
    fPlotMap.remove(plot->plotID());
    bool freeData;
    switch (type) {
        case EvictionType::kWillBeRemoved:
            return;
        case EvictionType::kReuse_FreeData:
            freeData = true;
            break;
        case EvictionType::kReuse_KeepData:
            freeData = false;
            break;
        default:
            SkUNREACHABLE;
    }
    plot->recycle(freeData);
    fPlotMap[plot->plotID()] = plot;
    int pageIdx = plot->plotCoord().fPageIdx;
    fPages[pageIdx].fMRUList.remove(plot);
    fPages[pageIdx].fMRUList.addToTail(plot);
}

void DrawAtlas::processEvictionAndResetRects(Plot* plot, bool freeData) {
    this->processEvictionAndResetRects(plot, freeData ? EvictionType::kReuse_FreeData : EvictionType::kReuse_KeepData);
}

std::optional<DrawAtlas::AddResult> DrawAtlas::addRectToPage(int pageIdx,
                                                             SkISize size,
                                                             const std::byte* image) {
    SkASSERT(!size.isEmpty());
    SkASSERT(fProxies[pageIdx]);
    // look through all allocated plots for one we can share, in Most Recently Refed order
    for (auto* plot : fPages[pageIdx].fMRUList) {
        if (std::optional<Plot::AddResult> addRectResult = plot->addRect(size, image)) {
            this->makeMRU(plot);
            return AddResult{.fRecord   = {plot->plotID(), addRectResult->fEntryID},
                             .fPageIdx  = pageIdx,
                             .fPosition = addRectResult->fPositionInAtlas};
        }
    }
    return {};
}

inline void DrawAtlas::makeMRU(Plot* plot) {
    int pageIdx = plot->plotCoord().fPageIdx;
    if (fPages[pageIdx].fMRUList.head() == plot) {
        return;
    }
    fPages[pageIdx].fMRUList.remove(plot);
    fPages[pageIdx].fMRUList.addToHead(plot);
}

bool DrawAtlas::recordUploads(DrawContext* dc, Recorder* recorder) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    const SkColorType maskCT = MaskFormatToColorType(fMaskFormat);
    // Src and dst colorInfo are the same
    const SkColorInfo colorInfo(maskCT, kUnknown_SkAlphaType, nullptr);
    unsigned int numPlots = this->plotsPerPage();
    for (int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        auto* plots = fPages[pageIdx].fPlotArray.get();
        Swizzle readSwizzle = ReadSwizzleForColorType(maskCT, fProxies[pageIdx]->format());
        TextureProxyView view{fProxies[pageIdx], readSwizzle};
        for (unsigned int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
            Plot* plot = plots[plotIndex].get();
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

bool DrawAtlas::hasRecord(Record record) const {
    auto* plot = this->findPlot(record.fPlotID);
    if (!plot) {
        return false;
    }
    SkASSERT(record.fEntryID == EntryID::kEmpty ||
             plot->entryAtlasRect(record.fEntryID).has_value());
    return true;
}

// Number of atlas-related flushes beyond which we consider a plot to no longer be in use.
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
    if (size.area() == 0) {
        if (fNumActivePages == 0) {
            if (!this->activateNewPage(recorder)) {
                return ErrorCode::kError;
            }
        }
        const Plot* plot = fPages[0].fMRUList.head();
        result->fRecord.fEntryID = EntryID::kEmpty;
        result->fRecord.fPlotID = plot->plotID();
        result->fPosition = {0, 0};
        return ErrorCode::kSucceeded;
    }
    for (int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
        if (auto addRectResult = this->addRectToPage(pageIdx, size, image)) {
            *result = *addRectResult;
            return ErrorCode::kSucceeded;
        }
    }
    if (fNumActivePages == this->maxPages()) {
        for (int pageIdx = 0; pageIdx < fNumActivePages; ++pageIdx) {
            auto* plot = fPages[pageIdx].fMRUList.tail();
            if (plot->lastUseToken() < recorder->priv().tokenTracker()->nextFlushToken()) {
                this->processEvictionAndResetRects(plot, EvictionType::kReuse_KeepData);
                auto addRectResult = plot->addRect(size, image);
                SkASSERT(addRectResult);
                result->fRecord = {plot->plotID(), addRectResult->fEntryID};
                result->fPageIdx = pageIdx;
                result->fPosition = addRectResult->fPositionInAtlas;
                this->makeMRU(plot);
                return ErrorCode::kSucceeded;
            }
        }
        return ErrorCode::kTryAgain;
    }
    if (!this->activateNewPage(recorder)) {
        return ErrorCode::kError;
    }
    if (auto addRectResult = this->addRectToPage(fNumActivePages - 1, size, image)) {
        *result = *addRectResult;
        return ErrorCode::kSucceeded;
    }
    return ErrorCode::kError;
}

DrawAtlas::ErrorCode DrawAtlas::addRect(Recorder* recorder,
                                        int width, int height,
                                        AtlasLocator* atlasLocator) {
    AddResult result;
    ErrorCode ec = this->addRect(recorder, SkISize{width, height}, &result, nullptr);
    if (ec == ErrorCode::kSucceeded) {
        Plot* plot = this->findPlot(result.fRecord.fPlotID);
        SkASSERT(plot);
        int plotIdx = plot->plotCoord().fY * (fTextureWidth / fPlotWidth) + plot->plotCoord().fX;
        PlotLocator plotLoc(result.fPageIdx, plotIdx, static_cast<uint64_t>(plot->plotID()));

        atlasLocator->updatePlotLocator(plotLoc);
        SkIRect rect = SkIRect::MakePtSize(result.fPosition, SkISize{width, height});
        atlasLocator->updateRect(rect);
        atlasLocator->updateRecord(result.fRecord);
    }
    return ec;
}

DrawAtlas::ErrorCode DrawAtlas::addToAtlas(Recorder* recorder,
                                           int width, int height,
                                           const void* image,
                                           AtlasLocator* atlasLocator) {
    AddResult result;
    ErrorCode ec = this->addRect(recorder, SkISize{width, height}, &result, static_cast<const std::byte*>(image));
    if (ec == ErrorCode::kSucceeded) {
        Plot* plot = this->findPlot(result.fRecord.fPlotID);
        SkASSERT(plot);
        int plotIdx = plot->plotCoord().fY * (fTextureWidth / fPlotWidth) + plot->plotCoord().fX;
        PlotLocator plotLoc(result.fPageIdx, plotIdx, static_cast<uint64_t>(plot->plotID()));

        atlasLocator->updatePlotLocator(plotLoc);
        SkIRect rect = SkIRect::MakePtSize(result.fPosition, SkISize{width, height});
        atlasLocator->updateRect(rect);
        atlasLocator->updateRecord(result.fRecord);
    }
    return ec;
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
        SkASSERT(rect);
        return std::make_tuple(plot->plotCoord().fPageIdx, *rect);
    }
    return {};
}

SkPixmap DrawAtlas::prepForRender(const AtlasLocator& locator,
                                  int padding,
                                  std::optional<SkColor> initialColor) {
    auto pixmapOpt = this->entryPixmap(locator.record(), padding, initialColor);
    SkASSERT(pixmapOpt);
    return *pixmapOpt;
}

void DrawAtlas::compact(Token startTokenForNextFlush) {
    if (fNumActivePages < 1) {
        fPrevFlushToken = startTokenForNextFlush;
        return;
    }
    SkDEBUGCODE(this->validate());

    bool atlasUsedThisFlush = false;
    unsigned int numPlots = this->plotsPerPage();
    for (int pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        auto* plots = fPages[pageIndex].fPlotArray.get();
        for (unsigned int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
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

    if (atlasUsedThisFlush || fFlushesSinceLastUse > kAtlasRecentlyUsedCount) {
        STArray<16, Plot*> availablePlots;
        int lastPageIndex = fNumActivePages - 1;

        for (int pageIndex = 0; pageIndex < lastPageIndex; ++pageIndex) {
            if constexpr (kDumpAtlasData) {
                SkDebugf("page %d: ", pageIndex);
            }
            auto* plots = fPages[pageIndex].fPlotArray.get();
            for (unsigned int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
                auto& plot = plots[plotIndex];
                if (!plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                    plot->incFlushesSinceLastUsed();
                }
                if constexpr (kDumpAtlasData) {
                    SkDebugf("%d ", plot->flushesSinceLastUsed());
                }
                if (plot->flushesSinceLastUsed() > kPlotRecentlyUsedCount) {
                    availablePlots.push_back(plot.get());
                }
            }
            if constexpr (kDumpAtlasData) {
                SkDebugf("\n");
            }
        }

        int usedPlots = 0;
        if constexpr (kDumpAtlasData) {
            SkDebugf("page %d: ", lastPageIndex);
        }
        auto* lastPagePlots = fPages[lastPageIndex].fPlotArray.get();
        for (unsigned int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
            auto& plot = lastPagePlots[plotIndex];
            if (!plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                plot->incFlushesSinceLastUsed();
            }
            if constexpr (kDumpAtlasData) {
                SkDebugf("%d ", plot->flushesSinceLastUsed());
            }
            if (plot->flushesSinceLastUsed() <= kPlotRecentlyUsedCount) {
                usedPlots++;
            } else if (plot->lastUseToken() != Token::InvalidToken()) {
                this->processEvictionAndResetRects(plot.get(), EvictionType::kReuse_KeepData);
            }
        }
        if constexpr (kDumpAtlasData) {
            SkDebugf("\n");
        }

        if (!availablePlots.empty() && usedPlots && static_cast<unsigned int>(usedPlots) <= this->plotsPerPage() / 4) {
            for (unsigned int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
                auto& plot = lastPagePlots[plotIndex];
                int plotFlushes = plot->flushesSinceLastUsed();
                if (kPlotUsedCountBeforeEvict <= plotFlushes &&
                    plotFlushes <= kPlotRecentlyUsedCount) {
                    if (!availablePlots.empty()) {
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

    unsigned int numPlots = this->plotsPerPage();
    fPages[fNumActivePages].fPlotArray = std::make_unique<std::unique_ptr<Plot>[]>(numPlots);
    for (int y = 0; y < fPlotGridDimensions.height(); ++y) {
        for (int x = 0; x < fPlotGridDimensions.width(); ++x) {
            PlotCoord coord {
                .fPageIdx = SkToU8(fNumActivePages),
                .fX       = SkToU8(x),
                .fY       = SkToU8(y)
            };
            int index = y * fPlotGridDimensions.width() + x;
            auto& plotUP = fPages[fNumActivePages].fPlotArray[index];
            plotUP = Plot::Make(coord, index, SkISize{fPlotWidth, fPlotHeight}, fMaskFormat);
            fPlotMap[plotUP->plotID()] = plotUP.get();
            fPages[fNumActivePages].fMRUList.addToTail(plotUP.get());
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
    unsigned int numPlots = this->plotsPerPage();
    auto* plots = fPages[lastPageIndex].fPlotArray.get();
    for (unsigned int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
        this->processEvictionAndResetRects(plots[plotIndex].get(), EvictionType::kWillBeRemoved);
    }
    fPages[lastPageIndex].fMRUList.reset();
    fPages[lastPageIndex].fPlotArray = {};
    fProxies[lastPageIndex].reset();
    --fNumActivePages;
}

void DrawAtlas::markUsedPlotsAsFull() {
    unsigned int numPlots = this->plotsPerPage();
    for (int pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
        auto* plots = fPages[pageIndex].fPlotArray.get();
        for (unsigned int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
            plots[plotIndex]->markFullIfUsed();
        }
    }
}

void DrawAtlas::freeGpuResources(Token token) {
    for (int pageIndex = fNumActivePages - 1; pageIndex >= 0; --pageIndex) {
        for (Plot* plot : fPages[pageIndex].fMRUList) {
            if (plot->lastUseToken().inInterval(fPrevFlushToken, token)) {
                return;
            }
        }
        this->deactivateLastPage();
    }
}

void DrawAtlas::evictAllPlots() {
    unsigned int numPlots = this->plotsPerPage();
    for (int pageIndex = fNumActivePages - 1; pageIndex >= 0; --pageIndex) {
        auto* plots = fPages[pageIndex].fPlotArray.get();
        for (unsigned int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
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

DrawAtlas::Plot::Plot(PlotCoord coord, uint32_t plotIndex, SkISize plotDimensions, MaskFormat maskFormat)
        : fRectanizer{plotDimensions.width(), plotDimensions.height()}
        , fPlotDimensions{plotDimensions}
        , fPlotIndex{plotIndex}
        , fPlotCoord{coord}
        , fMaskFormat{maskFormat} {
    SkASSERT((this->rowBytes() & 0x3) == 0);
}

DrawAtlas::Plot::~Plot() = default;

inline DrawAtlas::EntryID DrawAtlas::NextEntryID(EntryID e) {
    auto v = static_cast<std::underlying_type_t<EntryID>>(e);
    return static_cast<EntryID>(v + 1);
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

std::optional<SkIRect> DrawAtlas::Plot::entryAtlasRect(EntryID id) const {
    if (id == EntryID::kEmpty) {
        return SkIRect::MakeEmpty();
    }
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
        fData = std::make_unique<std::byte[]>(rb * fPlotDimensions.height());
    }
    SkASSERT(plotPoint.fX >= 0 && plotPoint.fX < fPlotDimensions.width());
    SkASSERT(plotPoint.fY >= 0 && plotPoint.fY < fPlotDimensions.height());
    size_t offset = rb * plotPoint.y() + this->bpp() * plotPoint.x();
    return fData.get() + offset;
}

std::pair<const std::byte*, SkIRect> DrawAtlas::Plot::prepareForUpload() {
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
