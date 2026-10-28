---
name: skia/gpu/avoid-caching-low-frequency-command-buffer-state
description: When setting low-frequency GPU command buffer state like viewports or scissors once per render pass, avoid caching member variables and redundant equality guards
rubric: design
severity: should_fix
---

**Rule.** Do not introduce caching member variables (such as fCurrentViewport)
or equality guards in GPU CommandBuffer backends to skip low-frequency state
updates set once per render pass. Set state unconditionally unless profiling
demonstrates that redundant API calls create measurable CPU overhead.

**Why.** Low-frequency dynamic states such as viewport or scissor rects are
typically set once per render pass, making their CPU recording overhead
negligible. Tracking previous values in member fields introduces unnecessary
mutable state, extra branch comparisons, and synchronization risks across
command buffers without measurable performance gains.

## Examples

Bad:

```cpp
// In VulkanCommandBuffer.h
SkIRect fCurrentViewport;

// In VulkanCommandBuffer.cpp
void VulkanCommandBuffer::setViewport(SkIRect viewport) {
    if (viewport == fCurrentViewport) {
        return;
    }
    VkViewport vkViewport = { ... };
    VULKAN_CALL(..., CmdSetViewport(..., &vkViewport));
    fCurrentViewport = viewport;
}
```

Good:

```cpp
void VulkanCommandBuffer::setViewport(SkIRect viewport) {
    VkViewport vkViewport = { ... };
    VULKAN_CALL(..., CmdSetViewport(..., &vkViewport));
}
```
