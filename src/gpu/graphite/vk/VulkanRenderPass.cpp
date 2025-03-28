/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanRenderPass.h"

#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

#include <limits>

namespace skgpu::graphite {

namespace {

void add_subpass_info_to_key(ResourceKey::Builder& builder,
                             int& builderIdx,
                             bool hasColorAttachment,
                             bool hasColorResolveAttachment,
                             bool hasDepthStencilAttachment,
                             bool loadMSAAFromResolve,
                             int subpassCount,
                             int subpassDependencyCount) {
    // TODO: Fetch actual attachment reference and index information for each
    // subpass from RenderPassDesc. For now, determine subpass data based upon whether we are
    // loading from MSAA or not.
    const int mainSubpassIdx = loadMSAAFromResolve ? 1 : 0;
    // Assign a smaller value to represent VK_ATTACHMENT_UNUSED.
    static constexpr int kAttachmentUnused = std::numeric_limits<uint8_t>::max();

    // The following key structure assumes that we only have up to one reference of each type per
    // subpass and that attachments are indexed in order of color, resolve, depth/stencil, then
    // input attachments. These indices are statically defined in the VulkanRenderPass header file.
    for (int j = 0; j < subpassCount; j++) {
        if (j == mainSubpassIdx) {
            uint32_t attachmentIdxKeyInfo;
            attachmentIdxKeyInfo = hasColorAttachment ? VulkanRenderPass::kColorAttachmentIdx
                                                      : kAttachmentUnused;
            attachmentIdxKeyInfo |=
                    (hasColorResolveAttachment ? VulkanRenderPass::kColorResolveAttachmentIdx
                                               : kAttachmentUnused) << 8;
            attachmentIdxKeyInfo |=
                    (hasDepthStencilAttachment ? VulkanRenderPass::kDepthStencilAttachmentIdx
                                               : kAttachmentUnused) << 16;
            // TODO: Add input attachment info to key once supported for use in main subpass
            attachmentIdxKeyInfo |= kAttachmentUnused << 24;

            builder[builderIdx++] = attachmentIdxKeyInfo;
        } else { // Loading MSAA from resolve subpass
            SkASSERT(hasColorAttachment);
            builder[builderIdx++] =
                    VulkanRenderPass::kColorAttachmentIdx | // color attachment
                    (kAttachmentUnused << 8)              | // No color resolve attachment
                    (kAttachmentUnused << 16)             | // No depth/stencil attachment
                    // The input attachment for the load subpass is the color resolve texture.
                    (VulkanRenderPass::kColorResolveAttachmentIdx << 24);
        }
    }

    // TODO: Query RenderPassDesc for subpass dependency information & populate the key accordingly.
    // For now, we know that the only subpass dependency will be that expected for loading MSAA from
    // resolve.
    for (int i = 0; i < subpassDependencyCount; i++) {
        builder[builderIdx++] = 0 | (mainSubpassIdx << 8); // srcSubpass, dstSubpass
        builder[builderIdx++] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // srcStageMask
        builder[builderIdx++] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // dstStageMask
        builder[builderIdx++] = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;          // srcAccessMask
        builder[builderIdx++] = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |          // dstAccessMask
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        builder[builderIdx++] = VK_DEPENDENCY_BY_REGION_BIT;                   // dependencyFlags
    }
}

} // anonymous namespace

VulkanRenderPass::Metadata::Metadata(const RenderPassDesc& renderPassDesc, bool compatibleOnly) {
    fLoadMSAAFromResolve = RenderPassDescWillLoadMSAAFromResolve(renderPassDesc);

    // TODO: These represent the attachment refs of the main subpass, RenderPassDesc will allow for
    // more attachments and subpasses in the future. Subpasses will not have more than 1 color,
    // resolve, or depth+stencil attachment but could have more input attachments.
    fHasColorAttachment =
            renderPassDesc.fColorAttachment.fFormat != TextureFormat::kUnsupported;
    fHasColorResolveAttachment =
            renderPassDesc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported;
    fHasDepthStencilAttachment =
            renderPassDesc.fDepthStencilAttachment.fFormat != TextureFormat::kUnsupported;

    // Accumulate attachments into a container to mimic future structure in RenderPassDesc
    if (fHasColorAttachment) {
        fAttachments.push_back(renderPassDesc.fColorAttachment);
    }
    if (fHasColorResolveAttachment) {
        fAttachments.push_back(renderPassDesc.fColorResolveAttachment);
    }
    if (fHasDepthStencilAttachment) {
        fAttachments.push_back(renderPassDesc.fDepthStencilAttachment);
    }

    if (compatibleOnly) {
        // Reset all load/store ops on the attachments since those do not affect compatibility.
        for (AttachmentDesc& d : fAttachments) {
            d.fLoadOp = LoadOp::kDiscard;
            d.fStoreOp = StoreOp::kDiscard;
        }
    }}

int VulkanRenderPass::Metadata::keySize() const {
    // The key will be formed such that bigger-picture items (such as the total attachment count)
    // will be near the front of the key to more quickly eliminate incompatible keys. Each
    // renderpass key will start with the total number of attachments associated with it
    // followed by how many subpasses and subpass dependencies the renderpass has. Packed together,
    // these will use one uint32.
    int num32DataCnt = 1;
    SkASSERT(static_cast<uint32_t>(rpAttachmentCount) <= (1u << 8));
    SkASSERT(static_cast<uint32_t>(this->subpassCount()) <= (1u << 8));
    SkASSERT(static_cast<uint32_t>(this->subpassDependencyCount()) <= (1u << 8));

    // The key will then contain format, sample count, and load/store ops for each attachment.
    // It packs up to 2 attachments per uint32_t
    num32DataCnt += (fAttachments.size() + 1) / 2;

    // Then, subpass information will be added in the form of attachment reference indices. Reserve
    // one int32 for each possible attachment reference type, of which there are 4.
    // There are 4 possible attachment reference types. Pack all 4 attachment reference indices into
    // one uint32.
    num32DataCnt += this->subpassCount();
    // Each subpass dependency will be allotted 6 int32s to store all its pertinent information.
    num32DataCnt += 6 * this->subpassDependencyCount();
    return num32DataCnt;
}

void VulkanRenderPass::Metadata::addToKey(ResourceKey::Builder& builder, int& builderIdx) {
    builder[builderIdx++] = fAttachments.size()  |
                            (this->subpassCount() << 8) |
                            (this->subpassDependencyCount() << 16);

    // Iterate through each renderpass attachment to add its information. Each attachment is packed
    // into 16 bits, so two attachments per key field
    for (int i = 0; i < fAttachments.size(); i++) {
        AttachmentDesc desc = fAttachments[i];
        uint16_t descKey = static_cast<uint8_t>(desc.fFormat) << 8 |
                           static_cast<uint8_t>(desc.fLoadOp) << 6 |
                           static_cast<uint8_t>(desc.fStoreOp) << 4 |
                           desc.fSampleCount;
        uint32_t& keySlot = builder[builderIdx + i/2];
        if (i % 2 == 0) {
            keySlot = descKey;
        } else {
            keySlot = (descKey << 16) | keySlot;
        }
    }
    builderIdx += (fAttachments.size() + 1) / 2;

    // TODO: Currently subpasses and their dependencies are keyed for maximum simplicity and ability
    // to represent all Vulkan features. This makes for a large key size. It is unlikely that the
    // full flexibility of Vulkan subpass dependencies will be exposed in the generalized subpasses
    // of RenderPassDesc, so if those are sufficient for the Vulkan backend as well then we may be
    // able to reduce the key size here.
    add_subpass_info_to_key(builder,
                            builderIdx,
                            fHasColorAttachment,
                            fHasColorResolveAttachment,
                            fHasDepthStencilAttachment,
                            fLoadMSAAFromResolve,
                            this->subpassCount(),
                            this->subpassDependencyCount());
}

namespace {

void setup_vk_attachment_description(VkAttachmentDescription* outAttachment,
                                     const AttachmentDesc& desc,
                                     const VkImageLayout initialLayout,
                                     const VkImageLayout finalLayout) {
    static_assert((int)LoadOp::kLoad == (int) VK_ATTACHMENT_LOAD_OP_LOAD);
    static_assert((int)LoadOp::kClear == (int) VK_ATTACHMENT_LOAD_OP_CLEAR);
    static_assert((int)LoadOp::kDiscard == (int) VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    static_assert((int)StoreOp::kStore == (int) VK_ATTACHMENT_STORE_OP_STORE);
    static_assert((int)StoreOp::kDiscard == (int) VK_ATTACHMENT_STORE_OP_DONT_CARE);

    VkAttachmentLoadOp vkLoadOp = static_cast<VkAttachmentLoadOp>(desc.fLoadOp);
    VkAttachmentStoreOp vkStoreOp = static_cast<VkAttachmentStoreOp>(desc.fStoreOp);

    outAttachment->flags = 0;
    outAttachment->format = TextureFormatToVkFormat(desc.fFormat);
    VkSampleCountFlagBits sampleCount;
    SkAssertResult(SampleCountToVkSampleCount(desc.fSampleCount, &sampleCount));
    outAttachment->samples = sampleCount;
    switch (initialLayout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_GENERAL:
            outAttachment->loadOp = vkLoadOp;
            outAttachment->storeOp = vkStoreOp;
            outAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            outAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // The loadOp and storeOp refer to the depth part of the attachment and the stencil*Ops
            // refer to the stencil bits in the attachment.
            outAttachment->loadOp = vkLoadOp;
            outAttachment->storeOp = vkStoreOp;
            outAttachment->stencilLoadOp = vkLoadOp;
            outAttachment->stencilStoreOp = vkStoreOp;
            break;
        default:
            SK_ABORT("Unexpected attachment layout");
    }
    outAttachment->initialLayout = initialLayout;
    outAttachment->finalLayout = finalLayout == VK_IMAGE_LAYOUT_UNDEFINED ? initialLayout
                                                                          : finalLayout;
}

} // anonymous namespace

sk_sp<VulkanRenderPass> VulkanRenderPass::MakeRenderPass(const VulkanSharedContext* context,
                                                         const RenderPassDesc& renderPassDesc,
                                                         bool compatibleOnly) {
    VkRenderPass renderPass;
    renderPass = VK_NULL_HANDLE;
    bool hasColorAttachment        = renderPassDesc.fColorAttachment.fFormat != TextureFormat::kUnsupported;
    bool hasColorResolveAttachment = renderPassDesc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported;
    bool hasDepthStencilAttachment = renderPassDesc.fDepthStencilAttachment.fFormat != TextureFormat::kUnsupported;

    skia_private::TArray<VkAttachmentDescription> attachmentDescs;
    // Create and track attachment references for the subpass.
    VkAttachmentReference colorRef;
    VkAttachmentReference resolveRef;
    VkAttachmentReference resolveLoadInputRef;
    VkAttachmentReference depthStencilRef;

    const bool loadMSAAFromResolve = RenderPassDescWillLoadMSAAFromResolve(renderPassDesc);
    if (hasColorAttachment) {
        colorRef.attachment = attachmentDescs.size();
        VkAttachmentDescription& vkColorAttachDesc = attachmentDescs.push_back();
        memset(&vkColorAttachDesc, 0, sizeof(VkAttachmentDescription));
        setup_vk_attachment_description(
                &vkColorAttachDesc,
                renderPassDesc.fColorAttachment,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (hasColorResolveAttachment) {
            resolveRef.attachment = attachmentDescs.size();
            VkAttachmentDescription& vkResolveAttachDesc = attachmentDescs.push_back();
            memset(&vkResolveAttachDesc, 0, sizeof(VkAttachmentDescription));
            setup_vk_attachment_description(
                    &vkResolveAttachDesc,
                    renderPassDesc.fColorResolveAttachment,
                    loadMSAAFromResolve ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                        : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else {
            resolveRef.attachment = VK_ATTACHMENT_UNUSED;
            resolveRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    } else {
        SkASSERT(false);
        colorRef.attachment = VK_ATTACHMENT_UNUSED;
        colorRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        resolveRef.attachment = VK_ATTACHMENT_UNUSED;
        resolveRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if (hasDepthStencilAttachment) {
        depthStencilRef.attachment = attachmentDescs.size();
        VkAttachmentDescription& vkDepthStencilAttachDesc = attachmentDescs.push_back();
        setup_vk_attachment_description(
                &vkDepthStencilAttachDesc,
                renderPassDesc.fDepthStencilAttachment,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        depthStencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    } else {
        depthStencilRef.attachment = VK_ATTACHMENT_UNUSED;
        depthStencilRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    // Create VkRenderPass
    VkRenderPassCreateInfo renderPassInfo;
    memset(&renderPassInfo, 0, sizeof(VkRenderPassCreateInfo));
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0;
    renderPassInfo.subpassCount = loadMSAAFromResolve ? 2 : 1;

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
    mainSubpassDesc.inputAttachmentCount = 0; // TODO: Add input attachment support in main subpass
    mainSubpassDesc.pInputAttachments = nullptr;
    mainSubpassDesc.colorAttachmentCount = 1;
    mainSubpassDesc.pColorAttachments = &colorRef;
    mainSubpassDesc.pResolveAttachments = &resolveRef;
    mainSubpassDesc.pDepthStencilAttachment = &depthStencilRef;
    mainSubpassDesc.preserveAttachmentCount = 0;
    mainSubpassDesc.pPreserveAttachments = nullptr;

    renderPassInfo.pSubpasses = subpassDescs.begin();
    renderPassInfo.dependencyCount = loadMSAAFromResolve ? 1 : 0;
    renderPassInfo.pDependencies = loadMSAAFromResolve ? &dependency : VK_NULL_HANDLE;
    renderPassInfo.attachmentCount = attachmentDescs.size();
    renderPassInfo.pAttachments = attachmentDescs.begin();

    VkResult result;
    VULKAN_CALL_RESULT(context,
                       result,
                       CreateRenderPass(context->device(), &renderPassInfo, nullptr, &renderPass));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    VkExtent2D granularity;
    VULKAN_CALL(context->interface(), GetRenderAreaGranularity(context->device(),
                                                               renderPass,
                                                               &granularity));
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context, renderPass, granularity));
}

VulkanRenderPass::VulkanRenderPass(const VulkanSharedContext* context,
                                   VkRenderPass renderPass,
                                   VkExtent2D granularity)
        : Resource(context,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0)
        , fSharedContext(context)
        , fRenderPass(renderPass)
        , fGranularity(granularity) {}

void VulkanRenderPass::freeGpuData() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyRenderPass(fSharedContext->device(), fRenderPass, nullptr));
}

} // namespace skgpu::graphite
