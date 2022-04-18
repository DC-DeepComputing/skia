/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawAtlas_DEFINED
#define skgpu_DrawAtlas_DEFINED

#include <cmath>
#include <vector>

#include "include/gpu/GrBackendSurface.h"
#include "include/private/SkTArray.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/RectanizerSkyline.h"
#include "src/gpu/ganesh/GrDeferredUpload.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/geometry/GrRect.h"

namespace skgpu {
/**
 ******** FIX
 * This class manages one or more atlas textures on behalf of GrDrawOps. The draw ops that use the
 * atlas perform texture uploads when preparing their draws during flush. The class provides
 * facilities for using GrDrawOpUploadToken to detect data hazards. Op's uploads are performed in
 * "ASAP" mode until it is impossible to add data without overwriting texels read by draws that
 * have not yet executed on the gpu. At that point, the atlas will attempt to allocate a new
 * atlas texture (or "page") of the same size, up to a maximum number of textures, and upload
 * to that texture. If that's not possible, the uploads are performed "inline" between draws. If a
 * single draw would use enough subimage space to overflow the atlas texture then the atlas will
 * fail to add a subimage. This gives the op the chance to end the draw and begin a new one.
 * Additional uploads will then succeed in inline mode.
 *
 * When the atlas has multiple pages, new uploads are prioritized to the lower index pages, i.e.,
 * it will try to upload to page 0 before page 1 or 2. To keep the atlas from continually using
 * excess space, periodic garbage collection is needed to shift data from the higher index pages to
 * the lower ones, and then eventually remove any pages that are no longer in use. "In use" is
 * determined by using the GrDrawUploadToken system: After a flush each subarea of the page
 * is checked to see whether it was used in that flush; if it is not, a counter is incremented.
 * Once that counter reaches a threshold that subarea is considered to be no longer in use.
 *
 * Garbage collection is initiated by the GrDrawOpAtlas's client via the compact() method. One
 * solution is to make the client a subclass of GrOnFlushCallbackObject, register it with the
 * GrContext via addOnFlushCallbackObject(), and the client's postFlush() method calls compact()
 * and passes in the given GrDrawUploadToken.
 */
class DrawAtlas {
public:
    /** Is the atlas allowed to use more than one texture? */
    enum class AllowMultitexturing : bool { kNo, kYes };

    // These are both restricted by the space they occupy in the PlotLocator.
    // MaxMultitexturePages is also limited by being crammed into the glyph uvs.
    // MaxPlots is also limited by the fPlotAlreadyUpdated bitfield in
    // GrDrawOpAtlas::BulkUseTokenUpdater
    inline static constexpr auto kMaxMultitexturePages = 4;
    inline static constexpr int kMaxPlots = 32;

    /**
     * A PlotLocator specifies the plot and is analogous to a directory path:
     *    page/plot/plotGeneration
     *
     * In fact PlotLocator is a portion of a glyph image location in the atlas fully specified by:
     *    format/atlasGeneration/page/plot/plotGeneration/rect
     *
     * TODO: Remove the small path renderer's use of the PlotLocator for eviction.
     */
    class PlotLocator {
    public:
        PlotLocator(uint32_t pageIdx, uint32_t plotIdx, uint64_t generation)
                : fGenID(generation)
                , fPlotIndex(plotIdx)
                , fPageIndex(pageIdx) {
            SkASSERT(pageIdx < kMaxMultitexturePages);
            SkASSERT(plotIdx < kMaxPlots);
            SkASSERT(generation < ((uint64_t)1 << 48));
        }

        PlotLocator() : fGenID(0), fPlotIndex(0), fPageIndex(0) {}

        bool isValid() const {
            return fGenID != 0 || fPlotIndex != 0 || fPageIndex != 0;
        }

        void makeInvalid() {
            fGenID = 0;
            fPlotIndex = 0;
            fPageIndex = 0;
        }

        bool operator==(const PlotLocator& other) const {
            return fGenID == other.fGenID &&
                   fPlotIndex == other.fPlotIndex &&
                   fPageIndex == other.fPageIndex; }

        uint32_t pageIndex() const { return fPageIndex; }
        uint32_t plotIndex() const { return fPlotIndex; }
        uint64_t genID() const { return fGenID; }

    private:
        uint64_t fGenID:48;
        uint64_t fPlotIndex:8;
        uint64_t fPageIndex:8;
    };

    static const uint64_t kInvalidAtlasGeneration = 0;


    // AtlasLocator handles atlas position information. It keeps a left-top, right-bottom pair of
    // encoded UV coordinates. The bits 13 & 14 of the U coordinates hold the atlas page index.
    // This information is handed directly as is from fUVs. This encoding has the nice property
    // that width = fUVs[2] - fUVs[0]; the page encoding in the top bits subtracts to zero.
    class AtlasLocator {
    public:
        std::array<uint16_t, 4> getUVs() const {
            return fUVs;
        }

        void invalidatePlotLocator() { fPlotLocator.makeInvalid(); }

        // TODO: Remove the small path renderer's use of this for eviction
        PlotLocator plotLocator() const { return fPlotLocator; }

        uint32_t pageIndex() const { return fPlotLocator.pageIndex(); }

        uint32_t plotIndex() const { return fPlotLocator.plotIndex(); }

        uint64_t genID() const { return fPlotLocator.genID(); }

        SkIPoint topLeft() const {
            return {fUVs[0] & 0x1FFF, fUVs[1]};
        }

        uint16_t width() const {
            return fUVs[2] - fUVs[0];
        }

        uint16_t height() const {
            return fUVs[3] - fUVs[1];
        }

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

        void updateRect(GrIRect16 rect) {
            SkASSERT(rect.fLeft <= rect.fRight);
            SkASSERT(rect.fRight <= 0x1FFF);
            fUVs[0] = (fUVs[0] & 0xE000) | rect.fLeft;
            fUVs[1] = rect.fTop;
            fUVs[2] = (fUVs[2] & 0xE000) | rect.fRight;
            fUVs[3] = rect.fBottom;
        }

    private:
        PlotLocator fPlotLocator{0, 0, 0};

        // The inset padded bounds in the atlas in the lower 13 bits, and page index in bits 13 &
        // 14 of the Us.
        std::array<uint16_t, 4> fUVs{0, 0, 0, 0};
    };

    /**
     * An interface for eviction callbacks. Whenever DrawAtlas evicts a specific PlotLocator,
     * it will call all of the registered listeners so they can process the eviction.
     */
    class EvictionCallback {
    public:
        virtual ~EvictionCallback() = default;
        virtual void evict(PlotLocator) = 0;
    };

    /**
     * Keep track of generation number for Atlases and Plots.
     */
    class GenerationCounter {
    public:
        inline static constexpr uint64_t kInvalidGeneration = 0;
        uint64_t next() {
            return fGeneration++;
        }

    private:
        uint64_t fGeneration{1};
    };

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

    uint32_t numActivePages() { return fNumActivePages; }

    uint32_t maxPages() const {
        return fMaxPages;
    }

    int numAllocated_TestingOnly() const;
    void setMaxPages_TestingOnly(uint32_t maxPages);

protected:
    DrawAtlas(GrProxyProvider*, const GrBackendFormat& format, GrColorType, int width,
              int height, int plotWidth, int plotHeight, GenerationCounter* generationCounter,
              AllowMultitexturing allowMultitexturing);

    /**
     * The backing texture for a DrawAtlas is broken into a spatial grid of Plots. The Plots
     * keep track of subimage placement via their Rectanizer. A Plot manages the lifetime of its
     * data using two tokens, a last use token and a last upload token. Once a Plot is "full" (i.e.
     * there is no room for the new subimage according to the Rectanizer), it can no longer be
     * used unless the last use of the Plot has already been flushed through to the gpu.
     */
    class Plot : public SkRefCnt {
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(Plot);

    public:
        uint32_t pageIndex() const { return fPageIndex; }

        /** plotIndex() is a unique id for the plot relative to the owning GrAtlas and page. */
        uint32_t plotIndex() const { return fPlotIndex; }
        /**
         * genID() is incremented when the plot is evicted due to a atlas spill. It is used to know
         * if a particular subimage is still present in the atlas.
         */
        uint64_t genID() const { return fGenID; }
        PlotLocator plotLocator() const {
            SkASSERT(fPlotLocator.isValid());
            return fPlotLocator;
        }
        SkDEBUGCODE(size_t bpp() const { return fBytesPerPixel; })

        bool addSubImage(int width, int height, const void* image, AtlasLocator* atlasLocator);

        void resetRects();

    private:
        Plot(int pageIndex, int plotIndex, GenerationCounter* generationCounter,
             int offX, int offY, int width, int height, GrColorType colorType);

        ~Plot() override;

        /**
         * Create a clone of this plot. The cloned plot will take the place of the current plot in
         * the atlas
         */
        Plot* clone() const {
            return new Plot(
                fPageIndex, fPlotIndex, fGenerationCounter, fX, fY, fWidth, fHeight, fColorType);
        }

        struct {
            const uint32_t fPageIndex : 16;
            const uint32_t fPlotIndex : 16;
        };
        GenerationCounter* const fGenerationCounter;
        uint64_t fGenID;
        PlotLocator fPlotLocator;
        unsigned char* fData;
        const int fWidth;
        const int fHeight;
        const int fX;
        const int fY;
        skgpu::RectanizerSkyline fRectanizer;
        const SkIPoint16 fOffset;  // the offset of the plot in the backing texture
        const GrColorType fColorType;
        const size_t fBytesPerPixel;
        SkIRect fDirtyRect;
        SkDEBUGCODE(bool fDirty);

        friend class GrDrawOpAtlas;

        using INHERITED = SkRefCnt;
    };

    typedef SkTInternalLList<Plot> PlotList;

    inline void makeMRU(Plot* plot, int pageIdx) {
        if (fPages[pageIdx].fPlotList.head() == plot) {
            return;
        }

        fPages[pageIdx].fPlotList.remove(plot);
        fPages[pageIdx].fPlotList.addToHead(plot);

        // No MRU update for pages -- since we will always try to add from
        // the front and remove from the back there is no need for MRU.
    }

    bool createPages(GrProxyProvider*, GenerationCounter*);
    bool activateNewPage(GrResourceProvider*);
    void deactivateLastPage();

    void processEviction(PlotLocator);
    inline void processEvictionAndResetRects(Plot* plot) {
        this->processEviction(plot->plotLocator());
        plot->resetRects();
    }

    GrColorType           fColorType;
    int                   fTextureWidth;
    int                   fTextureHeight;
    int                   fPlotWidth;
    int                   fPlotHeight;
    unsigned int          fNumPlots;

    GenerationCounter* const fGenerationCounter;
    uint64_t                 fAtlasGeneration;

    std::vector<EvictionCallback*> fEvictionCallbacks;

    struct Page {
        // allocated array of Plots
        std::unique_ptr<sk_sp<Plot>[]> fPlotArray;
        // LRU list of Plots (MRU at head - LRU at tail)
        PlotList fPlotList;
    };
    Page fPages[kMaxMultitexturePages];
    uint32_t fMaxPages;

    uint32_t fNumActivePages;

    SkDEBUGCODE(void validate(const AtlasLocator& atlasLocator) const;)
};

}  // End of namespace skgpu

#endif
