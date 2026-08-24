/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripGenerator_DEFINED
#define skgpu_graphite_sparse_strips_StripGenerator_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/ganesh/geometry/GrPathUtils.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>

namespace skgpu::graphite {

class StripGenerator {
public:
    static constexpr uint16_t kTileWidth = SparseStripConfig::kTileWidth;
    static constexpr uint16_t kTileHeight = SparseStripConfig::kTileHeight;

    StripGenerator(uint16_t viewportWidth, uint16_t viewportHeight, const SkTDArray<uint8_t>& lut,
                   Recorder* recorder)
        : fViewportWidth(viewportWidth), fViewportHeight(viewportHeight), fLUT(lut)
        , fRecorder(recorder) {}

    ~StripGenerator() {
        this->reset();
    }

    bool processGeometry(const SkPath& path, const SkMatrix& ctm) {
        this->reset();

        fFlattener.processPaths<FlattenMode::kSimd>(path, ctm, fViewportWidth, fViewportHeight,
                                                    &fFlattened);

        fTiles.makeTilesMSAA(fFlattened, fViewportWidth, fViewportHeight);
        fTiles.sortTiles();

        AlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->alphaAtlasManager();

        MakeStrips::MsaaSimd<kTileWidth, kTileHeight>(
                fTiles, fWides, fEnds, manager,
                path.getFillType(), fFlattened, fLUT, fViewportWidth, fViewportHeight);

        return true;
    }

    const WideTiles<kTileHeight>& wides() const { return fWides; }
    const EndCaps<kTileHeight>& ends() const { return fEnds; }
    WideTiles<kTileHeight>& wides() { return fWides; }
    EndCaps<kTileHeight>& ends() { return fEnds; }

private:
    void reset() {
        fFlattened.reset();
        fTiles.reset();
        fWides.clear();
        fEnds.clear();
    }

    const uint16_t fViewportWidth;
    const uint16_t fViewportHeight;

    Flatten fFlattener;
    Tiles<kTileWidth, kTileHeight> fTiles;
    Polyline fFlattened;
    const SkTDArray<uint8_t>& fLUT;
    Recorder* fRecorder;

    WideTiles<kTileHeight> fWides;
    EndCaps<kTileHeight> fEnds;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
