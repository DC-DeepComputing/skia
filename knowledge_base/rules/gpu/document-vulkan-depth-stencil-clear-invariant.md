---
name: skia/gpu/document-vulkan-depth-stencil-clear-invariant
description: When initializing VkClearValue for depth/stencil attachments in Vulkan, document that VkClearDepthStencilValue accepts both depth and stencil clear values unconditionally
rubric: readability
severity: should_fix
---

**Rule.** When initializing VkClearValue for depth/stencil attachments in Vulkan
render pass setup, document with an explanatory comment that
VkClearDepthStencilValue accepts both depth and stencil clear values
unconditionally regardless of whether the attachment format has a single aspect
or both. Do not add branching or format aspect checks when assigning
depthStencil clear values.

**Why.** Unlike graphics APIs such as Metal or WebGPU where depth and stencil
clear values are assigned separately based on attachment aspects, Vulkan uses a
single combined VkClearDepthStencilValue struct and automatically ignores the
unused aspect field on single-aspect attachments. Documenting this invariant
clarifies why the assignment does not branch and prevents confusion for
maintainers accustomed to other backends.

## Examples

Bad:

```cpp
// Resolve texture does not have a clear value
if (rpDesc.fDepthStencilAttachment.fFormat != TextureFormat::kUnsupported) {
    VkClearValue& depthStencilClear = clearValues->push_back();
    depthStencilClear.depthStencil = {rpDesc.fClearDepth, rpDesc.fClearStencil};
}
```

Good:

```cpp
// Vulkan takes the clear depth and clear stencil regardless of whether or not the DS attachment
// only has a single aspect or both.
if (rpDesc.fDepthStencilAttachment.fFormat != TextureFormat::kUnsupported) {
    VkClearValue& depthStencilClear = clearValues->push_back();
    depthStencilClear.depthStencil = {rpDesc.fClearDepth, rpDesc.fClearStencil};
}
```
