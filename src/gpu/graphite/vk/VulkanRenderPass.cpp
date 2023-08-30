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

GraphiteResourceKey VulkanRenderPass::GenerateCompatibleRenderPassKey(
        const RenderPassDesc& renderPassDesc) {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    // Compatible RenderPass key structure must account for color, resolve, and stencil attachment
    // compatibility (which is determined by having matching formats and sample counts).
    // TODO: Consider self-dependency needed to support dst reads.
    auto& colorAttachmentTextureInfo        = renderPassDesc.fColorAttachment.fTextureInfo;
    auto& colorResolveAttachmentTextureInfo = renderPassDesc.fColorResolveAttachment.fTextureInfo;
    auto& depthStencilAttachmentTextureInfo = renderPassDesc.fDepthStencilAttachment.fTextureInfo;

    // TODO: Once we support >1 subpass, we will have to account for appending subpass key
    // information. Eventually, we plan to have the RenderPassDesc house subpass information and >1
    // color attachment. However, for now, we only ever use >1 subpass in the case of loading MSAA
    // from resolve.
    bool loadMSAAFromResolve = renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid() &&
                               renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
    int numSubpasses = loadMSAAFromResolve ? 2 : 1;

    int num32DataCnt = 0;
    // Two texture attachments are compatible if they have the same format and sample count. Both
    // values are stored as, and thus necessitate for the key, an int32. So we require 2 * the
    // number of texture attachments int32s. As a special case, if we only have one subpass, then
    // resolve and depth/stencil attachment reference compatibility requirements are ignored.
    num32DataCnt = 2 * colorAttachmentTextureInfo.isValid();
    if (numSubpasses > 1) {
        num32DataCnt += 2 * ((colorResolveAttachmentTextureInfo.isValid()) +
                             (depthStencilAttachmentTextureInfo.isValid()));
        // Add one more int32 to the key to store whether or not we need to load from MSAA.
        num32DataCnt++;
    }


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
    if (numSubpasses > 1) {
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
        builder[currentBuilderIdx++] = loadMSAAFromResolve;
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
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    // Compatible RenderPass key structure must account for:
    // Color, resolve, and stencil attachment compatibility (which is determined by having matching
    // formats and sample counts).
    // TODO: Consider self-dependency to support dst reads.
    // TODO: Once we support >1 subpass, we will have to account for appending subpass key
    // information to load MSAA from resolve.
    int num32DataCnt = 0;

    auto& colorAttachmentTextureInfo        = renderPassDesc.fColorAttachment.fTextureInfo;
    auto& colorResolveAttachmentTextureInfo = renderPassDesc.fColorResolveAttachment.fTextureInfo;
    auto& depthStencilAttachmentTextureInfo = renderPassDesc.fDepthStencilAttachment.fTextureInfo;

    // For render passes, we must account for the format, sample count, and load/store values of an
    // attachment. Format and sample count are stored as, and thus necessitate for the key, an
    // int32. Their load/store ops can fit into one uint32. So we require 3 * the number of
    // texture attachments.
    num32DataCnt = 3 * ((colorAttachmentTextureInfo.isValid()) +
                        (colorResolveAttachmentTextureInfo.isValid()) +
                        (depthStencilAttachmentTextureInfo.isValid()));

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kNo);

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
            // TODO: Assert the sizes here work out.
            builder[currentBuilderIdx++] =
                    (vkLoadOp[static_cast<int>(renderPassDesc.fColorAttachment.fLoadOp)] << 8) |
                    (vkStoreOp[static_cast<int>(renderPassDesc.fColorAttachment.fStoreOp)]);
        } else {
            SkUNREACHABLE;
        }
    }
    // TODO: If there is only one subpass, then resolve and depth/stencil attachment references
    // compatibility requirements are ignored. Take this into consideration.
    if (colorResolveAttachmentTextureInfo.isValid()) {
        VulkanTextureInfo vkTexInfo;
        if (colorResolveAttachmentTextureInfo.getVulkanTextureInfo(&vkTexInfo)) {
            builder[currentBuilderIdx++] = vkTexInfo.fFormat;
            builder[currentBuilderIdx++] = vkTexInfo.fSampleCount;
            builder[currentBuilderIdx++] =
                    (vkLoadOp[static_cast<int>(renderPassDesc.fColorResolveAttachment.fLoadOp)] << 8) |
                    (vkStoreOp[static_cast<int>(renderPassDesc.fColorResolveAttachment.fStoreOp)]);
        } else {
            SkUNREACHABLE;
        }
    }
    if (depthStencilAttachmentTextureInfo.isValid()) {
        VulkanTextureInfo vkTexInfo;
        if (depthStencilAttachmentTextureInfo.getVulkanTextureInfo(&vkTexInfo)) {
            builder[currentBuilderIdx++] = vkTexInfo.fFormat;
            builder[currentBuilderIdx++] = vkTexInfo.fSampleCount;
            builder[currentBuilderIdx++] =
                    (vkLoadOp[static_cast<int>(renderPassDesc.fDepthStencilAttachment.fLoadOp)] << 8) |
                    (vkStoreOp[static_cast<int>(renderPassDesc.fDepthStencilAttachment.fStoreOp)]);
        } else {
            SkUNREACHABLE;
        }
    }

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
        // TODO handle unsuccessful call
        SkAssertResult(
                skgpu::SampleCountToVkSampleCount(vkColorAttachTexInfo.fSampleCount, &sampleCount));
        colorAttachDesc.samples = sampleCount;

        colorAttachDesc.flags = 0;
        colorAttachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // TODO: Verify layouts; may need to change in the case of input attachments
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
            // TODO: Verify layouts; may need to change in the case of input attachments
            resolveAttachDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            resolveAttachDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else {
            resolveRef.attachment = VK_ATTACHMENT_UNUSED;
            resolveRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    } else {
        // TODO: Assert we always have a color attachment.
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
        // TODO handle unsuccessful call
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
    subpassDesc.colorAttachmentCount = 1; //(colorTexture != nullptr);
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
        // TODO: Delete unused structs?
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
        // TODO handle unsuccessful call
        SkAssertResult(
                skgpu::SampleCountToVkSampleCount(vkColorAttachTexInfo.fSampleCount, &sampleCount));
        colorAttachDesc.samples = sampleCount;
        colorAttachDesc.loadOp = vkLoadOp[static_cast<int>(colorInfo.fLoadOp)];
        colorAttachDesc.storeOp = vkStoreOp[static_cast<int>(colorInfo.fStoreOp)];
        colorAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // TODO: Verify layouts; may need to change in the case of input attachments
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
            // TODO: Verify layouts; may need to change in the case of input attachments
            resolveAttachDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            resolveAttachDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else {
            resolveRef.attachment = VK_ATTACHMENT_UNUSED;
            resolveRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    } else {
        // TODO: Assert we always have a color attachment.
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
        // TODO handle unsuccessful call
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
    subpassDesc.colorAttachmentCount = 1; //(colorTexture != nullptr);
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
        // TODO: Delete unused structs?
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
