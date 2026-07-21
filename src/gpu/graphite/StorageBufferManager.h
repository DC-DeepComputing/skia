/*
 * Copyright 2024 Google LLC
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
#include <utility>

class SkGradientBaseShader;

namespace skgpu::graphite {

class DrawBufferManager;

/**
 * Manages allocation of storage buffer data (such as gradient colors/stops and vertex/instance
 * storage) per DrawContext, concatenating gradients and appended storage per context.
 */
class StorageBufferManager : public SkRefCnt {
public:
    static constexpr size_t kStructAlignment = 16;
    using ContextHandle = uint32_t;
    static constexpr ContextHandle kInvalidContextHandle = std::numeric_limits<uint32_t>::max();

    StorageBufferManager();
    ~StorageBufferManager() override;

    int size(ContextHandle handle) const;
    int size() const;

    void reset();

    // Registers a new DrawContext and returns an O(1) array-index ContextHandle.
    ContextHandle registerContext();

    // Unregisters a DrawContext's storage stream (marks it inactive).
    void unregisterContext(ContextHandle handle);

    // Checks if gradient data already exists for the requested gradient shader in the given context.
    std::pair<float*, int> allocateGradientData(ContextHandle handle, int numStops, const SkGradientBaseShader* shader);
    std::pair<float*, int> allocateGradientData(int numStops, const SkGradientBaseShader* shader);

    struct AppendedAllocation {
        VertexWriter fWriter;
        uint32_t     fOffset;
    };

    // Appends uncached vertex/instance storage allocation into the specified context's stream.
    AppendedAllocation append(ContextHandle handle, size_t count, size_t stride);
    AppendedAllocation appendVertexStorage(ContextHandle handle, size_t count, size_t stride) {
        return this->append(handle, count, stride);
    }

    // Default overloads without context handle (uses active/default context)
    AppendedAllocation append(size_t count, size_t stride);
    AppendedAllocation appendVertexStorage(size_t count, size_t stride) {
        return this->append(count, stride);
    }

    // Finalizes storage for a specific DrawContext/DrawPass and returns its BindBufferInfo.
    BindBufferInfo finalize(DrawBufferManager* bufferMgr, ContextHandle handle);
    BindBufferInfo finalize(DrawBufferManager* bufferMgr);

    BindBufferInfo getBufferInfo() const { return fBufferInfo.value_or(BindBufferInfo{}); }
    bool hasData() const { return fBufferInfo.has_value() && fBufferInfo->fBuffer != nullptr; }
    SkDEBUGCODE(bool isFinalized() const { return fBufferInfo.has_value(); })

private:
    struct ContextStorage {
        bool fActive = true;
        SkTDArray<char> fGradientData;
        skia_private::THashMap<const SkGradientBaseShader*, int> fGradientOffsetCache;
        SkTDArray<char> fAppendedData;

        ~ContextStorage();
    };

    std::pair<float*, int> allocateFloatData(ContextStorage* ctx, int floatCount);
    ContextStorage* getContext(ContextHandle handle);

    static constexpr int kMaxGradientStops = 1024 * 1024;
    static constexpr int kMaxStorageFloats =
            static_cast<int>(std::numeric_limits<uint32_t>::max() / sizeof(float));

    skia_private::STArray<4, std::unique_ptr<ContextStorage>> fContexts;

    std::optional<BindBufferInfo> fBufferInfo;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_StorageBufferManager_DEFINED
