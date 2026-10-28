---
name: skia/gpu/document-speculative-capability-checks
description: When querying speculative or forward-looking hardware capability combinations in GPU caps, document why unsupported feature combinations are queried and preserve rationale comments across backends
rubric: readability
severity: nit
---

**Rule.** When initializing GPU capabilities (such as in GrVkCaps or VulkanCaps)
with speculative or forward-looking checks querying hardware feature
combinations not supported by existing devices, document the defensive intent
with an explanatory comment. When mirroring capability logic between Ganesh and
Graphite, preserve accompanying rationale comments explaining future device
expectations.

**Why.** Querying hardware flag combinations or capability checks that no
current device supports looks like an API misunderstanding, dead code, or an
error without documentation. Explaining that the query is defensive for future
hardware clarifies the design intent and prevents accidental removal or
divergence when porting logic across backends.

## Examples

Bad:

```cpp
if (fSupportsProtectedContent) {
    requiredLazyFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
}
```

Good:

```cpp
if (fSupportsProtectedContent) {
    // If we have a protected context we can only use memoryless images if they also support
    // being protected. With current devices we don't actually expect this combination to be
    // supported, but this at least covers us for future devices that may allow it.
    requiredLazyFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
}
```
