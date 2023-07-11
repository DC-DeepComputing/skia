/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMeshGanesh_DEFINED
#define SkMeshGanesh_DEFINED

#include "include/core/SkMesh.h"

class GrDirectContext;

namespace SkMeshes {
/**
 * Makes a GPU-backed index buffer to be used with SkMeshes.
 *
 * @param  data              The data used to populate the buffer, or nullptr to create a zero-
 *                           initialized buffer.
 * @param  size              Both the size of the data in 'data' and the size of the resulting
 *                           buffer.
 */
SK_API sk_sp<SkMesh::IndexBuffer> MakeIndexBuffer(GrDirectContext*, const void* data, size_t size);

/**
 * Makes a copy of an index buffer. The implementation currently only supports a CPU-backed
 * source buffer.
 */
SK_API sk_sp<SkMesh::IndexBuffer> CopyIndexBuffer(GrDirectContext*, sk_sp<IndexBuffer>);

/**
 * Makes a CPU-backed vertex buffer to be used with SkMeshes.
 *
 * @param  data              The data used to populate the buffer, or nullptr to create a zero-
 *                           initialized buffer.
 * @param  size              Both the size of the data in 'data' and the size of the resulting
 *                           buffer.
 */
SK_API sk_sp<SkMesh::VertexBuffer> MakeVertexBuffer(const void*, size_t size);

/**
 * Makes a copy of a vertex buffer. The implementation currently only supports a CPU-backed
 * source buffer.
 */
SK_API sk_sp<SkMesh::VertexBuffer> CopyVertexBuffer(sk_sp<VertexBuffer>);
}

#endif