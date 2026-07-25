/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/StorageBufferManager.h"

#include "src/gpu/graphite/BufferManager.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

namespace skgpu::graphite {

StorageBufferManager::~StorageBufferManager() { this->reset(); }

void StorageBufferManager::GradientCache::reset() {
    for (auto [k, _] : fGradientHash) {
        k->unref();
    }
    fGradientHash.reset();
    fGradientData.clear();
}

void StorageBufferManager::reset() {
    fGradientCache.reset();
}

std::pair<float*, int> StorageBufferManager::allocateGradientData(
        StorageContext* ctx,
        int numStops,
        const SkGradientBaseShader* shader) {
    if (!ctx) {
        return {nullptr, -1};
    }
    SkASSERT(!ctx->fGradientsFinalized);
    if (numStops > kMaxGradientStops) {
        return {nullptr, -1};
    }

    int* existingLocalOffset = ctx->fLocalGradientOffsetCache.find(shader);
    if (existingLocalOffset) {
        return {nullptr, *existingLocalOffset};
    }

    float* dstData = nullptr;
    GradientCache::GradientInfo* globalInfo = fGradientCache.fGradientHash.find(shader);
    if (!globalInfo) {
        int globalOffset = fGradientCache.fGradientData.size();
        int floatCount = numStops * 5;
        if (kMaxStorageFloats - floatCount < globalOffset) {
            return {nullptr, -1};
        }

        fGradientCache.fGradientData.resize(globalOffset + floatCount);
        dstData = fGradientCache.fGradientData.data() + globalOffset;

        shader->ref();
        fGradientCache.fGradientHash.set(shader, {globalOffset, floatCount});
        globalInfo = fGradientCache.fGradientHash.find(shader);
    }

    int localFloatOffset = ctx->fGradientDataSize / sizeof(float);
    ctx->fLocalGradientOffsetCache.set(shader, localFloatOffset);
    ctx->fContextGradientsList.push_back(shader);
    ctx->fGradientDataSize += globalInfo->fFloatCount * sizeof(float);

    return {dstData, localFloatOffset};
}

BindBufferInfo StorageBufferManager::finalize(DrawBufferManager* bufferMgr, StorageContext* ctx) {
    if (!ctx) {
        return BindBufferInfo{};
    }

    ctx->finalizeGradientStorage();

    size_t gradSize = ctx->fGradientDataSize;
    size_t appSize = SkAlignTo<size_t>(ctx->fAppendedData.size(), StorageContext::kStructAlignment);
    size_t totalBytes = gradSize + appSize;

    if (totalBytes > 0) {
        auto [writer, bufferInfo, _] =
                bufferMgr->getMappedStorageBuffer(totalBytes, /*stride=*/1);
        if (writer) {
            // 1. Pack gradient floats for this context in local run order
            if (gradSize > 0) {
                size_t writtenGradientBytes = 0;
                for (const SkGradientBaseShader* shader : ctx->fContextGradientsList) {
                    GradientCache::GradientInfo* info =
                            fGradientCache.fGradientHash.find(shader);
                    if (info) {
                        size_t bytesToCopy = info->fFloatCount * sizeof(float);
                        writer.write(fGradientCache.fGradientData.data() + info->fGlobalFloatOffset,
                                     bytesToCopy);
                        writtenGradientBytes += bytesToCopy;
                    }
                }
                if (gradSize > writtenGradientBytes) {
                    writer.zeroBytes(gradSize - writtenGradientBytes);
                }
            }

            // 2. Write appended vertex/instance storage
            if (!ctx->fAppendedData.empty()) {
                writer.write(ctx->fAppendedData.data(), ctx->fAppendedData.size());
                size_t paddingBytes = appSize - ctx->fAppendedData.size();
                if (paddingBytes > 0) {
                    writer.zeroBytes(paddingBytes);
                }
            }

            ctx->fBufferInfo = bufferInfo;
            return bufferInfo;
        }
    }
    return BindBufferInfo{};
}

} // namespace skgpu::graphite
