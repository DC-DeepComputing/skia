/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanRenderPass_DEFINED
#define skgpu_graphite_VulkanRenderPass_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * Wrapper around VkRenderPass.
*/
class VulkanRenderPass : public Resource {
public:
    static GraphiteResourceKey GenerateCompatibleRenderPassKey(const RenderPassDesc&);
    static GraphiteResourceKey GenerateRenderPassKey(const RenderPassDesc&,
                                                     const Texture* colorTexture,
                                                     const Texture* resolveTexture,
                                                     const Texture* depthStencilTexture);

    static sk_sp<VulkanRenderPass> MakeCompatibleRenderPass(const VulkanSharedContext*,
                                                            const RenderPassDesc&);
    static sk_sp<VulkanRenderPass> MakeRenderPass(const VulkanSharedContext*,
                                                  const RenderPassDesc&,
                                                  VulkanCommandBuffer*,
                                                  const Texture* colorTexture,
                                                  const Texture* resolveTexture,
                                                  const Texture* depthStencilTexture);

    VkRenderPass* renderPass() {
        SkASSERT(fRenderPass != VK_NULL_HANDLE);
        return &fRenderPass;
    }

private:
    void freeGpuData() override;

    VulkanRenderPass(const VulkanSharedContext*, VkRenderPass);

    const VulkanSharedContext* fSharedContext;
    VkRenderPass fRenderPass;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanRenderPass_DEFINED
