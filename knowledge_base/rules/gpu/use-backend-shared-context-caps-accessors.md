---
name: skia/gpu/use-backend-shared-context-caps-accessors
description: When retrieving backend capabilities from Graphite SharedContext subclasses, call backend-specific caps accessors like dawnCaps(), mtlCaps(), or vulkanCaps() instead of manually casting caps()
rubric: readability
severity: should_fix
---

**Rule.** When accessing backend-specific Caps from a Graphite SharedContext
subclass (such as DawnSharedContext, MtlSharedContext, or VulkanSharedContext),
invoke the backend's typed caps accessor (dawnCaps(), mtlCaps(), or
vulkanCaps()). Do not call caps() and manually downcast the returned base
pointer or reference with static_cast.

**Why.** Graphite's backend-specific SharedContext subclasses provide inline
accessors that encapsulate downcasting to their concrete Caps subclass. Calling
these accessors reduces boilerplate, improves readability, and avoids redundant
manual casts across backend implementations.

## Examples

Bad:

```cpp
// In Dawn backend implementation:
const auto* caps = static_cast<const DawnCaps*>(sharedContext->caps());

// In Vulkan backend implementation:
const auto& vkCaps = static_cast<const VulkanCaps&>(*sharedContext->caps());
```

Good:

```cpp
// In Dawn backend implementation:
const auto* caps = sharedContext->dawnCaps();

// In Vulkan backend implementation:
const auto& vkCaps = sharedContext->vulkanCaps();
```
