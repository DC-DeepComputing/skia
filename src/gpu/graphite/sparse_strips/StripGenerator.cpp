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
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

namespace skgpu::graphite {

namespace {

// The scratch buffers are kept across draws, unless a large path grew them past this. Freeing
// those bounds the memory held between draws, and one allocation is negligible next to processing
// that many points or tiles.
constexpr size_t kMaxRetainedScratchBytes = 1 << 20;

}  // anonymous namespace

struct StripGenerator::Scratch {
    Polyline fPolyline;
    Flatten fFlatten;
    Tiles<SparseStripConfig::kTileWidth, SparseStripConfig::kTileHeight> fTiles;

    bool isLarge() const {
        const size_t pointBytes =
                sizeof(SkPoint) * static_cast<size_t>(fPolyline.points().capacity());
        const size_t tileBytes = sizeof(Tile) * static_cast<size_t>(fTiles.getTiles().capacity());
        return pointBytes > kMaxRetainedScratchBytes || tileBytes > kMaxRetainedScratchBytes;
    }
};

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
        , fScratch(std::make_unique<Scratch>())
        , fCulled(true) {}

StripGenerator::~StripGenerator() = default;

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm) {
    return this->processGeometry(path, ctm, SkIRect::MakeWH(fWidth, fHeight));
}

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
    const bool success = this->processGeometryImpl(path, ctm, clip);
    // The polyline and tiles have been consumed: the results are in fEnds and fWides.
    if (fScratch->isLarge()) {
        fScratch = std::make_unique<Scratch>();
    }
    return success;
}

bool StripGenerator::processGeometryImpl(const SkPath& path,
                                         const SkMatrix& ctm,
                                         const SkIRect& clip) {
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

    AlphaAtlasManager* atlasManager =
            fRecorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager();
    if (!atlasManager) {
        return false;
    }

    Polyline& polyline = fScratch->fPolyline;
    polyline.reset();
    fScratch->fFlatten.processPaths<FlattenMode::kSimd>(path, ctm, tileClip, &polyline);

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

    auto& tiler = fScratch->fTiles;
    tiler.reset();
    fCulled = tiler.makeTilesMSAA(polyline, tileClip, &fWindingHistogram);
    tiler.sortTiles();

    bool success =
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

    if (!success) {
        return false;
    }

    if (!fEnds.empty()) {
        atlasManager->populateProxies(&fEnds);
    }
    return true;
}

bool StripGenerator::cleanupNullCaps() {
    AlphaAtlasManager* atlasManager =
            fRecorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager();
    SkASSERT(atlasManager);
    return atlasManager->resolveNullCaps(&fEnds);
}

}  // namespace skgpu::graphite
