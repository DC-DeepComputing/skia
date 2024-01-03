/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanRenderPass_DEFINED
#define skgpu_graphite_VulkanRenderPass_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

namespace skgpu::graphite {

class VulkanCommandBuffer;
class VulkanSharedContext;

const static VkAttachmentStoreOp vkStoreOp[] {
    VK_ATTACHMENT_STORE_OP_STORE,
    VK_ATTACHMENT_STORE_OP_DONT_CARE
};
const static VkAttachmentLoadOp vkLoadOp[] {
    VK_ATTACHMENT_LOAD_OP_LOAD,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
    VK_ATTACHMENT_LOAD_OP_DONT_CARE
};

/**
 * Wrapper around VkRenderPass.
*/
class VulkanRenderPass : public Resource {
public:
    // Methods to create compatible (needed when creating a framebuffer and graphics pipeline) or
    // full (needed when beginning a render pass from the command buffer) render passes and keys.
    static GraphiteResourceKey MakeRenderPassKey(const RenderPassDesc&, bool compatibleOnly);
    static sk_sp<VulkanRenderPass> MakeRenderPass(
            const VulkanSharedContext*, const RenderPassDesc&, bool compatibleOnly);
    static void AddRenderPassInfoToKey(const RenderPassDesc&, bool compatibleOnly,
                                       ResourceKey::Builder&, int startingIdx);

    VkRenderPass renderPass() const {
        SkASSERT(fRenderPass != VK_NULL_HANDLE);
        return fRenderPass;
    }

    VkExtent2D granularity() { return fGranularity; }

    const char* getResourceType() const override { return "Vulkan RenderPass"; }

private:
    void freeGpuData() override;

    VulkanRenderPass(const VulkanSharedContext*, VkRenderPass, VkExtent2D granularity);


    static constexpr int kColorAttachmentIdx = 0;
    static constexpr int kColorResolveAttachmentIdx = 1;
    static constexpr int kDepthStencilAttachmentIdx = 2;
    static constexpr int kInputAttachmentIdx = 4;

    struct VulkanAttachmentDesc {
        static VulkanAttachmentDesc MakeFromAttachmentDesc(const AttachmentDesc&, int idx);
        TextureInfo fTextureInfo;
        LoadOp fLoadOp;
        StoreOp fStoreOp;
        int fAttachmentIndex;
    };

    struct SubpassDesc {
        int fAttachment[4];
    };

    struct SubpassDependency {
        int fSrcSubpass;
        int fDstSubpass;
        // TODO: To make this struct backend-agnostic, PipelineStageFlags can be used instead of
        // VkPipelineStageFlags
        VkPipelineStageFlags fSrcStageFlags;
        VkPipelineStageFlags fDstStageFlags;
        VkAccessFlagBits fSrcAccess;
        VkAccessFlagBits fDstAccess;
    };

    struct VulkanRenderPassDesc {
        static VulkanRenderPassDesc MakeFromRenderPassDesc(const RenderPassDesc&,
                                                           bool compatibleOnly);

        skia_private::TArray<const VulkanAttachmentDesc> fRpAttachments;
        skia_private::TArray<const SubpassDesc> fSubpasses;
        skia_private::TArray<const SubpassDependency> fSubpassDeps;

        std::array<float, 4> fClearColor;
        float fClearDepth;
        uint32_t fClearStencil;
        Swizzle fWriteSwizzle;
        // This samples count usually matches fColorAttachment & fDepthStencilAttachment's samples
        // count. The only exceptional case is when multisampled render to single sampled is used.
        // In that case, the fColorAttachment's samples count will be 1 and fSampleCount will be > 1
        uint32_t fSampleCount;
    };

    const VulkanSharedContext* fSharedContext;
    VkRenderPass fRenderPass;
    VkExtent2D fGranularity;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanRenderPass_DEFINED
