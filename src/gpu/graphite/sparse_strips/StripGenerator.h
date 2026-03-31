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
#include "src/gpu/graphite/sparse_strips/Tiler.h"
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
    static constexpr uint16_t kTileWidth = SparseStripsConfig::kTileWidth;
    static constexpr uint16_t kTileHeight = SparseStripsConfig::kTileHeight;
    static constexpr uint32_t kWorkgroupSize = SparseStripsConfig::kWorkgroupSize;

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
            return processGeometryCPU(path, ctm, clip);
        } else if constexpr (isGPU) {
            return processGeometryGPU(path, ctm, clip);
        } else {
            SkUNREACHABLE;
        }
    }

    void flushGpu() {
        constexpr bool isGPU = std::is_same_v<TileType, GpuTile<kTileWidth, kTileHeight>>;
        if constexpr (!isGPU) {
            return;
        }

        uint32_t tileCount = fTiles.getTiles().size();
        printf("[flushGpu] --- STARTING GPU FLUSH ---\n");
        printf("[flushGpu] tileCount: %u\n", tileCount);

        if (tileCount == 0) {
            printf("[flushGpu] tileCount is 0, resetting and returning early.\n");
            this->reset();
            return;
        }

        GpuAlphaAtlasManager* manager = fRecorder->priv().atlasProvider()->gpuAlphaAtlasManager();
        DrawBufferManager* bufMgr = fRecorder->priv().drawBufferManager();
        SkASSERT(manager && bufMgr);

        // 1) Calculate Workgroups
        constexpr uint32_t partSize = kWorkgroupSize / kTileWidth;
        uint32_t workgroups = (tileCount + partSize - 1) / partSize;
        printf("[flushGpu] kWorkgroupSize: %u, partSize: %u, calculated workgroups: %u\n",
               kWorkgroupSize, partSize, workgroups);

        // 2) Upload MSAA LUT
        size_t lutSize = fLUT.size() * sizeof(MSAAType);
        auto [lutWriter, lutBuf, _1] = bufMgr->getMappedStorageBuffer(lutSize, 1);
        printf("[flushGpu] LUT Upload: size=%zu bytes, mapped=%s\n",
               lutSize, lutWriter ? "SUCCESS" : "FAILED");
        if (lutWriter) {
            lutWriter.write(fLUT.data(), lutSize);
        }

        // 3) Upload Lines (fFlattenned)
        size_t linesSize = fFlattenned.count() * 4 * sizeof(float);
        auto [linesWriter, linesBuf, _2] = bufMgr->getMappedStorageBuffer(linesSize, 1);
        printf("[flushGpu] Lines Upload: count=%d, size=%zu bytes, mapped=%s\n",
               fFlattenned.count(), linesSize, linesWriter ? "SUCCESS" : "FAILED");
        if (linesWriter) {
            linesWriter.write(fFlattenned.points().data(), linesSize);
        }

        // 4) Upload GPU Tiles
        size_t tilesSize = tileCount * sizeof(TileType);
        auto [tilesWriter, tilesBuf, _3] = bufMgr->getMappedStorageBuffer(tilesSize, 1);
        printf("[flushGpu] Tiles Upload: size=%zu bytes, mapped=%s\n",
               tilesSize, tilesWriter ? "SUCCESS" : "FAILED");
        if (tilesWriter) {
            tilesWriter.write(fTiles.getTiles().data(), tilesSize);
        }

        // 5) Allocate Scratch Buffers
        size_t indicatorSize = workgroups * 2 * sizeof(uint32_t);
        BufferSubAllocator indicatorAlloc = bufMgr->getScratchStorage(indicatorSize);
        auto indicatorBuf = indicatorAlloc.getSubrange(indicatorSize, 1);

        size_t partRedSize = workgroups * 2 * kTileHeight * kTileWidth * 8;
        BufferSubAllocator partRedAlloc = bufMgr->getScratchStorage(partRedSize);
        auto partRedBuf = partRedAlloc.getSubrange(partRedSize, 1);

        size_t debugSize = 256 * sizeof(uint32_t);
        BufferSubAllocator debugAlloc = bufMgr->getScratchStorage(debugSize);
        auto debugBuf = debugAlloc.getSubrange(debugSize, 1);

        printf("[flushGpu] Scratch Buffers allocated: indicatorSize=%zu, partRedSize=%zu, debugSize=%zu\n",
               indicatorSize, partRedSize, debugSize);

        // 6) Create and Dispatch Compute Tasks
        auto proxies = manager->getProxies();
        printf("[flushGpu] Active proxies count: %zu\n", proxies.size());

        if (proxies.empty()) {
            printf("[flushGpu] WARNING: proxies array is empty! Nothing to bind.\n");
        } else {
            DispatchGroup::Builder builder(fRecorder);
            // Setup the Config Uniform
            struct Config {
                uint32_t tile_count;
                uint32_t end_tile_count;
                uint32_t workgroups;
                uint32_t d; // Texture page offset
            } configData = { tileCount, 0, workgroups, 0 };

            printf("[flushGpu] Config Data: tile_count=%u, end_tile_count=%u, workgroups=%u, d=%u\n",
                   configData.tile_count, configData.end_tile_count, configData.workgroups, configData.d);

            auto [configWriter, configBuf, _0] = bufMgr->getMappedUniformBuffer(sizeof(Config), 1);
            if (configWriter) {
                configWriter.write(&configData, sizeof(Config));
            } else {
                printf("[flushGpu] WARNING: Config uniform buffer failed to map!\n");
            }

            // Assign Shared Buffers
            builder.assignSharedBuffer(configBuf, SparseStripsSlots::kConfig);
            builder.assignSharedBuffer(lutBuf, SparseStripsSlots::kMsaaLut);
            builder.assignSharedBuffer(linesBuf, SparseStripsSlots::kLines);
            builder.assignSharedBuffer(tilesBuf, SparseStripsSlots::kPmtIn);

            // Assign Scratch Buffers (Must be cleared to 0)
            builder.assignSharedBuffer(indicatorBuf, SparseStripsSlots::kPartIndicator, ClearBuffer::kNo);
            builder.assignSharedBuffer(partRedBuf, SparseStripsSlots::kPartRed, ClearBuffer::kYes);
            builder.assignSharedBuffer(debugBuf, SparseStripsSlots::kDebug, ClearBuffer::kYes);

            // Assign Texture Output
            builder.assignSharedTexture(proxies[0], SparseStripsSlots::kOutput);

            // Append Steps
            WorkgroupSize dispatchSize(workgroups, 1, 1);
            printf("[flushGpu] Dispatching fMsaaStep with size: (%u, 1, 1)\n", workgroups);
            builder.appendStep(&fMsaaStep, dispatchSize);
            //builder.appendStep(&fStitchStep, dispatchSize);

            // Package into a DispatchGroupList for the ComputeTask
            ComputeTask::DispatchGroupList groups;
            groups.push_back(builder.finalize());
            fRecorder->priv().add(ComputeTask::Make(std::move(groups)));
            printf("[flushGpu] ComputeTask added to recorder.\n");
        }

        // 7) Clean up
        manager->postFlush();
        this->reset();
        printf("[flushGpu] --- FLUSH COMPLETE ---\n");
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
        int32_t offset = manager->getAlphaBufferSize();
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
        fAlphasSize += manager->getAlphaBufferSize() - offset;

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
                MakeStrip::MakeStripsGpu<kTileWidth, kTileHeight, kWorkgroupSize, MSAAType>(
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
            //fWides.addTile(startX, curr.y(), width);
        }
        if (totalBytes > 0) {
            fEnds.addCap(prev.x(), prev.y(), prevWidth, startIdx + globalOffset);
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

    uint32_t fFlattennedSize = 0;
    uint32_t fTilesSize = 0;
    uint32_t fAlphasSize = 0;

    SparseStripsMsaaStep fMsaaStep;
    SparseStripsStitchStep fStitchStep;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
