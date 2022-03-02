/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawBufferManager_DEFINED
#define skgpu_DrawBufferManager_DEFINED

#include "experimental/graphite/src/DrawTypes.h"
#include "experimental/graphite/src/ResourceTypes.h"
#include "include/core/SkRefCnt.h"
#include "src/gpu/BufferWriter.h"

#include <unordered_map>
#include <vector>

namespace skgpu {

class Buffer;
class CommandBuffer;
class ResourceProvider;

class DrawBufferManager {
public:
    DrawBufferManager(ResourceProvider*, size_t uniformStartAlignment);
    ~DrawBufferManager();

    std::tuple<VertexWriter, BindBufferInfo> getVertexWriter(size_t requiredBytes);
    std::tuple<IndexWriter, BindBufferInfo> getIndexWriter(size_t requiredBytes);
    std::tuple<UniformWriter, BindBufferInfo> getUniformWriter(size_t requiredBytes);

    // Finalizes all buffers and transfers ownership of them to the CommandBuffer.
    void transferToCommandBuffer(CommandBuffer*);

    using BufferSizeFn = size_t(*)();
    using InitializeBufferFn = void(*)(skgpu::VertexWriter, size_t bufferSize);
    BindBufferInfo getStaticBuffer(BufferType type, InitializeBufferFn, BufferSizeFn);

private:
    ResourceProvider* fResourceProvider;

    sk_sp<Buffer> fCurrentVertexBuffer;
    size_t fVertexOffset = 0;

    sk_sp<Buffer> fCurrentIndexBuffer;
    size_t fIndexOffset = 0;

    sk_sp<Buffer> fCurrentUniformBuffer;
    size_t fUniformOffset = 0;

    size_t fUniformStartAlignment;

    std::vector<sk_sp<Buffer>> fUsedBuffers;
    // TODO: How does this play with transferToCommandBuffer?
    std::unordered_map<uintptr_t, sk_sp<Buffer>> fStaticBuffers;
};

} // namespace skgpu

#endif // skgpu_DrawBufferManager_DEFINED
