/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/dawn/GrDawnBuffer.h"

#include "src/gpu/ganesh/dawn/GrDawnAsyncWait.h"
#include "src/gpu/ganesh/dawn/GrDawnGpu.h"

namespace {
    wgpu::BufferUsage GrGpuBufferTypeToDawnUsageBit(GrGpuBufferType type) {
        switch (type) {
            case GrGpuBufferType::kVertex:
                return wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
            case GrGpuBufferType::kIndex:
                return wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
            case GrGpuBufferType::kXferCpuToGpu:
                return wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
            case GrGpuBufferType::kXferGpuToCpu:
                return wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
            default:
                SkASSERT(!"buffer type not supported by Dawn");
                return wgpu::BufferUsage::Vertex;
        }
    }
}

GrDawnBuffer::GrDawnBuffer(GrDawnGpu* gpu, size_t sizeInBytes, GrGpuBufferType type,
                           GrAccessPattern pattern,
                           std::string_view label)
    : INHERITED(gpu, sizeInBytes, type, pattern, label) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeInBytes;
    bufferDesc.usage = GrGpuBufferTypeToDawnUsageBit(type);

    if (bufferDesc.usage & wgpu::BufferUsage::MapRead) {
        SkASSERT(!SkToBool(bufferDesc.usage & wgpu::BufferUsage::MapWrite));
        fMappable = Mappable::kReadOnly;
    } else if (bufferDesc.usage & wgpu::BufferUsage::MapWrite) {
        fMappable = Mappable::kWriteOnly;
    }

    if (fMappable == Mappable::kNot || fMappable == Mappable::kReadOnly) {
        // TODO(armansito): Map at creation if ReadOnly?
        fBuffer = this->getDawnGpu()->device().CreateBuffer(&bufferDesc);
    } else {
        bufferDesc.mappedAtCreation = true;
        fBuffer = this->getDawnGpu()->device().CreateBuffer(&bufferDesc);
        fMapPtr = fBuffer.GetMappedRange();
        if (!fMapPtr) {
            SkDebugf("GrDawnBuffer: failed to create mapped buffer\n");
            fUnmapped = true;
        }
    }

    this->registerWithCache(SkBudgeted::kYes);
}

GrDawnBuffer::~GrDawnBuffer() {
}

void GrDawnBuffer::onMap() {
    if (this->wasDestroyed()) {
        return;
    }

    if (fUnmapped) {
        SkDebugf("GrDawnBuffer: falling back to blocking map\n");
        if (!this->BlockingMap()) {
            SkDebugf("GrDawnBuffer: failed to map buffer\n");
            return;
        }
        fUnmapped = false;
    }

    if (fMappable == Mappable::kNot) {
        GrStagingBufferManager::Slice slice =
                this->getDawnGpu()->stagingBufferManager()->allocateStagingBufferSlice(
                        this->size());
        fStagingBuffer = static_cast<GrDawnBuffer*>(slice.fBuffer)->get();
        fStagingOffset = slice.fOffset;
        fMapPtr = slice.fOffsetMapPtr;
    } else {
        // We always create this buffers mapped or if they've been used on the gpu before we use the
        // async map callback to know when it is safe to reuse them. Thus by the time we get here
        // the buffer should always be mapped.
        SkASSERT(this->isMapped());
    }
}

void GrDawnBuffer::onUnmap() {
    if (this->wasDestroyed()) {
        return;
    }

    if (fMappable == Mappable::kNot) {
        this->getDawnGpu()->getCopyEncoder().CopyBufferToBuffer(fStagingBuffer, fStagingOffset,
                                                                fBuffer, 0, this->size());
    } else {
        fBuffer.Unmap();
        fUnmapped = true;
    }
}

bool GrDawnBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (this->wasDestroyed()) {
        return false;
    }

    this->map();
    if (!this->isMapped()) {
        return false;
    }

    memcpy(fMapPtr, src, srcSizeInBytes);
    this->unmap();
    return true;
}

GrDawnGpu* GrDawnBuffer::getDawnGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrDawnGpu*>(this->getGpu());
}

void GrDawnBuffer::mapAsync(MapAsyncCallback callback) {
    SkASSERT(fMappable != Mappable::kNot);
    SkASSERT(!fMapAsyncCallback);
    SkASSERT(!this->isMapped());

    fMapAsyncCallback = std::move(callback);
    fBuffer.MapAsync(
            (fMappable == Mappable::kReadOnly) ? wgpu::MapMode::Read : wgpu::MapMode::Write,
            0, 0,
            [](WGPUBufferMapAsyncStatus status, void* userData) {
                static_cast<GrDawnBuffer*>(userData)->onMapAsyncDone(status);
            },
            this);
}

void GrDawnBuffer::onMapAsyncDone(WGPUBufferMapAsyncStatus status) {
    SkASSERT(fMapAsyncCallback);

    if (fMappable == Mappable::kReadOnly) {
        fMapPtr = const_cast<void*>(fBuffer.GetConstMappedRange());
    } else {
        fMapPtr = fBuffer.GetMappedRange();
    }

    if (this->isMapped()) {
        fUnmapped = false;
    }

    // Run the callback as the last step in this function since the callback can deallocate this
    // GrDawnBuffer.
    auto callback = std::move(fMapAsyncCallback);
    callback(this->isMapped());
}

bool GrDawnBuffer::BlockingMap() {
    SkASSERT(fMappable != Mappable::kNot);

    bool readOnly = (fMappable == Mappable::kReadOnly);
    GrDawnAsyncWait wait(this->getDawnGpu()->device());
    fBuffer.MapAsync(
            readOnly ? wgpu::MapMode::Read : wgpu::MapMode::Write,
            0, 0,
            [](WGPUBufferMapAsyncStatus, void* userData) {
                static_cast<GrDawnAsyncWait*>(userData)->signal();
            },
            &wait);
    wait.busyWait();

    if (readOnly) {
        fMapPtr = const_cast<void*>(fBuffer.GetConstMappedRange());
    } else {
        fMapPtr = fBuffer.GetMappedRange();
    }

    return this->isMapped();
}
