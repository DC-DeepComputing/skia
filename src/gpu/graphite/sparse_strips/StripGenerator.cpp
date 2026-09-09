/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/StripGenerator.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"

namespace skgpu::graphite {

StripGenerator::StripGenerator(uint16_t viewportWidth,
                               uint16_t viewportHeight,
                               const SkTDArray<uint8_t>& lut,
                               Recorder* recorder)
        : fViewportWidth(viewportWidth)
        , fViewportHeight(viewportHeight)
        , fLUT(lut)
        , fRecorder(recorder) {}

StripGenerator::~StripGenerator() {
    this->reset();
}

void StripGenerator::reset() {
    fFlattened.reset();
    fTiles.reset();
    fWides.clear();
    fEnds.clear();
}

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm) {
    this->reset();

    AlphaAtlasManager* manager =
            fRecorder ? fRecorder->priv().atlasProvider()->alphaAtlasManager() : nullptr;
    if (!manager) {
        return false;
    }

    fFlattener.processPaths<FlattenMode::kSimd>(path, ctm, fViewportWidth, fViewportHeight,
                                                &fFlattened);

    fTiles.makeTilesMSAA(fFlattened, fViewportWidth, fViewportHeight);
    fTiles.sortTiles();

    if (!MakeStrips::MsaaSimd<kTileWidth, kTileHeight>(
            fTiles, &fWides, &fEnds, manager,
            path.getFillType(), fFlattened, fLUT, fViewportWidth, fViewportHeight)) {
        manager->abortRun();
        if (manager->inNullCaps()) {
            manager->resetNullCaps();
        }
        return false;
    }

    if (!this->hasNullCaps()) {
        manager->populateProxies(&fEnds);
    }
    return true;
}

bool StripGenerator::cleanupNullCaps(DrawContext* dc) {
    if (!this->hasNullCaps()) {
        return true;
    }
    AlphaAtlasManager* manager =
            fRecorder ? fRecorder->priv().atlasProvider()->alphaAtlasManager() : nullptr;
    if (!manager) {
        return false;
    }
    return manager->resolveNullCaps(&fEnds, dc);
}

}  // namespace skgpu::graphite
