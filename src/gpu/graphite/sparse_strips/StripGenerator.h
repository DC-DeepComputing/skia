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
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>

namespace skgpu::graphite {

class StripGenerator {
public:
    using MSAAType = SparseStripsConfig::MSAAType;
    static constexpr uint16_t kTileWidth  = SparseStripsConfig::kTileWidth;
    static constexpr uint16_t kTileHeight = SparseStripsConfig::kTileHeight;

    StripGenerator(SkISize deviceSize, const SkTDArray<MSAAType>& lut, Recorder* recorder)
            : fDeviceSize(SkIRect::MakeSize(deviceSize)), fCulled(true), fLUT(lut)
            , fRecorder(recorder)
            , fWindingHistogram((fDeviceSize.bottom() + kTileHeight - 1) / kTileHeight) {
        fAllocations.reserve(4);
    }

    ~StripGenerator() {
        this->reset();
        // printf("Lines size %u\n", fFlattennedSize);
        // printf("Tiles size %u\n", fTilesSize);
        // printf("Alphas size %u\n", fAlphasSize);
    }

    const std::pair<WideTiles<kTileHeight>&, EndCaps<kTileHeight>&> processGeometry(
                const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
        this->reset();

        fFlattener.processPaths<FlattenMode::kSimd>(path, ctm, fDeviceSize, fFlattenned);

        fWindingHistogram.resize((fDeviceSize.bottom() + kTileHeight - 1) / kTileHeight);
        if (fCulled) {
            fWindingHistogram.clear();
        }
        fCulled = fTiles.makeTilesMSAA(fFlattenned, fDeviceSize, fWindingHistogram);
        fTiles.sortTiles();

        AlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->alphaAtlasManager();
        int32_t offset = manager->getAlphaBufferSize();
        Strip::MakeStripsMsaaSimd<kTileWidth, kTileHeight, MSAAType>(fTiles, fStrips, manager->getAlphaBuffer(), path.getFillType(),
                                  fFlattenned, fLUT, fWides, fCulled, fWindingHistogram,
                                  fDeviceSize);
        fAlphasSize += manager->getAlphaBufferSize() - offset;

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
                            static_cast<int32_t>(prev.packedAlphaIdx & ~(1u << SparseStripsConstants::FILL_SHIFT));
                    currEndIdx =
                            static_cast<int32_t>(curr.packedAlphaIdx & ~(1u << SparseStripsConstants::FILL_SHIFT));

                    int32_t totalBytes = currEndIdx - startIdx;
                    int32_t numTiles = totalBytes / kBytesPerTile;
                    uint16_t prevWidth = numTiles * kTileWidth;
                    bool shouldFill = (curr.packedAlphaIdx >> SparseStripsConstants::FILL_SHIFT) & 1;
                    if (shouldFill && prev.y == curr.y) {
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

        fFlattennedSize += fFlattenned.points().size_bytes();
        fTilesSize += fTiles.getTiles().size() * 16;

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

    SkIRect fDeviceSize;
    bool fCulled;
    Flatten fFlattener;
    Tiles<kTileWidth, kTileHeight> fTiles;
    Polyline fFlattenned;
    SkTDArray<Strip> fStrips;
    const SkTDArray<MSAAType>& fLUT;
    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> fAllocations;
    Recorder* fRecorder;
    WindingHistogram fWindingHistogram;

    WideTiles<kTileHeight> fWides;
    EndCaps<kTileHeight> fEnds;

    uint32_t fFlattennedSize = 0;
    uint32_t fTilesSize = 0;
    uint32_t fAlphasSize = 0;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
