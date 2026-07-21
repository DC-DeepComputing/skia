/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/StorageBufferManager.h"

#include "src/gpu/graphite/BufferManager.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

StorageBufferManager::StorageBufferManager() {
    this->reset();
}

StorageBufferManager::~StorageBufferManager() {
    this->reset();
}

int StorageBufferManager::size(ContextHandle handle) const {
    if (handle < static_cast<ContextHandle>(fContexts.size()) && fContexts[handle] &&
        fContexts[handle]->fActive) {
        const auto& ctx = fContexts[handle];
        int gradSize = SkAlignTo<int>(ctx->fGradientDataSize, kStructAlignment);
        int appSize = SkAlignTo<int>(ctx->fAppendedData.size(), kStructAlignment);
        return gradSize + appSize;
    }
    return 0;
}

int StorageBufferManager::size() const {
    int total = 0;
    for (int i = 0; i < fContexts.size(); ++i) {
        total += this->size(static_cast<ContextHandle>(i));
    }
    return total;
}

void StorageBufferManager::reset() {
    for (auto [k, _] : fGradientOffsetCache) {
        k->unref();
    }
    fGradientOffsetCache.reset();
    fGradientData.clear();

    fContexts.clear();
    fBufferInfo.reset();
}

StorageBufferManager::ContextHandle StorageBufferManager::registerContext() {
    ContextHandle handle = static_cast<ContextHandle>(fContexts.size());
    fContexts.push_back(std::make_unique<ContextStorage>());
    return handle;
}

void StorageBufferManager::unregisterContext(ContextHandle handle) {
    if (handle < static_cast<ContextHandle>(fContexts.size()) && fContexts[handle]) {
        fContexts[handle]->fActive = false;
        fContexts[handle]->fAppendedData.clear();
    }
}

StorageBufferManager::ContextStorage* StorageBufferManager::getContext(ContextHandle handle) {
    if (handle < static_cast<ContextHandle>(fContexts.size()) && fContexts[handle]) {
        return fContexts[handle].get();
    }
    if (fContexts.empty()) {
        this->registerContext();
    }
    return fContexts[0].get();
}

std::pair<float*, int> StorageBufferManager::allocateGradientData(
        ContextHandle handle,
        int numStops,
        const SkGradientBaseShader* shader) {
    ContextStorage* ctx = this->getContext(handle);
    SkASSERT(!ctx->fGradientsFinalized);
    if (numStops > kMaxGradientStops) {
        return {nullptr, -1};
    }

    int* existingOffset = fGradientOffsetCache.find(shader);
    if (existingOffset) {
        ctx->fGradientDataSize = std::max(ctx->fGradientDataSize, static_cast<size_t>(*existingOffset + numStops * 5 * sizeof(float)));
        return {nullptr, *existingOffset};
    }

    auto [ptr, offset] = this->allocateFloatData(numStops * 5); // 4 for color, 1 for offset

    if (ptr) {
        SkASSERT(offset >= 0);
        shader->ref();
        fGradientOffsetCache.set(shader, offset);
        ctx->fGradientDataSize = fGradientData.size();
    }

    return {ptr, offset};
}

std::pair<float*, int> StorageBufferManager::allocateGradientData(
        int numStops,
        const SkGradientBaseShader* shader) {
    return this->allocateGradientData(0, numStops, shader);
}

std::pair<float*, int> StorageBufferManager::allocateFloatData(int floatCount) {
    int currentSizeBytes = fGradientData.size();
    int floatSizeBytes = floatCount * sizeof(float);
    if (kMaxStorageFloats * sizeof(float) - floatSizeBytes < (size_t)currentSizeBytes) {
        return {nullptr, -1};
    }

    fGradientData.resize(currentSizeBytes + floatSizeBytes);
    float* startPtr = reinterpret_cast<float*>(fGradientData.data() + currentSizeBytes);

    return {startPtr, currentSizeBytes};
}

void StorageBufferManager::finalizeCacheStorageData(ContextHandle handle) {
    ContextStorage* ctx = this->getContext(handle);
    if (ctx && ctx->fActive && !ctx->fGradientsFinalized) {
        ctx->fGradientDataSize = SkAlignTo<size_t>(ctx->fGradientDataSize, kStructAlignment);
        ctx->fGradientsFinalized = true;
    }
}

void StorageBufferManager::finalizeCacheStorageData() {
    this->finalizeCacheStorageData(0);
}

StorageBufferManager::AppendedAllocation StorageBufferManager::append(ContextHandle handle,
                                                                       size_t count,
                                                                       size_t stride) {
    ContextStorage* ctx = this->getContext(handle);
    SkASSERT(ctx->fGradientsFinalized);

    size_t currentAppendedOffset = SkAlignTo<size_t>(ctx->fAppendedData.size(), kStructAlignment);
    size_t allocationBytes = count * stride;
    ctx->fAppendedData.resize(currentAppendedOffset + allocationBytes);

    size_t gpuByteOffset = ctx->fGradientDataSize + currentAppendedOffset;

    void* ptr = ctx->fAppendedData.data() + currentAppendedOffset;
    return {VertexWriter{ptr, allocationBytes}, static_cast<uint32_t>(gpuByteOffset)};
}

StorageBufferManager::AppendedAllocation StorageBufferManager::append(size_t count,
                                                                       size_t stride) {
    return this->append(0, count, stride);
}

BindBufferInfo StorageBufferManager::finalize(DrawBufferManager* bufferMgr, ContextHandle handle) {
    ContextStorage* ctx = this->getContext(handle);
    if (!ctx || !ctx->fActive) {
        return BindBufferInfo{};
    }

    if (!ctx->fGradientsFinalized) {
        this->finalizeCacheStorageData(handle);
    }

    size_t gradSize = ctx->fGradientDataSize;
    size_t appSize = SkAlignTo<size_t>(ctx->fAppendedData.size(), kStructAlignment);
    size_t totalBytes = gradSize + appSize;

    if (totalBytes > 0) {
        auto [writer, bufferInfo, _] =
                bufferMgr->getMappedStorageBuffer(totalBytes, kStructAlignment);
        if (writer) {
            if (gradSize > 0) {
                size_t bytesToWrite = std::min(gradSize, static_cast<size_t>(fGradientData.size()));
                writer.write(fGradientData.data(), bytesToWrite);
                if (gradSize > bytesToWrite) {
                    writer.zeroBytes(gradSize - bytesToWrite);
                }
            }

            if (!ctx->fAppendedData.empty()) {
                writer.write(ctx->fAppendedData.data(), ctx->fAppendedData.size());
            }

            fBufferInfo = bufferInfo;
            this->unregisterContext(handle);
            return bufferInfo;
        }
    }
    return BindBufferInfo{};
}

BindBufferInfo StorageBufferManager::finalize(DrawBufferManager* bufferMgr) {
    if (!fContexts.empty()) {
        return this->finalize(bufferMgr, 0);
    }
    return BindBufferInfo{};
}



} // namespace skgpu::graphite
