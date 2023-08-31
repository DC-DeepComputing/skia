/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanRenderPass.h"

#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

namespace skgpu::graphite {

void add_attachment_compatibility_info_to_key(
        GraphiteResourceKey::Builder& builder, const TextureInfo& textureInfo, int& builderIdx) {
    VulkanTextureInfo vkTexInfo;
    if (textureInfo.getVulkanTextureInfo(&vkTexInfo)) {
        builder[builderIdx++] = vkTexInfo.fFormat;
        builder[builderIdx++] = vkTexInfo.fSampleCount;
    } else {
        // This helper only gets called after checking that TextureInfo::isValid() is true, so we
        // should always be able to extract VulkanTextureInfo.
        SkUNREACHABLE;
    }
}

void add_attachment_info_to_key(GraphiteResourceKey::Builder& builder,
                                const TextureInfo& textureInfo,
                                int& builderIdx,
                                LoadOp loadOp,
                                StoreOp storeOp) {
    add_attachment_compatibility_info_to_key(builder, textureInfo, builderIdx);
    builder[builderIdx++] = (vkLoadOp[static_cast<int>(loadOp)] << 8) |
                            (vkStoreOp[static_cast<int>(storeOp)]);
}

GraphiteResourceKey VulkanRenderPass::GenerateCompatibleRenderPassKey(
        const RenderPassDesc& renderPassDesc) {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    // Compatible RenderPass key structure must account for color, resolve, and stencil attachment
    // reference compatibility (which is determined by having matching formats and sample counts for
    // each attachment reference in each subpass).
    // TODO: Store whether self-dependency is needed to support dst reads in this key.
    auto& colorAttachment          = renderPassDesc.fColorAttachment;
    auto& colorResolveAttachment   = renderPassDesc.fColorResolveAttachment;
    auto& depthStencilAttachment   = renderPassDesc.fDepthStencilAttachment;

    bool hasColorAttachment        = colorAttachment.fTextureInfo.isValid();
    bool hasColorResolveAttachment = colorResolveAttachment.fTextureInfo.isValid();
    bool hasDepthStencilAttachment = depthStencilAttachment.fTextureInfo.isValid();

    // TODO: Once RenderPassDesc includes subpass breakdown information and supports >1 color
    // attachment, we should iterate through subpasses and store relevant attachment reference
    // compatibility information in this key. However, for now, we only ever use 1 color attachment
    // per render pass and only use >1 subpass in the special case of loading MSAA from resolve. So,
    // we can get away with simply storing attachment information for the entire render pass and
    // whether we have to load MSAA.
    bool loadMSAAFromResolve =
            hasColorResolveAttachment && colorResolveAttachment.fLoadOp == LoadOp::kLoad;
    int numSubpasses = loadMSAAFromResolve ? 2 : 1;

    int num32DataCnt = 0;
    // Two texture attachments are compatible if they have the same format and sample count. Both
    // values are stored as, and thus necessitate for the key, an int32. So we require 2 * the
    // number of texture attachments int32s. As a special case, if we only have one subpass, then
    // resolve and depth/stencil attachment reference compatibility requirements are ignored.
    num32DataCnt = 2 * hasColorAttachment;
    if (numSubpasses > 1) {
        num32DataCnt += 2 * (hasColorResolveAttachment + hasDepthStencilAttachment);
        // Add one more int32 to the key to store whether or not we need to load from MSAA.
        num32DataCnt++;
    }

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kNo);

    // The following checks encapsulate all the information required to determine render pass
    // compatability (presence of, format of, and sample count of attachments).
    int currentBuilderIdx = 0;
    if (hasColorAttachment) {
        add_attachment_compatibility_info_to_key(
                builder, colorAttachment.fTextureInfo, currentBuilderIdx);
    }
    // If there is only one subpass, then resolve and depth/stencil attachment reference
    // compatibility requirements are ignored.
    if (numSubpasses > 1) {
        if (hasColorResolveAttachment) {
            add_attachment_compatibility_info_to_key(
                    builder, colorResolveAttachment.fTextureInfo, currentBuilderIdx);
        }
        if (hasDepthStencilAttachment) {
            add_attachment_compatibility_info_to_key(
                    builder, depthStencilAttachment.fTextureInfo, currentBuilderIdx);
        }
        builder[currentBuilderIdx++] = loadMSAAFromResolve;
    }
    builder.finish();
    return key;
}

GraphiteResourceKey VulkanRenderPass::GenerateRenderPassKey(const RenderPassDesc& renderPassDesc,
                                                            const Texture* colorTexture,
                                                            const Texture* resolveTexture,
                                                            const Texture* depthStencilTexture) {
    // Fully fleshed-out renderpasses, as opposed to simply "compatible" renderpasses, must factor
    // in the load/store ops of attachment descriptions. This key structure will change to mirror
    // that of compatible renderpass keys once RenderPassDesc includes subpass information and >1
    // color attachments (see the comment in that method).
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    int num32DataCnt = 0;

    auto& colorAttachment          = renderPassDesc.fColorAttachment;
    auto& colorResolveAttachment   = renderPassDesc.fColorResolveAttachment;
    auto& depthStencilAttachment   = renderPassDesc.fDepthStencilAttachment;

    bool hasColorAttachment        = colorAttachment.fTextureInfo.isValid();
    bool hasColorResolveAttachment = colorResolveAttachment.fTextureInfo.isValid();
    bool hasDepthStencilAttachment = depthStencilAttachment.fTextureInfo.isValid();
    bool loadMSAAFromResolve       = hasColorResolveAttachment &&
                                     colorResolveAttachment.fLoadOp == LoadOp::kLoad;

    // For render passes, we must account for the format, sample count, and load/store values of an
    // attachment. Format and sample count are stored as, and thus necessitate for the key, an
    // int32. Each attachment's load/store ops can fit into one uint32. So we require 3 * the
    // number of texture attachments.
    num32DataCnt = 3 * (hasColorAttachment + hasColorResolveAttachment + hasDepthStencilAttachment);
    // Add one more int32 to the key to store whether or not we need to load from MSAA.
    num32DataCnt++;

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kNo);

    int currentBuilderIdx = 0;
    if (hasColorAttachment) {
        add_attachment_info_to_key(builder,
                                   colorAttachment.fTextureInfo,
                                   currentBuilderIdx,
                                   colorAttachment.fLoadOp,
                                   colorAttachment.fStoreOp);
    }
    if (hasColorResolveAttachment) {
        add_attachment_info_to_key(builder,
                                   colorResolveAttachment.fTextureInfo,
                                   currentBuilderIdx,
                                   renderPassDesc.fColorResolveAttachment.fLoadOp,
                                   renderPassDesc.fColorResolveAttachment.fStoreOp);
    }
    if (hasDepthStencilAttachment) {
        add_attachment_info_to_key(builder,
                                   depthStencilAttachment.fTextureInfo,
                                   currentBuilderIdx,
                                   renderPassDesc.fDepthStencilAttachment.fLoadOp,
                                   renderPassDesc.fDepthStencilAttachment.fStoreOp);
    }
    builder[currentBuilderIdx++] = loadMSAAFromResolve;
    builder.finish();
    return key;
}

sk_sp<VulkanRenderPass> VulkanRenderPass::MakeCompatibleRenderPass(
        const VulkanSharedContext* context, const RenderPassDesc& renderPassDesc) {
    // If we simply require a compatible render pass, then the following attributes do not need
    // to be considered:
    // image layout of attachment descriptions
    // load/store operations
    // image layout of attachment references
    // Create and track attachment references for the subpass.
    VkAttachmentReference colorRef;
    VkAttachmentReference resolveRef;
    VkAttachmentReference stencilRef;

    auto& colorAttachmentTextureInfo        = renderPassDesc.fColorAttachment.fTextureInfo;
    auto& colorResolveAttachmentTextureInfo = renderPassDesc.fColorResolveAttachment.fTextureInfo;
    auto& depthStencilAttachmentTextureInfo = renderPassDesc.fDepthStencilAttachment.fTextureInfo;

    int numAttachments = (colorAttachmentTextureInfo.isValid()) +
                         (colorResolveAttachmentTextureInfo.isValid()) +
                         (depthStencilAttachmentTextureInfo.isValid());

    skia_private::TArray<VkAttachmentDescription> attachmentDescs(numAttachments);

    if (colorAttachmentTextureInfo.isValid()) {
        VulkanTextureInfo vkColorAttachTexInfo;
        colorAttachmentTextureInfo.getVulkanTextureInfo(&vkColorAttachTexInfo);

        colorRef.attachment = attachmentDescs.size();
        VkAttachmentDescription& colorAttachDesc = attachmentDescs.push_back();
        memset(&colorAttachDesc, 0, sizeof(VkAttachmentDescription));
        colorAttachDesc.format = vkColorAttachTexInfo.fFormat;
        VkSampleCountFlagBits sampleCount;
        SkAssertResult(
                skgpu::SampleCountToVkSampleCount(vkColorAttachTexInfo.fSampleCount, &sampleCount));
        colorAttachDesc.samples = sampleCount;

        colorAttachDesc.flags = 0;
        colorAttachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (colorResolveAttachmentTextureInfo.isValid()) {
            VulkanTextureInfo vkResolveAttachTexInfo;
            colorResolveAttachmentTextureInfo.getVulkanTextureInfo(&vkResolveAttachTexInfo);

            resolveRef.attachment = attachmentDescs.size();
            VkAttachmentDescription& resolveAttachDesc = attachmentDescs.push_back();
            memset(&resolveAttachDesc, 0, sizeof(VkAttachmentDescription));
            resolveAttachDesc.format = vkResolveAttachTexInfo.fFormat;
            VkSampleCountFlagBits resolveSampleCount;
            SkAssertResult(skgpu::SampleCountToVkSampleCount(vkColorAttachTexInfo.fSampleCount,
                                                             &resolveSampleCount));
            resolveAttachDesc.samples = resolveSampleCount;
            resolveAttachDesc.flags = 0;
            resolveAttachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            resolveAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            resolveAttachDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            resolveAttachDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else {
            resolveRef.attachment = VK_ATTACHMENT_UNUSED;
            resolveRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    } else {
        SkASSERT(false);
        colorRef.attachment = VK_ATTACHMENT_UNUSED;
        colorRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if (depthStencilAttachmentTextureInfo.isValid()) {
        VulkanTextureInfo vkDepthStencilTexInfo;
        depthStencilAttachmentTextureInfo.getVulkanTextureInfo(&vkDepthStencilTexInfo);
        stencilRef.attachment = attachmentDescs.size();
        VkAttachmentDescription& depthStencilAttachDesc = attachmentDescs.push_back();

        // For compatible render passes, we only care about format and sample count attributes
        // matching.
        depthStencilAttachDesc.format = vkDepthStencilTexInfo.fFormat;
        VkSampleCountFlagBits depthStencilSampleCount;
        SkAssertResult(skgpu::SampleCountToVkSampleCount(vkDepthStencilTexInfo.fSampleCount,
                                                         &depthStencilSampleCount));
        depthStencilAttachDesc.samples = depthStencilSampleCount;

        depthStencilAttachDesc.flags = 0;
        depthStencilAttachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilAttachDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilAttachDesc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachDesc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        stencilRef.layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    } else {
        stencilRef.attachment = VK_ATTACHMENT_UNUSED;
        stencilRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    VkSubpassDescription subpassDesc;
    memset(&subpassDesc, 0, sizeof(VkSubpassDescription));
    subpassDesc.flags = 0;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // TODO: Add support for input attachments
    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = nullptr;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorRef;
    subpassDesc.pResolveAttachments = &resolveRef;
    subpassDesc.pDepthStencilAttachment = &stencilRef;
    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassInfo;
    memset(&renderPassInfo, 0, sizeof(VkRenderPassCreateInfo));
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0;
    // TODO: Support multiple subpasses. 2 are needed for loading MSAA from resolve.
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = VK_NULL_HANDLE;
    renderPassInfo.attachmentCount = numAttachments;
    renderPassInfo.pAttachments = attachmentDescs.begin();

    VkRenderPass renderPass;
    VkResult result;
    VULKAN_CALL_RESULT(context->interface(), result, CreateRenderPass(context->device(),
                                                                      &renderPassInfo,
                                                                      nullptr,
                                                                      &renderPass));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context, renderPass));
}

sk_sp<VulkanRenderPass> VulkanRenderPass::MakeRenderPass(const VulkanSharedContext* context,
                                                         const RenderPassDesc& renderPassDesc,
                                                         VulkanCommandBuffer* cmdBuf,
                                                         const Texture* colorTexture,
                                                         const Texture* resolveTexture,
                                                         const Texture* depthStencilTexture) {
    if (!cmdBuf) {
        return nullptr;
    }
    VkRenderPass renderPass;
    renderPass = VK_NULL_HANDLE;

    static_assert((int)LoadOp::kLoad == 0);
    static_assert((int)LoadOp::kClear == 1);
    static_assert((int)LoadOp::kDiscard == 2);
    static_assert(std::size(vkLoadOp) == kLoadOpCount);
    static_assert((int)StoreOp::kStore == 0);
    static_assert((int)StoreOp::kDiscard == 1);
    static_assert(std::size(vkStoreOp) == kStoreOpCount);

    // Process render pass attachments
    int numAttachments = (colorTexture        != nullptr) +
                         (resolveTexture      != nullptr) +
                         (depthStencilTexture != nullptr);

    skia_private::TArray<VkAttachmentDescription> attachmentDescs(numAttachments);
    // Create and track attachment references for the subpass.
    VkAttachmentReference colorRef;
    VkAttachmentReference resolveRef;
    VkAttachmentReference stencilRef;

    if (colorTexture) {
        VulkanTexture* vulkanColorTexture =
                const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(colorTexture));
        vulkanColorTexture->setImageLayout(cmdBuf,
                                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           false);

        VulkanTextureInfo vkColorAttachTexInfo;
        colorTexture->textureInfo().getVulkanTextureInfo(&vkColorAttachTexInfo);
        auto& colorInfo = renderPassDesc.fColorAttachment;

        colorRef.attachment = attachmentDescs.size();
        VkAttachmentDescription& colorAttachDesc = attachmentDescs.push_back();
        memset(&colorAttachDesc, 0, sizeof(VkAttachmentDescription));
        colorAttachDesc.flags = 0;
        colorAttachDesc.format = vkColorAttachTexInfo.fFormat;
        VkSampleCountFlagBits sampleCount;
        SkAssertResult(
                skgpu::SampleCountToVkSampleCount(vkColorAttachTexInfo.fSampleCount, &sampleCount));
        colorAttachDesc.samples = sampleCount;
        colorAttachDesc.loadOp = vkLoadOp[static_cast<int>(colorInfo.fLoadOp)];
        colorAttachDesc.storeOp = vkStoreOp[static_cast<int>(colorInfo.fStoreOp)];
        colorAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        if (resolveTexture) {
            SkASSERT(renderPassDesc.fColorResolveAttachment.fStoreOp == StoreOp::kStore);
            // TODO: check Texture matches RenderPassDesc
            VulkanTexture* vulkanResolveTexture =
                    const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(resolveTexture));
            vulkanResolveTexture->setImageLayout(cmdBuf,
                                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                 false);

            VulkanTextureInfo vkResolveAttachTexInfo;
            resolveTexture->textureInfo().getVulkanTextureInfo(&vkResolveAttachTexInfo);
            auto& resolveInfo = renderPassDesc.fColorResolveAttachment;

            resolveRef.attachment = attachmentDescs.size();
            VkAttachmentDescription& resolveAttachDesc = attachmentDescs.push_back();
            memset(&resolveAttachDesc, 0, sizeof(VkAttachmentDescription));
            resolveAttachDesc.flags = 0;
            resolveAttachDesc.format = vkResolveAttachTexInfo.fFormat;
            VkSampleCountFlagBits resolveSampleCount;
            SkAssertResult(skgpu::SampleCountToVkSampleCount(vkColorAttachTexInfo.fSampleCount,
                                                             &resolveSampleCount));
            resolveAttachDesc.samples = resolveSampleCount;
            resolveAttachDesc.loadOp = vkLoadOp[static_cast<int>(resolveInfo.fLoadOp)];
            resolveAttachDesc.storeOp = vkStoreOp[static_cast<int>(resolveInfo.fStoreOp)];
            resolveAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            resolveAttachDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            resolveAttachDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else {
            resolveRef.attachment = VK_ATTACHMENT_UNUSED;
            resolveRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    } else {
        colorRef.attachment = VK_ATTACHMENT_UNUSED;
        colorRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if (depthStencilTexture) {
        VulkanTexture* vulkanDepthStencilTexture =
                const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(depthStencilTexture));
        vulkanDepthStencilTexture->setImageLayout(cmdBuf,
                                                  VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                  false);

        VulkanTextureInfo depthStencilTexInfo;
        depthStencilTexture->textureInfo().getVulkanTextureInfo(&depthStencilTexInfo);
        auto& depthStencilInfo = renderPassDesc.fDepthStencilAttachment;

        stencilRef.attachment = attachmentDescs.size();
        VkAttachmentDescription& depthStencilAttachDesc = attachmentDescs.push_back();
        depthStencilAttachDesc.flags = 0;
        depthStencilAttachDesc.format = depthStencilTexInfo.fFormat;
        VkSampleCountFlagBits depthStencilSampleCount;
        SkAssertResult(skgpu::SampleCountToVkSampleCount(depthStencilTexInfo.fSampleCount,
                                                         &depthStencilSampleCount));
        depthStencilAttachDesc.samples = depthStencilSampleCount;
        depthStencilAttachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthStencilAttachDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthStencilAttachDesc.stencilLoadOp = vkLoadOp[static_cast<int>(depthStencilInfo.fLoadOp)];
        depthStencilAttachDesc.stencilStoreOp = vkStoreOp[static_cast<int>(depthStencilInfo.fStoreOp)];
        depthStencilAttachDesc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachDesc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

        stencilRef.layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    } else {
        stencilRef.attachment = VK_ATTACHMENT_UNUSED;
        stencilRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    // Create VkRenderPass
    VkRenderPassCreateInfo renderPassInfo;
    memset(&renderPassInfo, 0, sizeof(VkRenderPassCreateInfo));
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0;

    // TODO: Support multiple subpasses. 2 are needed for loading MSAA from resolve.
    renderPassInfo.subpassCount = 1;
    VkSubpassDescription subpassDesc;
    memset(&subpassDesc, 0, sizeof(VkSubpassDescription));
    subpassDesc.flags = 0;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // TODO: Add support for input attachments
    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = nullptr;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorRef;
    subpassDesc.pResolveAttachments = &resolveRef;
    subpassDesc.pDepthStencilAttachment = &stencilRef;
    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments = nullptr;

    renderPassInfo.pSubpasses = &subpassDesc;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = VK_NULL_HANDLE;
    renderPassInfo.attachmentCount = numAttachments;
    renderPassInfo.pAttachments = attachmentDescs.begin();

    VkResult result;
    VULKAN_CALL_RESULT(context->interface(), result, CreateRenderPass(context->device(),
                                                                      &renderPassInfo,
                                                                      nullptr,
                                                                      &renderPass));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
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
