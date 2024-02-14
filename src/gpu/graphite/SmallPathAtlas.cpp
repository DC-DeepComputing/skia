/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SmallPathAtlas.h"

/*
 #include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
*/
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

SmallPathAtlas::SmallPathAtlas(Recorder* recorder)
        : PathAtlas(recorder, kDefaultAtlasDim, kDefaultAtlasDim) {
}

bool SmallPathAtlas::initAtlas() {
    if (fDrawAtlas) {
        return true;
    }

    static constexpr size_t kPlotWidth = 512;
    static constexpr size_t kPlotHeight = 256;

    SkColorType colorType = kAlpha_8_SkColorType;
    // TODO: Set atlas size based on maxTextureSize()
    fDrawAtlas = DrawAtlas::Make(colorType,
                                 SkColorTypeBytesPerPixel(colorType),
                                 kDefaultAtlasDim, kDefaultAtlasDim,
                                 kPlotWidth, kPlotHeight,
                                 this,
                                 DrawAtlas::AllowMultitexturing::kYes,
                                 this,
                                 /*label=*/"SmallPathAtlas");
    fKeyLists.reset(fDrawAtlas->numPlots() * fDrawAtlas->maxPages());

    return SkToBool(fDrawAtlas);
}

bool SmallPathAtlas::recordUploads(DrawContext* dc) {
    return fDrawAtlas->recordUploads(dc, fRecorder);
}

const TextureProxy* SmallPathAtlas::onAddShape(const Shape& shape,
                                               const Transform& transform,
                                               const SkStrokeRec& strokeRec,
                                               skvx::half2 maskSize,
                                               skvx::half2* outPos) {
    skgpu::UniqueKey maskKey;
    bool hasKey = shape.hasKey();
    if (hasKey) {
        maskKey = GeneratePathMaskKey(shape, transform, strokeRec, maskSize);
        ShapeData* shapeData = fShapeCache.find(maskKey);
        if (shapeData) {
            SkIPoint topLeft = shapeData->fAtlasLocator.topLeft();
            *outPos = skvx::half2(topLeft.x(), topLeft.y());
            return fDrawAtlas->getProxies()[shapeData->fAtlasLocator.pageIndex()].get();
        }
    }

    // Render mask
    // TODO: Render directly into the atlas backing store, rather than doing a copy.
    // This will require some refactoring of DrawAtlas.
    SkAutoPixmapStorage dst;
    // Rasterize path to backing pixmap
    RasterMaskHelper helper(&dst);
    if (!helper.init({maskSize.x(), maskSize.y()})) {
        return nullptr;
    }
    SkIRect iAtlasBounds = SkIRect::MakeXYWH(0, 0, maskSize.x(), maskSize.y());
    helper.drawShape(shape, transform, strokeRec, iAtlasBounds);
    sk_sp<SkData> pixelData = dst.detachPixelsAsData();

    // Add to DrawAtlas
    AtlasLocator locator;
    DrawAtlas::ErrorCode errorCode = fDrawAtlas->addToAtlas(fRecorder,
                                                            iAtlasBounds.width(),
                                                            iAtlasBounds.height(),
                                                            pixelData->data(),
                                                            &locator);
    if (errorCode != DrawAtlas::ErrorCode::kSucceeded) {
        return nullptr;
    }
    SkIPoint topLeft = locator.topLeft();
    *outPos = skvx::half2(topLeft.x(), topLeft.y());

    if (hasKey) {
        // Add to caches
        ShapeData shapeData;
        shapeData.fAtlasLocator = locator;
        fShapeCache.set(maskKey, shapeData);
        uint32_t index = locator.pageIndex() * fDrawAtlas->numPlots() + locator.plotIndex();
        ShapeKeyEntry* keyEntry = new ShapeKeyEntry();
        keyEntry->fKey = maskKey;
        fKeyLists[index].addToTail(keyEntry);
    }

    return fDrawAtlas->getProxies()[locator.pageIndex()].get();
}

void SmallPathAtlas::evict(PlotLocator plotLocator) {
    // Remove all these entries from the shapecache
    uint32_t index = plotLocator.pageIndex() * fDrawAtlas->numPlots() + plotLocator.plotIndex();
    ShapeKeyList::Iter iter;
    iter.init(fKeyLists[index], ShapeKeyList::Iter::kHead_IterStart);
    ShapeKeyEntry* currEntry;
    while ((currEntry = iter.get())) {
        iter.next();
        fShapeCache.remove(currEntry->fKey);
        fKeyLists[index].remove(currEntry);
        delete currEntry;
    }
}

}  // namespace skgpu::graphite
