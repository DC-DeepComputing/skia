/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanBuffer_DEFINED
#define skgpu_graphite_VulkanBuffer_DEFINED

#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

class VulkanBuffer final : public Buffer {
public:
    static sk_sp<Buffer> Make(const VulkanSharedContext*, size_t, BufferType, PrioritizeGpuReads);
    void freeGpuData() override {}

private:
    VulkanBuffer(const VulkanSharedContext*, size_t, BufferType, PrioritizeGpuReads, VkBuffer,
                 const skgpu::VulkanAlloc&);

    void onMap() override;
    void onUnmap() override;

    void internalMap(size_t readOffset, size_t readSize);
    void internalUnmap(size_t flushOffset, size_t flushSize);

    bool isMappable() const { return fAlloc.fFlags & skgpu::VulkanAlloc::kMappable_Flag; }

    const VulkanSharedContext* vulkanSharedContext() const {
        return static_cast<const VulkanSharedContext*>(this->sharedContext());
    }

    VkBuffer fBuffer;
    skgpu::VulkanAlloc fAlloc;
    bool fBufferIsReadOnly = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanBuffer_DEFINED

