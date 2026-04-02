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

template<typename TileType>
class StripGenerator {
public:
    using MSAAType = SparseStripsConfig::MSAAType;
    static constexpr uint16_t kTileWidth     = SparseStripsConfig::kTileWidth;
    static constexpr uint16_t kTileHeight    = SparseStripsConfig::kTileHeight;
    static constexpr uint32_t kWorkgroupSize = SparseStripsConfig::kWorkgroupSize;

    StripGenerator(SkISize deviceSize, const SkTDArray<MSAAType>& lut, Recorder* recorder)
            : fDeviceSize(SkIRect::MakeSize(deviceSize)), fCulled(true), fAlphaIndex(0), fLUT(lut)
            , fRecorder(recorder)
            , fWindingHistogram((fDeviceSize.bottom() + kTileHeight - 1) / kTileHeight) {
        fAllocations.reserve(4);
    }

    ~StripGenerator() {
        this->reset();
    }

    const std::pair<WideTiles<kTileHeight>&, EndCaps<kTileHeight>&> processGeometry(
            const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
        constexpr bool isCPU = std::is_same_v<TileType, Tile<kTileWidth, kTileHeight>>;
        constexpr bool isGPU = std::is_same_v<TileType, GpuTile<kTileWidth, kTileHeight>>;
        if constexpr (isCPU) {
            return processGeometryCPU(path, ctm, clip);
        } else if constexpr (isGPU) {
            return processGeometryGPU(path, ctm, clip);
        } else {
            SkUNREACHABLE;
        }
    }

    void flushGpu() {
        constexpr bool isGPU = std::is_same_v<TileType, GpuTile<kTileWidth, kTileHeight>>;
        if constexpr (isGPU) {
            this->reset();
            fAlphaIndex = 0;
        }
    }

private:
    SK_ALWAYS_INLINE const std::pair<WideTiles<kTileHeight>&, EndCaps<kTileHeight>&>
    processGeometryCPU(const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
        this->reset();

        fFlattener.processPaths<FlattenMode::kSimd>(path, ctm, fDeviceSize, fFlattenned);

        fWindingHistogram.resize((fDeviceSize.bottom() + kTileHeight - 1) / kTileHeight);
        if (fCulled) {
            fWindingHistogram.clear();
        }
        fCulled = fTiles.makeTilesMSAA(fFlattenned, fDeviceSize, fWindingHistogram);
        fTiles.sortTiles();

        AlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->alphaAtlasManager();
        Strip::MakeStripsMsaaSimd(fTiles, fStrips, manager->getAlphaBuffer(), path.getFillType(),
                                  fFlattenned, fLUT, fWides, fCulled, fWindingHistogram,
                                  fDeviceSize);

        SkASSERT(fRecorder);
        int32_t globalOffset = manager->getUploadOffset();
        manager->getAssociatedProxies(fAllocations);
        if (fStrips.size() > 1 && !fAllocations.empty()) {
            int32_t stripIdx = 1;
            for (const auto& alloc : fAllocations) {
                fEnds.addProxy(alloc.second);
                int32_t endIdx = alloc.first;
                int32_t currEndIdx = 0;
                while (currEndIdx < endIdx && stripIdx < fStrips.size()) {
                    currEndIdx = processStripPair(stripIdx, globalOffset);
                    stripIdx++;
                }
            }
        }

        return {fWides, fEnds};
    }

    SK_ALWAYS_INLINE const std::pair<WideTiles<kTileHeight>&, EndCaps<kTileHeight>&>
    processGeometryGPU(const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
        fWides.clear();
        fEnds.clear();
        fStrips.clear();

        int32_t prevLineCount = fFlattenned.count();
        fFlattener.processPaths<FlattenMode::kSimd>(path, ctm, fDeviceSize, fFlattenned);

        fWindingHistogram.resize((fDeviceSize.bottom() + kTileHeight - 1) / kTileHeight);
        if (fCulled) {
            fWindingHistogram.clear();
        }

        int32_t prevTileCount = fTiles.getTiles().size();
        fCulled = fTiles.makeTilesMSAA(fFlattenned, fDeviceSize, fWindingHistogram, prevLineCount);
        fTiles.sortTiles(prevTileCount); //adjust if sktdarray

        int32_t alphaCount = Strip::MakeStripsGpu<kTileWidth, kTileHeight, kWorkgroupSize, MSAAType>(
                fTiles, fStrips, path.getFillType(), fFlattenned, fWides, fCulled, fWindingHistogram, fDeviceSize, prevTileCount);

        for (int stripIdx = 1 ; stripIdx < fStrips.size(); ++stripIdx) {
            processStripPair(stripIdx, fAlphaIndex);
        }

        fAlphaIndex += alphaCount;

        return {fWides, fEnds};
    }

    SK_ALWAYS_INLINE int32_t processStripPair(int32_t stripIdx, int32_t globalOffset) {
        constexpr size_t kBytesPerTile = kTileWidth * kTileHeight;
        const Strip& prev = fStrips[stripIdx - 1];
        const Strip& curr = fStrips[stripIdx];
        int32_t startIdx = static_cast<int32_t>(prev.packedAlphaIdx & ~(1u << Strip::FILL_SHIFT));
        int32_t currEndIdx = static_cast<int32_t>(curr.packedAlphaIdx & ~(1u << Strip::FILL_SHIFT));
        int32_t totalBytes = currEndIdx - startIdx;
        int32_t numTiles = totalBytes / kBytesPerTile;
        uint16_t prevWidth = numTiles * kTileWidth;
        bool shouldFill = (curr.packedAlphaIdx >> Strip::FILL_SHIFT) & 1;
        if (shouldFill && prev.y == curr.y) {
            SkASSERT(prev.y == curr.y);
            uint16_t startX = prev.x + prevWidth;
            uint16_t width = curr.x - startX;
            fWides.addTile(startX, curr.y, width);
        }
        if (totalBytes > 0) {
            fEnds.addCap(prev.x, prev.y, prevWidth, startIdx + globalOffset);
        }
        return currEndIdx;
    }

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
    int32_t fAlphaIndex;
    Flatten fFlattener;
    Tiles<kTileWidth, kTileHeight, TileType> fTiles;
    Polyline fFlattenned;
    SkTDArray<Strip> fStrips;
    const SkTDArray<MSAAType>& fLUT;
    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> fAllocations;
    Recorder* fRecorder;
    WindingHistogram fWindingHistogram;

    WideTiles<kTileHeight> fWides;
    EndCaps<kTileHeight> fEnds;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
