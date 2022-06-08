/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawAtlas_DEFINED
#define skgpu_graphite_DrawAtlas_DEFINED

#include <cmath>
#include <string>
#include <string_view>
#include <vector>

#include "src/core/SkIPoint16.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/RectanizerSkyline.h"

namespace skgpu::graphite {

class DrawContext;
class Recorder;
class TextureProxy;

/**
 * TODO: the process described here is tentative, and this comment revised once locked down.
 *
 * This class manages one or more atlas textures on behalf of primitive draws in Device. The drawing
 * processes that use the atlas add texture upload tasks when preparing their draws during flush.
 * The class provides facilities for using DrawTokens to detect data hazards. Uploads are queued in
 * the DrawContext until it is impossible to add data without overwriting texels read by draws that
 * have not yet been snapped to a RenderPassTask. At that point, the atlas will attempt to allocate
 * a new atlas texture (or "page") of the same size, up to a maximum number of textures, and upload
 * to that texture. If that's not possible, then the atlas will fail to add a subimage. This gives
 * Device the chance to end the current draw, snap a RenderpassTask, and begin a new one.
 * Additional uploads will then succeed.
 *
 * When the atlas has multiple pages, new uploads are prioritized to the lower index pages, i.e.,
 * it will try to upload to page 0 before page 1 or 2. To keep the atlas from continually using
 * excess space, periodic garbage collection is needed to shift data from the higher index pages to
 * the lower ones, and then eventually remove any pages that are no longer in use. "In use" is
 * determined by using the DrawToken system: After a DrawPass is snapped each subarea of the page
 * is checked to see whether it was used in that DrawPass; if it is not, a counter is incremented.
 * Once that counter reaches a threshold that subarea is considered to be no longer in use.
 *
 * Garbage collection is initiated by the DrawAtlas's client via the compact() method.
 */
class DrawAtlas {
public:
    /** Is the atlas allowed to use more than one texture? */
    enum class AllowMultitexturing : bool { kNo, kYes };

    /**
     * Returns a DrawAtlas.
     *  @param ct               The colorType which this atlas will store
     *  @param bpp              Size in bytes of each pixel
     *  @param width            width in pixels of the atlas
     *  @param height           height in pixels of the atlas
     *  @param numPlotsX        The number of plots the atlas should be broken up into in the X
     *                          direction
     *  @param numPlotsY        The number of plots the atlas should be broken up into in the Y
     *                          direction
     *  @param atlasGeneration  a pointer to the context's generation counter.
     *  @param allowMultitexturing Can the atlas use more than one texture.
     *  @param evictor          A pointer to an eviction callback class.
     *
     *  @return                 An initialized DrawAtlas, or nullptr if creation fails
     */
    static std::unique_ptr<DrawAtlas> Make(SkColorType ct, size_t bpp,
                                           int width, int height,
                                           int plotWidth, int plotHeight,
                                           AtlasGenerationCounter* generationCounter,
                                           AllowMultitexturing allowMultitexturing,
                                           PlotEvictionCallback* evictor,
                                           std::string_view label);

    /**
     * TODO: the process described here is tentative, and this comment revised once locked down.
     *
     * Adds a width x height subimage to the atlas. Upon success it returns 'kSucceeded' and returns
     * the ID and the subimage's coordinates in the backing texture. 'kTryAgain' is returned if
     * the subimage cannot fit in the atlas without overwriting texels that will be read in the
     * current list of draws. This indicates that the Device should end its current draw, snap a
     * DrawPass, and begin another before adding more data. Upon success, an upload of the provided
     * image data will have been added to the UploadList in DrawContext. Successive uploads in
     * either mode may be consolidated. 'kError' will be returned when some unrecoverable error was
     * encountered while trying to add the subimage. In this case the draw being created should be
     * discarded.
     *
     * NOTE: When a draw that reads from the atlas is added to the DrawList, it must immediately
     * call 'setUseToken' with the currentToken from the Recorder, otherwise the next call
     * to addToAtlas might cause the previous data to be overwritten before it has been read.
     */

    enum class ErrorCode {
        kError,
        kSucceeded,
        kTryAgain
    };

    ErrorCode addToAtlas(DrawContext*, Recorder*,
                         int width, int height, const void* image, AtlasLocator*);
    bool recordUploads(DrawContext*, Recorder*);

    const sk_sp<TextureProxy>* getProxies() const { return fProxies; }

    uint64_t atlasGeneration() const { return fAtlasGeneration; }

    bool hasID(const PlotLocator& plotLocator) {
        if (!plotLocator.isValid()) {
            return false;
        }

        uint32_t plot = plotLocator.plotIndex();
        uint32_t page = plotLocator.pageIndex();
        uint64_t plotGeneration = fPages[page].fPlotArray[plot]->genID();
        uint64_t locatorGeneration = plotLocator.genID();
        return plot < fNumPlots && page < fNumActivePages && plotGeneration == locatorGeneration;
    }

    /** To ensure the atlas does not evict a given entry, the client must set the last use token. */
    void setLastUseToken(const AtlasLocator& atlasLocator, DrawToken token) {
        SkASSERT(this->hasID(atlasLocator.plotLocator()));
        uint32_t plotIdx = atlasLocator.plotIndex();
        SkASSERT(plotIdx < fNumPlots);
        uint32_t pageIdx = atlasLocator.pageIndex();
        SkASSERT(pageIdx < fNumActivePages);
        Plot* plot = fPages[pageIdx].fPlotArray[plotIdx].get();
        this->makeMRU(plot, pageIdx);
        plot->setLastUseToken(token);
    }

    uint32_t numActivePages() { return fNumActivePages; }

    void setLastUseTokenBulk(const BulkUsePlotUpdater& updater,
                             DrawToken token) {
        int count = updater.count();
        for (int i = 0; i < count; i++) {
            const BulkUsePlotUpdater::PlotData& pd = updater.plotData(i);
            // it's possible we've added a plot to the updater and subsequently the plot's page
            // was deleted -- so we check to prevent a crash
            if (pd.fPageIndex < fNumActivePages) {
                Plot* plot = fPages[pd.fPageIndex].fPlotArray[pd.fPlotIndex].get();
                this->makeMRU(plot, pd.fPageIndex);
                plot->setLastUseToken(token);
            }
        }
    }

    void compact(DrawToken startTokenForNextFlush);

    uint32_t maxPages() const {
        return fMaxPages;
    }

    int numAllocated_TestingOnly() const;
    void setMaxPages_TestingOnly(uint32_t maxPages);

private:
    DrawAtlas(SkColorType, size_t bpp, int width, int height, int plotWidth, int plotHeight,
              AtlasGenerationCounter* generationCounter,
              AllowMultitexturing allowMultitexturing, std::string_view label);

    bool updatePlot(DrawContext*, Recorder*, AtlasLocator*, Plot* plot);

    inline void makeMRU(Plot* plot, int pageIdx) {
        if (fPages[pageIdx].fPlotList.head() == plot) {
            return;
        }

        fPages[pageIdx].fPlotList.remove(plot);
        fPages[pageIdx].fPlotList.addToHead(plot);

        // No MRU update for pages -- since we will always try to add from
        // the front and remove from the back there is no need for MRU.
    }

    bool uploadToPage(unsigned int pageIdx, DrawContext*, Recorder*,
                      int width, int height, const void* image, AtlasLocator*);

    bool createPages(AtlasGenerationCounter*);
    bool activateNewPage(Recorder*);
    void deactivateLastPage();

    void processEviction(PlotLocator);
    inline void processEvictionAndResetRects(Plot* plot) {
        this->processEviction(plot->plotLocator());
        plot->resetRects();
    }

    SkColorType           fColorType;
    size_t                fBytesPerPixel;
    int                   fTextureWidth;
    int                   fTextureHeight;
    int                   fPlotWidth;
    int                   fPlotHeight;
    unsigned int          fNumPlots;
    const std::string     fLabel;

    AtlasGenerationCounter* const fGenerationCounter;
    uint64_t                      fAtlasGeneration;

    // nextTokenToFlush() value at the end of the previous DrawPass
    // TODO: rename
    DrawToken fPrevFlushToken;

    // the number of flushes since this atlas has been last used
    // TODO: rename
    int fFlushesSinceLastUse;

    std::vector<PlotEvictionCallback*> fEvictionCallbacks;

    struct Page {
        // allocated array of Plots
        std::unique_ptr<sk_sp<Plot>[]> fPlotArray;
        // LRU list of Plots (MRU at head - LRU at tail)
        PlotList fPlotList;
    };
    // proxies kept separate to make it easier to pass them up to client
    sk_sp<TextureProxy> fProxies[PlotLocator::kMaxMultitexturePages];
    Page fPages[PlotLocator::kMaxMultitexturePages];
    uint32_t fMaxPages;

    uint32_t fNumActivePages;

    SkDEBUGCODE(void validate(const AtlasLocator& atlasLocator) const;)
};

// For text there are three atlases (A8, 565, ARGB) that are kept in relation with one another. In
// general, the A8 dimensions are 2x the 565 and ARGB dimensions with the constraint that an atlas
// size will always contain at least one plot. Since the ARGB atlas takes the most space, its
// dimensions are used to size the other two atlases.
class DrawAtlasConfig {
public:
    // The capabilities of the GPU define maxTextureSize. The client provides maxBytes, and this
    // represents the largest they want a single atlas texture to be. Due to multitexturing, we
    // may expand temporarily to use more space as needed.
    DrawAtlasConfig(int maxTextureSize, size_t maxBytes);

    // For testing only - make minimum sized atlases -- a single plot for ARGB, four for A8
    DrawAtlasConfig() : DrawAtlasConfig(kMaxAtlasDim, 0) {}

    SkISize atlasDimensions(MaskFormat type) const;
    SkISize plotDimensions(MaskFormat type) const;

private:
    // On some systems texture coordinates are represented using half-precision floating point,
    // which limits the largest atlas dimensions to 2048x2048.
    // For simplicity we'll use this constraint for all of our atlas textures.
    // This can be revisited later if we need larger atlases.
    inline static constexpr int kMaxAtlasDim = 2048;

    SkISize fARGBDimensions;
    int     fMaxTextureSize;
};

}  // namespace skgpu::graphite

#endif
