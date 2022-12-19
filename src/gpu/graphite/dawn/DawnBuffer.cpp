/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnBuffer.h"

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
};
#endif

sk_sp<Buffer> DawnBuffer::Make(const DawnSharedContext* sharedContext,
                               size_t size,
                               BufferType type,
                               PrioritizeGpuReads prioritizeGpuReads) {
    if (size <= 0) {
        return nullptr;
    }

    const DawnCaps* dawnCaps = sharedContext->dawnCaps();

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
        usage = wgpu::BufferUsage::Storage;
        break;
    }

    wgpu::BufferDescriptor desc;
#ifdef SK_DEBUG
    desc.label = kBufferTypeNames[static_cast<int>(type)];
#endif
    desc.usage = usage;
    desc.size  = SkAlignTo(size, dawnCaps->getMinBufferAlignment());
    desc.mappedAtCreation = usage & (wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::MapRead);

    auto buffer = sharedContext->device().CreateBuffer(&desc);
    if (!buffer) {
        return {};
    }

    return sk_sp<Buffer>(new DawnBuffer(sharedContext,
                                        desc.size,
                                        std::move(buffer)));
}

DawnBuffer::DawnBuffer(const DawnSharedContext* sharedContext,
                       size_t size,
                       wgpu::Buffer buffer)
        : Buffer(sharedContext, size)
        , fBuffer(std::move(buffer))
        , fAsyncWait(dawnSharedContext()->device()) {
    // Mappable buffer is created with mappedAtCreation == true, so we consider the buffer is
    // already mapped successfully.
    if (isMappable()) {
        fAsyncWait.signal();
    }
}

void DawnBuffer::onMap() {
    SkASSERT(fBuffer);
    SkASSERT(!this->isMapped());
    SkASSERT(isMappable());

    fAsyncWait.busyWait();
    fMapPtr = fBuffer.GetMappedRange();
    SkASSERT(fMapPtr);
}

void DawnBuffer::onUnmap() {
    SkASSERT(fBuffer);
    SkASSERT(this->isMapped());
    SkASSERT(isMappable());

    fBuffer.Unmap();
    fMapPtr = nullptr;
}

void DawnBuffer::onReturnedToCache() {
    if (!isMappable()) {
        return;
    }

    // If the buffer is mappable, we issue MapAsync() while returning the buffer to the cache to
    // avoid the expensive fAsyncWait.busyWait() in onMap() call.
    bool isWrite = fBuffer.GetUsage() & wgpu::BufferUsage::MapWrite;
    fAsyncWait.reset();
    fBuffer.MapAsync(isWrite ? wgpu::MapMode::Write: wgpu::MapMode::Read,
                     0,
                     fBuffer.GetSize(),
                     [](WGPUBufferMapAsyncStatus s, void* userData) {
                        reinterpret_cast<DawnAsyncWait*>(userData)->signal();
                     },
                     &fAsyncWait);
}

void DawnBuffer::freeGpuData() {
    fBuffer = nullptr;
}

} // namespace skgpu::graphite
