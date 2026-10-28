---
name: skia/when-adding-parameters-to-internal-functions-to-gate-optimiz
description: When adding parameters to internal functions to gate optimizations or hardware workarounds, provide default arguments representing the standard enabled behavior
rubric: readability
severity: nit
---

**Rule.** When adding a parameter to disable an optimization or feature for a
hardware workaround or fallback, provide a default argument representing the
standard, desired behavior (such as `= true`). Emphasize that the feature should
normally be active while allowing exceptional callers to override it without
updating typical call sites.

**Why.** Defaulting workaround or fallback parameters to their standard state
documents the baseline behavior, emphasizes that disabling the feature is
exceptional, and avoids requiring typical callers across the subsystem to supply
the argument explicitly.

## Examples

Bad:

```cpp
// Bad: Omitting default argument forces standard callers to supply the baseline value
static std::tuple<bool, size_t> DrawAsTiledImageRect(
        SkCanvas* canvas,
        const SkImage* image,
        ...,
        bool renderLazyPictureTilesOnGPU);
```

Good:

```cpp
// Good: Default argument documents baseline behavior and avoids caller churn
static std::tuple<bool, size_t> DrawAsTiledImageRect(
        SkCanvas* canvas,
        const SkImage* image,
        ...,
        bool renderLazyPictureTilesOnGPU = true);
```
