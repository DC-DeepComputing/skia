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

void StorageBufferManager::reset() {
    // Remove the manually added refs on the keys in fGradientOffsetCache
    for (auto [k, _] : fGradientOffsetCache) {
        k->unref();
    }
    fStorage.clear();
    fGradientOffsetCache.reset();
    fBufferInfo.reset();
}

std::pair<float*, int> StorageBufferManager::allocateGradientData(int numStops,
                                                                   const SkGradientBaseShader* shader) {
    SkASSERT(!this->isFinalized());
    if (numStops > kMaxGradientStops) {
        return {nullptr, -1};
    }

    int* existingOffset = fGradientOffsetCache.find(shader);
    if (existingOffset) {
        return {nullptr, *existingOffset};
    }
    auto [ptr, offset] = this->allocateFloatData(numStops * 5); // 4 for color, 1 for offset

    // Only cache the storage if it was allocated successfully.
    if (ptr) {
        SkASSERT(offset >= 0);
        shader->ref();
        fGradientOffsetCache.set(shader, offset);
    }

    return {ptr, offset};
}

std::pair<float*, int> StorageBufferManager::allocateFloatData(int floatCount) {
    int currentSizeBytes = fStorage.size();
    int floatSizeBytes = floatCount * sizeof(float);
    if (kMaxStorageFloats * sizeof(float) - floatSizeBytes < (size_t)currentSizeBytes) {
        return {nullptr, -1};
    }

    fStorage.resize(currentSizeBytes + floatSizeBytes);
    float* startPtr = reinterpret_cast<float*>(fStorage.data() + currentSizeBytes);

    return {startPtr, currentSizeBytes / static_cast<int>(sizeof(float))};
}

StorageBufferManager::AppendedAllocation StorageBufferManager::appendVertexStorage(size_t count,
                                                                                    size_t stride) {
    SkASSERT(!this->isFinalized());
    size_t byteOffset = SkAlignTo<size_t>(fStorage.size(), kStructAlignment);
    size_t allocationBytes = count * stride;
    fStorage.resize(byteOffset + allocationBytes);

    void* ptr = fStorage.data() + byteOffset;
    return {ptr, static_cast<uint32_t>(byteOffset)};
}

bool StorageBufferManager::finalize(DrawBufferManager* bufferMgr) {
    SkASSERT(!this->isFinalized());
    if (!fStorage.empty()) {
        auto [writer, bufferInfo, _] =
                bufferMgr->getMappedStorageBuffer(fStorage.size(), sizeof(float));
        if (writer) {
            writer.write(fStorage.data(), fStorage.size_bytes());
            this->reset();
            fBufferInfo = bufferInfo;
            return true;
        }
        return false;
    } else {
        fBufferInfo = BindBufferInfo();
    }
    return true;
}

} // namespace skgpu::graphite
