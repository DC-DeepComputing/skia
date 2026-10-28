---
name: skia/clarify-subset-sampling-strictness-in-docs
description: When documenting image or subset shader sampling methods or constraint parameters, explicitly specify whether sampling behavior is 'strict' or 'non-strict' (fast)
rubric: readability
severity: nit
---

**Rule.** When documenting image or subset shader sampling methods, helpers, or
parameters that assume coordinates remain within bounds and omit edge clamping,
explicitly state whether sampling is 'strict' or 'non-strict' (or fast). Use
established domain terminology to clarify the edge-sampling contract alongside
descriptive coordinate assumptions.

**Why.** In Skia, boundary sampling behavior is formally categorized as strict
(using extra shader code or hardware constraints to prevent bleeding outside
bounds) versus non-strict or fast (unclamped sampling where coordinates are
guaranteed not to sample outside bounds). Using established domain terms makes
sampling contracts and performance trade-offs immediately clear to developers.

## Examples

Bad:

```cpp
/**
 * Create an SkShader that samples the contents of this special image, assuming that the
 * coords it's evaluated at will not access pixels beyond its subset.
 */
virtual sk_sp<SkShader> asShader(SkTileMode, const SkSamplingOptions&, bool strict = false) const;
```

Good:

```cpp
/**
 * Create an SkShader that samples the contents of this special image, assuming that the
 * coords it's evaluated at will not access pixels beyond its subset
 * (i.e., non-strict sampling).
 */
virtual sk_sp<SkShader> asShader(SkTileMode, const SkSamplingOptions&, bool strict = false) const;
```
