---
name: skia/preserve-distinguishing-feature-suffixes
description: When naming variables, parameters, or configuration flags for specialized features or sub-paths, name them specifically after the concrete feature rather than using generic names or truncating suffixes
rubric: readability
severity: should_fix
---

**Rule.** When declaring variables, parameters, or configuration/binding flags
representing a specialized feature, sub-path, or resource category, name the
identifier specifically after the concrete feature, resource, or condition being
governed (such as renderLazyPictureTilesOnGPU, useDrawListLayer, or
fUsePushConstantsForIntrinsicConstants). Do not use broad, generic names (such
as shouldUseSubsets or fUseVulkanPushConstants) or truncate identifiers to match
base concepts (such as useDrawList) that falsely imply a wider scope across the
entire operation, hardware capability, or subsystem. Alternatively, inline
single-use capability checks directly into condition expressions.

**Why.** Generic parameter names, configuration flags, and truncated identifiers
falsely imply that a flag controls a general subsystem, global hardware
capability, or default path rather than a specialized sub-operation or specific
resource category. Explicitly identifying the concrete feature, resource, or
condition makes function signatures, structs, and call sites self-documenting,
clarifies what sub-path is actually modified, and prevents misconceptions about
the flag's scope.

## Examples

Bad:

```cpp
// Bad: 'useDrawList' implies controlling the entire DrawList system, but checks DrawListLayer.
bool useDrawList = device->fRecorder->priv().caps()->useDrawListLayer();
if (useDrawList) {
    ...
}

// Bad: 'shouldUseSubsets' implies general tiling subsetting, but only controls lazy picture tiles on GPU.
static std::tuple<bool, size_t> DrawAsTiledImageRect(..., bool shouldUseSubsets);

// Bad: 'fUseVulkanPushConstants' implies a global hardware capability toggle rather than intrinsic constants.
struct ResourceBindingRequirements {
    bool fUseVulkanPushConstants = false;
};
```

Good:

```cpp
// Good: Preserves distinguishing suffix matching the capability and feature class.
bool useDrawListLayer = device->fRecorder->priv().caps()->useDrawListLayer();
if (useDrawListLayer) {
    ...
}
// Also good: Inlines single-use capability check directly.
if (device->fRecorder->priv().caps()->useDrawListLayer()) {
    ...
}

// Good: Names the specific sub-feature and condition being governed.
static std::tuple<bool, size_t> DrawAsTiledImageRect(..., bool renderLazyPictureTilesOnGPU = true);

// Good: Explicitly names the specific resource or sub-feature being configured.
struct ResourceBindingRequirements {
    bool fUseVulkanPushConstantsForIntrinsicConstants = false;
};
```
