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

StripGenerator::StripGenerator(int width,
                               int height,
                               const SparseStripConfig::LutArray& maskLUT,
                               Recorder* recorder)
        : fWidth(width)
        , fHeight(height)
        , fMaskLUT(maskLUT)
        , fRecorder(recorder)
        , fWindingHistogram((height + SparseStripConfig::kTileHeight - 1) /
                            SparseStripConfig::kTileHeight)
        , fCulled(true) {}

StripGenerator::~StripGenerator() = default;

#if defined(GPU_TEST_UTILS)
static StripGenerator::DebugCaptureFn& debug_capture_fn() {
    static StripGenerator::DebugCaptureFn* gFn = new StripGenerator::DebugCaptureFn();
    return *gFn;
}

void StripGenerator::SetDebugCaptureCallback(DebugCaptureFn fn) {
    debug_capture_fn() = std::move(fn);
}
#endif

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm) {
    return this->processGeometry(path, ctm, SkIRect::MakeWH(fWidth, fHeight));
}

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
#if defined(GPU_TEST_UTILS)
    if (const auto& captureFn = debug_capture_fn()) {
        captureFn(path, ctm, clip, SkISize::Make(fWidth, fHeight));
    }
#endif
    fEnds.clear();
    fWides.clear();

    AlphaAtlasManager* atlasManager =
            fRecorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager();
    if (!atlasManager) {
        return false;
    }

    Flatten flattener;
    Polyline polyline;
    flattener.processPaths<FlattenMode::kSimd>(path, ctm, clip, &polyline);

    if (polyline.count() == 0) {
        return true;
    }

    fWindingHistogram.resize((fHeight + SparseStripConfig::kTileHeight - 1) /
                             SparseStripConfig::kTileHeight);
    if (fCulled) {
        fWindingHistogram.clear();
    }

    Tiles<SparseStripConfig::kTileWidth, SparseStripConfig::kTileHeight> tiler;
    fCulled = tiler.makeTilesMSAA(polyline, clip, fWindingHistogram);
    tiler.sortTiles();

    bool success = MakeStrips::MsaaSimd<SparseStripConfig::kTileWidth,
                                        SparseStripConfig::kTileHeight>(
            tiler,
            &fWides,
            &fEnds,
            atlasManager,
            path.getFillType(),
            polyline,
            fMaskLUT,
            fCulled,
            fWindingHistogram,
            clip);

    if (!success) {
        return false;
    }

    atlasManager->populateProxies(&fEnds);
    return true;
}

bool StripGenerator::cleanupNullCaps() {
    AlphaAtlasManager* atlasManager =
            fRecorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager();
    SkASSERT(atlasManager);
    return atlasManager->resolveNullCaps(&fEnds);
}

}  // namespace skgpu::graphite
