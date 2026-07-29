/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/StorageContext.h"

#include "include/private/SkAlign.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

StorageContext::StorageContext() = default;

StorageContext::~StorageContext() {
    this->resetCache();
}

void StorageContext::GradientCache::reset() {
    for (const SkGradientBaseShader* shader : fContextGradientsList) {
        shader->unref();
    }
    fLocalGradientOffsetCache.reset();
    fContextGradientsList.clear();
    fGradientData.clear();
    fGradientDataSize = 0;
}

void StorageContext::resetCache() {
    fGradientCache.reset();
}

void StorageContext::resetAppendedStorage() {
    fAppendedData.clear();
    fBufferInfo.reset();
    SkDEBUGCODE(fGradientsFinalized = false;)
}

std::pair<float*, int> StorageContext::allocateGradientData(int numStops,
                                                            const SkGradientBaseShader* shader) {
    SkASSERT(!fGradientsFinalized);
    if (numStops > GradientCache::kMaxGradientStops) {
        return {nullptr, -1};
    }

    int* existingLocalOffset = fGradientCache.fLocalGradientOffsetCache.find(shader);
    if (existingLocalOffset) {
        return {nullptr, *existingLocalOffset};
    }

    int floatOffset = fGradientCache.fGradientData.size();
    int floatCount = numStops * 5;
    if (GradientCache::kMaxStorageFloats - floatCount < floatOffset) {
        return {nullptr, -1};
    }

    fGradientCache.fGradientData.resize(floatOffset + floatCount);
    float* dstData = fGradientCache.fGradientData.data() + floatOffset;

    shader->ref();
    fGradientCache.fLocalGradientOffsetCache.set(shader, floatOffset);
    fGradientCache.fContextGradientsList.push_back(shader);
    fGradientCache.fGradientDataSize = fGradientCache.fGradientData.size_bytes();

    return {dstData, floatOffset};
}

int StorageContext::size() const {
    int gradSize = SkAlignTo<int>(fGradientCache.fGradientDataSize, kStructAlignment);
    int appSize = SkAlignTo<int>(fAppendedData.size(), kStructAlignment);
    return gradSize + appSize;
}

void StorageContext::finalizeGradientStorage() {
    fGradientCache.fGradientDataSize =
            SkAlignTo<size_t>(fGradientCache.fGradientDataSize, kStructAlignment);
    SkDEBUGCODE(fGradientsFinalized = true;)
}

[[maybe_unused]]
StorageContext::AppendedAllocation StorageContext::append(size_t count, size_t stride) {
    SkASSERT(fGradientsFinalized);
    size_t currentAppendedOffset = SkAlignTo<size_t>(fAppendedData.size(), kStructAlignment);
    size_t allocationBytes = count * stride;
    fAppendedData.resize(currentAppendedOffset + allocationBytes);

    size_t gpuByteOffset = fGradientCache.fGradientDataSize + currentAppendedOffset;

    void* ptr = fAppendedData.data() + currentAppendedOffset;
    return {VertexWriter{ptr, allocationBytes}, static_cast<uint32_t>(gpuByteOffset)};
}

BindBufferInfo StorageContext::finalize(DrawBufferManager* bufferMgr) {
    this->finalizeGradientStorage();

    size_t gradSize = fGradientCache.fGradientDataSize;
    size_t appSize = SkAlignTo<size_t>(fAppendedData.size(), kStructAlignment);
    size_t totalBytes = gradSize + appSize;

    if (totalBytes > 0) {
        auto [writer, bufferInfo, _] =
                bufferMgr->getMappedStorageBuffer(totalBytes, /*stride=*/1);
        if (writer) {
            if (gradSize > 0) {
                writer.write(fGradientCache.fGradientData.data(),
                             fGradientCache.fGradientData.size_bytes());
                if (gradSize > fGradientCache.fGradientData.size_bytes()) {
                    writer.zeroBytes(gradSize - fGradientCache.fGradientData.size_bytes());
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

} // namespace skgpu::graphite
