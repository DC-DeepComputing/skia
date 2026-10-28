---
name: skia/gpu/derive-depth-attachment-dimensions-from-color-attachment
description: When instantiating depth or stencil attachments for a render pass, derive attachment dimensions directly from the active color attachment rather than the render target
rubric: design
severity: should_fix
---

**Rule.** When configuring or instantiating depth or stencil attachments in
render pass setup, derive the depth/stencil dimensions directly from the active
color attachment (such as via
Caps::getDepthAttachmentDimensions(colorAttachment->textureInfo(),
colorAttachment->dimensions())). Do not recompute approximate or MSAA sizing
from the target proxy or resolve texture (fTarget).

**Why.** In a render pass, depth and stencil attachments must match the
dimensions of the active color attachment. Passing the instantiated color
attachment's texture info and dimensions ensures the depth/stencil attachment
automatically inherits any approximate or MSAA sizing adjustments without
duplicating sizing logic or risking dimension mismatches between attachments.

## Examples

Bad:

```cpp
SkISize dimensions = context->priv().caps()->getDepthAttachmentDimensions(
        fTarget->texture()->textureInfo(), fTarget->dimensions());
if (fRenderPassDesc.fSampleCount > 1) {
    SkASSERT(fRenderPassDesc.fColorResolveAttachment.fTextureInfo.isValid());
    dimensions = get_msaa_size(dimensions, *context->priv().caps());
}
depthStencilAttachment = resourceProvider->findOrCreateShareableTexture(
        dimensions, fRenderPassDesc.fDepthStencilAttachment.fTextureInfo);
```

Good:

```cpp
SkISize dimensions = context->priv().caps()->getDepthAttachmentDimensions(
        colorAttachment->textureInfo(), colorAttachment->dimensions());
depthStencilAttachment = resourceProvider->findOrCreateShareableTexture(
        dimensions, fRenderPassDesc.fDepthStencilAttachment.fTextureInfo);
```
