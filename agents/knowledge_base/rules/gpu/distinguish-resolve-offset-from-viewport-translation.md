---
name: skia/gpu/distinguish-resolve-offset-from-viewport-translation
description: When defining origin offsets between intermediate attachments and resolve targets in Graphite render passes, name them resolveOffset rather than viewportTranslation
rubric: readability
severity: should_fix
---

**Rule.** When specifying origin offsets between intermediate color or MSAA
attachments and their resolve targets in Graphite render passes, name the
parameter or field after its resolve role (such as resolveOffset or resolveArea)
rather than viewportTranslation. Keep viewport naming reserved for transforms
visible to shaders and NDC projection.

**Why.** In Graphite, logical rendering and NDC coordinates are defined in
device space and remain unaffected by attachment placement or sub-allocation.
Naming an attachment origin shift viewportTranslation misleadingly suggests a
transform applied to the viewport or vertex shader pipeline rather than a
sub-region resolve offset.

## Examples

Bad:

```cpp
// Misleadingly implies vertex shaders or viewport transforms are affected
bool addRenderPass(const RenderPassDesc&,
                   sk_sp<Texture> colorTexture,
                   sk_sp<Texture> resolveTexture,
                   sk_sp<Texture> depthStencilTexture,
                   const Texture* dstCopy,
                   SkIRect dstReadBounds,
                   SkIPoint viewportTranslation,
                   SkISize viewportDims,
                   const DrawPassList& drawPasses);
```

Good:

```cpp
// Clearly identifies offset between color attachment and resolve target
bool addRenderPass(const RenderPassDesc&,
                   sk_sp<Texture> colorTexture,
                   sk_sp<Texture> resolveTexture,
                   sk_sp<Texture> depthStencilTexture,
                   const Texture* dstCopy,
                   SkIRect dstReadBounds,
                   SkIPoint resolveOffset,
                   SkISize viewportDims,
                   const DrawPassList& drawPasses);
```
