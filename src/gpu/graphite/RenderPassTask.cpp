/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RenderPassTask.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

sk_sp<RenderPassTask> RenderPassTask::Make(std::vector<std::unique_ptr<DrawPass>> passes,
                                           const RenderPassDesc& desc,
                                           sk_sp<TextureProxy> target,
                                           TextureInfo msaaLoadTextureInfo) {
    // For now we have one DrawPass per RenderPassTask
    SkASSERT(passes.size() == 1);
    if (!target) {
        return nullptr;
    }

    return sk_sp<RenderPassTask>(new RenderPassTask(
            std::move(passes),
            desc,
            target,
            std::move(msaaLoadTextureInfo)));
}

RenderPassTask::RenderPassTask(std::vector<std::unique_ptr<DrawPass>> passes,
                               const RenderPassDesc& desc,
                               sk_sp<TextureProxy> target,
                               TextureInfo msaaLoadTextureInfo)
        : fDrawPasses(std::move(passes))
        , fRenderPassDesc(desc)
        , fTarget(std::move(target))
        , fMSAALoadTextureInfo(std::move(msaaLoadTextureInfo)) {}

RenderPassTask::~RenderPassTask() = default;

bool RenderPassTask::prepareResources(ResourceProvider* resourceProvider,
                                      const RuntimeEffectDictionary* runtimeDict) {
    SkASSERT(fTarget);
    if (!TextureProxy::InstantiateIfNotLazy(resourceProvider, fTarget.get())) {
        SKGPU_LOG_W("Failed to instantiate RenderPassTask target. Will not create renderpass!");
        SKGPU_LOG_W("Dimensions are (%d, %d).",
                    fTarget->dimensions().width(), fTarget->dimensions().height());
        return false;
    }

    // Assuming one draw pass per renderpasstask for now
    SkASSERT(fDrawPasses.size() == 1);
    for (const auto& drawPass: fDrawPasses) {
        if (!drawPass->prepareResources(resourceProvider, runtimeDict, fRenderPassDesc)) {
            return false;
        }
    }
    return true;
}

bool RenderPassTask::addCommands(Context* context, CommandBuffer* commandBuffer) {
    // TBD: Expose the surfaces that will need to be attached within the renderpass?

    // TODO: for task execution, start the render pass, then iterate passes and
    // possibly(?) start each subpass, and call DrawPass::addCommands() on the command buffer
    // provided to the task. Then close the render pass and we should have pixels..

    // Instantiate the target
    SkASSERT(fTarget && fTarget->isInstantiated());

    // We don't instantiate the MSAA or DS attachments in prepareResources because we want to use
    // the discardable attachments from the Context.
    ResourceProvider* resourceProvider = context->priv().resourceProvider();
    sk_sp<Texture> colorAttachment;
    sk_sp<Texture> msaaLoadTexture;
    sk_sp<Texture> resolveAttachment;
    if (fRenderPassDesc.fColorResolveAttachment.fTextureInfo.isValid()) {
        SkASSERT(fTarget->numSamples() == 1 &&
                 fRenderPassDesc.fColorAttachment.fTextureInfo.numSamples() > 1);
        colorAttachment = resourceProvider->findOrCreateDiscardableMSAAAttachment(
                fTarget->dimensions(), fRenderPassDesc.fColorAttachment.fTextureInfo);
        if (!colorAttachment) {
            SKGPU_LOG_W("Could not get Color attachment for RenderPassTask");
            return false;
        }
        resolveAttachment = fTarget->refTexture();

        if (fMSAALoadTextureInfo.isValid()) {
            msaaLoadTexture = resourceProvider->findOrCreateDiscardableMSAALoadTexture(
                    fTarget->dimensions(), fMSAALoadTextureInfo);
        } else {
            msaaLoadTexture = resolveAttachment;
        }
    } else {
        colorAttachment = fTarget->refTexture();
    }

    sk_sp<Texture> depthStencilAttachment;
    if (fRenderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid()) {
        // TODO: ensure this is a scratch/recycled texture
        depthStencilAttachment = resourceProvider->findOrCreateDepthStencilAttachment(
                fTarget->dimensions(), fRenderPassDesc.fDepthStencilAttachment.fTextureInfo);
        if (!depthStencilAttachment) {
            SKGPU_LOG_W("Could not get DepthStencil attachment for RenderPassTask");
            return false;
        }
    }

    // TODO: We need to handle the case where we need to load the single sampled target's data into
    // the discardable MSAA Surface. On different backends this will be done in various ways. On
    // Metal we can simply insert a draw at the start of the render pass sampling the single target
    // texture as normal. In Vulkan, we need to create a whole new subpass at the start, use the
    // single sample resolve as an input attachment in that subpass, and then do a draw. The big
    // thing with Vulkan is that this input attachment and subpass means we also need to update
    // the fRenderPassDesc here.

    if (msaaLoadTexture != nullptr && msaaLoadTexture != resolveAttachment) {
        // If we cannot directly load resolve texture's old content in one single render
        // pass, then do a copying to msaaLoadTexture here.
        if (!commandBuffer->copyTextureToTexture(resolveAttachment,
                                                 SkIRect::MakeSize(resolveAttachment->dimensions()),
                                                 msaaLoadTexture,
                                                 SkIPoint::Make(0, 0))) {
            return false;
        }
    }

    return commandBuffer->addRenderPass(fRenderPassDesc,
                                        std::move(colorAttachment),
                                        std::move(msaaLoadTexture),
                                        std::move(resolveAttachment),
                                        std::move(depthStencilAttachment),
                                        SkRect::Make(fTarget->dimensions()),
                                        fDrawPasses);
}

} // namespace skgpu::graphite
