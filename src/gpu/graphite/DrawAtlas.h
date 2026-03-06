/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawAtlas_DEFINED
#define skgpu_graphite_DrawAtlas_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkTHash.h"
#include "src/gpu/MaskFormat.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/gpu/Token.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class SkPixmap;
enum SkColorType : int;

namespace skgpu {
enum class MaskFormat : int;
}

namespace skgpu::graphite {

class DrawContext;
class Recorder;
class TextureProxy;

/**
 * TODO: the process described here is tentative, and this comment revised once locked down.
 *
 * This class manages one or more atlas textures on behalf of primitive draws in Device. The
 * drawing processes that use the atlas add preceding UploadTasks when generating RenderPassTasks.
 * The class provides facilities for using DrawTokens to detect data hazards. Plots that need
 * uploads are tracked until it is impossible to add data without overwriting texels read by draws
 * that have not yet been snapped to a RenderPassTask. At that point, the atlas will attempt to
 * allocate a new atlas texture (or "page") of the same size, up to a maximum number of textures,
 * and upload to that texture. If that's not possible, then the atlas will fail to add a subimage.
 * This gives the Device the chance to end the current draw, snap a RenderpassTask, and begin a new
 * one. Additional uploads will then succeed.
 *
 * When the atlas has multiple pages, new uploads are prioritized to the lower index pages, i.e.,
 * it will try to upload to page 0 before page 1 or 2. To keep the atlas from continually using
 * excess space, periodic garbage collection is needed to shift data from the higher index pages to
 * the lower ones, and then eventually remove any pages that are no longer in use. "In use" is
 * determined by using the AtlasToken system: After a DrawPass is snapped a subarea of the page, or
 * "plot" is checked to see whether it was used in that DrawPass. If less than a quarter of the
 * plots have been used recently (within kPlotRecentlyUsedCount iterations) and there are available
 * plots in lower index pages, the higher index page will be deactivated, and its glyphs will
 * gradually migrate to other pages via the usual upload system.
 *
 * Garbage collection is initiated by the DrawAtlas's client via the compact() method.
 */
class DrawAtlas final {
    class Plot;
    enum class PlotID : uint32_t { kInvalid = 0 };
    enum class EntryID : uint32_t { kInvalid = 0 };

public:
    static constexpr auto kMaxMultitexturePages = 4;

    /**
     * This is intended to be a storage-only type for space efficiency. It converts to/from SkIRect
     * for operations. Note that it stores unsigned values and will assert if built from an SkIRect
     * with a negative value.
     */
    class Rect16 final {
        uint16_t fL = 0, fT = 0, fR = 0, fB = 0;

    public:
        Rect16() = default;
        Rect16(const Rect16&) = default;
        Rect16& operator=(const Rect16&) = default;
        Rect16(SkIRect r)
                : fL{SkToU16(r.fLeft)}
                , fT{SkToU16(r.fTop)}
                , fR{SkToU16(r.fRight)}
                , fB{SkToU16(r.fBottom)} {}
        operator SkIRect() const { return SkIRect::MakeLTRB(fL, fT, fR, fB); }
    };

    class Record final {
        friend class DrawAtlas;
        friend class Plot;
        friend class BulkRecordUseUpdater;

        Record(PlotID plotID, EntryID entryID) : fPlotID{plotID}, fEntryID{entryID} {}

        PlotID fPlotID = PlotID::kInvalid;
        EntryID fEntryID = EntryID::kInvalid;

    public:
        bool operator==(const Record& that) const {
            return fPlotID == that.fPlotID && fEntryID == that.fEntryID;
        }
        bool operator!=(const Record& that) const { return !(*this == that); }

        Record() = default;
    };

    class RecordVisitor final {
    public:
        void visitRecords(std::invocable<Record> auto&& fn) const;

    private:
        friend class DrawAtlas;
        explicit RecordVisitor(Plot* p) : fPlot{p} {}
        Plot* fPlot;
    };

    /**
     * An interface for eviction callbacks. Whenever an atlas evicts Records,
     * it will call all the registered listeners so they can process the evictions.
     */
    class EvictionCallback {
    public:
        virtual ~EvictionCallback() = default;
        virtual void evict(const RecordVisitor& visitor) = 0;
    };

    class BulkRecordUseUpdater;

    /** Is the atlas allowed to use more than one texture? */
    enum class AllowMultitexturing : bool { kNo, kYes };

    /** Should the atlas use storage textures? */
    enum class UseStorageTextures : bool { kNo, kYes };

    /**
     * Returns a DrawAtlas.
     *  @param maskFormat          The maskFormat which this atlas will store.
     *  @param width               Width in pixels of the atlas.
     *  @param height              Height in pixels of the atlas.
     *  @param plotWidth           The width of each plot. width/plotWidth should be an integer.
     *  @param plotWidth           The height of each plot. height/plotHeight should be an integer.
     *  @param allowMultitexturing Can the atlas use more than one texture.
     *  @param useStorageTextures  Should the atlas use storage textures.
     *  @param evictor             A pointer to an eviction callback class.
     *  @param label               Label for texture resources.
     *
     *  @return                    An initialized DrawAtlas, or nullptr if creation fails.
     */
    static std::unique_ptr<DrawAtlas> Make(MaskFormat maskFormat,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           AllowMultitexturing allowMultitexturing,
                                           UseStorageTextures useStorageTextures,
                                           EvictionCallback* evictor,
                                           std::string_view label);

    /**
     * Adds a width x height subimage to the atlas. Upon success it returns 'kSucceeded' and returns
     * the ID and the subimage's coordinates in the backing texture. 'kTryAgain' is returned if
     * the subimage cannot fit in the atlas without overwriting texels that will be read in the
     * current list of draws. This indicates that the Device should end its current draw, snap a
     * DrawPass, and begin another before adding more data. 'kError' will be returned when some
     * unrecoverable error was encountered while trying to add the subimage. In this case the draw
     * being created should be discarded.
     *
     * This tracking does not generate UploadTasks per se. Instead, when the RenderPassTask is
     * ready to be snapped, recordUploads() will be called by the Device and that will generate the
     * necessary UploadTasks. If the useCachedUploads argument in recordUploads() is true, this
     * will generate uploads for the entire area of each Plot that has changed since the last
     * eviction. Otherwise it will only generate uploads for newly added changes.
     *
     * NOTE: When a draw that reads from the atlas is added to the DrawList, the client using this
     * DrawAtlas must immediately call 'setLastUseToken' with the currentToken from the Recorder,
     * otherwise the next call to addToAtlas might cause the previous data to be overwritten before
     * it has been read.
     */

    enum class ErrorCode {
        kError,
        kSucceeded,
        kTryAgain
    };

    struct AddResult {
        Record   fRecord;
        int      fPageIdx{};
        SkIPoint fPosition{};
    };

    ErrorCode addRect(Recorder*, SkISize, AddResult*, const std::byte* image = nullptr);

    std::optional<SkPixmap> entryPixmap(Record, int padding = 0, std::optional<SkColor> = {});
    std::optional<std::tuple<int /*pageIndex*/, SkIRect>> entryLocation(Record) const;

    bool recordUploads(DrawContext*, Recorder*);

    const sk_sp<TextureProxy>& getProxy(int pageIdx) { return fProxies[pageIdx]; }
    const sk_sp<TextureProxy>* getProxies() const { return fProxies; }

    int numActivePages() const { return fNumActivePages; }

    bool hasRecord(Record record) const;

    enum class EvictionCounter : uint64_t { kInvalid = 0 };

    /**
     * Instead of using EvictionCallback to track granular per-Record evictions, a client
     * can just track the number of eviction events. This method will return a larger number every
     * time *any* eviction has ocurred. A client can know that at least one Record's texture coords
     * will have been invalidated if this number increases between usages.
     */
    EvictionCounter evictionCounter() const { return fEvictionCounter; }

    /** To ensure the atlas does not evict a given entry, the client must set the last use token. */
    void setLastUseToken(Record record, Token token) {
        Plot* plot = this->findPlot(record);
        this->internalSetLastUseToken(plot, token);
    }

    void setLastUseTokenBulk(const BulkRecordUseUpdater& updater, Token token);

    void compact(Token startTokenForNextFlush);

    // Mark all plots with any content as full. Used only with Vello because it can't do
    // new renders to a texture without a clear.
    void markUsedPlotsAsFull();

    // Will try to clear out any GPU resources that aren't needed for any pending uploads or draws.
    // TODO: Delete backing data for Plots that don't have pending uploads.
    void freeGpuResources(Token token);

    void evictAllPlots();

    int maxPages() const { return fMaxPages; }

#if defined(GPU_TEST_UTILS)
    int numAllocatedPlots() const;
    int numNonEmptyPlots() const;
#endif

private:
    struct PlotCoord {
        int fPageIdx = 0;
        int fX       = 0;
        int fY       = 0;
        bool operator==(const PlotCoord& other) const {
            return fPageIdx == other.fPageIdx && fX == other.fX && fY == other.fY;
        }
        bool operator!=(const PlotCoord& other) const { return !(*this == other); }
    };

    static EntryID NextEntryID(DrawAtlas::EntryID);

    DrawAtlas(MaskFormat,
              int width,
              int height,
              int plotWidth,
              int plotHeight,
              AllowMultitexturing allowMultitexturing,
              UseStorageTextures useStorageTextures,
              std::string_view label);

    int plotsPerPage() const { return fPlotGridDimensions.area(); }
    SkISize plotSize() const { return {fPlotWidth, fPlotHeight}; }

    std::optional<AddResult> addRectToPage(int pageIdx, SkISize size, const std::byte* image);

    void makeMRU(int pageIndex, int plotIndex);

    Plot* findPlot(PlotID id) const;

    Plot* findPlot(Record record) const;

    void internalSetLastUseToken(Plot* plot, Token token);

    bool activateNewPage(Recorder*);
    void deactivateLastPage();

#if defined(GPU_TEST_UTILS)
    template <std::predicate<const Plot*> F> int iteratePlots(F&& func) const {
        int count = 0;
        int numPlots = this->plotsPerPage();
        for (int pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
            for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
                if (func(fPages[pageIndex].fPlotArray[plotIndex].get())) {
                    ++count;
                }
            }
        }
        return count;
    }
#endif

    enum class EvictionType {
        // The plot will be deleted (the Page is being removed)
        kWillBeRemoved,
        // Keep the plot at its plot coord for reuse but deallocate CPU backing store.
        kReuse_FreeData,
        // Same, but also keep (and clear) the CPU backing store.
        kReuse_KeepData,
    };
    void processEvictionAndResetRects(Plot*, EvictionType);

    MaskFormat fMaskFormat;
    int        fTextureWidth;
    int        fTextureHeight;
    SkISize    fPlotGridDimensions;
    int        fPlotWidth;
    int        fPlotHeight;

    UseStorageTextures fUseStorageTextures;
    const std::string  fLabel;

    static EvictionCounter NextEvictionCounter(EvictionCounter ec) {
        auto value = static_cast<std::underlying_type_t<EvictionCounter>>(ec);
        return static_cast<EvictionCounter>(value + 1);
    }

    // A counter that is bumped every time Records become invalidated.
    EvictionCounter fEvictionCounter = NextEvictionCounter(EvictionCounter::kInvalid);

    // nextFlushToken() value at the end of the previous DrawPass
    // TODO: rename
    Token fPrevFlushToken;

    // the number of flushes since this atlas has been last used
    // TODO: rename
    int fFlushesSinceLastUse;

    std::vector<EvictionCallback*> fEvictionCallbacks;

    struct Page {
        // allocated array of Plots, in MRU order (most recently used is entry [0]).
        std::unique_ptr<std::unique_ptr<Plot>[]> fPlotArray;
    };
    skia_private::THashMap<PlotID, Plot*> fPlotMap;

    // proxies kept separate to make it easier to pass them up to client
    sk_sp<TextureProxy> fProxies[kMaxMultitexturePages];
    Page fPages[kMaxMultitexturePages];
    int fMaxPages;

    int fNumActivePages;

    SkDEBUGCODE(void validate() const;)
};

/**
 * A class which can be handed back to an atlas for observing Record usages
 * in bulk.
 */
class DrawAtlas::BulkRecordUseUpdater final {
public:
    BulkRecordUseUpdater() = default;

    BulkRecordUseUpdater(const BulkRecordUseUpdater& that) : fPlotsToUpdate(that.fPlotsToUpdate) {}

    bool add(Record record) {
        auto plotID = record.fPlotID;
        if (fPlotsToUpdate.contains(plotID)) {
            return false;
        }
        fPlotsToUpdate.add(plotID);
        return true;
    }

    void reset() { fPlotsToUpdate.reset(); }

    // Call fn on every PlotID of every Plot that will be updated. fn may not manipulate this
    // BulkUseUpdater.
    template <std::invocable<PlotID> Fn>  // f(PlotID)
    void foreach(Fn&& fn) const {
        fPlotsToUpdate.foreach(std::forward<Fn>(fn));
    }

private:
    skia_private::THashSet<PlotID> fPlotsToUpdate;
};

/**
 * The backing texture for an atlas is broken into a spatial grid of Plots, which are the
 * unit size for cache purging. The Plots keep track of subimage placement via their Rectanizer.
 * Plots are initialized to zero for all mask formats. The client can configure the subdivision
 * of a DrawAtlas into plots (grid dimensions and size of plot). Otherwise, plots are an
 * implementation detail of DrawAtlas.
 */
class DrawAtlas::Plot final {
    Plot(PlotCoord, int indexInPageMru, SkISize plotDimensions, MaskFormat);

    Plot(const Plot&) = delete;
    Plot(Plot&&) = delete;

    Plot& operator=(const Plot&) = delete;
    Plot& operator=(Plot&&) = delete;

public:
    ~Plot();

    static std::unique_ptr<Plot> Make(PlotCoord plotCoord,
                                      int indexInPageMru,
                                      SkISize plotDimensions,
                                      MaskFormat maskFormat) {
        return std::unique_ptr<Plot>{new Plot{plotCoord, indexInPageMru, plotDimensions, maskFormat}};
    }

    PlotCoord plotCoord() const { return fPlotCoord; }
    PlotID plotID() const { return fPlotID; }

    size_t bpp() const { return MaskFormatBytesPerPixel(fMaskFormat); }
    size_t rowBytes() const { return fPlotDimensions.width() * this->bpp(); }

    struct AddResult {
        EntryID  fEntryID;
        SkIPoint fPositionInAtlas;  // Absolute coord in atlas, not plot-relative.
    };

    /*
     * Adds a rectangle to the plot. If image is not null it is copied into the backing store of the
     * plot. The plot's backing store is allocated if it was not previously. The row bytes of image
     * are assumed to be tight.
     */
    std::optional<AddResult> addRect(SkISize, const std::byte* image = nullptr);

    /**
     * Finds an entry in the plot and returns its rectangle in absolute atlas coords (not
     * plot-relative).
     */
    std::optional<SkIRect> entryAtlasRect(EntryID) const;

    /**
     * Finds an entry and returns a SkPixmap corresponding to the entry in the atlas backing store.
     * The backing store is allocated if it was not previously. If padding is not 0 then the pixmap
     * will be inset by this amout relative to the
     */
    std::optional<SkPixmap> entryPixmap(EntryID,
                                        int padding = 0,
                                        std::optional<SkColor> clearColor = {});

    void visitEntries(std::invocable<EntryID> auto&& fn) {
        fEntries.foreach([&fn](const auto& key, const auto& /*value*/) { std::invoke(fn, key); });
    }

    /**
     * To manage the lifetime of a plot, we use two tokens. We use the last upload token to
     * know when we can 'piggy back' uploads, i.e. if the last upload hasn't been flushed to
     * the gpu, we don't need to issue a new upload even if we update the cpu backing store. We
     * use lastUse to determine when we can evict a plot from the cache, i.e. if the last use
     * has already flushed through the gpu then we can reuse the plot.
     */
    Token lastUseToken() const { return fLastUse; }
    void setLastUseToken(Token token) { fLastUse = token; }

    int flushesSinceLastUsed() { return fFlushesSinceLastUse; }
    void resetFlushesSinceLastUsed() { fFlushesSinceLastUse = 0; }
    void incFlushesSinceLastUsed() { fFlushesSinceLastUse++; }

    int indexInPageMru() const { return fIndexInPageMru; }
    void updateIndexInPageMru(int index) { fIndexInPageMru = index; }

    bool needsUpload() { return !fDirtyRect.isEmpty(); }
    std::pair<const std::byte*, SkIRect> prepareForUpload();
    // Re-initialize Plot. The client should ensure that they process any eviction callbacks
    // before calling this, otherwise any cached references will point to invalid data.
    // If freeData is true, this will free the backing data as well. This should only be used
    // when we know we won't be adding to the Plot immediately afterwards.
    void recycle(bool freeData);

    void markFullIfUsed() { fIsFull = !fDirtyRect.isEmpty(); }
    bool isEmpty() const { return fRectanizer.percentFull() == 0; }
    bool hasAllocation() const { return fData != nullptr; }

private:
    std::byte* dataAt(SkIPoint atlasPoint);

    SkISize dimensions() const { return SkISize(fPlotDimensions); }

    SkIPoint topLeftInAtlas() const {
        auto [w, h]    = fPlotDimensions;
        auto [_, x, y] = fPlotCoord;
        return {x * w, y * h};
    }

    SkIRect boundsInAtlas() const {
        auto [w, h] = fPlotDimensions;
        auto [x, y] = this->topLeftInAtlas();
        return SkIRect::MakeXYWH(x, y, w, h);
    }

    SkIRect localBounds() const { return SkIRect::MakeSize(this->dimensions()); }

    SkIRect dirtyRect() const { return SkIRect(fDirtyRect); }

    void dirty(SkIRect rect) {
        SkASSERT(this->localBounds().contains(rect));
        auto dirtyRect = this->dirtyRect();
        dirtyRect.join(rect);
        fDirtyRect = dirtyRect;
    }

    // Returns the dirty rect where the left and right edges are 4 byte aligned
    SkIRect alignedDirtyRect() const {
        // bpp          1    2    4
        // clearBits 0b11 0b01 0b00
        unsigned clearBits = 0b11 >> (this->bpp() - 1);

        // Ensure that offsetting this rect from plot-local to atlas coord won't change alignment.
        SkASSERT(!(fPlotDimensions.width() & clearBits));

        auto [l, t, r, b] = fDirtyRect;

        SkIRect result = {
            .fLeft   = SkToS32(l & ~clearBits),
            .fTop    = t,
            .fRight  = SkToS32((r + clearBits) & ~clearBits),
            .fBottom = b
        };
        SkASSERT(this->localBounds().contains(result));
        return result;
    }

    std::byte* accessData() {
        if (!fData) {
            // This will zero-initialize the bytes.
            fData = std::make_unique<std::byte[]>(this->bpp() * fPlotDimensions.area());
        }
        return fData.get();
    }

    std::optional<std::tuple<EntryID, SkIPoint>> makeEntry(SkISize);

    static PlotID NextPlotID();

    std::unique_ptr<std::byte[]> fData;
    skia_private::THashMap<EntryID, Rect16> fEntries;
    RectanizerSkyline fRectanizer;

    Token fLastUse = Token::InvalidToken();
    int   fFlushesSinceLastUse = 0;

    PlotID fPlotID = NextPlotID();
    EntryID fPrevEntryID = EntryID::kInvalid;
    const SkISize fPlotDimensions;
    PlotCoord fPlotCoord;
    int fIndexInPageMru;

    MaskFormat fMaskFormat;
    SkIRect fDirtyRect = SkIRect::MakeEmpty();  // area in the Plot that needs to be uploaded
    bool fIsFull = false;
};

inline void DrawAtlas::RecordVisitor::visitRecords(std::invocable<Record> auto&& fn) const {
    fPlot->visitEntries([fn, pid = fPlot->plotID()](EntryID eid) {
        std::invoke(fn, Record{pid, eid});
    });
}

}  // namespace skgpu::graphite

#endif
