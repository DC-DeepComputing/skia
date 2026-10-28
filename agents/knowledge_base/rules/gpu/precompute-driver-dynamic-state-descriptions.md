---
name: skia/gpu/precompute-driver-dynamic-state-descriptions
description: When configuring GPU dynamic pipeline states, precompute driver description structs at pipeline creation time rather than rebuilding them during command recording
rubric: safety
severity: should_fix
---

**Rule.** Precompute driver dynamic state structures (such as Vulkan dynamic
vertex input binding and attribute descriptions) during graphics pipeline
creation and store them as pipeline members. Do not rebuild, reallocate, or
format driver dynamic state descriptors on the fly during command buffer
recording or dynamic state updates.

**Why.** Pipeline binding and dynamic state updates execute on the command
recording hot path, where frequent pipeline switches occur during rendering.
Rebuilding driver descriptor structs per bind incurs repeated CPU allocation,
traversal, and formatting overhead that slows command recording.

## Examples

Bad:

```cpp
void VulkanGraphicsPipeline::updateDynamicState(VulkanCommandBuffer* cmdBuffer, ...) {
    if (vertexInputDirty) {
        skia_private::STArray<2, VkVertexInputBindingDescription2EXT> bindingDescs;
        skia_private::STArray<16, VkVertexInputAttributeDescription2EXT> attributeDescs;
        get_vertex_input_state(..., &bindingDescs, &attributeDescs);
        CmdSetVertexInput(cmdBuffer->vkCommandBuffer(),
                          bindingDescs.size(), bindingDescs.begin(),
                          attributeDescs.size(), attributeDescs.begin());
    }
}
```

Good:

```cpp
// Precomputed during MakePipeline() and stored in member arrays
void VulkanGraphicsPipeline::updateDynamicState(VulkanCommandBuffer* cmdBuffer, ...) {
    if (vertexInputDirty) {
        CmdSetVertexInput(cmdBuffer->vkCommandBuffer(),
                          fVertexBindingDescriptions.size(),
                          fVertexBindingDescriptions.begin(),
                          fVertexAttributeDescriptions.size(),
                          fVertexAttributeDescriptions.begin());
    }
}
```
