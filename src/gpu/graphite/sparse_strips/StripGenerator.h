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
#include "src/gpu/graphite/sparse_strips/GpuAlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrip.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/CompressedRowTiler.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/RowTiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/compute/SparseStripsRenderer.h"
#include "src/gpu/graphite/task/ComputeTask.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>

namespace skgpu::graphite {

template <typename TileType> class StripGenerator {
public:
    using MSAAType = SparseStripsConfig::MSAAType;
    static constexpr uint16_t kTileWidth           = SparseStripsConfig::kTileWidth;
    static constexpr uint16_t kTileHeight          = SparseStripsConfig::kTileHeight;
    static constexpr uint32_t kPartSize            = SparseStripsConfig::kPartSize;
    static constexpr uint32_t kMSAAWorkgroupSize   = SparseStripsConfig::kMSAAWorkgroupSize;
    static constexpr uint32_t kStitchWorkgroupSize = SparseStripsConfig::kStitchWorkgroupSize;

    StripGenerator(SkISize deviceSize, const SkTDArray<MSAAType>& lut, Recorder* recorder)
            : fDeviceSize(SkIRect::MakeSize(deviceSize))
            , fCulled(true)
            , fLUT(lut)
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
            //return processGeometryCPU(path, ctm, clip);
            return processGeometryCPURow(path, ctm, clip);
        } else if constexpr (isGPU) {
            return processGeometryGPU(path, ctm, clip);
        } else {
            SkUNREACHABLE;
        }
    }

    BindBufferInfo flushGpu(BindBufferInfo (*initDebugBuffer)(DrawBufferManager*) = nullptr) {
        constexpr bool isGPU = std::is_same_v<TileType, GpuTile<kTileWidth, kTileHeight>>;
        if constexpr (!isGPU) {
            return {};
        }

        uint32_t tileCount = fTiles.getTiles().size();
        if (tileCount == 0) {
            this->reset();
            return {};
        }

        GpuAlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->gpuAlphaAtlasManager();
        DrawBufferManager* bufMgr = fRecorder->priv().drawBufferManager();
        SkASSERT(manager && bufMgr);

        uint32_t msaaWorkgroups = (tileCount + kPartSize - 1) / kPartSize;
        uint32_t stitchWorkgroups = (msaaWorkgroups + kStitchWorkgroupSize - 1) /
                                    kStitchWorkgroupSize;

        size_t lutSize = fLUT.size() * sizeof(MSAAType);
        auto [lutWriter, lutBuf, _1] = bufMgr->getMappedStorageBuffer(lutSize, 1);
        if (lutWriter) lutWriter.write(fLUT.data(), lutSize);

        size_t linesSize = fFlattenned.count() * 4 * sizeof(float);
        auto [linesWriter, linesBuf, _2] = bufMgr->getMappedStorageBuffer(linesSize, 1);
        if (linesWriter) linesWriter.write(fFlattenned.points().data(), linesSize);

        size_t tilesSize = tileCount * sizeof(TileType);
        auto [tilesWriter, tilesBuf, _3] = bufMgr->getMappedStorageBuffer(tilesSize, 1);
        if (tilesWriter) tilesWriter.write(fTiles.getTiles().data(), tilesSize);

        size_t indicatorSize = msaaWorkgroups * 2 * sizeof(uint32_t);
        auto indicatorBuf = bufMgr->getScratchStorage(indicatorSize).getSubrange(indicatorSize, 1);

        size_t partRedSize = msaaWorkgroups * 2 * kTileHeight * kTileWidth * 8;
        auto partRedBuf = bufMgr->getScratchStorage(partRedSize).getSubrange(partRedSize, 1);

        BindBufferInfo debugBufInfo = initDebugBuffer ? initDebugBuffer(bufMgr) :
                                      bufMgr->getScratchStorage(4).getSubrange(4, 1);

        auto proxies = manager->getProxies();
        if (!proxies.empty()) {
            DispatchGroup::Builder builder(fRecorder);

            struct Config {
                uint32_t tile_count;
                uint32_t end_tile_count;
                uint32_t workgroups;
                uint32_t d;
            } configData = { tileCount, 0, msaaWorkgroups, 0 };

            auto [configWriter, configBuf, _0] = bufMgr->getMappedUniformBuffer(sizeof(Config), 1);
            if (configWriter) configWriter.write(&configData, sizeof(Config));

            builder.assignSharedBuffer(configBuf, SparseStripsSlots::kConfig);
            builder.assignSharedBuffer(lutBuf, SparseStripsSlots::kMsaaLut);
            builder.assignSharedBuffer(linesBuf, SparseStripsSlots::kLines);
            builder.assignSharedBuffer(tilesBuf, SparseStripsSlots::kPmtIn);
            builder.assignSharedBuffer(indicatorBuf, SparseStripsSlots::kPartIndicator, ClearBuffer::kYes);
            builder.assignSharedBuffer(partRedBuf, SparseStripsSlots::kPartRed, ClearBuffer::kNo);
            builder.assignSharedBuffer(debugBufInfo, SparseStripsSlots::kDebug, ClearBuffer::kYes);

            builder.assignSharedTexture(proxies[0], SparseStripsSlots::kOutput0);
            if (proxies.size() > 1) {
                builder.assignSharedTexture(proxies[1], SparseStripsSlots::kOutput1);
            }

            builder.appendStep(&fMsaaStep, WorkgroupSize(msaaWorkgroups, 1, 1));
            builder.appendStep(&fStitchStep, WorkgroupSize(stitchWorkgroups, 1, 1));

            ComputeTask::DispatchGroupList groups;
            groups.push_back(builder.finalize());
            fRecorder->priv().add(ComputeTask::Make(std::move(groups)));
        }

        manager->postFlush();
        this->reset();

        return debugBufInfo;
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
        MakeStrip::MakeStripsMsaaSimd(fTiles,
                                      fStrips,
                                      manager->getAlphaBuffer(),
                                      path.getFillType(),
                                      fFlattenned,
                                      fLUT,
                                      fWides,
                                      fCulled,
                                      fWindingHistogram,
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
        fTiles.sortTiles(prevTileCount);  // adjust if sktdarray

        GpuAlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->gpuAlphaAtlasManager();
        SkASSERT(manager);

        int32_t currentGlobalOffset = manager->getAlphaIndex();
        int32_t alphaCount =
                MakeStrip::MakeStripsGpu<kTileWidth, kTileHeight, kMSAAWorkgroupSize, MSAAType>(
                        fTiles,
                        fStrips,
                        path.getFillType(),
                        fFlattenned,
                        fWides,
                        fCulled,
                        fWindingHistogram,
                        fDeviceSize,
                        prevTileCount,
                        currentGlobalOffset);

        for (int stripIdx = 1; stripIdx < fStrips.size(); ++stripIdx) {
            processStripPair(stripIdx, currentGlobalOffset);
        }

        manager->advanceAlphaIndex(alphaCount);
        for (auto& proxy : manager->getProxies()){
            fEnds.addProxy(proxy);
        }

        return {fWides, fEnds};
    }

    SK_ALWAYS_INLINE const std::pair<WideTiles<kTileHeight>&, EndCaps<kTileHeight>&>
    processGeometryCPURow(const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
        this->reset();

        fFlattener.processPaths<FlattenMode::kSimd>(path, ctm, fDeviceSize, fFlattenned);

        fWindingHistogram.resize((fDeviceSize.bottom() + kTileHeight - 1) / kTileHeight);
        if (fCulled) {
            fWindingHistogram.clear();
        }

        // CHANGED: Use the RowTiles state and generation logic
        // fCulled = fRowTiles.makeTilesMSAA(fFlattenned, fDeviceSize, fWindingHistogram);
        // fRowTiles.sortTiles();

        fCulled = fComRowTiles.makeCompressedTilesMSAA(fFlattenned, fDeviceSize, fWindingHistogram);
        fComRowTiles.sortTiles();

        AlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->alphaAtlasManager();

        // CHANGED: Route to the new SIMD strip maker
        // MakeStripImpl::MakeRowStripsSimd<kTileWidth, kTileHeight, true, MSAAType>(fRowTiles,
        //                              fStrips,
        //                              manager->getAlphaBuffer(),
        //                              false,
        //                              fFlattenned,
        //                              fLUT,
        //                              fWides,
        //                              fCulled,
        //                              fWindingHistogram,
        //                              fDeviceSize);

        MakeStripImpl::MakeCompressedRowStripsSimd<kTileWidth, kTileHeight, true, MSAAType>(fComRowTiles,
                                     fStrips,
                                     manager->getAlphaBuffer(),
                                     false,
                                     fFlattenned,
                                     fLUT,
                                     fWides,
                                     fCulled,
                                     fWindingHistogram,
                                     fDeviceSize,
                                     firstIdxForX);

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

    SK_ALWAYS_INLINE int32_t processStripPair(int32_t stripIdx, int32_t globalOffset) {
        constexpr size_t kBytesPerTile = kTileWidth * kTileHeight;
        const Strip& prev = fStrips[stripIdx - 1];
        const Strip& curr = fStrips[stripIdx];
        int32_t startIdx = prev.alphaIdx();
        int32_t currEndIdx = curr.alphaIdx();
        int32_t totalBytes = currEndIdx - startIdx;
        int32_t numTiles = totalBytes / kBytesPerTile;
        uint16_t prevWidth = numTiles * kTileWidth;
        bool shouldFill = curr.shouldFill();
        if (shouldFill && prev.y() == curr.y()) {
            SkASSERT(prev.y() == curr.y());
            uint16_t startX = prev.x() + prevWidth;
            uint16_t width = curr.x() - startX;
            fWides.addTile(startX, curr.y(), width);
        }
        if (totalBytes > 0) {
            fEnds.addCap(prev.x(), prev.y(), prevWidth, startIdx + globalOffset);
        }
        return currEndIdx;
    }

    void reset() {
        fFlattenned.reset();
        fComRowTiles.reset();
        firstIdxForX.clear();
        fTiles.reset();
        fRowTiles.reset();
        fStrips.clear();
        fAllocations.clear();
        fWides.clear();
        fEnds.clear();
    }

    SkIRect fDeviceSize;
    bool fCulled;
    Flatten fFlattener;
    Tiles<kTileWidth, kTileHeight, TileType> fTiles;
    RowTiles<kTileWidth, kTileHeight, TileType> fRowTiles;
    CompressedRowTiles<> fComRowTiles;
    Polyline fFlattenned;
    SkTDArray<Strip> fStrips;
    const SkTDArray<MSAAType>& fLUT;
    SkTDArray<std::pair<int32_t, sk_sp<TextureProxy>>> fAllocations;
    SkTDArray<size_t> firstIdxForX;
    Recorder* fRecorder;
    WindingHistogram fWindingHistogram;

    WideTiles<kTileHeight> fWides;
    EndCaps<kTileHeight> fEnds;

    SparseStripsMsaaStep fMsaaStep;
    SparseStripsStitchStep fStitchStep;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
