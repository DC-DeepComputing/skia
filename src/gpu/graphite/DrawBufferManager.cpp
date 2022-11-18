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

void* map_offset(BindBufferInfo binding) {
    // DrawBufferManager owns the Buffer, and this is only ever called when we know
    // it's okay to remove 'const' from the Buffer*
    auto* buffer = binding.fStagingBuffer ? binding.fStagingBuffer : binding.fBuffer;
    return SkTAddOffset<void>(const_cast<Buffer*>(buffer)->map(),
                              static_cast<ptrdiff_t>(binding.fOffset));
}

template <size_t BufferBlockSize>
size_t sufficient_block_size(size_t requiredBytes) {
    // Always request a buffer at least 'requiredBytes', but keep them in multiples of
    // 'BufferBlockSize' for improved reuse.
    static constexpr size_t kMaxSize   = std::numeric_limits<size_t>::max();
    static constexpr size_t kMaxBlocks = kMaxSize / BufferBlockSize;
    size_t blocks = (requiredBytes / BufferBlockSize) + 1;
    size_t bufferSize = blocks > kMaxBlocks ? kMaxSize : (blocks * BufferBlockSize);
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
        , fCurrentBuffers{1, 1, uniformStartAlignment, ssboStartAlignment} {}

DrawBufferManager::~DrawBufferManager() {}

std::tuple<VertexWriter, BindBufferInfo> DrawBufferManager::getVertexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    if (!prepareBuffer(kVertexBufferIndex, requiredBytes)) {
        return {};
    }

    auto& info = fCurrentBuffers[kVertexBufferIndex];
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = info.fBuffer.get();
    bindInfo.fStagingBuffer = info.fStagingBuffer.get();
    bindInfo.fOffset = info.fOffset;
    info.fOffset += requiredBytes;
    return {VertexWriter(map_offset(bindInfo), requiredBytes), bindInfo};
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
    BindBufferInfo bindInfo;
    bindInfo.fBuffer = info.fBuffer.get();
    bindInfo.fStagingBuffer = info.fStagingBuffer.get();
    bindInfo.fOffset = info.fOffset;
    info.fOffset += requiredBytes;
    return {IndexWriter(map_offset(bindInfo), requiredBytes), bindInfo};
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
    bindInfo.fStagingBuffer = info.fStagingBuffer.get();
    SkASSERT(info.fStagingBuffer);
    bindInfo.fOffset = info.fOffset;
    info.fOffset += requiredBytes;
    return {UniformWriter(map_offset(bindInfo), requiredBytes), bindInfo};
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
    bindInfo.fStagingBuffer = info.fStagingBuffer.get();
    bindInfo.fOffset = info.fOffset;
    info.fOffset += requiredBytes;
    return {UniformWriter(map_offset(bindInfo), requiredBytes), bindInfo};
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
        return {cachedBuffer->second.get(),nullptr, 0};
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
    return {buffer.get(), nullptr, 0};
}

void DrawBufferManager::transferToRecording(Recording* recording) {
    bool useStagingBuffer = caps()->stagingBufferPreferred();

    for (auto& buffer : fUsedBuffers) {
        if (buffer->isMapped()) {
            buffer->unmap();
        }
        recording->priv().addResourceRef(std::move(buffer));
    }
    fUsedBuffers.clear();


    // The current draw buffers have not been added to fUsedBuffers,
    // so we need to handle them as well.
    for (auto &info : fCurrentBuffers) {
        if (!info.fBuffer) {
            continue;
        }
        if (useStagingBuffer) {
            recording->priv().addResourceRef(info.fBuffer);
            info.fStagingBuffer->unmap();
            recording->priv().addResourceRef(std::move(info.fStagingBuffer));
        } else {
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

const Caps* DrawBufferManager::caps() const {
    return fResourceProvider->sharedContext()->caps();
}

bool DrawBufferManager::prepareBuffer(int bufferIndex, size_t requiredBytes) {
    SkASSERT(requiredBytes);

    bool useStagingBuffer = caps()->stagingBufferPreferred();

    SkASSERT(static_cast<size_t>(bufferIndex) < fCurrentBuffers.size());
    auto& info = fCurrentBuffers[bufferIndex];

    if (info.fBuffer &&
        !can_fit(requiredBytes, info.fBuffer.get(), info.fOffset, info.fStartAlignment)) {
        fUsedBuffers.push_back(std::move(info.fBuffer));
        if (info.fStagingBuffer) {
            fUsedBuffers.push_back(std::move(info.fStagingBuffer));
        }
    }

    static std::function<size_t(size_t)> kBlockSizeFunctions[] = {
        &sufficient_block_size<kVertexBufferSize>,
        &sufficient_block_size<kIndexBufferSize>,
        &sufficient_block_size<kUniformBufferSize>,
        &sufficient_block_size<kStorageBufferSize>,
    };
    SkASSERT(std::size(kBlockSizeFunctions) > static_cast<size_t>(bufferIndex));

    constexpr static BufferType kBufferTypes[] = {
        BufferType::kVertex,
        BufferType::kIndex,
        BufferType::kUniform,
        BufferType::kStorage,
    };
    SkASSERT(std::size(kBufferTypes) > static_cast<size_t>(bufferIndex));

    if (!info.fBuffer) {
        size_t bufferSize = kBlockSizeFunctions[bufferIndex](requiredBytes);
        info.fBuffer = fResourceProvider->findOrCreateBuffer(
                bufferSize,
                kBufferTypes[bufferIndex],
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
        SkASSERT(info.fStagingBuffer);
        if (!info.fStagingBuffer) {
            return false;
        }
    }

    return true;
}

DrawBufferManager::BufferInfo::BufferInfo(size_t startAlignment)
        : fStartAlignment(startAlignment) {}

DrawBufferManager::BufferInfo::~BufferInfo() = default;

} // namespace skgpu::graphite
