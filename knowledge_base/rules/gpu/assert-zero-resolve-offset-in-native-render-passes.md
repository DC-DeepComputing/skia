---
name: skia/gpu/assert-zero-resolve-offset-in-native-render-passes
description: When configuring Graphite backend render passes that assume 1:1 multisample-to-resolve alignment, assert with SkASSERT that attachment offsets are zero using resolveOffset.isZero()
rubric: correctness
severity: should_fix
---

**Rule.** In Graphite backend CommandBuffer render pass implementations (such as
onAddRenderPass or DawnCommandBuffer pass setup), assert with SkASSERT that
resolveOffset.isZero() on native resolve paths that assume 1:1 coordinate
alignment between multisample and resolve attachments. Do not execute native
resolve passes with unasserted attachment offsets when backend extensions or
emulation are required to handle non-zero translations.

**Why.** Native GPU backend render passes typically resolve multisampled
attachments directly into resolve targets at identical coordinates without
coordinate shifting. If a render pass with a non-zero resolveOffset executes
through a native path that lacks offset support instead of triggering emulation
or extension offsets, resolves will be misaligned, producing clipped or
corrupted rendering.

## Examples

Bad:

```cpp
// In backend CommandBuffer render pass setup:
if (fSharedContext->dawnCaps()->supportsPartialLoadResolve()) {
    wgpuPartialRect.x = renderPassBounds.x();
    wgpuPartialRect.y = renderPassBounds.y();
    wgpuPartialRect.width = renderPassBounds.width();
    wgpuPartialRect.height = renderPassBounds.height();
    wgpuRenderPass.nextInChain = &wgpuPartialRect;
} else {
    // Native path assumes 1:1 alignment but does not assert zero resolveOffset
    wgpuRenderPass.colorAttachments = &wgpuColorAttachment;
}
```

Good:

```cpp
// In backend CommandBuffer render pass setup:
if (fSharedContext->dawnCaps()->supportsPartialLoadResolve()) {
    ...
} else {
    // Assert 1:1 alignment invariant on native path
    SkASSERT(resolveOffset.isZero());
    wgpuRenderPass.colorAttachments = &wgpuColorAttachment;
}
```
