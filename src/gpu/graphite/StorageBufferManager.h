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
#include "src/gpu/graphite/DescriptorData.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <utility>

class SkGradientBaseShader;

namespace skgpu::graphite {

class DrawBufferManager;

class StorageContext {
public:
    static constexpr size_t kStructAlignment = 16;

    struct AppendedAllocation {
        VertexWriter fWriter;
        uint32_t     fOffset;
    };

    void reset() {
        fLocalGradientOffsetCache.reset();
        fContextGradientsList.clear();
        fGradientDataSize = 0;
        fAppendedData.clear();
        fBufferInfo.reset();
        SkDEBUGCODE(fGradientsFinalized = false;)
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

    SkDEBUGCODE(bool fGradientsFinalized = false;)

    skia_private::THashMap<const SkGradientBaseShader*, int> fLocalGradientOffsetCache;
    skia_private::STArray<4, const SkGradientBaseShader*> fContextGradientsList;
    size_t fGradientDataSize = 0;

    SkTDArray<char> fAppendedData;
    std::optional<BindBufferInfo> fBufferInfo;
};

/**
 * Manages allocation of storage buffer data (such as gradient colors/stops and vertex/instance
 * storage) per DrawContext, concatenating gradients and appended storage per context.
 */
class StorageBufferManager : public SkRefCnt {
public:
    static constexpr int kMaxGradientStops = 1024 * 1024;
    static constexpr int kMaxStorageFloats =
            static_cast<int>(std::numeric_limits<uint32_t>::max() / sizeof(float));

    StorageBufferManager() = default;
    ~StorageBufferManager() override;

    void reset();

    // Checks if gradient data already exists for the requested gradient shader.
    std::pair<float*, int> allocateGradientData(StorageContext* ctx,
                                                int numStops, const SkGradientBaseShader* shader);

    // Finalizes storage for a specific DrawContext/DrawPass and returns its BindBufferInfo.
    BindBufferInfo finalize(DrawBufferManager* bufferMgr, StorageContext* ctx);

private:
    struct GradientCache {
        struct GradientInfo {
            int fGlobalFloatOffset;
            int fFloatCount;
        };

        SkTDArray<float> fGradientData;
        skia_private::THashMap<const SkGradientBaseShader*, GradientInfo> fGradientHash;

        void reset();
    };

    GradientCache fGradientCache;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_StorageBufferManager_DEFINED
