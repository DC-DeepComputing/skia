/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/BufferManager.h"

#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

namespace {

// TODO: Tune these values on real world data
static constexpr size_t kVertexBufferSize = 16 << 10; // 16 KB
static constexpr size_t kIndexBufferSize =   2 << 10; //  2 KB
static constexpr size_t kUniformBufferSize = 2 << 10; //  2 KB
static constexpr size_t kStorageBufferSize = 2 << 10; //  2 KB

// TODO: Is it better to keep this above the max data size so we typically have one transfer buffer
// allocation? Or have it line up with kVertexBufferSize so if we end up needing to use transfer
// buffers for dynamic vertex data we can just reuse the first one?
static constexpr size_t kStaticTransferBufferSize = 2 << 10; // 2 KB

// The limit for all data created by the StaticBufferManager. This data remains alive for
// the entire SharedContext so we want to keep it small and give a concrete upper bound to
// clients for our steady-state memory usage.
// FIXME The current usage is 4732 bytes across static vertex and index buffers, but that includes
// multiple copies of tessellation data, and an unoptimized AnalyticRRect mesh. Once those issues
// are addressed, we can tighten this and decide on the transfer buffer sizing as well.
static constexpr size_t kMaxStaticDataSize = 6 << 10;

static constexpr size_t kVertexStartAlignment = 4;
static constexpr size_t kIndexStartAlignment = 4;

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

// ------------------------------------------------------------------------------------------------
// DrawBufferManager

DrawBufferManager::DrawBufferManager(ResourceProvider* resourceProvider,
                                     size_t uniformStartAlignment,
                                     size_t ssboStartAlignment)
        : fResourceProvider(resourceProvider)
        , fCurrentBuffers{{
                { BufferType::kVertex,  kVertexStartAlignment, kVertexBufferSize  },
                { BufferType::kIndex,   kIndexStartAlignment,  kIndexBufferSize   },
                { BufferType::kUniform, uniformStartAlignment, kUniformBufferSize },
                { BufferType::kStorage, ssboStartAlignment,    kStorageBufferSize } }} {}

DrawBufferManager::~DrawBufferManager() {}

std::tuple<VertexWriter, BindBufferInfo> DrawBufferManager::getVertexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kVertexBufferIndex];
    auto [ptr, bindInfo] = this->prepareBindBuffer(&info, requiredBytes);
    if (!ptr) {
        return {};
    }

    return {VertexWriter(ptr, requiredBytes), bindInfo};
}

void DrawBufferManager::returnVertexBytes(size_t unusedBytes) {
    SkASSERT(fCurrentBuffers[kVertexBufferIndex].fOffset >= unusedBytes);
    fCurrentBuffers[kVertexBufferIndex].fOffset -= unusedBytes;
}

std::tuple<IndexWriter, BindBufferInfo> DrawBufferManager::getIndexWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kIndexBufferIndex];
    auto [ptr, bindInfo] = this->prepareBindBuffer(&info, requiredBytes);
    if (!ptr) {
        return {};
    }

    return {IndexWriter(ptr, requiredBytes), bindInfo};
}

std::tuple<UniformWriter, BindBufferInfo> DrawBufferManager::getUniformWriter(
        size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kUniformBufferIndex];
    auto [ptr, bindInfo] = this->prepareBindBuffer(&info, requiredBytes);
    if (!ptr) {
        return {};
    }

    return {UniformWriter(ptr, requiredBytes), bindInfo};
}

std::tuple<UniformWriter, BindBufferInfo> DrawBufferManager::getSsboWriter(size_t requiredBytes) {
    if (!requiredBytes) {
        return {};
    }

    auto& info = fCurrentBuffers[kStorageBufferIndex];
    auto [ptr, bindInfo] = this->prepareBindBuffer(&info, requiredBytes);
    if (!ptr) {
        return {};
    }
    return {UniformWriter(ptr, requiredBytes), bindInfo};
}

void DrawBufferManager::transferToRecording(Recording* recording) {
    bool useTransferBuffer = !fResourceProvider->sharedContext()->caps()->drawBufferCanBeMapped();
    for (auto& [buffer, transferBuffer] : fUsedBuffers) {
        if (useTransferBuffer) {
            if (transferBuffer) {
                SkASSERT(buffer);
                transferBuffer->unmap();
                recording->priv().addTask(CopyBufferToBufferTask::Make(std::move(transferBuffer),
                                                                       std::move(buffer)));
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
        if (useTransferBuffer) {
            if (info.fTransferBuffer) {
                info.fTransferBuffer->unmap();
                SkASSERT(info.fBuffer);
                recording->priv().addTask(CopyBufferToBufferTask::Make(
                        std::move(info.fTransferBuffer), info.fBuffer));
            }
        } else {
            info.fBuffer->unmap();
            recording->priv().addResourceRef(std::move(info.fBuffer));
        }
        info.fOffset = 0;
    }
}

std::pair<void*, BindBufferInfo> DrawBufferManager::prepareBindBuffer(BufferInfo* info,
                                                                      size_t requiredBytes) {
    SkASSERT(info);
    SkASSERT(requiredBytes);

    bool useTransferBuffer = !fResourceProvider->sharedContext()->caps()->drawBufferCanBeMapped();

    if (info->fBuffer &&
        !can_fit(requiredBytes, info->fBuffer.get(), info->fOffset, info->fStartAlignment)) {
        SkASSERT(!info->fTransferBuffer || info->fBuffer->size() == info->fTransferBuffer->size());
        fUsedBuffers.emplace_back(std::move(info->fBuffer), std::move(info->fTransferBuffer));
    }

    if (!info->fBuffer) {
        size_t bufferSize = sufficient_block_size(requiredBytes, info->fBlockSize);
        info->fBuffer = fResourceProvider->findOrCreateBuffer(
                bufferSize,
                info->fType,
                useTransferBuffer ? PrioritizeGpuReads::kYes : PrioritizeGpuReads::kNo);
        info->fOffset = 0;
        if (!info->fBuffer) {
            return {nullptr, {}};
        }
    }

    if (useTransferBuffer && !info->fTransferBuffer) {
        info->fTransferBuffer = fResourceProvider->findOrCreateBuffer(
                info->fBuffer->size(),
                BufferType::kXferCpuToGpu,
                PrioritizeGpuReads::kNo);
        SkASSERT(info->fBuffer->size() == info->fTransferBuffer->size());
        SkASSERT(info->fOffset == 0);
        if (!info->fTransferBuffer) {
            return {nullptr, {}};
        }
    }

    info->fOffset = SkAlignTo(info->fOffset, info->fStartAlignment);
    BindBufferInfo bindInfo{info->fBuffer.get(), info->fOffset};
    info->fOffset += requiredBytes;

    void* ptr = SkTAddOffset<void>(info->getMappableBuffer()->map(),
                                   static_cast<ptrdiff_t>(bindInfo.fOffset));
    return {ptr, bindInfo};
}

// ------------------------------------------------------------------------------------------------
// StaticBufferManager

StaticBufferManager::StaticBufferManager(ResourceProvider* resourceProvider)
        : fResourceProvider(resourceProvider)
        , fCurrentTransferBuffer(nullptr)
        , fCurrentOffset(0) {}
StaticBufferManager::~StaticBufferManager() = default;

VertexWriter StaticBufferManager::getVertexWriter(size_t size, BindBufferInfo* binding) {
    void* data = this->prepareStaticData(BufferType::kVertex, kVertexStartAlignment, size, binding);
    return VertexWriter{data, size};
}

VertexWriter StaticBufferManager::getIndexWriter(size_t size, BindBufferInfo* binding) {
    void* data = this->prepareStaticData(BufferType::kIndex, kIndexStartAlignment, size, binding);
    return VertexWriter{data, size};
}

void* StaticBufferManager::prepareStaticData(BufferType type,
                                             size_t alignment,
                                             size_t size,
                                             BindBufferInfo* target) {
    // Zero-out the target binding in the event of any failure in actually transfering data later.
    SkASSERT(target);
    *target = {nullptr, 0};
    if (!size) {
        return nullptr;
    }

    // Since the static buffers always are written to cpu-to-gpu transfer buffers, we pack the
    // data tightly, and handle aligning based on buffer type as part of the copy task.
    if (fCurrentTransferBuffer &&
        !can_fit(size, fCurrentTransferBuffer.get(), fCurrentOffset, /*alignment=*/1)) {
        fCurrentTransferBuffer->unmap();
        fUsedBuffers.push_back(std::move(fCurrentTransferBuffer));
    }
    if (!fCurrentTransferBuffer) {
        size_t bufferSize = sufficient_block_size(size, kStaticTransferBufferSize);
        fCurrentTransferBuffer = fResourceProvider->findOrCreateBuffer(
                bufferSize,
                BufferType::kXferCpuToGpu,
                PrioritizeGpuReads::kNo);
        fCurrentOffset = 0;
    }

    fStaticBufferData.push_back({type,
                                 alignment,
                                 BindBufferInfo{fCurrentTransferBuffer.get(), fCurrentOffset},
                                 size,
                                 target});
    void* ptr = SkTAddOffset<void>(fCurrentTransferBuffer->map(),
                                  static_cast<ptrdiff_t>(fCurrentOffset));
    fCurrentOffset += size;
    return ptr;
}

bool StaticBufferManager::finalize(QueueManager* queueManager, GlobalCache* cache) {
    // Used buffers were already unmapped, but we're also done with the current transfer buffer
    if (fCurrentTransferBuffer) {
        fCurrentTransferBuffer->unmap();
    }

    // Collect the total size required for each buffer type.
    std::array<size_t, kBufferTypeCount> bufferSizes = {0, 0, 0, 0, 0, 0};
    for (BufferData& data : fStaticBufferData) {
        SkASSERT(data.fSize > 0);
        size_t currentOffset = SkAlignTo(bufferSizes[(int) data.fBufferType], data.fAlignment);
        // The target's buffer pointer will be filled in later once the final size is known, but
        // this will be the offset the data will be written to.
        data.fTargetInfo->fOffset = currentOffset;
        bufferSizes[(int) data.fBufferType] = currentOffset + data.fSize;
    }

    // Create one Buffer per type that will hold the compacted data
    std::array<sk_sp<Buffer>, kBufferTypeCount> staticBuffers;
    SkDEBUGCODE(size_t totalStaticDataSize = 0;)
    for (int type = 0; type < kBufferTypeCount; ++type) {
        SkDEBUGCODE(totalStaticDataSize += bufferSizes[type];)

        if (!bufferSizes[type]) {
            // No static buffer required for this type
            staticBuffers[type] = nullptr;
            continue;
        }

        staticBuffers[type] = fResourceProvider->findOrCreateBuffer(
                bufferSizes[type],
                static_cast<BufferType>(type),
                PrioritizeGpuReads::kYes);

        if (!staticBuffers[type]) {
            SKGPU_LOG_W("Failed to create static buffer for type %d of size %zu bytes.\n",
                        type, bufferSizes[type]);
            return false;
        }
        // The final static buffers must remain owned by the GlobalCache so that users of the
        // buffers (i.e. the holders of the target BindBufferInfos) do not need to worry about the
        // data being removed from the GPU until the SharedContext is destroyed.
        cache->addStaticResource(staticBuffers[type]);
    }
    SkASSERT(totalStaticDataSize <= kMaxStaticDataSize);

    // Issue copies to transfer the CPU-mapped data into the potentially GPU-private Buffer
    // and update the target BindBufferInfo with the final static buffer.
    for (const BufferData& data : fStaticBufferData) {
        data.fTargetInfo->fBuffer = staticBuffers[(int) data.fBufferType].get();
        SkASSERT(data.fTransferInfo.fBuffer && data.fTargetInfo->fBuffer);
        auto task = CopyBufferToBufferTask::Make(
                sk_ref_sp(data.fTransferInfo.fBuffer), data.fTransferInfo.fOffset,
                sk_ref_sp(data.fTargetInfo->fBuffer),  data.fTargetInfo->fOffset,
                data.fSize);
        if (!queueManager->addTask(task.get(), fResourceProvider)) {
            SKGPU_LOG_W("Failed to copy data to static buffer.\n");
            return false;
        }
    }

    // Reset the static buffer manager since the Recording's copy tasks now manage ownership of
    // the transfer buffers and the GlobalCache owns the final static buffers.
    fCurrentTransferBuffer = nullptr;
    fCurrentOffset = 0;
    fUsedBuffers.clear();
    fStaticBufferData.clear();

    return true;
}

} // namespace skgpu::graphite
