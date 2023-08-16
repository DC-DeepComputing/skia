/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanRenderPass.h"

#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

GraphiteResourceKey VulkanRenderPass::GenerateCompatibleRenderPassKey(
        const RenderPassDesc& renderPassDesc) {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    // RenderPass key structure:
    // Color attachment compatibility, resolve attachment compat, stencil attach compat,
    // self dependency, count needed to encapsulate subpass information
    // TODO: Consider self dependency like in ganesh?
    int num32DataCnt = 0;

    auto& colorAttachmentTextureInfo        = renderPassDesc.fColorAttachment.fTextureInfo;
    auto& colorResolveAttachmentTextureInfo = renderPassDesc.fColorResolveAttachment.fTextureInfo;
    auto& depthStencilAttachmentTextureInfo = renderPassDesc.fDepthStencilAttachment.fTextureInfo;

    // Two textures are compatible if they have the same format and sample count. Both values are
    // stored as, and thus necessitate for the key, an int32.
    // TODO: Add a static assertion that format can only be up to VK_FORMAT_MAX_ENUM (int32).
    num32DataCnt = 2 * ((colorAttachmentTextureInfo.isValid()) +
                        (colorResolveAttachmentTextureInfo.isValid()) +
                        (depthStencilAttachmentTextureInfo.isValid()));

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kNo);

    // TODO: Consider how many uint32s will be necessary to store specific subpass information and
    // the fact that we will need to support >1 color attachment per render pass.

    // The following checks encapsulate all the information required to determine render pass
    // compatability (presence of, format of, and sample count of attachments).
    int currentBuilderIdx = 0;
    // If an attachment's texture info is valid, we should always be able to obtain
    // VulkanTextureInfo.
    if (colorAttachmentTextureInfo.isValid()) {
        VulkanTextureInfo vkTexInfo;
        if (colorAttachmentTextureInfo.getVulkanTextureInfo(&vkTexInfo)) {
            builder[currentBuilderIdx++] = vkTexInfo.fFormat;
            builder[currentBuilderIdx++] = vkTexInfo.fSampleCount;
        } else {
            SkUNREACHABLE;
        }
    }
    if (colorResolveAttachmentTextureInfo.isValid()) {
        VulkanTextureInfo vkTexInfo;
        if (colorResolveAttachmentTextureInfo.getVulkanTextureInfo(&vkTexInfo)) {
            builder[currentBuilderIdx++] = vkTexInfo.fFormat;
            builder[currentBuilderIdx++] = vkTexInfo.fSampleCount;
        } else {
            SkUNREACHABLE;
        }
    }
    if (depthStencilAttachmentTextureInfo.isValid()) {
        VulkanTextureInfo vkTexInfo;
        if (depthStencilAttachmentTextureInfo.getVulkanTextureInfo(&vkTexInfo)) {
            builder[currentBuilderIdx++] = vkTexInfo.fFormat;
            builder[currentBuilderIdx++] = vkTexInfo.fSampleCount;
        } else {
            SkUNREACHABLE;
        }
    }

    builder.finish();
    return key;
}

GraphiteResourceKey VulkanRenderPass::GenerateRenderPassKey(const RenderPassDesc& renderPassDesc,
                                                            const Texture* colorTexture,
                                                            const Texture* resolveTexture,
                                                            const Texture* depthStencilTexture) {
    // TODO: Implement. For now, simply generate a compatible renderpass key.
    return GenerateCompatibleRenderPassKey(renderPassDesc);
}

sk_sp<VulkanRenderPass> VulkanRenderPass::MakeCompatibleRenderPass(
        const VulkanSharedContext* context, const RenderPassDesc& renderPassDesc) {
    VkRenderPass renderPass;
    renderPass = VK_NULL_HANDLE;
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context, renderPass));
}

sk_sp<VulkanRenderPass> VulkanRenderPass::MakeRenderPass(const VulkanSharedContext* context,
                                                         const RenderPassDesc& renderPassDesc,
                                                         const Texture* colorTexture,
                                                         const Texture* resolveTexture,
                                                         const Texture* depthStencilTexture) {
    VkRenderPass renderPass;
    renderPass = VK_NULL_HANDLE;
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context, renderPass));
}

VulkanRenderPass::VulkanRenderPass(const VulkanSharedContext* context, VkRenderPass renderPass)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kYes, sizeof(VkRenderPass))
        , fSharedContext(context)
        , fRenderPass (renderPass) {
}

void VulkanRenderPass::freeGpuData() {
    // TODO: Make sure there are no external attachments (?)
    VULKAN_CALL(fSharedContext->interface(),
                DestroyRenderPass(fSharedContext->device(), fRenderPass, nullptr));
}

} // namespace skgpu::graphite
