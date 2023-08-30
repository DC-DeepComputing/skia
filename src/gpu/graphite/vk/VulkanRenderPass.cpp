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
    // Compatible RenderPass key structure must account for color, resolve, and stencil attachment
    // compatibility (which is determined by having matching formats and sample counts).
    // TODO: Consider self-dependency needed to support dst reads.
    auto& colorAttachmentTextureInfo        = renderPassDesc.fColorAttachment.fTextureInfo;
    auto& colorResolveAttachmentTextureInfo = renderPassDesc.fColorResolveAttachment.fTextureInfo;
    auto& depthStencilAttachmentTextureInfo = renderPassDesc.fDepthStencilAttachment.fTextureInfo;

    int num32DataCnt = 0;
    // Two texture attachments are compatible if they have the same format and sample count. Both
    // values are stored as, and thus necessitate for the key, an int32. So we require 2 * the
    // number of texture attachments.
    num32DataCnt = 2 * ((colorAttachmentTextureInfo.isValid()) +
                        (colorResolveAttachmentTextureInfo.isValid()) +
                        (depthStencilAttachmentTextureInfo.isValid()));

    // TODO: Once we support >1 subpass, we will have to account for appending subpass key
    // information. Eventually, we plan to have the RenderPassDesc house subpass information and >1
    // color attachment. However, for now, we only ever use >1 subpass in the case of loading MSAA
    // from resolve.
    bool loadMSAAFromResolve = renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid() &&
                               renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
    int numSubpasses = loadMSAAFromResolve ? 1 : 2;

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kNo);

    // The following checks encapsulate all the information required to determine render pass
    // compatability (presence of, format of, and sample count of attachments).
    int currentBuilderIdx = 0;
    if (colorAttachmentTextureInfo.isValid()) {
        VulkanTextureInfo vkTexInfo;
        if (colorAttachmentTextureInfo.getVulkanTextureInfo(&vkTexInfo)) {
            builder[currentBuilderIdx++] = vkTexInfo.fFormat;
            builder[currentBuilderIdx++] = vkTexInfo.fSampleCount;
        } else {
            // If an attachment's texture info is valid, we should always be able to obtain
            // VulkanTextureInfo. We also should always have a color resolve attachment anyhow.
            SkUNREACHABLE;
        }
    }
    // If there is only one subpass, then resolve and depth/stencil attachment reference
    // compatibility requirements are ignored.
    if (numSubpasses != 1) {
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
    }
    builder.finish();
    return key;
}

GraphiteResourceKey VulkanRenderPass::GenerateRenderPassKey(const RenderPassDesc& renderPassDesc,
                                                            const Texture* colorTexture,
                                                            const Texture* resolveTexture,
                                                            const Texture* depthStencilTexture) {
    // TODO: Implement. For now, simply generate a compatible renderpass key. On top of having a
    // render pass be "compatible", fully fleshed-out renderpasses must consider load/store ops.
    return GenerateCompatibleRenderPassKey(renderPassDesc);
}

sk_sp<VulkanRenderPass> VulkanRenderPass::MakeCompatibleRenderPass(
        const VulkanSharedContext* context, const RenderPassDesc& renderPassDesc) {
    // TODO: Create VkRenderPass.
    VkRenderPass renderPass = VK_NULL_HANDLE;
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context, renderPass));
}

sk_sp<VulkanRenderPass> VulkanRenderPass::MakeRenderPass(const VulkanSharedContext* context,
                                                         const RenderPassDesc& renderPassDesc,
                                                         VulkanCommandBuffer* cmdBuf,
                                                         const Texture* colorTexture,
                                                         const Texture* resolveTexture,
                                                         const Texture* depthStencilTexture) {
    // TODO: Create VkRenderPass.
    VkRenderPass renderPass = VK_NULL_HANDLE;
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context, renderPass));
}

VulkanRenderPass::VulkanRenderPass(const VulkanSharedContext* context, VkRenderPass renderPass)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kYes, sizeof(VkRenderPass))
        , fSharedContext(context)
        , fRenderPass (renderPass) {
}

void VulkanRenderPass::freeGpuData() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyRenderPass(fSharedContext->device(), fRenderPass, nullptr));
}

} // namespace skgpu::graphite
