---
name: skia/gpu/gpu-promote-shared-capability-queries-to-base-caps
description: When shared GPU subsystem logic or cross-backend tests query optional hardware features, promote capability accessors to base Caps with a default false fallback
rubric: design
severity: should_fix
---

**Rule.** When shared GPU subsystem logic or cross-backend tests need to query
or assert on an optional hardware or driver capability, expose or promote the
capability query on the base `Caps` class (with a safe default fallback, such as
`false`) rather than confining it to a backend-specific subclass (such as
`VulkanCaps` or `DawnCaps`). Do not use backend downcasts or ad-hoc `BackendApi`
branching in shared code or test suites to inspect backend-only capabilities.

**Why.** Placing capability queries on base Caps preserves subsystem abstraction
and allows cross-backend test harnesses and shared pipeline helpers to inspect
feature support uniformly without downcasting or branching on backend identity.
Providing a safe default (like false) in the base class avoids boilerplate
across backends that do not implement the feature.

## Examples

Bad:

```cpp
// In VulkanCaps.h:
bool supportsHostImageCopy() const { return fSupportsHostImageCopy; }

// In cross-backend test or shared GPU code:
if (context->backend() == BackendApi::kVulkan) {
    auto vkCaps = static_cast<const VulkanCaps*>(context->priv().caps());
    if (!vkCaps->supportsHostImageCopy()) {
        return;
    }
}
```

Good:

```cpp
// In base Caps.h:
bool supportsHostImageCopy() const { return fSupportsHostImageCopy; }
protected:
bool fSupportsHostImageCopy = false;

// In cross-backend test or shared GPU code:
if (!context->priv().caps()->supportsHostImageCopy()) {
    return;
}
```
