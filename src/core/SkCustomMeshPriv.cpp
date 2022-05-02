/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCustomMeshPriv.h"

#ifdef SK_ENABLE_SKSL

#include "include/core/SkData.h"

#if SK_SUPPORT_GPU
#include "src/gpu/ganesh/GrClientBufferManager.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#endif

bool SkCustomMeshPriv::Validate(const SkCustomMesh& cm) { return cm.validate(); }

//////////////////////////////////////////////////////////////////////////////

SkCustomMeshPriv::CpuIndexBuffer::CpuIndexBuffer() = default;

SkCustomMeshPriv::CpuIndexBuffer::~CpuIndexBuffer() = default;

sk_sp<SkCustomMesh::IndexBuffer> SkCustomMeshPriv::CpuIndexBuffer::Make(sk_sp<const SkData> data) {
    auto result = new CpuIndexBuffer;
    result->fData = std::move(data);
    return sk_sp<IndexBuffer>(result);
}

sk_sp<const SkData> SkCustomMeshPriv::CpuIndexBuffer::asData() const { return fData; }

size_t SkCustomMeshPriv::CpuIndexBuffer::size() const { return fData->size(); }

//////////////////////////////////////////////////////////////////////////////

SkCustomMeshPriv::CpuVertexBuffer::CpuVertexBuffer() = default;

SkCustomMeshPriv::CpuVertexBuffer::~CpuVertexBuffer() = default;

sk_sp<SkCustomMesh::VertexBuffer> SkCustomMeshPriv::CpuVertexBuffer::Make(
        sk_sp<const SkData> data) {
    auto result = new CpuVertexBuffer;
    result->fData = std::move(data);
    return sk_sp<VertexBuffer>(result);
}

sk_sp<const SkData> SkCustomMeshPriv::CpuVertexBuffer::asData() const { return fData; }

size_t SkCustomMeshPriv::CpuVertexBuffer::size() const { return fData->size(); }
#endif

#if SK_SUPPORT_GPU
//////////////////////////////////////////////////////////////////////////////

SkCustomMeshPriv::GpuIndexBuffer::GpuIndexBuffer() = default;

SkCustomMeshPriv::GpuIndexBuffer::~GpuIndexBuffer()  {
    GrClientBufferManager::BufferFinishedMessage message(std::move(fBuffer), fContextID);
}

sk_sp<SkCustomMesh::IndexBuffer> SkCustomMeshPriv::GpuIndexBuffer::Make(
        sk_sp<GrGpuBuffer> buffer,
        GrDirectContext::DirectContextID contextID) {
    if (!buffer) {
        return nullptr;
    }
    if (buffer->intendedType() != GrGpuBufferType::kIndex) {
        return nullptr;
    }
    auto result = new GpuIndexBuffer;
    result->fBuffer    = std::move(buffer);
    result->fContextID = contextID;
    return sk_sp<IndexBuffer>(result);
}

sk_sp<const GrGpuBuffer> SkCustomMeshPriv::GpuIndexBuffer::asGpuBuffer() const { return fBuffer; }

size_t SkCustomMeshPriv::GpuIndexBuffer::size() const { return fBuffer->size(); }

//////////////////////////////////////////////////////////////////////////////

SkCustomMeshPriv::GpuVertexBuffer::GpuVertexBuffer() = default;

SkCustomMeshPriv::GpuVertexBuffer::~GpuVertexBuffer() {
    GrClientBufferManager::BufferFinishedMessage message(std::move(fBuffer), fContextID);
}

sk_sp<SkCustomMesh::VertexBuffer> SkCustomMeshPriv::GpuVertexBuffer::Make(
        sk_sp<GrGpuBuffer> buffer,
        GrDirectContext::DirectContextID contextID) {
    if (!buffer) {
        return nullptr;
    }
    if (buffer->intendedType() != GrGpuBufferType::kVertex) {
        return nullptr;
    }
    auto result = new GpuVertexBuffer;
    result->fBuffer    = std::move(buffer);
    result->fContextID = contextID;
    return sk_sp<VertexBuffer>(result);
}

sk_sp<const GrGpuBuffer> SkCustomMeshPriv::GpuVertexBuffer::asGpuBuffer() const { return fBuffer; }

size_t SkCustomMeshPriv::GpuVertexBuffer::size() const { return fBuffer->size(); }

#endif
