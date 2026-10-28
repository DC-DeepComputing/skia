---
name: skia/simplify-redundant-conditional-override-ternary
description: When conditionally overriding an existing setting or object using a ternary, fall back directly to the original value without redundant target checks
rubric: readability
severity: nit
---

**Rule.** When conditionally overriding an existing configuration, setting, or
layout object with a target value via a ternary expression, write `override_cond
? target_val : original_val` directly. Do not write convoluted compound
conditions checking whether the value already equals the target (such as
`(override || val == target) ? target : fallback`) or reconstruct the fallback
instance.

**Why.** Testing whether an incoming value already matches the target in
`(override || val == target) ? target : val` is logically redundant because
falling back directly to `val` preserves the target value when already set.
Passing the incoming object directly avoids redundant compound conditions and
avoids unnecessary object reconstruction or hardcoded fallback assumptions.

## Examples

Bad:

```cpp
// Redundant check whether layout already matches target, and reconstructed fallback:
const MemoryLayout arrayMemoryLayout =
        fCaps.fForceStd430ArrayLayout ||
        memoryLayout.getStandard() == MemoryLayout::Standard::k430
                ? MemoryLayout(MemoryLayout::Standard::k430)
                : MemoryLayout(MemoryLayout::Standard::k140);
```

Good:

```cpp
// Direct conditional override falling back to existing layout:
const MemoryLayout arrayMemoryLayout =
        fCaps.fForceStd430ArrayLayout
                ? MemoryLayout(MemoryLayout::Standard::k430)
                : memoryLayout;
```
