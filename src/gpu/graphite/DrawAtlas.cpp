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
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTArray.h"
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

void DrawAtlas::internalSetLastUseToken(Plot* plot, Token token) {
    plot->setLastUseToken(token);
    this->makeMRU(plot);
}

#ifdef SK_DEBUG
void DrawAtlas::validate() const {
    if (!fPlots) {
        SkASSERT(fPlotMap.empty());
        SkASSERT(fPlotMRUList.isEmpty());
        SkASSERT(!fProxy);
        return;
    }

    SkASSERT(fProxy);
    SkASSERT(fPlotMap.count() == this->numPlots());
    SkASSERT(fPlotSize.width()  * fPlotGridDimensions.width()  == fProxy->dimensions().width());
    SkASSERT(fPlotSize.height() * fPlotGridDimensions.height() == fProxy->dimensions().height());

    std::vector foundCoord(size_t(this->numPlots()), false);
    for (const auto& plot : this->plots()) {
        SkASSERT(plot);
        auto coord = plot->plotCoord();
        int coordIndex = coord.fY * fPlotGridDimensions.width() + coord.fX;
        SkASSERT(!foundCoord[coordIndex]);
        foundCoord[coordIndex] = true;
        SkASSERT(plot->plotCoord() == coord);
        SkASSERT(plot->plotID() != PlotID::kInvalid);
        auto* mapEntry = fPlotMap.find(plot->plotID());
        SkASSERT(mapEntry && *mapEntry == plot.get());
    }
    SkASSERT(std::all_of(foundCoord.begin(), foundCoord.end(), [](bool v) { return v; }));
    int n = 0;
    for (Plot* prev = nullptr; Plot* plot : fPlotMRUList) {
        SkASSERT(!prev || plot->lastUseToken() <= prev->lastUseToken());
        prev = plot;
        ++n;

        auto isPlot = [plot](const std::unique_ptr<Plot>& p) { return p.get() == plot; };
        auto end = this->plots().end();
        SkASSERT(std::find_if(this->plots().begin(), end, isPlot) != end);
    }
    SkASSERT(n == this->numPlots());
}
#endif

SkISize DrawAtlas::MinPlotGridDimsForMaxBytes(size_t maxBytes,
                                              SkISize plotSize,
                                              MaskFormat format) {
    size_t  bpp          = MaskFormatBytesPerPixel(format);
    size_t  bytesPerPlot = plotSize.area() * bpp;
    int     maxPlots     = std::max<size_t>(1, maxBytes / bytesPerPlot);
    SkISize grid         = {1, 1};
    while (grid.area() * 2 <= maxPlots) {
        grid = NextLargerPlotGridDims(grid, plotSize);
    }
    return grid;
}

std::unique_ptr<DrawAtlas> DrawAtlas::Make(MaskFormat         maskFormat,
                                           const Config&      config,
                                           int                maxTextureSize,
                                           UseStorageTextures useStorageTextures,
                                           EvictionCallback*  evictor,
                                           std::string_view   label) {
    SkISize plotSize = config.fPlotSize;
    if (plotSize.width() > maxTextureSize || plotSize.height() > maxTextureSize) {
        return nullptr;
    }
    SkISize minGrid = config.fMinPlotGridDims;
    if (minGrid.width() < 1 || minGrid.height() < 1) {
        return nullptr;
    }
    minGrid.fWidth  = std::min(minGrid.fWidth,  maxTextureSize / plotSize.width());
    minGrid.fHeight = std::min(minGrid.fHeight, maxTextureSize / plotSize.height());

    // Compute max grid dims by applying allowed doublings then clamping to max texture size.
    SkISize maxGrid = minGrid;
    for (int i = 0; i < config.fAllowedDoublings; i++) {
        maxGrid = NextLargerPlotGridDims(maxGrid, plotSize);
    }
    maxGrid.fWidth  = std::min(maxGrid.fWidth,  maxTextureSize / plotSize.width());
    maxGrid.fHeight = std::min(maxGrid.fHeight, maxTextureSize / plotSize.height());

    std::unique_ptr<DrawAtlas> atlas(new DrawAtlas(maskFormat,
                                                   plotSize,
                                                   minGrid,
                                                   maxGrid,
                                                   useStorageTextures,
                                                   label));

    if (evictor != nullptr) {
        atlas->fEvictionCallbacks.emplace_back(evictor);
    }
    SkDEBUGCODE(atlas->validate());
    return atlas;
}

///////////////////////////////////////////////////////////////////////////////

DrawAtlas::DrawAtlas(MaskFormat         maskFormat,
                     SkISize            plotSize,
                     SkISize            minPlotGridDimensions,
                     SkISize            maxPlotGridDimensions,
                     UseStorageTextures useStorageTextures,
                     std::string_view   label)
        : fMaskFormat(maskFormat)
        , fPlotSize(plotSize)
        , fMinPlotGridDimensions(minPlotGridDimensions)
        , fMaxPlotGridDimensions(maxPlotGridDimensions)
        , fUseStorageTextures(useStorageTextures)
        , fLabel(label)
        , fPrevFlushToken(Token::InvalidToken()) {}

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
    fPlotMRUList.remove(plot);
    fPlotMRUList.addToTail(plot);
}

std::optional<DrawAtlas::AddResult> DrawAtlas::addRectToAtlas(SkISize size,
                                                              const std::byte* image) {
    SkASSERT(!size.isEmpty());
    SkASSERT(fProxy || fPlotMRUList.isEmpty());

    // look through all allocated plots for one we can share, in Most Recently Refed order
    for (auto* plot : fPlotMRUList) {
        if (std::optional<Plot::AddResult> addRectResult = plot->addRect(size, image)) {
            this->makeMRU(plot);
            return AddResult{.fRecord   = {plot->plotID(), addRectResult->fEntryID},
                             .fPosition = addRectResult->fPositionInAtlas};
        }
    }
    return {};
}

inline void DrawAtlas::makeMRU(Plot* plot) {
    if (fPlotMRUList.head() == plot) {
        return;
    }
    fPlotMRUList.remove(plot);
    fPlotMRUList.addToHead(plot);
}

bool DrawAtlas::recordUploads(DrawContext* dc, Recorder* recorder) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    auto process = [&](sk_sp<TextureProxy> proxy,
                       const std::byte*    dataPtr,
                       size_t              rowBytes,
                       SkIRect             dstRect) {
        if (dstRect.isEmpty()) {
            return true;
        }

        std::vector<MipLevel> levels;
        levels.push_back({dataPtr, rowBytes});

        // Src and dst colorInfo are the same
        SkColorInfo colorInfo(MaskFormatToColorType(fMaskFormat), kUnknown_SkAlphaType, nullptr);
        const UploadSource uploadSource = UploadSource::Make(
                recorder->priv().caps(), *proxy, colorInfo, colorInfo, levels, dstRect);
        if (!uploadSource.isValid()) {
            return false;
        }
        if (!dc->recordUpload(recorder,
                              std::move(proxy),
                              colorInfo,
                              colorInfo,
                              uploadSource,
                              dstRect,
                              /*ConditionalUploadContext=*/nullptr)) {
            return false;
        }
        return true;
    };

    for (const auto& opu : fOldProxyUploads) {
        for (const auto& u : opu.fUploads) {
            if (!process(opu.fProxy, u.fDataPtr, u.fRowBytes, u.fDstRect)) {
                return false;
            }
        }
    }
    fOldProxyUploads = {};

    if (!fProxy) {
        return true;
    }
    for (const auto& plot : this->plots()) {
        if (!plot->needsUpload()) {
            continue;
        }

        auto [dataPtr, dstRect] = plot->prepareForUpload();
        if (!process(fProxy, dataPtr, plot->rowBytes(), dstRect)) {
            return false;
        }
    }
    return true;
}

bool DrawAtlas::hasRecord(Record record) const {
    auto* plot = this->findPlot(record.fPlotID);
    if (!plot) {
        return false;
    }
    // Since our eviction is always a full plot it should not be possible to have
    // an AtlasRecord with a valid plot ID but an invalid entry ID for that plot.
    SkASSERT(record.fEntryID == EntryID::kEmpty ||
             plot->entryAtlasRect(record.fEntryID).has_value());
    return true;
}

void DrawAtlas::setLastUseTokenBulk(const BulkRecordUseUpdater& updater, Token token) {
    updater.foreach([&](PlotID pid) {
        // it's possible we've added a plot to the updater and subsequently the plot
        // was recycled -- so we check to prevent a crash
        if (auto* plot = this->findPlot(pid)) {
            this->internalSetLastUseToken(plot, token);
        }
    });
}

DrawAtlas::ErrorCode DrawAtlas::addRect(Recorder* recorder,
                                        SkISize size,
                                        AddResult* result,
                                        const std::byte* image) {
    if (size.width() > fPlotSize.width() || size.height() > fPlotSize.height() || size.isEmpty()) {
        return ErrorCode::kError;
    }

    // We permit zero-sized rects to allow inverse fills in the PathAtlases to work,
    // but we don't want to enter them in the Rectanizer. So we handle this special case here.
    // For text this should be caught at a higher level, but if not the only end result
    // will be rendering a degenerate quad.
    if (size.area() == 0) {
        if (!fProxy) {
            if (!this->increaseProxySize(recorder, fMinPlotGridDimensions)) {
                return ErrorCode::kError;
            }
        }
        // Use the MRU Plot
        const Plot* plot = fPlotMRUList.head();
        result->fRecord.fEntryID = EntryID::kEmpty;
        result->fRecord.fPlotID = plot->plotID();
        result->fPosition = {0, 0};
        return ErrorCode::kSucceeded;
    }

    // See if we can upload without having to flush
    if (auto addRectResult = this->addRectToAtlas(size, image)) {
        *result = *addRectResult;
        return ErrorCode::kSucceeded;
    }

    // If we're already at the maximum size, try evicting the LRU plot as a last resort.
    if (fPlotGridDimensions == fMaxPlotGridDimensions) {
        auto* plot = fPlotMRUList.tail();
        if (plot->lastUseToken() < recorder->priv().tokenTracker()->nextFlushToken()) {
            this->processEvictionAndResetRects(plot, EvictionType::kReuse_KeepData);
            auto addRectResult = plot->addRect(size, image);
            SkASSERT(addRectResult);
            result->fRecord = {plot->plotID(), addRectResult->fEntryID};
            result->fPosition = addRectResult->fPositionInAtlas;
            this->makeMRU(plot);
            return ErrorCode::kSucceeded;
        }
        // All plots are currently in use by the current set of draws, so we need to fail. This
        // gives the Device a chance to snap the current set of uploads and draws, advance the draw
        // token, and call back into this function. The subsequent call will have plots available
        // for fresh uploads.
        return ErrorCode::kTryAgain;
    }

    SkISize newProxyPlotGridDims;
    if (!fProxy) {
        newProxyPlotGridDims = fMinPlotGridDimensions;
    } else {
        newProxyPlotGridDims = this->nextLargerPlotGridDims(fPlotGridDimensions);
        if (newProxyPlotGridDims.width()  > fMaxPlotGridDimensions.width() ||
            newProxyPlotGridDims.height() > fMaxPlotGridDimensions.height()) {
            return ErrorCode::kTryAgain;
        }
    }

    if (!this->increaseProxySize(recorder, newProxyPlotGridDims)) {
        return ErrorCode::kError;
    }

    auto addRectResult = this->addRectToAtlas(size, image);
    SkASSERT(addRectResult);
    *result = *addRectResult;
    return ErrorCode::kSucceeded;
}

std::optional<SkPixmap> DrawAtlas::entryPixmap(Record record,
                                               int padding,
                                               std::optional<SkColor> color) {
    if (auto* plot = findPlot(record.fPlotID)) {
        return plot->entryPixmap(record.fEntryID, padding, color);
    }
    return {};
}

std::optional<SkIRect> DrawAtlas::entryLocation(Record record) const {
    if (auto* plot = findPlot(record.fPlotID)) {
        auto rect = plot->entryAtlasRect(record.fEntryID);
        // Purging is at the plot level. So it shouldn't be possible to have a record that
        // has a valid PlotID for an existing Plot with an EntryID that isn't still valid.
        SkASSERT(rect);
        return *rect;
    }
    return {};
}

void DrawAtlas::compact(Recorder* recorder) {
    Token startTokenForNextFlush = recorder->priv().tokenTracker()->nextFlushToken();

    if (!fProxy) {
        fPrevFlushToken = startTokenForNextFlush;
        return;
    }

    SkDEBUGCODE(this->validate());

    // For all plots, reset number of flushes since used if used this frame.
    bool atlasUsedThisFlush = false;
    for (auto& plot : this->plots()) {
        if (plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
            plot->resetFlushesSinceLastUsed();
            atlasUsedThisFlush = true;
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
        int numPlotsRecycled = 0;
        int numPlotsInUse    = 0;
        // For all plots update number of flushes since used, and check to see
        // if there are Plots that can be overwritten.
        for (auto& plot : this->plots()) {
            // Update number of flushes since plot was last used
            if (!plot->lastUseToken().inInterval(fPrevFlushToken, startTokenForNextFlush)) {
                plot->incFlushesSinceLastUsed();
            }
            if (plot->flushesSinceLastUsed() > kPlotRecentlyUsedCount) {
                if (!plot->isEmpty()) {
                    this->processEvictionAndResetRects(plot.get(), EvictionType::kReuse_KeepData);
                    ++numPlotsRecycled;
                }
            } else {
                ++numPlotsInUse;
            }
        }
        if (numPlotsRecycled > 0) {
            SKIA_LOG_I("Recycled %d of %d Plots in %s DrawAtlas. %d Plots recently used.",
                        numPlotsRecycled, this->numPlots(), fLabel.c_str(), numPlotsInUse);
        }
        if (numPlotsInUse == 0 && fFlushesSinceLastUse > kAtlasRecentlyUsedCount) {
            deactivateProxy();
            fFirstCompactibleToken = Token::InvalidToken();
        } else if (numPlotsInUse < this->numPlots() / 2 &&
                   fPlotGridDimensions.area() > fMinPlotGridDimensions.area()) {
            // We would still have at least one unused plot if we compacted to a smaller
            // texture.
            int numCompactibleFlushes = 0;
            if (fFirstCompactibleToken == Token::InvalidToken()) {
                fFirstCompactibleToken = startTokenForNextFlush;
            } else {
                numCompactibleFlushes =
                        startTokenForNextFlush.value() - fFirstCompactibleToken.value();
            }
            if (numCompactibleFlushes > kCompactionRecentlyUsedCount) {
                auto newPlotGridDims = this->nextSmallerPlotGridDims(fPlotGridDimensions);
                this->decreaseProxySize(recorder, newPlotGridDims);
                fFirstCompactibleToken = Token::InvalidToken();
            }
        }
    }

    fPrevFlushToken = startTokenForNextFlush;
    SkDEBUGCODE(this->validate());
}

bool DrawAtlas::increaseProxySize(Recorder* recorder, SkISize newPlotGridDims) {
    SkDEBUGCODE(this->validate());
    SkASSERT(newPlotGridDims.width() >= fPlotGridDimensions.width());
    SkASSERT(newPlotGridDims.height() >= fPlotGridDimensions.height());
    SkASSERT(newPlotGridDims.area() > fPlotGridDimensions.area());

    int textureWidth  = newPlotGridDims.width()*fPlotSize.width();
    int textureHeight = newPlotGridDims.height()*fPlotSize.height();
    const Caps* caps  = recorder->priv().caps();
    auto ct           = MaskFormatToColorType(fMaskFormat);
    auto textureInfo  = fUseStorageTextures == UseStorageTextures::kYes
                               ? caps->getDefaultStorageTextureInfo(ct)
                               : caps->getDefaultSampledTextureInfo(ct,
                                                                    Mipmapped::kNo,
                                                                    recorder->priv().isProtected(),
                                                                    Renderable::kNo);
    auto newProxy     = TextureProxy::Make(caps,
                                           recorder->priv().resourceProvider(),
                                           {textureWidth, textureHeight},
                                           textureInfo,
                                           fLabel,
                                           Budgeted::kYes);
    if (!newProxy) {
        return false;
    }

    OldProxyUploads oldUploads;
    oldUploads.fProxy = fProxy;

    auto newPlotArray = std::make_unique<std::unique_ptr<Plot>[]>(newPlotGridDims.area());
    for (PlotCoord c{}; c.fY < newPlotGridDims.height(); ++c.fY) {
        for (c.fX = 0; c.fX < newPlotGridDims.width(); ++c.fX) {
            auto newIndex = c.fY * newPlotGridDims.width() + c.fX;
            if (c.fX < fPlotGridDimensions.width() && c.fY < fPlotGridDimensions.height()) {
                auto oldIndex = c.fY * fPlotGridDimensions.width() + c.fX;
                newPlotArray[newIndex] = std::move(fPlots[oldIndex]);
                // In the future we'd like to issue a GPU->GPU copy here. For now, we just mark the
                // plot fully dirty in order to trigger a re-upload.
                if (newPlotArray[newIndex]->needsUpload()) {
                    auto [dataPtr, dstRect] = newPlotArray[newIndex]->prepareForUpload();
                    if (!dstRect.isEmpty()) {
                        OldProxyUploads::Upload upload{
                            .fDataPtr  = dataPtr,
                            .fRowBytes = newPlotArray[newIndex]->rowBytes(),
                            .fDstRect  = dstRect
                        };
                        oldUploads.fUploads.push_back(upload);
                    }
                }
                newPlotArray[newIndex]->markAllEntriesDirty();
            } else {
                newPlotArray[newIndex] = Plot::Make(c, fPlotSize, fMaskFormat);
                fPlotMRUList.addToTail(newPlotArray[newIndex].get());
                fPlotMap[newPlotArray[newIndex]->plotID()] = newPlotArray[newIndex].get();
            }
        }
    }
    fPlotGridDimensions = newPlotGridDims;
    fPlots = std::move(newPlotArray);
    fProxy = std::move(newProxy);
    if (!oldUploads.fUploads.empty()) {
        fOldProxyUploads.push_back(std::move(oldUploads));
    }
    SkDEBUGCODE(this->validate());
    SKIA_LOG_I("Increased %s DrawAtlas size to %d x %d",
               fLabel.c_str(),
               fProxy->dimensions().width(),
               fProxy->dimensions().height());
    return true;
}

void DrawAtlas::decreaseProxySize(Recorder* recorder, SkISize newPlotGridDims) {
    SkDEBUGCODE(this->validate());
    SkASSERT(newPlotGridDims.width() <= fPlotGridDimensions.width());
    SkASSERT(newPlotGridDims.height() <= fPlotGridDimensions.height());
    SkASSERT(newPlotGridDims.area() < fPlotGridDimensions.area());

    int textureWidth  = newPlotGridDims.width()*fPlotSize.width();
    int textureHeight = newPlotGridDims.height()*fPlotSize.height();
    const Caps* caps  = recorder->priv().caps();
    auto ct           = MaskFormatToColorType(fMaskFormat);
    auto textureInfo  = fUseStorageTextures == UseStorageTextures::kYes
                               ? caps->getDefaultStorageTextureInfo(ct)
                               : caps->getDefaultSampledTextureInfo(ct,
                                                                    Mipmapped::kNo,
                                                                    recorder->priv().isProtected(),
                                                                    Renderable::kNo);
    auto newProxy     = TextureProxy::Make(caps,
                                           recorder->priv().resourceProvider(),
                                           {textureWidth, textureHeight},
                                           textureInfo,
                                           fLabel,
                                           Budgeted::kYes);
    if (!newProxy) {
        return;
    }

    OldProxyUploads oldUploads;
    oldUploads.fProxy = fProxy;

    int numNewPlots = newPlotGridDims.area();
    auto newPlotArray = std::make_unique<std::unique_ptr<Plot>[]>(numNewPlots);
    TArray<std::unique_ptr<Plot>> scratchPlots;
    PlotCoord newCoord = {0, 0};
    int currNewPlotIdx = 0;
    auto advanceCoord = [&] {
        ++newCoord.fX;
        if (newCoord.fX == newPlotGridDims.width()) {
            ++newCoord.fY;
            newCoord.fX = 0;
            SkASSERT(newCoord.fX == 0 || newCoord.fY < newPlotGridDims.height());
        }
    };
    for (auto& plot : this->plots()) {
        // Here we're assuming that compact() has already recycled plots that shouldn't
        // be moved to the smaller texture.
        if (!plot->isEmpty()) {
            PlotCoord oldCoord = plot->plotCoord();
            plot->setPlotCoord(newCoord);
            SkIVector translation{fPlotSize.width()  * (newCoord.fX - oldCoord.fX),
                                  fPlotSize.height() * (newCoord.fY - oldCoord.fY)};
            const RecordVisitor visitor{plot.get()};
            for (EvictionCallback* evictor : fEvictionCallbacks) {
                evictor->relocate(translation, visitor);
            }
            fEvictionCounter = NextEvictionCounter(fEvictionCounter);
            // We'd have to use the same "old uploads" strategy as in increaseProxySize()
            // if there were could be draws referring to the old proxy/coord.
            // However, we are assuming those have all been flushed before compact() is
            // called.
            SkASSERT(!plot->needsUpload());
            plot->markAllEntriesDirty();
            newPlotArray[currNewPlotIdx++] = std::move(plot);
            advanceCoord();
        } else if (plot->hasAllocation()) {
            fPlotMRUList.remove(plot.get());
            scratchPlots.push_back(std::move(plot));
        } else {
            fPlotMap.remove(plot->plotID());
            fPlotMRUList.remove(plot.get());
        }
    }

    while (currNewPlotIdx < numNewPlots) {
        std::unique_ptr<Plot> plot;
        if (scratchPlots.empty()) {
            plot = Plot::Make(newCoord, fPlotSize, fMaskFormat);
            fPlotMap[plot->plotID()] = plot.get();
        } else {
            plot = std::move(scratchPlots.back());
            plot->setPlotCoord(newCoord);
            scratchPlots.pop_back();
        }
        fPlotMRUList.addToTail(plot.get());
        newPlotArray[currNewPlotIdx++] = std::move(plot);
        advanceCoord();
    }
    SkASSERT(newCoord.fX == 0 && newCoord.fY == newPlotGridDims.height());

    for (auto& extraPlot : scratchPlots) {
        fPlotMap.remove(extraPlot->plotID());
    }

    fPlotGridDimensions = newPlotGridDims;
    fPlots = std::move(newPlotArray);
    fProxy = std::move(newProxy);

    SkDEBUGCODE(this->validate());
    SKIA_LOG_I("Decreased %s DrawAtlas size to %d x %d",
               fLabel.c_str(),
               fProxy->dimensions().width(),
               fProxy->dimensions().height());
}

inline void DrawAtlas::deactivateProxy() {
    SkASSERT(fProxy);
    // Old proxy uploads contain data pointers into Plot data that will be destroyed below,
    // so discard them. Any draws referencing old proxies are also being discarded.
    fOldProxyUploads = {};

    fPlotMRUList.reset();
    for (auto& plot : plots()) {
        this->processEvictionAndResetRects(plot.get(), EvictionType::kWillBeRemoved);
        plot->resetFlushesSinceLastUsed();
    }
    fPlots.reset();
    // remove ref to the texture proxy
    fProxy = {};
    fPlotGridDimensions = {0, 0};
    SkDEBUGCODE(this->validate());
}

void DrawAtlas::markUsedPlotsAsFull() {
    for (const auto& plot : this->plots()) {
        plot->markFullIfUsed();
    }
}

void DrawAtlas::freeGpuResources(Token token) {
    for (Plot* plot : fPlotMRUList) {
        if (plot->lastUseToken().inInterval(fPrevFlushToken, token)) {
            return;
        }
    }
    this->deactivateProxy();
}

void DrawAtlas::evictAllPlots() {
    if (!fProxy) {
        return;
    }
    for (const auto& plot : this->plots()) {
        this->processEvictionAndResetRects(plot.get(), EvictionType::kReuse_FreeData);
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

void DrawAtlas::clearPendingUploads() {
    fOldProxyUploads = {};
    for (const auto& plot : this->plots()) {
        if (plot->needsUpload()) {
            plot->prepareForUpload();
        }
    }
}
#endif

inline DrawAtlas::PlotID DrawAtlas::Plot::NextPlotID() {
    static std::atomic<std::underlying_type_t<PlotID>> gID;
    auto next = gID.fetch_add(1, std::memory_order_relaxed) + 1;
    return static_cast<PlotID>(next);
}

DrawAtlas::Plot::Plot(PlotCoord coord, SkISize plotDimensions, MaskFormat maskFormat)
        : fRectanizer{plotDimensions.width(), plotDimensions.height()}
        , fPlotDimensions{plotDimensions}
        , fPlotCoord{coord}
        , fMaskFormat{maskFormat} {
    // We expect the allocated dimensions to be a multiple of 4 bytes
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
        if (*initialColor == 0) {
            SKIA_LOG_W("Plot Data: potential redundant clear of Plot to zero.");
        }
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
        // make_unique will init to 0.
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
