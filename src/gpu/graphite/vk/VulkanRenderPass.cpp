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

GraphiteResourceKey VulkanRenderPass::GenerateRenderPassKey(
        const RenderPassDesc& renderPassDesc,
        const Texture* colorTexture,
        const Texture* resolveTexture,
        const Texture* depthStencilTexture) {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    // RenderPass key structure:
    // Color attachment compatibility, resolve attachment compat, stencil attach compat,
    // self dependency, load from resolve, count needed to encapsulate subpass information
    int num32DataCnt = 0;


    // Two textures are compatible if they have the same format and sample count. Both values are
    // stored as, and thus necessitate for the key, a int32.
    // TODO: Add a static assertion that format can only be up to VK_FORMAT_MAX_ENUM (int32).
    num32DataCnt = 2 * ((colorTexture != nullptr ) + (resolveTexture != nullptr) +
                        (depthStencilTexture != nullptr));

    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kNo);


    // TODO: Consider how many uint32s will be necessary to store specific subpass information and
    // the fact that we will need to support >1 color attachment per render pass.
    int currentBuilderIdx = 0;
    if (colorTexture) {
        VulkanTextureInfo vkTexInfo;
        builder[currentBuilderIdx++] = vkTexInfo.fFormat;
        builder[currentBuilderIdx++] = vkTexInfo.fSampleCount;
    }
    if (resolveTexture) {
        VulkanTextureInfo vkTexInfo;
        resolveTexture->textureInfo().getVulkanTextureInfo(&vkTexInfo);
        builder[currentBuilderIdx++] = vkTexInfo.fFormat;
        builder[currentBuilderIdx++] = vkTexInfo.fSampleCount;
    }
    if (depthStencilTexture) {
        VulkanTextureInfo vkTexInfo;
        depthStencilTexture->textureInfo().getVulkanTextureInfo(&vkTexInfo);
        builder[currentBuilderIdx++] = vkTexInfo.fFormat;
        builder[currentBuilderIdx++] = vkTexInfo.fSampleCount;
    }

    builder.finish();
    return key;
}

sk_sp<VulkanRenderPass> VulkanRenderPass::Make(const VulkanSharedContext* context) {
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context));
}

VulkanRenderPass::VulkanRenderPass(const VulkanSharedContext* context)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kYes, sizeof(VkRenderPass))
        , fSharedContext(context)
        , fRenderPass (VK_NULL_HANDLE) {
    // TODO: Verify Resource arg correctness
}

void VulkanRenderPass::freeGpuData() {
    // TODO: Make sure there are no external attachments (?)
    VULKAN_CALL(fSharedContext->interface(),
                DestroyRenderPass(fSharedContext->device(), fRenderPass, nullptr));
}

} // namespace skgpu::graphite
