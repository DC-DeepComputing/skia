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
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight>
class StripGenerator {
public:
    StripGenerator(SkISize deviceSize, const SkTDArray<uint8_t>& lut, Recorder* recorder)
            : fDeviceSize(SkIRect::MakeSize(deviceSize)), fCulled(true), fLUT(lut)
            , fRecorder(recorder)
            , fWindingHistogram((fDeviceSize.bottom() + kTileHeight - 1) / kTileHeight) {
            fAllocations.reserve(4);
        }

    ~StripGenerator() {
        this->reset();
    }

public:
    void reset() {
        fFlattenned.reset();
        fTiles.reset();
        fStrips.clear();
        fAllocations.clear();
        fWides.clear();
        fEnds.clear();
    }

    const std::pair<WideTiles<kTileHeight>&, EndCaps<kTileHeight>&> processGeometryCPU(
                const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
        this->reset(); // CPU resets per path

        fFlattener.processPaths<FlattenMode::kSimd>(path, ctm, fDeviceSize, fFlattenned);

        if (fCulled) {
            fWindingHistogram.clear();
        }
        fWindingHistogram.resize((fDeviceSize.bottom() + kTileHeight - 1) / kTileHeight);
        fCulled = fTiles.makeTilesMSAA(fFlattenned, fDeviceSize, fWindingHistogram);
        fTiles.sortTiles();

        AlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->alphaAtlasManager();
        Strip::MakeStripsMsaaSimd(fTiles, fStrips, manager->getAlphaBuffer(), path.getFillType(),
                                  fFlattenned, fLUT, fWides, fCulled, fWindingHistogram,
                                  fDeviceSize);

        return this->buildWidesAndEnds();
    }

    const std::pair<WideTiles<kTileHeight>&, EndCaps<kTileHeight>&> processGeometryGPU(
                const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
        // NO reset() here.

        // 1. Track where the new tiles will start
        int priorTileCount = fTiles.count(); // Assuming fTiles has a count() or size()

        fFlattener.processPaths<FlattenMode::kSimd>(path, ctm, fDeviceSize, fFlattenned);

        if (fCulled) {
            fWindingHistogram.clear();
        }
        fWindingHistogram.resize((fDeviceSize.bottom() + kTileHeight - 1) / kTileHeight);
        fCulled = fTiles.makeTilesMSAA(fFlattenned, fDeviceSize, fWindingHistogram);

        // 2. Sort ONLY the newly added tiles
        // You may need to add a sortTiles(startIndex) to your Tiles class
        fTiles.sortTiles(priorTileCount);

        // 3. GPU-specific MakeStrips call (accumulating counts in a u32, no atlas manager)
        // uint32_t alphaCount = 0;
        // Strip::MakeStripsMsaaGpu(fTiles, fStrips, &alphaCount, path.getFillType(),
        //                          /* ... other args ... */);

        // 4. Populate fAllocations for the GPU path here if needed,
        //    before calling the shared bottom-half builder.

        return this->buildWidesAndEnds();
    }

private:
    const std::pair<WideTiles<kTileHeight>&, EndCaps<kTileHeight>&> buildWidesAndEnds() {
        SkASSERT(fRecorder);
        AlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->alphaAtlasManager();
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
                    int32_t startIdx = static_cast<int32_t>(prev.packedAlphaIdx & ~(1u << 31));
                    currEndIdx = static_cast<int32_t>(curr.packedAlphaIdx & ~(1u << 31));

                    int32_t totalBytes = currEndIdx - startIdx;
                    int32_t numTiles = totalBytes / kBytesPerTile;
                    uint16_t prevWidth = numTiles * kTileWidth;
                    bool shouldFill = (curr.packedAlphaIdx >> 31) & 1;
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

    SkIRect fDeviceSize;
    bool fCulled;
    Flatten fFlattener;
    Tiles<kTileWidth, kTileHeight> fTiles;
    Polyline fFlattenned;
    SkTDArray<Strip> fStrips;
    const SkTDArray<uint8_t>& fLUT;
    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> fAllocations;
    bool fDrawn = false;
    Recorder* fRecorder;
    WindingHistogram fWindingHistogram;

    WideTiles<kTileHeight> fWides;
    EndCaps<kTileHeight> fEnds;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
