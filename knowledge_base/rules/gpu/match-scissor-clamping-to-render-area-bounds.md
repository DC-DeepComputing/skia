---
name: skia/gpu/match-scissor-clamping-to-render-area-bounds
description: When clamping scissor rects or culling draw passes in Graphite CommandBuffer backends, clamp to fRenderAreaBounds for backends with render areas and fRenderTargetBounds otherwise
rubric: correctness
severity: should_fix
---

**Rule.** In Graphite CommandBuffer backend implementations, ensure scissor
clipping and draw pass culling use the bounds matching the backend's render area
capabilities. For backends with explicit render areas (such as Vulkan and Dawn),
clamp scissor rectangles to fRenderAreaBounds to avoid validation errors; for
backends without explicit render areas (such as Metal), consistently use
fRenderTargetBounds.

**Why.** APIs like Vulkan and Dawn require scissor rectangles to stay strictly
within the render pass render area bounds, triggering validation layer errors if
scissors exceed fRenderAreaBounds. Conversely, backends like Metal do not
establish render areas, so mixing fRenderAreaBounds into scissor clipping causes
inconsistencies with draw pass culling that operates on fRenderTargetBounds.

## Examples

Bad:

```cpp
void MtlCommandBuffer::setScissor(const Scissor& scissor) {
    SkASSERT(fActiveRenderCommandEncoder);

    SkIRect rect = scissor.getRect(fReplayTranslation, fRenderAreaBounds);
    fDrawIsOffscreen = rect.isEmpty();
    if (!fDrawIsOffscreen) {
        [fActiveRenderCommandEncoder setScissorRect:make_mtl_scissor(rect)];
    }
}
```

Good:

```cpp
void MtlCommandBuffer::setScissor(const Scissor& scissor) {
    SkASSERT(fActiveRenderCommandEncoder);

    SkIRect rect = scissor.getRect(fReplayTranslation, fRenderTargetBounds);
    fDrawIsOffscreen = rect.isEmpty();
    if (!fDrawIsOffscreen) {
        [fActiveRenderCommandEncoder setScissorRect:make_mtl_scissor(rect)];
    }
}
```
