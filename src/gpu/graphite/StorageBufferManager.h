/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_StorageBufferManager_DEFINED
#define skgpu_graphite_StorageBufferManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkAlign.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkTHash.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

class DrawBufferManager;

class StorageContext {
public:
    static constexpr size_t kStructAlignment = 16;
    static constexpr int kMaxGradientStops = 1024 * 1024;
    static constexpr int kMaxStorageFloats =
            static_cast<int>(std::numeric_limits<uint32_t>::max() / sizeof(float));

    struct AppendedAllocation {
        VertexWriter fWriter;
        uint32_t     fOffset;
    };

    StorageContext() = default;
    ~StorageContext() { this->resetCache(); }

    // Resets cached gradient data. Should only occur at natural flush time (e.g. canvas/recorder flush).
    void resetCache() {
        for (const SkGradientBaseShader* shader : fContextGradientsList) {
            shader->unref();
        }
        fLocalGradientOffsetCache.reset();
        fContextGradientsList.clear();
        fGradientData.clear();
        fGradientDataSize = 0;
    }

    // Resets appended vertex/instance storage. Called when finalizing a DrawPass snap.
    void resetAppendedStorage() {
        fAppendedData.clear();
        fBufferInfo.reset();
        SkDEBUGCODE(fGradientsFinalized = false;)
    }

    void reset() {
        this->resetCache();
        this->resetAppendedStorage();
    }

    std::pair<float*, int> allocateGradientData(int numStops, const SkGradientBaseShader* shader) {
        SkASSERT(!fGradientsFinalized);
        if (numStops > kMaxGradientStops) {
            return {nullptr, -1};
        }

        int* existingLocalOffset = fLocalGradientOffsetCache.find(shader);
        if (existingLocalOffset) {
            return {nullptr, *existingLocalOffset};
        }

        int floatOffset = fGradientData.size();
        int floatCount = numStops * 5;
        if (kMaxStorageFloats - floatCount < floatOffset) {
            return {nullptr, -1};
        }

        fGradientData.resize(floatOffset + floatCount);
        float* dstData = fGradientData.data() + floatOffset;

        shader->ref();
        fLocalGradientOffsetCache.set(shader, floatOffset);
        fContextGradientsList.push_back(shader);
        fGradientDataSize = fGradientData.size_bytes();

        return {dstData, floatOffset};
    }

    int size() const {
        int gradSize = SkAlignTo<int>(fGradientDataSize, kStructAlignment);
        int appSize = SkAlignTo<int>(fAppendedData.size(), kStructAlignment);
        return gradSize + appSize;
    }

    void finalizeGradientStorage() {
        fGradientDataSize = SkAlignTo<size_t>(fGradientDataSize, kStructAlignment);
        SkDEBUGCODE(fGradientsFinalized = true;)
    }

    void finalizeCacheStorageData() {
        this->finalizeGradientStorage();
    }

    AppendedAllocation append(size_t count, size_t stride) {
        SkASSERT(fGradientsFinalized);
        size_t currentAppendedOffset = SkAlignTo<size_t>(fAppendedData.size(), kStructAlignment);
        size_t allocationBytes = count * stride;
        fAppendedData.resize(currentAppendedOffset + allocationBytes);

        size_t gpuByteOffset = fGradientDataSize + currentAppendedOffset;

        void* ptr = fAppendedData.data() + currentAppendedOffset;
        return {VertexWriter{ptr, allocationBytes}, static_cast<uint32_t>(gpuByteOffset)};
    }

    AppendedAllocation appendVertexStorage(size_t count, size_t stride) {
        return this->append(count, stride);
    }

    BindBufferInfo finalize(DrawBufferManager* bufferMgr) {
        this->finalizeGradientStorage();

        size_t gradSize = fGradientDataSize;
        size_t appSize = SkAlignTo<size_t>(fAppendedData.size(), kStructAlignment);
        size_t totalBytes = gradSize + appSize;

        if (totalBytes > 0) {
            auto [writer, bufferInfo, _] =
                    bufferMgr->getMappedStorageBuffer(totalBytes, /*stride=*/1);
            if (writer) {
                if (gradSize > 0) {
                    writer.write(fGradientData.data(), fGradientData.size_bytes());
                    if (gradSize > fGradientData.size_bytes()) {
                        writer.zeroBytes(gradSize - fGradientData.size_bytes());
                    }
                }

                if (!fAppendedData.empty()) {
                    writer.write(fAppendedData.data(), fAppendedData.size());
                    size_t paddingBytes = appSize - fAppendedData.size();
                    if (paddingBytes > 0) {
                        writer.zeroBytes(paddingBytes);
                    }
                }

                fBufferInfo = bufferInfo;
                this->resetAppendedStorage();
                return bufferInfo;
            }
        }
        this->resetAppendedStorage();
        return BindBufferInfo{};
    }

    SkDEBUGCODE(bool fGradientsFinalized = false;)

    skia_private::THashMap<const SkGradientBaseShader*, int> fLocalGradientOffsetCache;
    skia_private::STArray<4, const SkGradientBaseShader*> fContextGradientsList;
    SkTDArray<float> fGradientData;
    size_t fGradientDataSize = 0;

    SkTDArray<char> fAppendedData;
    std::optional<BindBufferInfo> fBufferInfo;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_StorageBufferManager_DEFINED
