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
#include "src/base/SkTInternalLList.h"
#include "src/core/SkTHash.h"
#include "src/gpu/MaskFormat.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/gpu/Token.h"

#include <cstddef>
#include <cstdint>
#include <functional>
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
    enum class EntryID : int32_t {
        kEmpty = -1,
        kInvalid = 0
    };

public:
    // These are both restricted by the space they occupy in the PlotLocator.
    // maxPages was limited by being crammed into the glyph uvs in Ganesh
    // (no longer necessary in Graphite).
    // maxPlots is also limited by the fPlotAlreadyUpdated bitfield in
    // BulkUsePlotUpdater (below).
    static constexpr auto kMaxMultitexturePages = 4;
    static constexpr int kMaxPlots = 32;

    /**
     * This is intended to be a storage-only type for space efficiency. It converts to/from SkIRect
     * for operations. Note that it stores unsigned values and will assert if built from an SkIRect
     * with a negative value.
     */
    class Rect16 final {
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

    private:
        uint16_t fL = 0, fT = 0, fR = 0, fB = 0;
    };

    /**
     * Used to identify an entry in the atlas. The texture or coordinates of an entry may change
     * between uses and a Record can be used to find the current texture position.
     */
    class Record final {
    public:
        bool operator==(const Record& that) const {
            return fPlotID == that.fPlotID && fEntryID == that.fEntryID;
        }
        bool operator!=(const Record& that) const { return !(*this == that); }

        Record() = default;

    private:
        friend class DrawAtlas;
        friend class Plot;
        friend class BulkRecordUseUpdater;

        Record(PlotID plotID, EntryID entryID) : fPlotID{plotID}, fEntryID{entryID} {}

        PlotID fPlotID   = PlotID::kInvalid;
        EntryID fEntryID = EntryID::kInvalid;
    };

    /**
     * Used to visit a set of evicted Records. See EvictionCallback below.
     */
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

    // --- Legacy Polyfill classes ---
    class GenerationCounter {
    public:
        static constexpr uint64_t kInvalidGeneration = 0;
        uint64_t next() { return fGeneration++; }

    private:
        uint64_t fGeneration{1};
    };

    class PlotLocator {
    public:
        PlotLocator(uint32_t pageIdx, uint32_t plotIdx, uint64_t generation)
                : fGenID(generation), fPlotIndex(plotIdx), fPageIndex(pageIdx) {
            SkASSERT(pageIdx < kMaxMultitexturePages);
            SkASSERT(plotIdx < kMaxPlots);
            SkASSERT(generation < ((uint64_t)1 << 48));
        }

        PlotLocator() : fGenID(GenerationCounter::kInvalidGeneration), fPlotIndex(0), fPageIndex(0) {}

        bool isValid() const {
            return fGenID != GenerationCounter::kInvalidGeneration || fPlotIndex != 0 ||
                   fPageIndex != 0;
        }

        void makeInvalid() {
            fGenID = GenerationCounter::kInvalidGeneration;
            fPlotIndex = 0;
            fPageIndex = 0;
        }

        bool operator==(const PlotLocator& other) const {
            return fGenID == other.fGenID && fPlotIndex == other.fPlotIndex &&
                   fPageIndex == other.fPageIndex;
        }

        uint32_t pageIndex() const { return fPageIndex; }
        uint32_t plotIndex() const { return fPlotIndex; }
        uint64_t genID() const { return fGenID; }

    private:
        uint64_t fGenID : 48;
        uint64_t fPlotIndex : 8;
        uint64_t fPageIndex : 8;
    };

    // AtlasLocator handles atlas position information. It keeps a left-top, right-bottom pair of
    // encoded UV coordinates. The bits 13 & 14 of the U coordinates hold the atlas page index.
    // This information is handed directly as is from fUVs. This encoding has the nice property
    // that width = fUVs[2] - fUVs[0]; the page encoding in the top bits subtracts to zero.
    class AtlasLocator {
    public:
        std::array<uint16_t, 4> getUVs() const { return fUVs; }

        void invalidatePlotLocator() { fPlotLocator.makeInvalid(); }

        PlotLocator plotLocator() const { return fPlotLocator; }

        uint32_t pageIndex() const { return fPlotLocator.pageIndex(); }

        uint32_t plotIndex() const { return fPlotLocator.plotIndex(); }

        uint64_t genID() const { return fPlotLocator.genID(); }

        SkIPoint topLeft() const { return {fUVs[0] & 0x1FFF, fUVs[1]}; }

        SkISize dimensions() const { return {fUVs[2] - fUVs[0], fUVs[3] - fUVs[1]}; }

        uint16_t width() const { return fUVs[2] - fUVs[0]; }

        uint16_t height() const { return fUVs[3] - fUVs[1]; }

        void insetSrc(int padding) {
            SkASSERT(2 * padding <= this->width());
            SkASSERT(2 * padding <= this->height());

            fUVs[0] += padding;
            fUVs[1] += padding;
            fUVs[2] -= padding;
            fUVs[3] -= padding;
        }

        void updatePlotLocator(PlotLocator p) {
            fPlotLocator = p;
            SkASSERT(fPlotLocator.pageIndex() <= 3);
            uint16_t page = fPlotLocator.pageIndex() << 13;
            fUVs[0] = (fUVs[0] & 0x1FFF) | page;
            fUVs[2] = (fUVs[2] & 0x1FFF) | page;
        }

        void updateRect(SkIRect rect) {
            SkASSERT(rect.fLeft <= rect.fRight);
            SkASSERT(rect.fRight <= 0x1FFF);
            fUVs[0] = (fUVs[0] & 0xE000) | SkToU16(rect.fLeft);
            fUVs[1] = SkToU16(rect.fTop);
            fUVs[2] = (fUVs[2] & 0xE000) | SkToU16(rect.fRight);
            fUVs[3] = SkToU16(rect.fBottom);
        }

        Record record() const { return fRecord; }
        void updateRecord(Record r) { fRecord = r; }

    private:
        PlotLocator fPlotLocator{0, 0, GenerationCounter::kInvalidGeneration};
        std::array<uint16_t, 4> fUVs{0, 0, 0, 0};
        Record fRecord;
    };

    class PlotEvictionCallback {
    public:
        virtual ~PlotEvictionCallback() = default;
        virtual void evict(PlotLocator) = 0;
    };

    /**
     * A class which can be handed back to an atlas for updating plots in bulk.  The
     * current max number of plots per page an atlas can handle is 32. If in the future
     * this is insufficient then we can move to a 64 bit int.
     */
    class BulkUsePlotUpdater {
    public:
        BulkUsePlotUpdater() { memset(fPlotAlreadyUpdated, 0, sizeof(fPlotAlreadyUpdated)); }
        BulkUsePlotUpdater(const BulkUsePlotUpdater& that) : fPlotsToUpdate(that.fPlotsToUpdate) {
            memcpy(fPlotAlreadyUpdated, that.fPlotAlreadyUpdated, sizeof(fPlotAlreadyUpdated));
        }

        bool add(const AtlasLocator& atlasLocator) {
            int plotIdx = atlasLocator.plotIndex();
            int pageIdx = atlasLocator.pageIndex();
            if (this->find(pageIdx, plotIdx)) {
                return false;
            }
            this->set(pageIdx, plotIdx);
            return true;
        }

        void reset() {
            fPlotsToUpdate.clear();
            memset(fPlotAlreadyUpdated, 0, sizeof(fPlotAlreadyUpdated));
        }

        struct PlotData {
            PlotData(int pageIdx, int plotIdx) : fPageIndex(pageIdx), fPlotIndex(plotIdx) {}
            uint32_t fPageIndex;
            uint32_t fPlotIndex;
        };

        int count() const { return fPlotsToUpdate.size(); }

        const PlotData& plotData(int index) const { return fPlotsToUpdate[index]; }

    private:
        bool find(int pageIdx, int index) const {
            SkASSERT(index < kMaxPlots);
            return (fPlotAlreadyUpdated[pageIdx] >> index) & 1;
        }

        void set(int pageIdx, int index) {
            SkASSERT(!this->find(pageIdx, index));
            fPlotAlreadyUpdated[pageIdx] |= (1 << index);
            fPlotsToUpdate.push_back(PlotData(pageIdx, index));
        }

        inline static constexpr int kMinItems = 4;
        skia_private::STArray<kMinItems, PlotData, true> fPlotsToUpdate;
        uint32_t fPlotAlreadyUpdated[kMaxMultitexturePages];
    };

    enum class AllowMultitexturing : bool { kNo, kYes };
    enum class UseStorageTextures : bool { kNo, kYes };

    static std::unique_ptr<DrawAtlas> Make(MaskFormat maskFormat,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           AllowMultitexturing allowMultitexturing,
                                           UseStorageTextures useStorageTextures,
                                           EvictionCallback* evictor,
                                           std::string_view label);

    // Legacy Make overload
    static std::unique_ptr<DrawAtlas> Make(MaskFormat maskFormat,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           GenerationCounter* generationCounter,
                                           AllowMultitexturing allowMultitexturing,
                                           UseStorageTextures useStorageTextures,
                                           PlotEvictionCallback* evictor,
                                           std::string_view label);

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
    bool hasRecord(Record record) const;

    std::optional<SkPixmap> entryPixmap(Record, int padding = 0, std::optional<SkColor> = {});
    std::optional<std::tuple<int /*pageIndex*/, SkIRect>> entryLocation(Record) const;

    // Legacy public methods
    ErrorCode addToAtlas(Recorder*, int width, int height, const void* image, AtlasLocator*);
    ErrorCode addRect(Recorder*, int width, int height, AtlasLocator*);
    // Returns a Pixmap pointing to the backing data for the locator. Optionally, the caller can
    // provide an inset that is applied to all four sides. This is useful for use cases that need
    // to leave space between items in the atlas. The pixmap will exclude the padding. The entire
    // atlas is cleared to zero when allocated. By passing an initialColor here, the caller can
    // re-clear the entire locator's rect (including any padding) to any color.
    SkPixmap prepForRender(const AtlasLocator&,
                           int padding = 0,
                           std::optional<SkColor> initialColor = {});

    bool recordUploads(DrawContext*, Recorder*);

    const sk_sp<TextureProxy>& getProxy(int pageIdx) { return fProxies[pageIdx]; }
    const sk_sp<TextureProxy>* getProxies() const { return fProxies; }

    int numActivePages() const { return fNumActivePages; }
    uint64_t atlasGeneration() const { return static_cast<uint64_t>(fEvictionCounter); }
    unsigned int plotsPerPage() const { return fPlotGridDimensions.area(); }
    unsigned int numPlots() const { return this->plotsPerPage(); }
    SkISize plotSize() const { return {fPlotWidth, fPlotHeight}; }
    uint32_t getListIndex(const PlotLocator& locator) const;

    bool hasID(const PlotLocator& plotLocator);

    /** To ensure the atlas does not evict a given entry, the client must set the last use token. */
    void setLastUseToken(const AtlasLocator& atlasLocator, Token token);

    void setLastUseToken(Record record, Token token) {
        Plot* plot = this->findPlot(record.fPlotID);
        this->internalSetLastUseToken(plot, token);
    }

    void setLastUseTokenBulk(const BulkUsePlotUpdater& updater, Token token);
    void setLastUseTokenBulk(const BulkRecordUseUpdater& updater, Token token);

    enum class EvictionCounter : uint64_t { kInvalid = 0 };

    EvictionCounter evictionCounter() const { return fEvictionCounter; }

    void compact(Token startTokenForNextFlush);
    void markUsedPlotsAsFull();
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
        int fX       = 0;  // x position in grid of plots
        int fY       = 0;  // y position in grid of plots
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

    std::optional<AddResult> addRectToPage(int pageIdx, SkISize size, const std::byte* image);

    void makeMRU(Plot* plot);

    Plot* findPlot(PlotID id) const;
    Plot* findPlot(Record record) const;
    Plot* findPlot(const AtlasLocator& atlasLocator);

    void internalSetLastUseToken(Plot* plot, Token token);
    void internalSetLastUseToken(Plot* plot, uint32_t pageIdx, Token token);

    bool activateNewPage(Recorder*);
    void deactivateLastPage();

#if defined(GPU_TEST_UTILS)
    template <typename F> int iteratePlots(F&& func) const {
        int count = 0;
        int numPlots = this->plotsPerPage();
        for (int pageIndex = 0; pageIndex < fNumActivePages; ++pageIndex) {
            for (int plotIndex = 0; plotIndex < numPlots; ++plotIndex) {
                if (func(fPages[pageIndex].fPlotArray[plotIndex].get())) {
                    count++;
                }
            }
        }
        return count;
    }
#endif

    enum class EvictionType {
        kWillBeRemoved,
        kReuse_FreeData,
        kReuse_KeepData,
    };
    void processEvictionAndResetRects(Plot* plot, EvictionType type);
    // If freeData is true, this will free the backing data as well. This should only be used
    // when we know we won't be adding to the Plot immediately afterwards.
    void processEvictionAndResetRects(Plot* plot, bool freeData);

    static EvictionCounter NextEvictionCounter(EvictionCounter ec) {
        auto value = static_cast<std::underlying_type_t<EvictionCounter>>(ec);
        return static_cast<EvictionCounter>(value + 1);
    }

    MaskFormat         fMaskFormat;
    int                fTextureWidth;
    int                fTextureHeight;
    SkISize            fPlotGridDimensions;
    int                fPlotWidth;
    int                fPlotHeight;
    UseStorageTextures fUseStorageTextures;
    const std::string  fLabel;

    EvictionCounter fEvictionCounter = NextEvictionCounter(EvictionCounter::kInvalid);

    // nextFlushToken() value at the end of the previous DrawPass
    // TODO: rename
    Token fPrevFlushToken;

    // the number of flushes since this atlas has been last used
    // TODO: rename
    int fFlushesSinceLastUse;

    std::vector<PlotEvictionCallback*> fPlotEvictionCallbacks;
    std::vector<EvictionCallback*> fEvictionCallbacks;

    struct Page {
        // allocated array of Plots
        std::unique_ptr<std::unique_ptr<Plot>[]> fPlotArray;
        // LRU list of Plots (MRU at head - LRU at tail)
        SkTInternalLList<Plot> fMRUList;
    };
    skia_private::THashMap<PlotID, Plot*> fPlotMap;

    // proxies kept separate to make it easier to pass them up to client
    sk_sp<TextureProxy> fProxies[kMaxMultitexturePages];
    Page fPages[kMaxMultitexturePages];
    const int fMaxPages;
    int fNumActivePages;

    SkDEBUGCODE(void validate() const;)
};

/**
 * The backing texture for an atlas is broken into a spatial grid of Plots, which are the
 * unit size for cache purging. The Plots keep track of subimage placement via their Rectanizer.
 * Plots are initialized to zero for all mask formats. The client can configure the subdivision
 * of a DrawAtlas into plots (grid dimensions and size of plot). Otherwise, plots are an
 * implementation detail of DrawAtlas.
 */
class DrawAtlas::Plot final {
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Plot);
    Plot(PlotCoord, uint32_t plotIndex, SkISize plotDimensions, MaskFormat);
    Plot(const Plot&) = delete;
    Plot(Plot&&) = delete;

    Plot& operator=(const Plot&) = delete;
    Plot& operator=(Plot&&) = delete;

public:
    static std::unique_ptr<Plot> Make(PlotCoord plotCoord,
                                      uint32_t plotIndex,
                                      SkISize plotDimensions,
                                      MaskFormat maskFormat) {
        return std::unique_ptr<Plot>{new Plot{plotCoord, plotIndex, plotDimensions, maskFormat}};
    }

    ~Plot();

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

    std::optional<SkPixmap> entryPixmap(EntryID,
                                        int padding = 0,
                                        std::optional<SkColor> clearColor = {});

    void visitEntries(std::invocable<EntryID> auto&& fn) const {
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

    bool needsUpload() { return !fDirtyRect.isEmpty(); }
    std::pair<const std::byte*, SkIRect> prepareForUpload();
    void recycle(bool freeData);

    void markFullIfUsed() { fIsFull = !fDirtyRect.isEmpty(); }
    bool isEmpty() const { return fRectanizer.percentFull() == 0; }
    bool hasAllocation() const { return fData != nullptr; }

    // --- Legacy Plot wrappers/helpers ---
    uint64_t genID() const { return static_cast<uint64_t>(fPlotID); }
    PlotLocator plotLocator() const {
        return PlotLocator(fPlotCoord.fPageIdx, fPlotIndex, static_cast<uint64_t>(fPlotID));
    }

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
        auto dRect = this->dirtyRect();
        dRect.join(rect);
        fDirtyRect = dRect;
    }
    // Returns the dirty rect where the left and right edges are 4 byte aligned
    SkIRect alignedDirtyRect() const {
        unsigned clearBits = 0b11 >> (this->bpp() - 1);
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
            fData = std::make_unique<std::byte[]>(this->bpp() * fPlotDimensions.area());
        }
        return fData.get();
    }
    std::optional<std::tuple<EntryID, SkIPoint>> makeEntry(SkISize size)  {
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
    static PlotID NextPlotID();

    std::unique_ptr<std::byte[]> fData;
    skia_private::THashMap<EntryID, Rect16> fEntries;
    RectanizerSkyline fRectanizer;
    Token fLastUse             = Token::InvalidToken();
    int   fFlushesSinceLastUse = 0;
    PlotID fPlotID = NextPlotID();
    EntryID fPrevEntryID = EntryID::kInvalid;
    const SkISize fPlotDimensions;
    const uint32_t fPlotIndex;
    PlotCoord fPlotCoord;
    MaskFormat fMaskFormat;
    SkIRect fDirtyRect = SkIRect::MakeEmpty();
    bool fIsFull = false;
};

inline uint32_t DrawAtlas::getListIndex(const PlotLocator& locator) const {
    return locator.pageIndex() * this->plotsPerPage() + locator.plotIndex();
}

inline bool DrawAtlas::hasID(const PlotLocator& plotLocator) {
    if (!plotLocator.isValid()) {
        return false;
    }

    uint32_t plot = plotLocator.plotIndex();
    uint32_t page = plotLocator.pageIndex();
    if (page >= static_cast<uint32_t>(fNumActivePages) || plot >= static_cast<uint32_t>(this->plotsPerPage())) {
        return false;
    }
    Plot* plotPtr = fPages[page].fPlotArray[plot].get();
    return plotPtr && static_cast<uint64_t>(plotPtr->plotID()) == plotLocator.genID();
}

inline void DrawAtlas::setLastUseToken(const AtlasLocator& atlasLocator, Token token) {
    Plot* plot = this->findPlot(atlasLocator);
    this->internalSetLastUseToken(plot, atlasLocator.pageIndex(), token);
}

inline void DrawAtlas::setLastUseTokenBulk(const BulkUsePlotUpdater& updater, Token token) {
    int count = updater.count();
    for (int i = 0; i < count; i++) {
        const BulkUsePlotUpdater::PlotData& pd = updater.plotData(i);
        if (pd.fPageIndex < static_cast<uint32_t>(fNumActivePages)) {
            Plot* plot = fPages[pd.fPageIndex].fPlotArray[pd.fPlotIndex].get();
            this->internalSetLastUseToken(plot, pd.fPageIndex, token);
        }
    }
}

inline DrawAtlas::Plot* DrawAtlas::findPlot(const AtlasLocator& atlasLocator) {
    SkASSERT(this->hasID(atlasLocator.plotLocator()));
    uint32_t pageIdx = atlasLocator.pageIndex();
    uint32_t plotIdx = atlasLocator.plotIndex();
    return fPages[pageIdx].fPlotArray[plotIdx].get();
}

inline void DrawAtlas::internalSetLastUseToken(Plot* plot, uint32_t pageIdx, Token token) {
    this->makeMRU(plot);
    plot->setLastUseToken(token);
}

inline void DrawAtlas::internalSetLastUseToken(Plot* plot, Token token) {
    this->makeMRU(plot);
    plot->setLastUseToken(token);
}

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

    template <std::invocable<PlotID> Fn>
    void foreach(Fn&& fn) const {
        fPlotsToUpdate.foreach(std::forward<Fn>(fn));
    }

private:
    skia_private::THashSet<PlotID> fPlotsToUpdate;
};

inline void DrawAtlas::setLastUseTokenBulk(const BulkRecordUseUpdater& updater, Token token) {
    updater.foreach([&](PlotID pid) {
        if (auto* plot = this->findPlot(pid)) {
            this->internalSetLastUseToken(plot, token);
        }
    });
}

inline void DrawAtlas::RecordVisitor::visitRecords(std::invocable<Record> auto&& fn) const {
    fPlot->visitEntries([fn, pid = fPlot->plotID()](EntryID eid) {
        std::invoke(fn, Record{pid, eid});
    });
}

}  // namespace skgpu::graphite

#endif
