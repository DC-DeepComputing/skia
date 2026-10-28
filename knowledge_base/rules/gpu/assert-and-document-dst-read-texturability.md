---
name: skia/gpu/assert-and-document-dst-read-texturability
description: When implementing Graphite destination reads or fallback copy paths, verify target texturability and read views, size scratch surfaces to dstReadPixelBounds, and document MSAA limits
rubric: correctness
severity: should_fix
---

**Rule.** When implementing destination reads in Graphite, assert target
texturability via caps()->isTexturable() only on paths that sample the target
(direct texture sampling or sample-based copies), not for input attachment reads
or framebuffer fetch, and document unsupported MSAA sampling with an attributed
TODO(b/...). When implementing fallback copy paths (such as copy-as-draw),
verify that the target has a valid read view (TextureProxyView) before creating
an Image or scratch surface (aborting if null), and size scratch surfaces or
proxy textures to dstReadPixelBounds rather than the full target ImageInfo.

**Why.** Vulkan input attachments do not require texturable targets, while
non-texturable targets lack a TextureProxyView and crash if wrapped in an Image
during copy-as-draw without a null check. Furthermore, destination copies only
require the subset of pixels read by the draw pass; allocating full target
dimensions wastes VRAM and corrupts shader coordinate normalization, which
calculates UV reciprocal factors using destination copy dimensions.

## Examples

Bad:

```cpp
// Bad: asserting texturability unconditionally for all dst reads, including input attachments
SkASSERT(recorder->priv().caps()->isTexturable(fTarget->textureInfo()));

// Bad: unchecked read view and sizing copy scratch surface to full target ImageInfo
sk_sp<Image> srcImage(new Image(fReadView, fImageInfo.colorInfo()));
sk_sp<Surface> surface = Surface::MakeScratch(recorder, fImageInfo, "DstCopyFallback");
if (!surface) {
    return;
}
```

Good:

```cpp
// Good: assert texturability only when sampling dst; document MSAA limit
if (drawPassUsesDstSampler) {
    // TODO(b/390458117): Add support for MSAA texture dst reads on backends that support it.
    SkASSERT(recorder->priv().caps()->isTexturable(fTarget->textureInfo()));
}

// Good: verify valid read view and size scratch surface to dstReadPixelBounds
if (!fReadView) {
    SKGPU_LOG_W("DrawContext attempting CopyAsDraw without read view; dropping draw pass!");
    return;
}
sk_sp<Image> srcImage(new Image(fReadView, fImageInfo.colorInfo()));
SkImageInfo copyInfo = fImageInfo.makeDimensions(dstReadPixelBounds.size());
sk_sp<Surface> surface = Surface::MakeScratch(recorder, copyInfo, "DstCopyFallback");
if (!surface) {
    return;
}
```
