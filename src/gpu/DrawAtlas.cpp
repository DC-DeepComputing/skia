/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/DrawAtlas.h"

#include <memory>

#include "include/private/SkTPin.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkOpts.h"
#include "src/core/SkTraceEvent.h"

namespace skgpu {

#ifdef DUMP_ATLAS_DATA
static bool gDumpAtlasData = false;
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

////////////////////////////////////////////////////////////////////////////////
DrawAtlas::Plot::Plot(int pageIndex, int plotIndex, GenerationCounter* generationCounter,
        int offX, int offY, int width, int height, SkColorType colorType, size_t bpp)
        : fPageIndex(pageIndex)
        , fPlotIndex(plotIndex)
        , fGenerationCounter(generationCounter)
        , fGenID(fGenerationCounter->next())
        , fPlotLocator(fPageIndex, fPlotIndex, fGenID)
        , fData(nullptr)
        , fWidth(width)
        , fHeight(height)
        , fX(offX)
        , fY(offY)
        , fRectanizer(width, height)
        , fOffset(SkIPoint16::Make(fX * fWidth, fY * fHeight))
        , fColorType(colorType)
        , fBytesPerPixel(bpp)
#ifdef SK_DEBUG
        , fDirty(false)
#endif
{
    // We expect the allocated dimensions to be a multiple of 4 bytes
    SkASSERT(((width*fBytesPerPixel) & 0x3) == 0);
    // The padding for faster uploads only works for 1, 2 and 4 byte texels
    SkASSERT(fBytesPerPixel != 3 && fBytesPerPixel <= 4);
    fDirtyRect.setEmpty();
}

DrawAtlas::Plot::~Plot() {
    sk_free(fData);
}

bool DrawAtlas::Plot::addSubImage(
        int width, int height, const void* image, AtlasLocator* atlasLocator) {
    SkASSERT(width <= fWidth && height <= fHeight);

    SkIPoint16 loc;
    if (!fRectanizer.addRect(width, height, &loc)) {
        return false;
    }

    auto rect = skgpu::IRect16::MakeXYWH(loc.fX, loc.fY, width, height);

    if (!fData) {
        fData = reinterpret_cast<unsigned char*>(
                sk_calloc_throw(fBytesPerPixel * fWidth * fHeight));
    }
    size_t rowBytes = width * fBytesPerPixel;
    const unsigned char* imagePtr = (const unsigned char*)image;
    // point ourselves at the right starting spot
    unsigned char* dataPtr = fData;
    dataPtr += fBytesPerPixel * fWidth * rect.fTop;
    dataPtr += fBytesPerPixel * rect.fLeft;
    // copy into the data buffer, swizzling as we go if this is ARGB data
    constexpr bool kBGRAIsNative = kN32_SkColorType == kBGRA_8888_SkColorType;
    if (4 == fBytesPerPixel && kBGRAIsNative) {
        for (int i = 0; i < height; ++i) {
            SkOpts::RGBA_to_BGRA((uint32_t*)dataPtr, (const uint32_t*)imagePtr, width);
            dataPtr += fBytesPerPixel * fWidth;
            imagePtr += rowBytes;
        }
    } else {
        for (int i = 0; i < height; ++i) {
            memcpy(dataPtr, imagePtr, rowBytes);
            dataPtr += fBytesPerPixel * fWidth;
            imagePtr += rowBytes;
        }
    }

    fDirtyRect.join({rect.fLeft, rect.fTop, rect.fRight, rect.fBottom});

    rect.offset(fOffset.fX, fOffset.fY);
    atlasLocator->updateRect(rect);
    SkDEBUGCODE(fDirty = true;)

    return true;
}

void DrawAtlas::Plot::resetRects() {
    fRectanizer.reset();

    fGenID = fGenerationCounter->next();
    fPlotLocator = PlotLocator(fPageIndex, fPlotIndex, fGenID);
    this->onResetRects();

    // zero out the plot
    if (fData) {
        sk_bzero(fData, fBytesPerPixel * fWidth * fHeight);
    }

    this->clearDirtyRect();
}

std::tuple<const void*, size_t, SkIRect> DrawAtlas::Plot::getUploadData() {
    // We should only be issuing uploads if we are in fact dirty
    SkASSERT(fDirty && fData);
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    size_t rowBytes = fBytesPerPixel * fWidth;
    const unsigned char* dataPtr = fData;
    // Clamp to 4-byte aligned boundaries
    unsigned int clearBits = 0x3 / fBytesPerPixel;
    fDirtyRect.fLeft &= ~clearBits;
    fDirtyRect.fRight += clearBits;
    fDirtyRect.fRight &= ~clearBits;
    SkASSERT(fDirtyRect.fRight <= fWidth);
    // Set up dataPtr
    dataPtr += rowBytes * fDirtyRect.fTop;
    dataPtr += fBytesPerPixel * fDirtyRect.fLeft;
    SkIRect offsetRect = fDirtyRect.makeOffset(fOffset.fX, fOffset.fY);

    this->clearDirtyRect();

    return { dataPtr, rowBytes, offsetRect };
}

void DrawAtlas::Plot::clearDirtyRect() {
    fDirtyRect.setEmpty();
    SkDEBUGCODE(fDirty = false;)
}

///////////////////////////////////////////////////////////////////////////////

DrawAtlas::DrawAtlas(SkColorType colorType, size_t bpp, int width, int height,
                     int plotWidth, int plotHeight, GenerationCounter* generationCounter,
                     EvictionCallback* evictor, AllowMultitexturing allowMultitexturing)
        : fColorType(colorType)
        , fBytesPerPixel(bpp)
        , fTextureWidth(width)
        , fTextureHeight(height)
        , fPlotWidth(plotWidth)
        , fPlotHeight(plotHeight)
        , fGenerationCounter(generationCounter)
        , fAtlasGeneration(fGenerationCounter->next())
        , fEvictionCallback(evictor)
        , fMaxPages(AllowMultitexturing::kYes == allowMultitexturing ? kMaxMultitexturePages : 1)
        , fNumActivePages(0) {
    int numPlotsX = width/plotWidth;
    int numPlotsY = height/plotHeight;
    SkASSERT(numPlotsX * numPlotsY <= DrawAtlas::kMaxPlots);
    SkASSERT(fPlotWidth * numPlotsX == fTextureWidth);
    SkASSERT(fPlotHeight * numPlotsY == fTextureHeight);

    fNumPlots = numPlotsX * numPlotsY;
}

void DrawAtlas::processEviction(PlotLocator plotLocator) {
    fEvictionCallback->evict(plotLocator);

    fAtlasGeneration = fGenerationCounter->next();
}

void DrawAtlas::initPage(int index) {
    int numPlotsX = fTextureWidth/fPlotWidth;
    int numPlotsY = fTextureHeight/fPlotHeight;

    fPages[index].fPlotArray = std::make_unique<sk_sp<Plot>[]>(numPlotsX * numPlotsY);

    sk_sp<Plot>* currPlot = fPages[index].fPlotArray.get();
    for (int y = numPlotsY - 1, r = 0; y >= 0; --y, ++r) {
        for (int x = numPlotsX - 1, c = 0; x >= 0; --x, ++c) {
            uint32_t plotIndex = r * numPlotsX + c;
            currPlot->reset(this->makePlot(index, plotIndex, fGenerationCounter, x, y));

            // build LRU list
            fPages[index].fPlotList.addToHead(currPlot->get());
            ++currPlot;
        }
    }
}

uint32_t DrawAtlas::internalActivateNewPage() {
    SkASSERT(fNumActivePages < this->maxPages());

#ifdef DUMP_ATLAS_DATA
    if (gDumpAtlasData) {
        SkDebugf("activated page#: %d\n", fNumActivePages);
    }
#endif

    ++fNumActivePages;
    return fNumActivePages - 1;
}


uint32_t DrawAtlas::internalDeactivateLastPage() {
    SkASSERT(fNumActivePages);

    uint32_t lastPageIndex = fNumActivePages - 1;

    int numPlotsX = fTextureWidth/fPlotWidth;
    int numPlotsY = fTextureHeight/fPlotHeight;

    fPages[lastPageIndex].fPlotList.reset();
    for (int r = 0; r < numPlotsY; ++r) {
        for (int c = 0; c < numPlotsX; ++c) {
            uint32_t plotIndex = r * numPlotsX + c;

            Plot* currPlot = fPages[lastPageIndex].fPlotArray[plotIndex].get();
            currPlot->resetAll();

            // rebuild the LRU list
            SkDEBUGCODE(currPlot->fPrev = currPlot->fNext = nullptr);
            SkDEBUGCODE(currPlot->fList = nullptr);
            fPages[lastPageIndex].fPlotList.addToHead(currPlot);
        }
    }

    --fNumActivePages;

    return lastPageIndex;
}

DrawAtlasConfig::DrawAtlasConfig(int maxTextureSize, size_t maxBytes) {
    static const SkISize kARGBDimensions[] = {
        {256, 256},   // maxBytes < 2^19
        {512, 256},   // 2^19 <= maxBytes < 2^20
        {512, 512},   // 2^20 <= maxBytes < 2^21
        {1024, 512},  // 2^21 <= maxBytes < 2^22
        {1024, 1024}, // 2^22 <= maxBytes < 2^23
        {2048, 1024}, // 2^23 <= maxBytes
    };

    // Index 0 corresponds to maxBytes of 2^18, so start by dividing it by that
    maxBytes >>= 18;
    // Take the floor of the log to get the index
    int index = maxBytes > 0
        ? SkTPin<int>(SkPrevLog2(maxBytes), 0, SK_ARRAY_COUNT(kARGBDimensions) - 1)
        : 0;

    SkASSERT(kARGBDimensions[index].width() <= kMaxAtlasDim);
    SkASSERT(kARGBDimensions[index].height() <= kMaxAtlasDim);
    fARGBDimensions.set(std::min<int>(kARGBDimensions[index].width(), maxTextureSize),
                        std::min<int>(kARGBDimensions[index].height(), maxTextureSize));
    fMaxTextureSize = std::min<int>(maxTextureSize, kMaxAtlasDim);
}

SkISize DrawAtlasConfig::atlasDimensions(MaskFormat type) const {
    if (MaskFormat::kA8 == type) {
        // A8 is always 2x the ARGB dimensions, clamped to the max allowed texture size
        return { std::min<int>(2 * fARGBDimensions.width(), fMaxTextureSize),
                 std::min<int>(2 * fARGBDimensions.height(), fMaxTextureSize) };
    } else {
        return fARGBDimensions;
    }
}

SkISize DrawAtlasConfig::plotDimensions(MaskFormat type) const {
    if (MaskFormat::kA8 == type) {
        SkISize atlasDimensions = this->atlasDimensions(type);
        // For A8 we want to grow the plots at larger texture sizes to accept more of the
        // larger SDF glyphs. Since the largest SDF glyph can be 170x170 with padding, this
        // allows us to pack 3 in a 512x256 plot, or 9 in a 512x512 plot.

        // This will give us 512x256 plots for 2048x1024, 512x512 plots for 2048x2048,
        // and 256x256 plots otherwise.
        int plotWidth = atlasDimensions.width() >= 2048 ? 512 : 256;
        int plotHeight = atlasDimensions.height() >= 2048 ? 512 : 256;

        return { plotWidth, plotHeight };
    } else {
        // ARGB and LCD always use 256x256 plots -- this has been shown to be faster
        return { 256, 256 };
    }
}

}  // End of namespace skgpu
