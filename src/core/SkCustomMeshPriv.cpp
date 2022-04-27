/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCustomMeshPriv.h"

#ifdef SK_ENABLE_SKSL

#include "include/core/SkData.h"

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
