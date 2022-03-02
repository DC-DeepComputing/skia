/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawBufferManager.h"

#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/ResourceProvider.h"

namespace skgpu {

namespace {

// TODO: Tune these values on real world data
static constexpr size_t kVertexBufferSize = 1 << 14;
static constexpr size_t kIndexBufferSize = 1 << 15;
static constexpr size_t kUniformBufferSize = 1 << 15;

void* map_offset(BindBufferInfo binding) {
    // DrawBufferManager owns the Buffer, and this is only ever called when we know
    // it's okay to remove 'const' from the Buffer*
    return SkTAddOffset<void>(const_cast<Buffer*>(binding.fBuffer)->map(),
                              static_cast<ptrdiff_t>(binding.fOffset));
}

} // anonymous namespace

DrawBufferManager::DrawBufferManager(ResourceProvider* resourceProvider,
                                     size_t uniformStartAlignment)
        : fResourceProvider(resourceProvider)
        , fUniformStartAlignment(uniformStartAlignment) {}

DrawBufferManager::~DrawBufferManager() {}

static bool can_fit(size_t requestedSize,
                    Buffer* buffer,
                    size_t currentOffset,
                    size_t alignment) {
    size_t startOffset = SkAlignTo(currentOffset, alignment);
    return requestedSize <= (buffer->size() - startOffset);
}

std::tuple<VertexWriter, BindBufferInfo> DrawBufferManager::getVertexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        BindBufferInfo bindInfo;
        bindInfo.fBuffer = nullptr;
        bindInfo.fOffset = 0;
        return {VertexWriter(nullptr), bindInfo};
    }
    if (fCurrentVertexBuffer &&
        !can_fit(requiredBytes, fCurrentVertexBuffer.get(), fVertexOffset, /*alignment=*/1)) {
        fUsedBuffers.push_back(std::move(fCurrentVertexBuffer));
    }

    if (!fCurrentVertexBuffer) {
        SkASSERT(requiredBytes <= kVertexBufferSize);
        fCurrentVertexBuffer = fResourceProvider->findOrCreateBuffer(kVertexBufferSize,
                                                                     BufferType::kVertex,
                                                                     PrioritizeGpuReads::kNo);
        fVertexOffset = 0;
        if (!fCurrentVertexBuffer) {
            return {VertexWriter(), BindBufferInfo()};
        }
    }
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = fCurrentVertexBuffer.get();
    bindInfo.fOffset = fVertexOffset;
    fVertexOffset += requiredBytes;
    return {VertexWriter(map_offset(bindInfo)), bindInfo};
}

std::tuple<IndexWriter, BindBufferInfo> DrawBufferManager::getIndexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        BindBufferInfo bindInfo;
        bindInfo.fBuffer = nullptr;
        bindInfo.fOffset = 0;
        return {IndexWriter(nullptr), bindInfo};
    }
    if (fCurrentIndexBuffer &&
        !can_fit(requiredBytes, fCurrentIndexBuffer.get(), fIndexOffset, /*alignment=*/1)) {
        fUsedBuffers.push_back(std::move(fCurrentIndexBuffer));
    }

    if (!fCurrentIndexBuffer) {
        SkASSERT(requiredBytes <= kIndexBufferSize);
        fCurrentIndexBuffer = fResourceProvider->findOrCreateBuffer(kIndexBufferSize,
                                                                    BufferType::kIndex,
                                                                    PrioritizeGpuReads::kNo);
        fIndexOffset = 0;
        if (!fCurrentIndexBuffer) {
            return {IndexWriter(), BindBufferInfo()};
        }
    }
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = fCurrentIndexBuffer.get();
    bindInfo.fOffset = fIndexOffset;
    fIndexOffset += requiredBytes;
    return {IndexWriter(map_offset(bindInfo)), bindInfo};
}

std::tuple<UniformWriter, BindBufferInfo> DrawBufferManager::getUniformWriter(
        size_t requiredBytes) {
    if (!requiredBytes) {
        BindBufferInfo bindInfo;
        bindInfo.fBuffer = nullptr;
        bindInfo.fOffset = 0;
        return {UniformWriter(nullptr), bindInfo};
    }
    if (fCurrentUniformBuffer &&
        !can_fit(requiredBytes,
                 fCurrentUniformBuffer.get(),
                 fUniformOffset,
                 fUniformStartAlignment)) {
        fUsedBuffers.push_back(std::move(fCurrentUniformBuffer));
    }

    if (!fCurrentUniformBuffer) {
        SkASSERT(requiredBytes <= kUniformBufferSize);
        fCurrentUniformBuffer = fResourceProvider->findOrCreateBuffer(kUniformBufferSize,
                                                                      BufferType::kUniform,
                                                                      PrioritizeGpuReads::kNo);
        fUniformOffset = 0;
        if (!fCurrentUniformBuffer) {
            return {UniformWriter(), BindBufferInfo()};
        }
    }
    fUniformOffset = SkAlignTo(fUniformOffset, fUniformStartAlignment);
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = fCurrentUniformBuffer.get();
    bindInfo.fOffset = fUniformOffset;
    fUniformOffset += requiredBytes;
    return {UniformWriter(map_offset(bindInfo)), bindInfo};
}

void DrawBufferManager::transferToCommandBuffer(CommandBuffer* commandBuffer) {
    for (auto& buffer : fUsedBuffers) {
        buffer->unmap();
        commandBuffer->trackResource(std::move(buffer));
    }
    fUsedBuffers.clear();
}

BindBufferInfo DrawBufferManager::getStaticBuffer(BufferType type,
                                                  InitializeBufferFn initFn,
                                                  BufferSizeFn sizeFn) {
    uintptr_t key = reinterpret_cast<uintptr_t>(initFn);
    auto cachedBuffer = fStaticBuffers.find(key);
    if (cachedBuffer != fStaticBuffers.end()) {
        return {cachedBuffer->second.get(), 0};
    }

    // Create a new one and add it to the DBM
    // TODO: These should probably be REsourceProvider cached, but that requires multithread safety
    // since multiple recordings may want to init the buffer at the same time.
    size_t size = sizeFn();
    sk_sp<Buffer> buffer = fResourceProvider->findOrCreateBuffer(size, type, PrioritizeGpuReads::kNo);
    if (!buffer) {
        return {nullptr, 0};
    }

    BindBufferInfo bindInfo = {buffer.get(), 0};
    initFn(VertexWriter(map_offset(bindInfo)), size);
    fStaticBuffers.insert({key, buffer});
    return bindInfo;
}

} // namespace skgpu
