/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnBuffer.h"

#include "include/private/base/SkAlign.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/dawn/DawnAsyncWait.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

#ifdef SK_DEBUG
static const char* kBufferTypeNames[kBufferTypeCount] = {
        "Vertex",
        "Index",
        "Xfer CPU to GPU",
        "Xfer GPU to CPU",
        "Uniform",
        "Storage",
        "Indirect",
        "VertexStorage",
        "IndexStorage",
};
#endif

sk_sp<Buffer> DawnBuffer::Make(const DawnSharedContext* sharedContext,
                               size_t size,
                               BufferType type,
                               AccessPattern,
                               PremapBuffer premap) {
    if (size <= 0) {
        return nullptr;
    }

    wgpu::BufferUsage usage = wgpu::BufferUsage::None;
    switch (type) {
    case BufferType::kVertex:
        usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        break;
    case BufferType::kIndex:
        usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        break;
    case BufferType::kXferCpuToGpu:
        usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
        break;
    case BufferType::kXferGpuToCpu:
        usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
        break;
    case BufferType::kUniform:
        usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        break;
    case BufferType::kStorage:
        usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst |
                wgpu::BufferUsage::CopySrc;
        break;
    case BufferType::kIndirect:
        usage = wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage;
        break;
    case BufferType::kVertexStorage:
        usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Storage;
        break;
    case BufferType::kIndexStorage:
        usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::Storage;
        break;
    }

    wgpu::BufferDescriptor desc;
#ifdef SK_DEBUG
    desc.label = kBufferTypeNames[static_cast<int>(type)];
#endif
    desc.usage = usage;
    desc.size  = size;
    desc.mappedAtCreation = static_cast<bool>(premap);

    auto buffer = sharedContext->device().CreateBuffer(&desc);
    if (!buffer) {
        return {};
    }

    void* premapPtr = nullptr;
    if (desc.mappedAtCreation) {
        SkASSERT(buffer.GetMapState() == wgpu::BufferMapState::Mapped);
        premapPtr = buffer.GetMappedRange();
        SkASSERT(premapPtr);
    }

    return sk_sp<Buffer>(new DawnBuffer(sharedContext, size, std::move(buffer), premapPtr));
}

DawnBuffer::DawnBuffer(const DawnSharedContext* sharedContext,
                       size_t size,
                       wgpu::Buffer buffer,
                       void* premapPtr)
        : Buffer(sharedContext, size)
        , fBuffer(std::move(buffer)) {
    fMapPtr = premapPtr;
}

void DawnBuffer::onAsyncMap() {
    SkASSERT(fBuffer);
    SkASSERT((fBuffer.GetUsage() & wgpu::BufferUsage::MapRead) ||
             (fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite));
    if (this->isMapped() || this->hasPendingAsyncMap()) {
        SkASSERT(fBuffer.GetMapState() == wgpu::BufferMapState::Pending ||
                 fBuffer.GetMapState() == wgpu::BufferMapState::Mapped);
        return;
    }
    bool isWrite = fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite;
    auto buffer = sk_ref_sp(this);
    fMapAsyncWait.emplace(dawnSharedContext()->device());

    fBuffer.MapAsync(
            isWrite ? wgpu::MapMode::Write : wgpu::MapMode::Read,
            0,
            fBuffer.GetSize(),
            [](WGPUBufferMapAsyncStatus s, void* userData) {
                sk_sp<DawnBuffer> buffer(static_cast<DawnBuffer*>(userData));
                if (s == WGPUBufferMapAsyncStatus_Success) {
                    if (buffer->fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite) {
                        buffer->fMapPtr = buffer->fBuffer.GetMappedRange();
                    } else {
                        buffer->fMapPtr = const_cast<void*>(buffer->fBuffer.GetConstMappedRange());
                    }
                } else {
                    SKGPU_LOG_E("Buffer async map failed with status %s.", [s] {
                        switch (s) {
                            case WGPUBufferMapAsyncStatus_ValidationError:
                                return "ValidationError";
                            case WGPUBufferMapAsyncStatus_Unknown:
                                return "Unknown";
                            case WGPUBufferMapAsyncStatus_DeviceLost:
                                return "DeviceLost";
                            case WGPUBufferMapAsyncStatus_DestroyedBeforeCallback:
                                return "DestroyedBeforeCallback";
                            case WGPUBufferMapAsyncStatus_UnmappedBeforeCallback:
                                return "UnmappedBeforeCallback";
                            case WGPUBufferMapAsyncStatus_MappingAlreadyPending:
                                return "MappingAlreadyPending";
                            case WGPUBufferMapAsyncStatus_OffsetOutOfRange:
                                return "OffsetOutOfRange";
                            case WGPUBufferMapAsyncStatus_SizeOutOfRange:
                                return "SizeOutOfRange";
                            default:
                                return "<Other>";
                        }
                    }());
                }
                SkASSERT(buffer->fMapAsyncWait.has_value());
                buffer->fMapAsyncWait->signal();
            },
            buffer.release());
}

void DawnBuffer::onMap() {
    SkASSERT(fBuffer);
    SkASSERT(!this->isMapped());

    this->onAsyncMap();

    SkASSERT(fMapAsyncWait.has_value());
    fMapAsyncWait->busyWait();

    SkASSERT(fMapPtr);
}

void DawnBuffer::onUnmap() {
    SkASSERT(fBuffer);
    SkASSERT(this->isMapped() || this->hasPendingAsyncMap());

    fBuffer.Unmap();
    fMapPtr = nullptr;
    fMapAsyncWait.reset();
}

bool DawnBuffer::hasPendingAsyncMap() const {
    bool result = fMapAsyncWait.has_value() && !fMapPtr;
    SkASSERT(result == (fBuffer.GetMapState() == wgpu::BufferMapState::Pending));
    return result;
}

void DawnBuffer::freeGpuData() {
    fBuffer = nullptr;
}

} // namespace skgpu::graphite
