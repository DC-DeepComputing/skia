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
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

namespace skgpu::graphite {

struct AttachmentDesc;
struct RenderPassDesc;
class VulkanCommandBuffer;
class VulkanSharedContext;

/**
 * Wrapper around VkRenderPass.
*/
class VulkanRenderPass : public Resource {
public:
    // Statically assign attachment indices until such information can be fetched from
    // graphite-level structures (likely RenderPassDesc)
    static constexpr int kColorAttachmentIdx = 0;
    static constexpr int kColorResolveAttachmentIdx = 1;
    static constexpr int kDepthStencilAttachmentIdx = 2;

    static constexpr int kMaxExpectedAttachmentCount = kDepthStencilAttachmentIdx + 1;

    // Methods to create compatible (needed when creating a framebuffer and graphics pipeline) or
    // full (needed when beginning a render pass from the command buffer) render passes and keys.
    // static GraphiteResourceKey MakeRenderPassKey(const RenderPassDesc&, bool compatibleOnly);

    static sk_sp<VulkanRenderPass> MakeRenderPass(const VulkanSharedContext*,
                                                  const RenderPassDesc&,
                                                  bool compatibleOnly);

    VkRenderPass renderPass() const {
        SkASSERT(fRenderPass != VK_NULL_HANDLE);
        return fRenderPass;
    }

    VkExtent2D granularity() { return fGranularity; }

    const char* getResourceType() const override { return "Vulkan RenderPass"; }

    struct Subpass {
        // Attachments are indices into the list of attachments, or -1 marks the attachment unused.
        int fColorAttachment = -1;
        int fResolveAttachment = -1;
        int fDepthStencilAttachment = -1;

        skia_private::STArray<1, int> fDependencies;f
    }

    // Struct to store Vulkan information surrounding a RenderPassDescf.
    // FIXME roll back these changes, it'll make the CL too big and not sure if we're ready for it
    // But we can reduce the metadata, and change the API cleanup a bit, if not the structure.
    struct Metadata {
        Metadata(const RenderPassDesc& renderPassDesc, bool compatibleOnly);

        // bool fLoadMSAAFromResolve;

        int fNumColorAttachments;
        int fNumResolveAttachments;
        int fNumDepthStencilAttachments;

        int fSubpassCount;
        int fSubpassDependencyCount;

        // Currently there are at most 3 attachments: color, resolve, depth+stencil.
        // When created for compatible-only renderpasses, all load and store ops will be kDiscard.
        skia_private::STArray<3, AttachmentDesc> fAttachments;

        // Currently there are up to 2 subpasses (load-from-resolve and main).
        skia_private::STArray<2, Subpass> fSubpasses;

        int keySize() const;

        void addToKey(ResourceKey::Builder&, int& builderIdx);
    };

private:
    void freeGpuData() override;

    VulkanRenderPass(const VulkanSharedContext*, VkRenderPass, VkExtent2D granularity);

    const VulkanSharedContext* fSharedContext;
    VkRenderPass fRenderPass;
    VkExtent2D fGranularity;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanRenderPass_DEFINED
