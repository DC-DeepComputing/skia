---
name: skia/gpu/bind-pass-invariant-vulkan-resources-upfront
description: When recording Vulkan command buffers, bind render-pass-invariant resources like push constants and descriptor sets once at render pass start using a compatible layout
rubric: safety
severity: should_fix
---

**Rule.** When recording Vulkan command buffers, bind descriptor sets and push
constants that remain invariant across an entire render pass (such as
destination input attachments or intrinsic uniforms) once at render pass
initialization using a compatible mock pipeline layout. Do not defer invariant
resource binding or push constant updates to bindGraphicsPipeline() using
per-pipeline state flags or repeated uniform collection.

**Why.** In Vulkan, resource bindings and push constants recorded against a
compatible pipeline layout persist across pipeline switches within the same
subpass. Updating invariant resources once upfront avoids redundant uniform
packing and driver command recording overhead, removes unnecessary state flags
on the draw path, and prevents complex state invalidation across graphics
pipeline changes.

## Examples

Bad:

```cpp
// Deferring push constants or descriptor sets to bindGraphicsPipeline:
case DrawPassCommands::Type::kBindGraphicsPipeline: {
    UniformManager intrinsicValues{Layout::kStd140};
    CollectIntrinsicUniforms(fSharedContext->caps(), fCurrentViewport, fDstCopyBounds, &intrinsicValues);
    this->pushConstants(PushConstantInfo{..., intrinsicValues.finish().data()}, pipelineLayout);
    if (fBindDstAsInputAttachment) {
        this->updateAndBindInputAttachment(*fTargetTexture, kDstAsInputDescSetIndex);
        fBindDstAsInputAttachment = false;
    }
    this->bindGraphicsPipeline(drawPass->getPipeline(bgp->fPipelineIndex));
    break;
}
```

Good:

```cpp
// Updating once per render pass at pass initialization (e.g. in performOncePerRPUpdates):
void VulkanCommandBuffer::performOncePerRPUpdates(SkIRect viewport, bool bindDstAsInputAttachment) {
    UniformManager intrinsicValues{Layout::kStd140};
    CollectIntrinsicUniforms(fSharedContext->caps(), viewport, fDstReadBounds, &intrinsicValues);
    this->pushConstants(PushConstantInfo{..., intrinsicValues.finish().data()},
                        fResourceProvider->mockPipelineLayout());
    if (bindDstAsInputAttachment) {
        this->updateAndBindInputAttachment(*fTargetTexture, kDstAsInputDescSetIndex,
                                           fResourceProvider->mockPipelineLayout());
    }
}
```
