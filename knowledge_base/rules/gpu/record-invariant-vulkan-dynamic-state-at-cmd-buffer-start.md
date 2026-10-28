---
name: skia/gpu/record-invariant-vulkan-dynamic-state-at-cmd-buffer-start
description: When configuring invariant Vulkan dynamic pipeline state, record commands like line width or depth bias at command buffer begin rather than inside render passes
rubric: safety
severity: should_fix
---

**Rule.** Record invariant Vulkan dynamic pipeline state commands (such as line
width, depth bias, depth bounds, and cull mode) once at command buffer start
rather than repeatedly inside each render pass. Reserve render-pass dynamic
state updates strictly for states that vary between passes or draws.

**Why.** In Vulkan, dynamic pipeline state commands can be recorded outside of
render passes and their values persist across render passes within the command
buffer. Recording invariant dynamic states once at command buffer begin
eliminates redundant command recording and driver overhead on every render pass.

## Examples

Bad:

```cpp
bool VulkanCommandBuffer::beginRenderPass(...) {
    VULKAN_CALL(fSharedContext->interface(),
                CmdBeginRenderPass(fPrimaryCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE));
    fActiveRenderPass = true;

    if (fSharedContext->caps()->useBasicDynamicState()) {
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetLineWidth(fPrimaryCommandBuffer, /*lineWidth=*/1.0f));
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetDepthBias(fPrimaryCommandBuffer, ...));
    }
}
```

Good:

```cpp
void VulkanCommandBuffer::begin() {
    VULKAN_CALL_ERRCHECK(fSharedContext,
                         BeginCommandBuffer(fPrimaryCommandBuffer, &cmdBufferBeginInfo));
    fActive = true;

    if (fSharedContext->caps()->useBasicDynamicState()) {
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetLineWidth(fPrimaryCommandBuffer, /*lineWidth=*/1.0f));
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetDepthBias(fPrimaryCommandBuffer, ...));
    }
}
```
