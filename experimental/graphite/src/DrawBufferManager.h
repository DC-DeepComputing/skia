/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawBufferManager_DEFINED
#define skgpu_DrawBufferManager_DEFINED

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"

namespace skgpu {

class Buffer;
class ResourceProvider;

class DrawBufferManager {
public:
    DrawBufferManager(ResourceProvider*,
                      size_t uniformStartAlignment);
    ~DrawBufferManager();

    std::tuple<VertexWriter, BindBufferInfo> getVertexWriter(size_t requiredBytes);
    std::tuple<IndexWriter, BindBufferInfo> getIndexWriter(size_t requiredBytes);
    std::tuple<UniformWriter, BindBufferInfo> getUniformWriter(size_t requiredBytes);

    void passUsedBuffersToCommandBuffer(CommandBuffer*);

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
};

} // namespace skgpu

#endif // skgpu_DrawBufferManager_DEFINED
