---
name: skia/gpu/qualify-candidate-probe-arrays
description: When declaring arrays or collections for sequential capability probing or fallback testing, name them with descriptive suffixes like kToCheck rather than bare plural nouns
rubric: readability
severity: nit
---

**Rule.** When declaring arrays or collections used for sequential hardware
capability probing, feature validation, or fallback testing, qualify the
identifier to reflect its query role (such as `k<Entity>ToCheck` or
`kCandidate<Entities>`). Do not use bare plural nouns (such as `kFamilies` or
`kFormats`) that imply an exhaustive domain enumeration.

**Why.** A bare plural noun like `kFamilies` implies an exhaustive collection of
all known instances in that domain. Qualifying the identifier clarifies that the
array is a prioritized candidate list to probe sequentially for runtime or
hardware support.

## Examples

Bad:

```cpp
// Bad: 'kFamilies' misleadingly suggests an exhaustive list of all Metal GPU families.
static constexpr MTLGPUFamily kFamilies[] = {
    MTLGPUFamilyApple9,
    MTLGPUFamilyApple8,
    MTLGPUFamilyApple7,
};
for (MTLGPUFamily family : kFamilies) {
    if ([device supportsFamily:family]) {
        fGPUFamily = family;
        return;
    }
}
```

Good:

```cpp
// Good: 'kFamiliesToCheck' clarifies that this is a prioritized candidate probe list.
static constexpr MTLGPUFamily kFamiliesToCheck[] = {
    MTLGPUFamilyApple9,
    MTLGPUFamilyApple8,
    MTLGPUFamilyApple7,
};
for (MTLGPUFamily family : kFamiliesToCheck) {
    if ([device supportsFamily:family]) {
        fGPUFamily = family;
        return;
    }
}
```
