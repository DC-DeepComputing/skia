---
name: skia/gpu/document-capability-noop-semantics
description: When declaring backend helper methods that handle capability checks internally, document that they safely no-op when unsupported to clarify unconditional invocation
rubric: readability
severity: nit
---

**Rule.** When declaring backend helper or pipeline methods that encapsulate
capability checks or dynamic feature availability internally, document in the
declaration comment that the method is safe to invoke unconditionally.
Explicitly state that the method safely acts as a no-op when the underlying
capability or dynamic state is unsupported or absent, clarifying caller
expectations.

**Why.** Without explicit declaration comments, callers cannot deduce whether a
method requires pre-checking Caps or handles unsupported configurations
internally. Clarifying unconditional no-op fallback behavior prevents callers
from adding redundant capability checks or assuming the call is invalid on
unsupported hardware.

## Examples

Bad:

```cpp
// Bad: callers cannot tell whether they must verify caps before calling.
void updateDynamicState(const VulkanSharedContext*,
                        VkCommandBuffer commandBuffer,
                        const VulkanGraphicsPipeline* previous) const;
```

Good:

```cpp
// Good: declaration explicitly documents safe no-op behavior when unsupported.
// Update any dynamic state (including none, if dynamic state is not available) that has changed
// since the previously bound Graphite pipeline.
void updateDynamicState(const VulkanSharedContext*,
                        VkCommandBuffer commandBuffer,
                        const VulkanGraphicsPipeline* previous) const;
```
