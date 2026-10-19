/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/StripGenerator.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/GpuAlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <type_traits>

namespace skgpu::graphite {

StripGenerator::StripGenerator(int width,
                               int height,
                               const SparseStripConfig::LUTArray& maskLUT,
                               Recorder* recorder)
        : fWidth(width)
        , fHeight(height)
        , fMaskLUT(maskLUT)
        , fRecorder(recorder)
        , fWindingHistogram((height + SparseStripConfig::kTileHeight - 1) /
                            SparseStripConfig::kTileHeight)
        , fCulled(true)
        , fUseGpuCoverage(recorder->priv().rendererProvider()->pathRendererStrategy() ==
                          PathRendererStrategy::kGPUSparseStripsMSAA8) {}

StripGenerator::~StripGenerator() = default;

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm) {
    return this->processGeometry(path, ctm, SkIRect::MakeWH(fWidth, fHeight));
}

template <typename AtlasManager>
bool StripGenerator::makeStrips(const SkPath& path,
                                const SkMatrix& ctm,
                                const SkIRect& tileClip,
                                AtlasManager* atlasManager) {
    if (!atlasManager) {
        return false;
    }

    Flatten flattener;
    Polyline polyline;
    flattener.processPaths<FlattenMode::kSimd>(path, ctm, tileClip, &polyline);

    if (polyline.count() == 0 && !path.isInverseFillType()) {
        return true;
    }

    fWindingHistogram.resize((fHeight + SparseStripConfig::kTileHeight - 1) /
                             SparseStripConfig::kTileHeight);
    if (fCulled) {
        fWindingHistogram.clear();
    }
#if defined(SK_DEBUG)
    // Tiler only writes to the histogram when it reports a culling event, so if the previous draw
    // did not cull, the histogram must still be zero from the last time it was cleared.
    for (int16_t winding : fWindingHistogram.data()) {
        SkASSERT(winding == 0);
    }
#endif

    Tiles<SparseStripConfig::kTileWidth, SparseStripConfig::kTileHeight> tiler;
    fCulled = tiler.makeTilesMSAA(polyline, tileClip, &fWindingHistogram);
    tiler.sortTiles();

    bool success;
    if constexpr (std::is_same_v<AtlasManager, GpuAlphaAtlasManager>) {
        success =
                MakeStrips::MsaaGpu<SparseStripConfig::kTileWidth, SparseStripConfig::kTileHeight>(
                        tiler,
                        &fWides,
                        &fEnds,
                        atlasManager,
                        path.getFillType(),
                        polyline,
                        fCulled,
                        fWindingHistogram,
                        tileClip);
    } else {
        static_assert(std::is_same_v<AtlasManager, AlphaAtlasManager>);
        success =
                MakeStrips::MsaaSimd<SparseStripConfig::kTileWidth, SparseStripConfig::kTileHeight>(
                        tiler,
                        &fWides,
                        &fEnds,
                        atlasManager,
                        path.getFillType(),
                        polyline,
                        fMaskLUT,
                        fCulled,
                        fWindingHistogram,
                        tileClip);
    }

    if (!success) {
        return false;
    }

    if (!fEnds.empty()) {
        atlasManager->populateProxies(&fEnds);
    }
    return true;
}

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
    fEnds.clear();
    fWides.clear();

    SkIRect tileClip = clip;
    if (!tileClip.intersect(SkIRect::MakeWH(fWidth, fHeight))) {
        return true;
    }
    tileClip.fLeft =
            (tileClip.fLeft / SparseStripConfig::kTileWidth) * SparseStripConfig::kTileWidth;
    tileClip.fTop =
            (tileClip.fTop / SparseStripConfig::kTileHeight) * SparseStripConfig::kTileHeight;

    AtlasProvider* atlasProvider = fRecorder->priv().getOrCreateAtlasProvider();
    if (fUseGpuCoverage) {
        return this->makeStrips(path, ctm, tileClip, atlasProvider->gpuAlphaAtlasManager());
    }
    return this->makeStrips(path, ctm, tileClip, atlasProvider->alphaAtlasManager());
}

bool StripGenerator::cleanupNullCaps() {
    AtlasProvider* atlasProvider = fRecorder->priv().getOrCreateAtlasProvider();
    if (fUseGpuCoverage) {
        GpuAlphaAtlasManager* atlasManager = atlasProvider->gpuAlphaAtlasManager();
        SkASSERT(atlasManager);
        return atlasManager->resolveNullCaps(&fEnds);
    }
    AlphaAtlasManager* atlasManager = atlasProvider->alphaAtlasManager();
    SkASSERT(atlasManager);
    return atlasManager->resolveNullCaps(&fEnds);
}

}  // namespace skgpu::graphite
