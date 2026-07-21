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

StorageBufferManager::ContextStorage::~ContextStorage() {
    for (auto [k, _] : fGradientOffsetCache) {
        k->unref();
    }
}

StorageBufferManager::StorageBufferManager() {
    this->reset();
}

StorageBufferManager::~StorageBufferManager() {
    this->reset();
}

int StorageBufferManager::size(ContextHandle handle) const {
    if (handle < static_cast<ContextHandle>(fContexts.size()) && fContexts[handle] && fContexts[handle]->fActive) {
        const auto& ctx = fContexts[handle];
        int gradSize = SkAlignTo<int>(ctx->fGradientData.size(), kStructAlignment);
        int appSize = SkAlignTo<int>(ctx->fAppendedData.size(), kStructAlignment);
        return gradSize + appSize;
    }
    return 0;
}

int StorageBufferManager::size() const {
    int total = 0;
    for (size_t i = 0; i < fContexts.size(); ++i) {
        total += this->size(static_cast<ContextHandle>(i));
    }
    return total;
}

void StorageBufferManager::reset() {
    fContexts.clear();
    fBufferInfo.reset();
}

StorageBufferManager::ContextHandle StorageBufferManager::registerContext() {
    SkASSERT(!this->isFinalized());
    ContextHandle handle = static_cast<ContextHandle>(fContexts.size());
    fContexts.push_back(std::make_unique<ContextStorage>());
    return handle;
}

void StorageBufferManager::unregisterContext(ContextHandle handle) {
    if (handle < static_cast<ContextHandle>(fContexts.size()) && fContexts[handle]) {
        fContexts[handle]->fActive = false;
        fContexts[handle]->fGradientData.clear();
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
    SkASSERT(!this->isFinalized());
    if (numStops > kMaxGradientStops) {
        return {nullptr, -1};
    }

    ContextStorage* ctx = this->getContext(handle);
    int* existingOffset = ctx->fGradientOffsetCache.find(shader);
    if (existingOffset) {
        return {nullptr, *existingOffset};
    }
    auto [ptr, offset] = this->allocateFloatData(ctx, numStops * 5); // 4 for color, 1 for offset

    if (ptr) {
        SkASSERT(offset >= 0);
        shader->ref();
        ctx->fGradientOffsetCache.set(shader, offset);
    }

    return {ptr, offset};
}

std::pair<float*, int> StorageBufferManager::allocateGradientData(
        int numStops,
        const SkGradientBaseShader* shader) {
    return this->allocateGradientData(0, numStops, shader);
}

std::pair<float*, int> StorageBufferManager::allocateFloatData(ContextStorage* ctx, int floatCount) {
    int currentSizeBytes = ctx->fGradientData.size();
    int floatSizeBytes = floatCount * sizeof(float);
    if (kMaxStorageFloats * sizeof(float) - floatSizeBytes < (size_t)currentSizeBytes) {
        return {nullptr, -1};
    }

    ctx->fGradientData.resize(currentSizeBytes + floatSizeBytes);
    float* startPtr = reinterpret_cast<float*>(ctx->fGradientData.data() + currentSizeBytes);

    return {startPtr, currentSizeBytes};
}

StorageBufferManager::AppendedAllocation StorageBufferManager::append(ContextHandle handle,
                                                                       size_t count,
                                                                       size_t stride) {
    SkASSERT(!this->isFinalized());
    ContextStorage* ctx = this->getContext(handle);

    size_t currentAppendedOffset = SkAlignTo<size_t>(ctx->fAppendedData.size(), kStructAlignment);
    size_t allocationBytes = count * stride;
    ctx->fAppendedData.resize(currentAppendedOffset + allocationBytes);

    size_t gradientSectionBytes = SkAlignTo<size_t>(ctx->fGradientData.size(), kStructAlignment);
    size_t gpuByteOffset = gradientSectionBytes + currentAppendedOffset;

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

    size_t gradSize = SkAlignTo<size_t>(ctx->fGradientData.size(), kStructAlignment);
    size_t appSize = SkAlignTo<size_t>(ctx->fAppendedData.size(), kStructAlignment);
    size_t totalBytes = gradSize + appSize;

    if (totalBytes > 0) {
        auto [writer, bufferInfo, _] =
                bufferMgr->getMappedStorageBuffer(totalBytes, kStructAlignment);
        if (writer) {
            static const char kZeros[16] = {0};
            if (!ctx->fGradientData.empty()) {
                writer.write(ctx->fGradientData.data(), ctx->fGradientData.size());
                size_t pad = gradSize - ctx->fGradientData.size();
                if (pad > 0) {
                    writer.write(kZeros, pad);
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

bool StorageBufferManager::finalize(DrawBufferManager* bufferMgr, ContextHandle handle, bool) {
    return this->finalize(bufferMgr, handle).fBuffer != nullptr;
}

} // namespace skgpu::graphite
