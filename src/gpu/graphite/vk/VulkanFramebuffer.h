/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanFramebuffer_DEFINED
#define skgpu_graphite_VulkanFramebuffer_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

namespace skgpu::graphite {

class VulkanSharedContext;
class VulkanTexture;

/**
 * Resource wrapper for VkFramebuffer
*/
class VulkanFramebuffer : public Resource {
public:
    static sk_sp<VulkanFramebuffer> Make(const VulkanSharedContext*,
                                         const VkFramebufferCreateInfo&,
                                         sk_sp<VulkanTexture> msaaTexture,
                                         sk_sp<VulkanTexture> depthStencilTexture);

    VkFramebuffer framebuffer() {
        return fFramebuffer;
    }

    // We only check compatibility with the msaa and depthStencil textures. We assume the caller
    // has already made sure that the single sample color or resolve attachments match the
    // framebuffer. We currently also assume that we will always use the same attachment view of
    // the textures each time.
    bool compatible(const VulkanTexture* msaaTexture, const VulkanTexture* depthStencilTexture);

    const char* getResourceType() const override { return "Vulkan Framebuffer"; }

private:
    VulkanFramebuffer(const VulkanSharedContext*,
                      VkFramebuffer,
                      sk_sp<VulkanTexture> msaaTexture,
                      sk_sp<VulkanTexture> depthStencilTexture);
    void freeGpuData() override;

    const VulkanSharedContext* fSharedContext;
    VkFramebuffer fFramebuffer;

    sk_sp<VulkanTexture> fMsaaTexture;
    sk_sp<VulkanTexture> fDepthStencilTexture;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanFramebuffer_DEFINED
