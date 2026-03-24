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
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight>
class StripGenerator {
public:
    StripGenerator(uint16_t viewportWidth, uint16_t viewportHeight, const SkTDArray<uint8_t>& lut,
                   Recorder* recorder)
        : fViewportWidth(viewportWidth), fViewportHeight(viewportHeight) , fLUT(lut)
        , fRecorder(recorder) {
            fAllocations.reserve(4);
        }

    ~StripGenerator() {
        this->reset();
    }

    const std::pair<WideTiles<kTileHeight>&, EndCaps<kTileHeight>&> processGeometry(
                const SkPath& path, const SkMatrix& ctm) {
        this->reset();

        fFlattener.processPaths<FlattenMode::kSimd>(path, ctm, fViewportWidth, fViewportHeight,
                                                    fFlattenned);

        fTiles.makeTilesMSAA(fFlattenned, fViewportWidth, fViewportHeight);
        fTiles.sortTiles();

        AlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->alphaAtlasManager();
        Strip::MakeStripsMsaaSimd(fTiles, fStrips, manager->getAlphaBuffer(), path.getFillType(),
                                  fFlattenned, fLUT, fWides, fViewportWidth, fViewportHeight);

        // Retrieve the mapping of offsets to proxies
        SkASSERT(fRecorder);
        int32_t globalOffset = manager->getUploadOffset();
        manager->getAssociatedProxies(fAllocations);
        constexpr size_t kBytesPerTile = kTileWidth * kTileHeight;

        if (fStrips.size() > 1 && !fAllocations.empty()) {
            int32_t stripIdx = 1;
            for (const auto& alloc : fAllocations) {
                fEnds.addProxy(alloc.second);
                int32_t endIdx = alloc.first;
                int32_t currEndIdx = 0;
                while (currEndIdx < endIdx && stripIdx < fStrips.size()) {
                    const Strip& prev = fStrips[stripIdx - 1];
                    const Strip& curr = fStrips[stripIdx];
                    int32_t startIdx =
                            static_cast<int32_t>(prev.packedAlphaIdx & ~(1u << Strip::FILL_SHIFT));
                    currEndIdx =
                            static_cast<int32_t>(curr.packedAlphaIdx & ~(1u << Strip::FILL_SHIFT));

                    int32_t totalBytes = currEndIdx - startIdx;
                    int32_t numTiles = totalBytes / kBytesPerTile;
                    uint16_t prevWidth = numTiles * kTileWidth;
                    bool shouldFill = (curr.packedAlphaIdx >> Strip::FILL_SHIFT) & 1;
                    if (shouldFill) {
                        SkASSERT(prev.y == curr.y);
                        uint16_t startX = prev.x + prevWidth;
                        uint16_t width = curr.x - startX;
                        fWides.addTile(startX, curr.y, width);
                    }
                    if (totalBytes > 0) {
                        fEnds.addCap(prev.x, prev.y, prevWidth, startIdx + globalOffset);
                    }
                    stripIdx++;
                }
            }
        }

        return {fWides, fEnds};
    }

private:
    void reset() {
        fFlattenned.reset();
        fTiles.reset();
        fStrips.clear();
        fAllocations.clear();
        fWides.clear();
        fEnds.clear();
    }

    const uint16_t fViewportWidth;
    const uint16_t fViewportHeight;

    Flatten fFlattener;
    Tiles<kTileWidth, kTileHeight> fTiles;
    Polyline fFlattenned;
    SkTDArray<Strip> fStrips;
    const SkTDArray<uint8_t>& fLUT;
    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> fAllocations;
    bool fDrawn = false;
    Recorder* fRecorder;

    WideTiles<kTileHeight> fWides;
    EndCaps<kTileHeight> fEnds;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
