---
name: skia/document-conditions-dependent-on-upstream-state
description: When branching or adjusting state based on non-obvious conditions or proxy invariants established upstream, document what the condition represents in an explanatory comment
rubric: readability
severity: nit
---

**Rule.** When conditional branching or corrective logic inspects a non-obvious
condition or proxy invariant configured elsewhere (such as shader code checking
uniform relationships set up in host pipeline helpers), add an explanatory
comment. Explicitly document what upstream state or configuration the condition
represents and why the adjustment or branch is required so readers do not have
to inspect remote caller setup to understand local intent.

**Why.** Conditions that test proxy invariants or remote pipeline state are not
self-evident from local context alone. Without explanatory comments, maintainers
cannot tell whether unusual branch logic is deliberate or a workaround for a
bug, leading to accidental regressions or erroneous simplifications during
maintenance.

## Examples

Bad:

```cpp
if (filterModeY != filterModeUV) {
    coords = floor(coords) + 0.5;
}
```

Good:

```cpp
// If the filter modes are different between Y and UV, this means that
// the base filtermode is nearest and we have to snap the coords to Y's
// texel centers to get the correct positions for UV.
if (filterModeY != filterModeUV) {
    coords = floor(coords) + 0.5;
}
```
