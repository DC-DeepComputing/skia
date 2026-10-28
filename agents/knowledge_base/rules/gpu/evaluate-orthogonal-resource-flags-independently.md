---
name: skia/gpu/evaluate-orthogonal-resource-flags-independently
description: When configuring GPU resource usage or creation flags, evaluate independent feature conditions in separate if statements rather than an else-if chain
rubric: correctness
severity: should_fix
---

**Rule.** When determining or configuring GPU resource flags (such as texture
usage or creation flags across Vulkan, Dawn, or Metal), evaluate independent
capabilities and properties in separate `if` statements rather than an `if ...
else if` chain. Ensure orthogonal flags—such as transient/memoryless attachment
flags and multisampled-render-to-single-sampled flags—can be set simultaneously
when multiple conditions are satisfied.

**Why.** Chaining orthogonal flag checks in an `if ... else if` ladder makes
them mutually exclusive. Textures satisfying an earlier condition (such as
discardable or memoryless attachments) are unintentionally prevented from
receiving independent creation or usage flags required by other features.

## Examples

Bad:

```cpp
if (discardable == Discardable::kYes && fSupportsMemorylessAttachments) {
    vkUsage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
} else if (desc.fSampleCount == 1 && this->msaaRenderToSingleSampledSupport()) {
    createFlags |= VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
}
```

Good:

```cpp
if (discardable == Discardable::kYes && fSupportsMemorylessAttachments) {
    vkUsage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
}
if (desc.fSampleCount == 1 && this->msaaRenderToSingleSampledSupport()) {
    createFlags |= VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
}
```
