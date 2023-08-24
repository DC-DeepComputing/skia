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
    // Compatible RenderPass key structure must account for:
    // Color, resolve, and stencil attachment compatibility (which is determined by having matching
    // formats and sample counts).
    // TODO: Consider self-dependency to support dst reads.
    int num32DataCnt = 0;

    auto& colorAttachmentTextureInfo        = renderPassDesc.fColorAttachment.fTextureInfo;
    auto& colorResolveAttachmentTextureInfo = renderPassDesc.fColorResolveAttachment.fTextureInfo;
    auto& depthStencilAttachmentTextureInfo = renderPassDesc.fDepthStencilAttachment.fTextureInfo;

    // Two textures are compatible if they have the same format and sample count. Both values are
    // stored as, and thus necessitate for the key, an int32. So we require 2 * the number of
    // texture attachments. For now, simply append the number of subpasses as a part of the key
    // since currently we only expect 1 or 2 subpasses.
    num32DataCnt = 2 * ((colorAttachmentTextureInfo.isValid()) +
                        (colorResolveAttachmentTextureInfo.isValid()) +
                        (depthStencilAttachmentTextureInfo.isValid()))
                   + 1;

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

    int numSubpasses = 1;
    bool loadMSAAFromResolve = renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid()
                               ? renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad
                               : false;
    if (loadMSAAFromResolve) {
        numSubpasses++;
    }
    builder[currentBuilderIdx++] = numSubpasses;

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
    bool loadMSAAFromResolve =
            resolveTexture ? renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad
                           : false;

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
                    (vkStoreOp[static_cast<int>(renderPassDesc.fColorResolveAttachment.fStoreOp)] << 16 |
                    loadMSAAFromResolve);
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
    VkAttachmentReference resolveLoadInputRef;
    VkAttachmentReference stencilRef;

    auto& colorAttachmentTextureInfo        = renderPassDesc.fColorAttachment.fTextureInfo;
    auto& colorResolveAttachmentTextureInfo = renderPassDesc.fColorResolveAttachment.fTextureInfo;
    auto& depthStencilAttachmentTextureInfo = renderPassDesc.fDepthStencilAttachment.fTextureInfo;

    int numAttachments = (colorAttachmentTextureInfo.isValid()) +
                         (colorResolveAttachmentTextureInfo.isValid()) +
                         (depthStencilAttachmentTextureInfo.isValid());

    skia_private::TArray<VkAttachmentDescription> attachmentDescs(numAttachments);
    bool loadMSAAFromResolve = false;
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
            loadMSAAFromResolve = renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
            VulkanTextureInfo vkResolveAttachTexInfo;
            colorResolveAttachmentTextureInfo.getVulkanTextureInfo(&vkResolveAttachTexInfo);

            resolveRef.attachment = attachmentDescs.size();
            VkAttachmentDescription& resolveAttachDesc = attachmentDescs.push_back();
            memset(&resolveAttachDesc, 0, sizeof(VkAttachmentDescription));
            resolveAttachDesc.format = vkResolveAttachTexInfo.fFormat;
            // VkSampleCountFlagBits resolveSampleCount;
            // SkAssertResult(skgpu::SampleCountToVkSampleCount(vkResolveAttachTexInfo.fSampleCount,
            //                                                  &resolveSampleCount));
            resolveAttachDesc.samples = VK_SAMPLE_COUNT_1_BIT;//resolveSampleCount;

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
    int subpassCount = 1 + loadMSAAFromResolve;
    skia_private::TArray<VkSubpassDescription> subpassDescs(subpassCount);
    memset(subpassDescs.begin(), 0, subpassCount * sizeof(VkSubpassDescription));

    if (loadMSAAFromResolve) {
        resolveLoadInputRef.attachment = resolveRef.attachment;
        resolveLoadInputRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkSubpassDescription& loadSubpassDesc = subpassDescs.push_back();
        memset(&loadSubpassDesc, 0, sizeof(VkSubpassDescription));
        loadSubpassDesc.flags = 0;
        loadSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        loadSubpassDesc.inputAttachmentCount = 1;
        loadSubpassDesc.pInputAttachments = &resolveLoadInputRef;
        loadSubpassDesc.colorAttachmentCount = 1;
        loadSubpassDesc.pColorAttachments = &colorRef;
        loadSubpassDesc.pResolveAttachments = nullptr;
        loadSubpassDesc.pDepthStencilAttachment = nullptr;
        loadSubpassDesc.preserveAttachmentCount = 0;
        loadSubpassDesc.pPreserveAttachments = nullptr;

        // TODO: Do subpass dependencies matter for determining renderpass compat?
    }

    VkSubpassDescription& mainSubpassDesc = subpassDescs.push_back();
    memset(&mainSubpassDesc, 0, sizeof(VkSubpassDescription));
    mainSubpassDesc.flags = 0;
    mainSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    mainSubpassDesc.inputAttachmentCount = 0;
    mainSubpassDesc.pInputAttachments = nullptr;
    mainSubpassDesc.colorAttachmentCount = 1; //(colorTexture != nullptr);
    mainSubpassDesc.pColorAttachments = &colorRef;
    mainSubpassDesc.pResolveAttachments = &resolveRef;
    mainSubpassDesc.pDepthStencilAttachment = &stencilRef;
    mainSubpassDesc.preserveAttachmentCount = 0;
    mainSubpassDesc.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassInfo;
    memset(&renderPassInfo, 0, sizeof(VkRenderPassCreateInfo));
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0;
    // TODO: Support multiple subpasses. 2 are needed for loading MSAA from resolve.
    renderPassInfo.subpassCount = subpassCount;
    renderPassInfo.pSubpasses = subpassDescs.begin();
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
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context, renderPass, nullptr));
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
    skia_private::TArray<VkImageView> attachmentViews(numAttachments); // Needed for frame buffer

    // Create and track attachment references for the subpass.
    VkAttachmentReference colorRef;
    VkAttachmentReference resolveRef;
    VkAttachmentReference resolveLoadInputRef;
    VkAttachmentReference stencilRef;
    bool loadMSAAFromResolve =
            resolveTexture ? renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad
                           : false;
    if (loadMSAAFromResolve) {
        SkDebugf("\nLoading MSAA from resolve\n");
    }

    if (colorTexture) {
        VulkanTexture* vulkanColorTexture =
                const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(colorTexture));
        // TODO: Check for loading MSAA to determine image layout (general for input attach,
        // color attach optimal for color attach)

        if (loadMSAAFromResolve) {
            VkAccessFlags dstAccess = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            VkPipelineStageFlags dstStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            vulkanColorTexture->setImageLayout(cmdBuf,
                                               VK_IMAGE_LAYOUT_GENERAL,
                                               dstAccess,
                                               dstStages,
                                               false);
        } else {
            vulkanColorTexture->setImageLayout(cmdBuf,
                                               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                               false);
        }

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
        VkImageLayout layout =
                loadMSAAFromResolve ? VK_IMAGE_LAYOUT_GENERAL
                                    : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // TODO: Verify layouts; may need to change in the case of input attachments
        colorAttachDesc.initialLayout = layout;
        colorAttachDesc.finalLayout = layout;
        colorRef.layout = layout;

        VkImageView& colorAttachmentView = attachmentViews.push_back();
        //auto vulkanColorTexture = static_cast<const VulkanTexture*>(colorTexture);
        colorAttachmentView = loadMSAAFromResolve ? vulkanColorTexture->getImageView(
                VulkanImageView::Usage::kShaderInput)->imageView()
                                                  : vulkanColorTexture->getImageView(
                VulkanImageView::Usage::kAttachment)->imageView();

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
            // VkSampleCountFlagBits resolveSampleCount;
            // SkAssertResult(skgpu::SampleCountToVkSampleCount(vkResolveAttachTexInfo.fSampleCount,
            //                                                  &resolveSampleCount));
            resolveAttachDesc.samples = VK_SAMPLE_COUNT_1_BIT;//resolveSampleCount;
            resolveAttachDesc.loadOp = vkLoadOp[static_cast<int>(resolveInfo.fLoadOp)];
            resolveAttachDesc.storeOp = vkStoreOp[static_cast<int>(resolveInfo.fStoreOp)];
            resolveAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            // TODO: Verify layouts; may need to change in the case of input attachments
            resolveAttachDesc.initialLayout =
                    // loadMSAAFromResolve ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    //                     : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            resolveAttachDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkImageView& resolveView = attachmentViews.push_back();
            //auto vulkanResolveTexture = static_cast<const VulkanTexture*>(resolveTexture);
            resolveView = vulkanResolveTexture->getImageView(VulkanImageView::Usage::kAttachment)->
                          imageView();

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

        VkImageView& stencilView = attachmentViews.push_back();
        memset(&stencilView, 0, sizeof(VkImageView));
        //auto vulkanDepthStencilTexture = static_cast<const VulkanTexture*>(depthStencilTexture);
        stencilView = vulkanDepthStencilTexture->getImageView(VulkanImageView::Usage::kAttachment)->
                      imageView();
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

    renderPassInfo.subpassCount = 1 + loadMSAAFromResolve;
    skia_private::TArray<VkSubpassDescription> subpassDescs(renderPassInfo.subpassCount);
    memset(subpassDescs.begin(), 0, renderPassInfo.subpassCount * sizeof(VkSubpassDescription));

    // If we are loading MSAA from resolve, that subpass must always be first.
    VkSubpassDependency dependency;
    if (loadMSAAFromResolve) {
        resolveLoadInputRef.attachment = resolveRef.attachment;
        resolveLoadInputRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkSubpassDescription& loadSubpassDesc = subpassDescs.push_back();
        memset(&loadSubpassDesc, 0, sizeof(VkSubpassDescription));
        loadSubpassDesc.flags = 0;
        loadSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        loadSubpassDesc.inputAttachmentCount = 1;
        loadSubpassDesc.pInputAttachments = &resolveLoadInputRef;
        loadSubpassDesc.colorAttachmentCount = 1;
        loadSubpassDesc.pColorAttachments = &colorRef;
        loadSubpassDesc.pResolveAttachments = nullptr;
        loadSubpassDesc.pDepthStencilAttachment = nullptr;
        loadSubpassDesc.preserveAttachmentCount = 0;
        loadSubpassDesc.pPreserveAttachments = nullptr;

        // Set up the subpass dependency
        const int mainSubpassIdx = loadMSAAFromResolve ? 1 : 0;
        dependency.srcSubpass = 0;
        dependency.dstSubpass = mainSubpassIdx;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    VkSubpassDescription& mainSubpassDesc = subpassDescs.push_back();
    memset(&mainSubpassDesc, 0, sizeof(VkSubpassDescription));
    mainSubpassDesc.flags = 0;
    mainSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // TODO: Add support for input attachments
    mainSubpassDesc.inputAttachmentCount = 0;
    mainSubpassDesc.pInputAttachments = nullptr;
    mainSubpassDesc.colorAttachmentCount = 1; //(colorTexture != nullptr);
    mainSubpassDesc.pColorAttachments = &colorRef;
    mainSubpassDesc.pResolveAttachments = loadMSAAFromResolve ? nullptr : &resolveRef; // TODO check this logic
    mainSubpassDesc.pDepthStencilAttachment = &stencilRef;
    mainSubpassDesc.preserveAttachmentCount = 0;
    mainSubpassDesc.pPreserveAttachments = nullptr;

    renderPassInfo.pSubpasses = subpassDescs.begin();
    renderPassInfo.dependencyCount = loadMSAAFromResolve;
    renderPassInfo.pDependencies = loadMSAAFromResolve ? &dependency : nullptr;
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

    // Create VkFrameBuffer for each render pass. TODO: Consider caching these in the future.
    VkFramebufferCreateInfo frameBufferInfo;
    memset(&frameBufferInfo, 0, sizeof(VkFramebufferCreateInfo));
    frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferInfo.pNext = nullptr;
    frameBufferInfo.flags = 0;
    frameBufferInfo.renderPass = renderPass;
    frameBufferInfo.attachmentCount = numAttachments;
    frameBufferInfo.pAttachments = attachmentViews.begin();
    frameBufferInfo.width = colorTexture->dimensions().width();
    frameBufferInfo.height = colorTexture->dimensions().height();
    frameBufferInfo.layers = 1;
    auto frameBuffer = VulkanFrameBuffer::Make(context, frameBufferInfo);

    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context, renderPass, frameBuffer));
}

VulkanRenderPass::VulkanRenderPass(const VulkanSharedContext* context, VkRenderPass renderPass,
                                   sk_sp<VulkanFrameBuffer> frameBuffer)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kYes, sizeof(VkRenderPass))
        , fSharedContext(context)
        , fRenderPass (renderPass)
        , fFrameBuffer (std::move(frameBuffer)) {
    if (fFrameBuffer) {
        fFrameBuffer->ref();
    }
}

void VulkanRenderPass::freeGpuData() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyRenderPass(fSharedContext->device(), fRenderPass, nullptr));
    if (fFrameBuffer) {
        fFrameBuffer->unref();
    }
}

} // namespace skgpu::graphite
