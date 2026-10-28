---
name: skia/gpu/assert-max-supported-api-version-in-feature-promotion
description: When initializing backend feature promotion helpers with caller API versions, assert that the version does not exceed the maximum supported version
rubric: correctness
severity: should_fix
---

**Rule.** When initializing or configuring backend feature promotion helpers
that branch on core API versions, assert with SkASSERT that caller-provided API
versions do not exceed the maximum version supported by the promotion logic
(e.g., SkASSERT(apiVersion <= VK_API_VERSION_1_4)). Do not assert only a lower
baseline bound without bounding the upper version.

**Why.** Feature promotion and extension negotiation logic branches on known
core API versions to replace separate extensions with core feature structs. If a
caller passes an unhandled newer API version, execution falls through without
necessary promotion checks, causing silent misconfiguration.

## Examples

Bad:

```cpp
void VulkanPreferredFeatures::init(uint32_t appAPIVersion) {
    fAPIVersion = appAPIVersion;
    // Only verifies minimum supported baseline
    SkASSERT(fAPIVersion >= VK_API_VERSION_1_1);
}
```

Good:

```cpp
void VulkanPreferredFeatures::init(uint32_t appAPIVersion) {
    fAPIVersion = appAPIVersion;
    SkASSERT(fAPIVersion >= VK_API_VERSION_1_1);
    // Alert maintainers if caller passes a higher version needing updated promotion logic
    SkASSERT(fAPIVersion <= VK_API_VERSION_1_4);
}
```
