/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanFrameBuffer_DEFINED
#define skgpu_graphite_VulkanFrameBuffer_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * Resource wrapper for VkFramebuffer
*/
class VulkanFrameBuffer : public Resource {
public:
    static sk_sp<VulkanFrameBuffer> Make(const VulkanSharedContext*,
                                         const VkFramebufferCreateInfo&);

    VkFramebuffer frameBuffer() {
        return fFrameBuffer;
    }

private:
    VulkanFrameBuffer(const VulkanSharedContext*, VkFramebuffer);
    void freeGpuData() override;

    const VulkanSharedContext* fSharedContext;
    VkFramebuffer fFrameBuffer;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanFrameBuffer_DEFINED
