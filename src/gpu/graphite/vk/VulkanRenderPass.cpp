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

void add_attachment_description_info_to_key(
        GraphiteResourceKey::Builder& builder, const TextureInfo& textureInfo, int& builderIdx) {
    VulkanTextureInfo vkTexInfo;
    if (textureInfo.isValid() && textureInfo.getVulkanTextureInfo(&vkTexInfo)) {
        builder[builderIdx++] = vkTexInfo.fFormat;
        builder[builderIdx++] = vkTexInfo.fSampleCount;
    } else {
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused;
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused;
    }
}

GraphiteResourceKey VulkanRenderPass::GenerateCompatibleRenderPassKey(
        const RenderPassDesc& renderPassDesc) {
    auto& colorAttachment           = renderPassDesc.fColorAttachment;
    auto& colorResolveAttachment    = renderPassDesc.fColorResolveAttachment;
    auto& depthStencilAttachment    = renderPassDesc.fDepthStencilAttachment;
    bool  hasColorAttachment        = colorAttachment.fTextureInfo.isValid();
    bool  hasColorResolveAttachment = colorResolveAttachment.fTextureInfo.isValid();
    bool  hasDepthStencilAttachment = depthStencilAttachment.fTextureInfo.isValid();

    // Compatible RenderPass key structure must account for color, resolve, and stencil attachment
    // reference compatibility (which is determined by having matching formats and sample counts for
    // each attachment reference in each subpass). Calculate how many int32s will be required to
    // store all the necessary information.
    int num32DataCnt = 0;
    // Each renderpass key will contain the total number of attachments used for that render pass.
    // This will use one uint32.
    int numColorAttachments = hasColorAttachment;
    int numResolveAttachments = hasColorResolveAttachment;
    int numDepthStencilAttachments = hasDepthStencilAttachment;
    int totalRenderPassAttachments = numColorAttachments + numResolveAttachments +
                                     numDepthStencilAttachments;
    num32DataCnt++;
    // The key will then contain attachment compatibility information for every attachment (format
    // and sample count), which means we need two uint32s for each attachment on the renderpass.
    num32DataCnt += 2 * totalRenderPassAttachments;
    // Then, subpass compatibility information will be added (color
    // attachment count which will be alloted one uint32, then attachment reference indices which
    // will take one uint32 for each attachment reference a subpass has).
    int subpassCount = 1; // TODO explain/elaborate here
    // For each subpass, add one int32 to store a color attachment count. Add another int32 for
    // each attachment reference.
    for (int i = 0; i < subpassCount; i++) {
        num32DataCnt++;
        int subpassAttachmentRefCount = 0;
        for (int j = 0; j < subpassAttachmentRefCount; j++) {
            num32DataCnt++;
        }
    }
    // Finally, subpass dependency information will be recorded. Each dependency is currently
    // allotted 6 int32s to store all its relevant information. TODO explain how we can cut down ltr
    int subpassDependencyCount = 0;
    for (int k = 0; k < subpassDependencyCount; k++) {
        num32DataCnt += 6;
    }

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kNo);

    // Now we can actually populate the key.
    int builderIdx = 0;
    builder[builderIdx++] = totalRenderPassAttachments;
    // Iterate through each renderpass attachment to add its compatibility information
    for (int j = 0; j < totalRenderPassAttachments; j++) {
        // For now, we will only have up to one color attachment, resolve attachment, and
        // depth/stencil attachment. TODO: Confirm this is correct.
        add_attachment_description_info_to_key(
                builder, colorAttachment.fTextureInfo, builderIdx);
        add_attachment_description_info_to_key(
                builder, colorResolveAttachment.fTextureInfo, builderIdx);
        add_attachment_description_info_to_key(
                builder, depthStencilAttachment.fTextureInfo, builderIdx);
    }

    // Add subpass information to the key
    for (int j = 0; j < subpassCount; j++) {
        // TODO: Fetch actual color attachment count per subpass from RenderPassDesc. For now, we
        // can just determine whether we have 0 or 1 by checking if one color attachment we are
        // currently allowed is present or not.
        int colorAttachmentCount = hasColorResolveAttachment;
        builder[builderIdx++] = colorAttachmentCount;
        int attachmentIdx = 0;
        for (int k = 0; k < colorAttachmentCount; k++) {
            // For each color attachment referenced by a subpass, store the attachment index.
            // Eventually, we can fetch this information from RenderPassDesc. For now, the color
            // attachment index should always be 0 or unused.
            builder[builderIdx++] = hasColorResolveAttachment ? attachmentIdx++
                                                              : VK_ATTACHMENT_UNUSED;
        }
        // Store attachment indices for other subpass attachment references (input, depth/stencil,
        // color attachment resolve)
        // TODO: Consider number of resolve attachment references
        builder[builderIdx++] = hasColorResolveAttachment ? attachmentIdx++ : VK_ATTACHMENT_UNUSED;
        builder[builderIdx++] = hasDepthStencilAttachment ? attachmentIdx++ : VK_ATTACHMENT_UNUSED;
        // TODO: Check if we have an input attachment reference.
    }
    // Add dependency information to the key
    // TODO: Should we store the count of subpass dependencies, or just each dependency information?
    builder[builderIdx++] = subpassDependencyCount;
    for (int i = 0; i < subpassDependencyCount; i++) {
        // Query RenderPassDesc for subpass dependency information and populate the key accordingly
        // Placeholder for source subpass index
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused;
        // Placeholder for destination subpass index
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused;
        // Placeholder for destination subpass index
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused;
        // Placeholder for srcStageMask
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused;
        // Placeholder for dstStageMask
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused;
        // Placeholder for dependency flags
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused;
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
