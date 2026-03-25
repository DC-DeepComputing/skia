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
        return;
    }

    SkASSERT(fPlotMap.count() == this->numPlots());

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

std::unique_ptr<DrawAtlas> DrawAtlas::Make(MaskFormat maskFormat,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           UseStorageTextures useStorageTextures,
                                           EvictionCallback* evictor,
                                           std::string_view label) {
    std::unique_ptr<DrawAtlas> atlas(new DrawAtlas(maskFormat,
                                                   width, height,
                                                   plotWidth, plotHeight,


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
        , fFlushesSinceLastUse(0) {
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
    SkASSERT(fProxy);

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
    if (!fProxy) {
        return true;
    }
    for (const auto& plot : this->plots()) {
        if (!plot->needsUpload()) {
            continue;
        }

        auto [dataPtr, dstRect] = plot->prepareForUpload();
        if (dstRect.isEmpty()) {
            continue;
        }

        std::vector<MipLevel> levels;
        levels.push_back({dataPtr, plot->rowBytes()});

        // Src and dst colorInfo are the same
        SkColorInfo colorInfo(MaskFormatToColorType(fMaskFormat), kUnknown_SkAlphaType, nullptr);
        const UploadSource uploadSource = UploadSource::Make(
                recorder->priv().caps(), *fProxy, colorInfo, colorInfo, levels, dstRect);
        if (!uploadSource.isValid()) {
            return false;
        }
        if (!dc->recordUpload(recorder,
                              fProxy,
                              colorInfo,
                              colorInfo,
                              uploadSource,
                              dstRect,
                              /*ConditionalUploadContext=*/nullptr)) {
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
    if (size.width() > fPlotWidth || size.height() > fPlotHeight || size.isEmpty()) {
        return ErrorCode::kError;
    }

    if (!fProxy) {
        if (!activateProxy(recorder)) {
            return ErrorCode::kError;
        }
    }

    // We permit zero-sized rects to allow inverse fills in the PathAtlases to work,
    // but we don't want to enter them in the Rectanizer. So we handle this special case here.
    // For text this should be caught at a higher level, but if not the only end result
    // will be rendering a degenerate quad.
    if (size.area() == 0) {
        if (!fProxy) {
            if (!this->activateProxy(recorder)) {
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

    // TODO: Try to increase the atlas size here.

    // If the above fails, then see if the least recently used plot has already been
    // queued for upload or if the plot has aged out otherwise.
    // We wait until we've filled all the plots before we begin evicting already queued
    // plots so that we can maximize the opportunity for reuse.
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

void DrawAtlas::compact(Token startTokenForNextFlush) {
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
    }

    fPrevFlushToken = startTokenForNextFlush;
    SkDEBUGCODE(this->validate());
}

bool DrawAtlas::activateProxy(Recorder* recorder) {
    SkDEBUGCODE(this->validate());
    SkASSERT(!fProxy);

    const Caps* caps = recorder->priv().caps();
    auto ct          = MaskFormatToColorType(fMaskFormat);
    auto textureInfo = fUseStorageTextures == UseStorageTextures::kYes
                               ? caps->getDefaultStorageTextureInfo(ct)
                               : caps->getDefaultSampledTextureInfo(ct,
                                                                    Mipmapped::kNo,
                                                                    recorder->priv().isProtected(),
                                                                    Renderable::kNo);
    fProxy = TextureProxy::Make(caps,
                                recorder->priv().resourceProvider(),
                                {fTextureWidth, fTextureHeight},
                                textureInfo,
                                fLabel,
                                Budgeted::kYes);
    if (!fProxy) {
        return false;
    }
    fPlots = std::make_unique<std::unique_ptr<Plot>[]>(this->numPlots());
    for (int y = 0, index = 0; y < fPlotGridDimensions.height(); ++y) {
        for (int x = 0; x < fPlotGridDimensions.width(); ++x, ++index) {
            PlotCoord coord{.fX = x, .fY = y};
            auto& plot = fPlots[index];
            plot = Plot::Make(coord, {fPlotWidth, fPlotHeight}, fMaskFormat);
            fPlotMap[plot->plotID()] = plot.get();
            fPlotMRUList.addToTail(plot.get());
            fPlots[y * fPlotGridDimensions.width() + x] = std::move(plot);
        }
    }

    SkDEBUGCODE(this->validate());

    SKIA_LOG_I("Activating %d x %d proxy in DrawAtlas %s.", fTextureWidth, fTextureHeight, fLabel.c_str());

    return true;
}

inline void DrawAtlas::deactivateProxy() {
    SkASSERT(fProxy);

    fPlotMRUList.reset();
    for (auto& plot : plots()) {
        this->processEvictionAndResetRects(plot.get(), EvictionType::kWillBeRemoved);
        plot->resetFlushesSinceLastUsed();
    }
    fPlots.reset();
    // remove ref to the texture proxy
    fProxy = {};
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
