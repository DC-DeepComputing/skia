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
    // We only count attachments that are valid textures when calculating the total number of
    // render pass attachments, so if a texture is invalid, simply skip it rather than using
    // VK_ATTACHMENT_UNUSED and incrementing the builderIdx. Attachments can be differentiated from
    // one another by their sample count and format (i.e. depth/stencil attachments will have a
    // depth/stencil format)
    VulkanTextureInfo vkTexInfo;
    if (textureInfo.isValid() && textureInfo.getVulkanTextureInfo(&vkTexInfo)) {
        builder[builderIdx++] = vkTexInfo.fFormat;
        builder[builderIdx++] = vkTexInfo.fSampleCount;
    }
}

GraphiteResourceKey VulkanRenderPass::GenerateCompatibleRenderPassKey(
        const RenderPassDesc& renderPassDesc) {
    bool  hasColorAttachment        = renderPassDesc.fColorAttachment.fTextureInfo.isValid();
    bool  hasColorResolveAttachment = renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid();
    bool  hasDepthStencilAttachment = renderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid();

    // TODO: Query for more attachments once the RenderPassDesc struct contains that information.
    // For now, we only ever expect to see 0 or 1 of each attachment type (color, resolve, and
    // depth/stencil), so the count of each of those can simply be represented by a bool.
    const int numColorAttachments = hasColorAttachment;
    const int numResolveAttachments = hasColorResolveAttachment;
    const int numDepthStencilAttachments = hasDepthStencilAttachment;
    const int totalRenderPassAttachments = numColorAttachments + numResolveAttachments +
                                           numDepthStencilAttachments;

    // Accumulate attachments into a container to mimic future structure in RenderPassDesc
    skia_private::TArray<const AttachmentDesc*> rpAttachments(totalRenderPassAttachments);
    rpAttachments.push_back(&renderPassDesc.fColorAttachment);
    rpAttachments.push_back(&renderPassDesc.fColorResolveAttachment);
    rpAttachments.push_back(&renderPassDesc.fDepthStencilAttachment);

    // Calculate how many int32s we need to capture all relevant information.
    int num32DataCnt = 0;
    // Each renderpass key will start with the total number of attachments associated with it.
    // This will use one uint32.
    num32DataCnt++;
    // The key will then contain attachment compatibility information for every attachment (format
    // and sample count), which means we need two uint32s for each attachment on the renderpass.
    num32DataCnt += 2 * totalRenderPassAttachments;
    // Then, subpass compatibility information will be added (color attachment count, which will be
    // alloted one uint32; then attachment reference indices which will take one uint32 for each
    // attachment reference a subpass has).
    // TODO: Reference RenderPassDesc to determine number and makeup of subpasses. For now,
    // we only ever expect 1 (in most cases) or 2 (when loading MSAA from resolve).
    int subpassCount = 1;
    // For each subpass, add one int32 to store a color attachment count. Add another int32 for
    // each attachment reference.
    for (int i = 0; i < subpassCount; i++) {
        num32DataCnt++;
        // TODO: Once we support loading from MSAA, we can determine attachment reference counts
        // that way. Long-term, RenderPassDesc will house that information. For now, we only support
        // 1 subpass, so all present & valid attachments will be referenced once in the 1 subpass.
        int subpassAttachmentRefCount = totalRenderPassAttachments;
        for (int j = 0; j < subpassAttachmentRefCount; j++) {
            num32DataCnt++;
        }
    }
    // Finally, subpass dependency information will be recorded. Each dependency is currently
    // allotted 7 int32s to store all its relevant information.
    // TODO: Reserved for future use. This will become relevant once we support multiple subpasses.
    int subpassDependencyCount = 0;
    num32DataCnt += 7 * subpassDependencyCount;

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kNo);

    // Now we can actually populate the key.
    int builderIdx = 0;
    builder[builderIdx++] = totalRenderPassAttachments;
    // Iterate through each renderpass attachment to add its compatibility information
    for (int i = 0; i < totalRenderPassAttachments; i++) {
        add_attachment_description_info_to_key(builder, rpAttachments[i]->fTextureInfo, builderIdx);
    }
    // Add subpass information to the key
    for (int j = 0; j < subpassCount; j++) {
        // TODO: Fetch actual color, resolve, and input attachment reference counts and attachment
        // indices for each subpass from RenderPassDesc. For now, we only support 1 subpass, so we
        // can infer that the attachment reference counts are equal to the attachment type counts of
        // the renderpass itself.
        int subpassColorAttachRefCount = numColorAttachments;
        int subpassResolveAttachRefCount = numResolveAttachments;
        // We only ever have up to one depth/stencil attachment per subpass. Until we support >1
        // subpass, this will simply be equivalent to whether the renderpass has a d/s attachment.
        bool subpassHasDepthStencilAttachRef = hasDepthStencilAttachment;

        // The following assumes attachment indices are indexed in order of color, resolve,
        // depth/stencil, then input attachment(s).
        int attachmentIdx = 0;
        builder[builderIdx++] = subpassColorAttachRefCount;
        for (int k = 0; k < subpassColorAttachRefCount; k++) {
            builder[builderIdx++] = attachmentIdx++;
        }
        for (int k = 0; k < subpassResolveAttachRefCount; k++) {
            builder[builderIdx++] = attachmentIdx++;
        }
        if (subpassHasDepthStencilAttachRef) {
             builder[builderIdx++] = attachmentIdx++;
        }
        // TODO: Check for input attachment references once we support input attachments.
        int subpassInputAttachRefCount = 0;
        for (int k = 0; k < subpassInputAttachRefCount; k++) {
            builder[builderIdx++] = attachmentIdx++;
        }
    }
    // TODO: Query RenderPassDesc for subpass dependency information & populate the key accordingly
    for (int i = 0; i < subpassDependencyCount; i++) {
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused; // srcSubpass index
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused; // dstSubpass index
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused; // srcStageMask
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused; // dstStageMask
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused; // srcAccessMask
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused; // dstAccessMask
        builder[builderIdx++] = VulkanRenderPass::kKeyValueUnused; // dependencyFlags
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
