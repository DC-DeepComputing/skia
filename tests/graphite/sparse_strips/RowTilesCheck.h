/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_sparse_strips_RowTilesCheck_DEFINED
#define skgpu_graphite_sparse_strips_RowTilesCheck_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <cstdint>
#include <cstring>
#include <memory>

namespace skgpu::graphite {

class Recorder;

// StripGenerator tiles into per-row buffers (Tiles<..., /*kRows=*/true>), while the validators tile
// flat, to inspect the tiles. RowTilesRun runs MakeStrips over per-row tiles of the same polyline,
// so that the validators can check that both layouts produce identical output.
template <uint16_t kTileWidth, uint16_t kTileHeight> struct RowTilesRun {
    WideTiles wides;
    EndCaps ends;
    std::unique_ptr<AlphaAtlasManager> atlas;
    SkTDArray<uint8_t> masks;
    SkTDArray<skvx::int8> windings;
    bool isCulled = false;

    void run(Recorder* recorder,
             bool scalar,
             const Polyline& polyline,
             const SkIRect& clip,
             uint16_t numTileRows,
             SkPathFillType fillType,
             const SparseStripConfig::LUTArray& maskLut) {
        WindingHistogram histogram(numTileRows);
        histogram.resize(numTileRows);
        histogram.clear();
        Tiles<kTileWidth, kTileHeight, /*kRows=*/true> tiles;
        isCulled = tiles.makeTilesMSAA(polyline, clip, &histogram);
        tiles.sortTiles();

        atlas = std::make_unique<AlphaAtlasManager>(recorder);
        auto observer = [this](uint8_t mask, skvx::int8 winding) {
            masks.push_back(mask);
            windings.push_back(winding);
        };
        if (scalar) {
            MakeStrips::MsaaScalar<kTileWidth, kTileHeight>(tiles,
                                                            &wides,
                                                            &ends,
                                                            atlas.get(),
                                                            fillType,
                                                            polyline,
                                                            maskLut,
                                                            isCulled,
                                                            histogram,
                                                            clip,
                                                            observer);
        } else {
            MakeStrips::MsaaSimd<kTileWidth, kTileHeight>(tiles,
                                                          &wides,
                                                          &ends,
                                                          atlas.get(),
                                                          fillType,
                                                          polyline,
                                                          maskLut,
                                                          isCulled,
                                                          histogram,
                                                          clip,
                                                          observer);
        }
    }
};

// True if two MakeStrips outputs are identical: the same WideTiles, and the same EndCaps with the
// same alpha bytes.
template <uint16_t kTileWidth, uint16_t kTileHeight>
bool SameStripOutput(const WideTiles& widesA,
                     const EndCaps& endsA,
                     const AlphaAtlasManager& atlasA,
                     const WideTiles& widesB,
                     const EndCaps& endsB,
                     const AlphaAtlasManager& atlasB) {
    if (widesA.size() != widesB.size() || endsA.size() != endsB.size()) {
        return false;
    }
    for (int i = 0; i < widesA.tiles().size(); ++i) {
        const auto& a = widesA.tiles()[i];
        const auto& b = widesB.tiles()[i];
        if (a.fX != b.fX || a.fY != b.fY || a.fWidth != b.fWidth) {
            return false;
        }
    }
    for (int i = 0; i < endsA.caps().size(); ++i) {
        const auto& a = endsA.caps()[i];
        const auto& b = endsB.caps()[i];
        if (a.fX != b.fX || a.fY != b.fY || a.fWidth != b.fWidth || a.fTexPage != b.fTexPage ||
            a.fAlphaIndex != b.fAlphaIndex) {
            return false;
        }
        const uint8_t* alphaA = atlasA.getPageData(a.fTexPage);
        const uint8_t* alphaB = atlasB.getPageData(b.fTexPage);
        if ((alphaA == nullptr) != (alphaB == nullptr)) {
            return false;
        }
        const size_t bytes = static_cast<size_t>(a.fWidth) * kTileHeight;
        if (alphaA && memcmp(alphaA + a.fAlphaIndex, alphaB + b.fAlphaIndex, bytes) != 0) {
            return false;
        }
    }
    return true;
}

template <typename T> bool SameArrays(const SkTDArray<T>& a, const SkTDArray<T>& b) {
    return a.size() == b.size() &&
           (a.empty() ||
            memcmp(a.begin(), b.begin(), sizeof(T) * static_cast<size_t>(a.size())) == 0);
}

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_RowTilesCheck_DEFINED
