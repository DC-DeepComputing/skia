/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawBufferManager.h"

#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

namespace {

// TODO: Tune these values on real world data
static constexpr size_t kVertexBufferSize = 128 << 10; // 16 KB
static constexpr size_t kIndexBufferSize =   2 << 10; //  2 KB
static constexpr size_t kUniformBufferSize = 2 << 10; //  2 KB
static constexpr size_t kStorageBufferSize = 2 << 10; //  2 KB

void* map_offset(const sk_sp<Buffer>& buffer, const sk_sp<Buffer>& stagingBuffer, size_t offset) {
    return SkTAddOffset<void>((stagingBuffer ? stagingBuffer.get() : buffer.get())->map(),
                              static_cast<ptrdiff_t>(offset));
}

size_t sufficient_block_size(size_t requiredBytes, size_t blockSize) {
    // Always request a buffer at least 'requiredBytes', but keep them in multiples of
    // 'blockSize' for improved reuse.
    static constexpr size_t kMaxSize   = std::numeric_limits<size_t>::max();
    size_t maxBlocks = kMaxSize / blockSize;
    size_t blocks = (requiredBytes / blockSize) + 1;
    size_t bufferSize = blocks > maxBlocks ? kMaxSize : (blocks * blockSize);
    SkASSERT(requiredBytes < bufferSize);
    return bufferSize;
}

bool can_fit(size_t requestedSize,
             Buffer* buffer,
             size_t currentOffset,
             size_t alignment) {
    size_t startOffset = SkAlignTo(currentOffset, alignment);
    return requestedSize <= (buffer->size() - startOffset);
}

} // anonymous namespace

DrawBufferManager::DrawBufferManager(ResourceProvider* resourceProvider,
                                     size_t uniformStartAlignment,
                                     size_t ssboStartAlignment)
        : fResourceProvider(resourceProvider)
        , fCurrentBuffers{{
                {BufferType::kVertex,  /*fStartAlignment=*/4, kVertexBufferSize},
                {BufferType::kIndex,   /*fStartAlignment=*/1, kIndexBufferSize},
                {BufferType::kUniform, uniformStartAlignment, kUniformBufferSize},
                {BufferType::kStorage, ssboStartAlignment,    kStorageBufferSize} }} {}

DrawBufferManager::~DrawBufferManager() {}

std::tuple<VertexWriter, BindBufferInfo> DrawBufferManager::getVertexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    if (!this->prepareBuffer(kVertexBufferIndex, requiredBytes)) {
        return {};
    }

    auto& info = fCurrentBuffers[kVertexBufferIndex];
    info.fOffset = SkAlignTo(info.fOffset, info.fStartAlignment);
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = info.fBuffer.get();
    bindInfo.fOffset = info.fOffset;
    info.fOffset += requiredBytes;
    void* ptr = map_offset(info.fBuffer, info.fStagingBuffer, bindInfo.fOffset);
    return {VertexWriter(ptr, requiredBytes), bindInfo};
}

void DrawBufferManager::returnVertexBytes(size_t unusedBytes) {
    auto& info = fCurrentBuffers[kVertexBufferIndex];
    SkASSERT(info.fOffset >= unusedBytes);
    info.fOffset -= unusedBytes;
}

std::tuple<IndexWriter, BindBufferInfo> DrawBufferManager::getIndexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    if (!prepareBuffer(kIndexBufferIndex, requiredBytes)) {
        return {};
    }

    auto& info = fCurrentBuffers[kIndexBufferIndex];
    info.fOffset = SkAlignTo(info.fOffset, info.fStartAlignment);
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = info.fBuffer.get();
    bindInfo.fOffset = info.fOffset;
    info.fOffset += requiredBytes;
    void* ptr = map_offset(info.fBuffer, info.fStagingBuffer, bindInfo.fOffset);
    return {IndexWriter(ptr, requiredBytes), bindInfo};
}

std::tuple<UniformWriter, BindBufferInfo> DrawBufferManager::getUniformWriter(
        size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    if (!prepareBuffer(kUniformBufferIndex, requiredBytes)) {
        return {};
    }

    auto& info = fCurrentBuffers[kUniformBufferIndex];
    info.fOffset = SkAlignTo(info.fOffset, info.fStartAlignment);
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = info.fBuffer.get();
    bindInfo.fOffset = info.fOffset;
    info.fOffset += requiredBytes;
    void* ptr = map_offset(info.fBuffer, info.fStagingBuffer, bindInfo.fOffset);
    return {UniformWriter(ptr, requiredBytes), bindInfo};
}

std::tuple<UniformWriter, BindBufferInfo> DrawBufferManager::getSsboWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    if (!prepareBuffer(kStorageBufferIndex, requiredBytes)) {
        return {};
    }

    auto& info = fCurrentBuffers[kStorageBufferIndex];
    info.fOffset = SkAlignTo(info.fOffset, info.fStartAlignment);
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = info.fBuffer.get();
    bindInfo.fOffset = info.fOffset;
    info.fOffset += requiredBytes;
    void* ptr = map_offset(info.fBuffer, info.fStagingBuffer, bindInfo.fOffset);
    return {UniformWriter(ptr, requiredBytes), bindInfo};
}

BindBufferInfo DrawBufferManager::getStaticBuffer(BufferType type,
                                                  InitializeBufferFn initFn,
                                                  BufferSizeFn sizeFn) {
    // TODO(skbug:13059) - This should really be encapsulated in ResourceProvider using a shareable
    // GraphiteResourceKey. However, this lets us track resources cleanly and since DBM "shares" the
    // read-only BindBufferInfo across draw steps, we get equivalent behavior.
    uintptr_t key = reinterpret_cast<uintptr_t>(initFn);
    auto cachedBuffer = fStaticBuffers.find(key);
    if (cachedBuffer != fStaticBuffers.end()) {
        return {cachedBuffer->second.get(), 0};
    }

    // Create a new buffer and fill it in.
    // TODO: Ideally created once and then copied into a vertex buffer with PrioritizeGpuReads::kYes
    // but this lets us easily lazily initialize it for now.
    size_t size = sizeFn();
    auto buffer = fResourceProvider->findOrCreateBuffer(size, type, PrioritizeGpuReads::kNo);
    if (!buffer) {
        return {};
    }

    initFn(VertexWriter{buffer->map(), size}, size);
    buffer->unmap();
    fStaticBuffers.insert({key, buffer});
    return {buffer.get(), 0};
}

void DrawBufferManager::transferToRecording(Recording* recording) {
    bool useStagingBuffer = fResourceProvider->sharedContext()->caps()->stagingBufferPreferred();
    for (auto&[buffer, stagingBuffer] : fUsedBuffers) {
        if (useStagingBuffer) {
            if (stagingBuffer) {
                stagingBuffer->unmap();
                recording->priv().addTask(CopyBufferToBufferTask::Make(std::move(stagingBuffer),
                                                                       std::move(buffer)));
            } else {
                recording->priv().addResourceRef(std::move(buffer));
            }
        } else {
           buffer->unmap();
           recording->priv().addResourceRef(std::move(buffer));
        }
    }
    fUsedBuffers.clear();

    // The current draw buffers have not been added to fUsedBuffers,
    // so we need to handle them as well.
    for (auto &info : fCurrentBuffers) {
        if (!info.fBuffer) {
            continue;
        }
        if (useStagingBuffer) {
            if (info.fStagingBuffer) {
                info.fStagingBuffer->unmap();
                SkASSERT(info.fBuffer);
                recording->priv().addTask(CopyBufferToBufferTask::Make(
                        std::move(info.fStagingBuffer), info.fBuffer));
            }
        }else {
            info.fBuffer->unmap();
            recording->priv().addResourceRef(std::move(info.fBuffer));
        }
    }

    // Assume all static buffers were used, but don't lose our ref
    // TODO(skbug:13059) - If static buffers are stored in the ResourceProvider and queried on each
    // draw or owned by the RenderStep, we still need a way to track the static buffer *once* per
    // frame that relies on it.
    for (auto [_, buffer] : fStaticBuffers) {
        recording->priv().addResourceRef(buffer);
    }
}

bool DrawBufferManager::prepareBuffer(size_t bufferIndex, size_t requiredBytes) {
    SkASSERT(requiredBytes);

    bool useStagingBuffer = fResourceProvider->sharedContext()->caps()->stagingBufferPreferred();

    SkASSERT(static_cast<size_t>(bufferIndex) < fCurrentBuffers.size());
    auto& info = fCurrentBuffers[bufferIndex];

    if (info.fBuffer &&
        !can_fit(requiredBytes, info.fBuffer.get(), info.fOffset, info.fStartAlignment)) {
        SkASSERT(!info.fStagingBuffer || info.fBuffer->size() == info.fStagingBuffer->size());
        fUsedBuffers.emplace_back(std::move(info.fBuffer), std::move(info.fStagingBuffer));
    }

    if (!info.fBuffer) {
        size_t bufferSize = sufficient_block_size(requiredBytes, info.fBlockSize);
        info.fBuffer = fResourceProvider->findOrCreateBuffer(
                bufferSize,
                info.fType,
                useStagingBuffer ? PrioritizeGpuReads::kYes : PrioritizeGpuReads::kNo);
        info.fOffset = 0;
        if (!info.fBuffer) {
            return false;
        }
    }

    if (useStagingBuffer && !info.fStagingBuffer) {
        info.fStagingBuffer = fResourceProvider->findOrCreateBuffer(
                info.fBuffer->size(),
                BufferType::kXferCpuToGpu,
                PrioritizeGpuReads::kNo);
        SkASSERT(info.fBuffer->size() == info.fStagingBuffer->size());
        info.fOffset = 0;
        if (!info.fStagingBuffer) {
            return false;
        }
    }

    return true;
}

} // namespace skgpu::graphite
