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

class DrawContext;
class DrawPass;
class DrawBufferManager;
class Recorder;
class TextureProxy;
class TextureProxyView;

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
        fStrideAlignment = 0;
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
        fStrideAlignment = kStructAlignment;
        SkDEBUGCODE(fGradientsFinalized = true;)
    }

    void finalizeCacheStorageData() {
        this->finalizeGradientStorage();
    }

    AppendedAllocation append(size_t count, size_t stride, size_t align = 4) {
        SkASSERT(fGradientsFinalized);
        if (count == 0 || stride == 0) {
            return {VertexWriter{nullptr, 0}, 0};
        }

        size_t reqAlign = std::max(align, (size_t)4);
        size_t currentGpuOffset = fGradientDataSize + fAppendedData.size();

        size_t alignedGpuOffset = SkAlignTo<size_t>(currentGpuOffset, reqAlign);
        if (alignedGpuOffset % stride != 0) {
            alignedGpuOffset = SkAlignTo<size_t>(alignedGpuOffset, stride);
        }

        size_t padding = alignedGpuOffset - currentGpuOffset;
        size_t allocationBytes = count * stride;
        size_t oldAppendedSize = fAppendedData.size();

        fAppendedData.resize(oldAppendedSize + padding + allocationBytes);
        if (padding > 0) {
            memset(fAppendedData.data() + oldAppendedSize, 0, padding);
        }

        uint32_t strideIndex = static_cast<uint32_t>(alignedGpuOffset / stride);
        fStrideAlignment = reqAlign;

        void* ptr = fAppendedData.data() + oldAppendedSize + padding;
        return {VertexWriter{ptr, allocationBytes}, strideIndex};
    }

    AppendedAllocation appendVertexStorage(size_t count, size_t stride, size_t align = 4) {
        return this->append(count, stride, align);
    }

    SkDEBUGCODE(bool fGradientsFinalized = false;)

    skia_private::THashMap<const SkGradientBaseShader*, int> fLocalGradientOffsetCache;
    skia_private::STArray<4, const SkGradientBaseShader*> fContextGradientsList;
    size_t fGradientDataSize = 0;
    size_t fStrideAlignment = 0;

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

    explicit StorageBufferManager(bool storageBufferSupport = true);
    ~StorageBufferManager() override;

    bool storageBufferSupport() const { return fStorageBufferSupport; }

    void reset();

    // Checks if gradient data already exists for the requested gradient shader.
    std::pair<float*, int> allocateGradientData(StorageContext* ctx,
                                                int numStops, const SkGradientBaseShader* shader);

    // Finalizes storage for a DrawPass/DrawContext. Returns SSBO BindBufferInfo if SSBO is supported,
    // or records texture fallback to DrawContext and returns empty {} BindBufferInfo if unsupported.
    BindBufferInfo finalize(DrawBufferManager* bufferMgr,
                            StorageContext* ctx,
                            Recorder* recorder = nullptr,
                            DrawPass* drawPass = nullptr);

private:
    BindBufferInfo finalizeSSBO(DrawBufferManager* bufferMgr, StorageContext* ctx);
    TextureProxyView finalizeTexture(Recorder* recorder, StorageContext* ctx, DrawPass* drawPass);

private:
    sk_sp<TextureProxy> createTextureProxy(Recorder* recorder, int width, int height);

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
    bool fStorageBufferSupport = true;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_StorageBufferManager_DEFINED
