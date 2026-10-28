---
name: skia/quantization-tolerance-half-step-size
description: When caching or comparing quantized float parameters, use canonical descriptor quantization helpers instead of ad-hoc scaling, or set tolerances to half the step size
rubric: design
severity: should_fix
---

**Rule.** When packing floating-point parameters into cache keys, indexing
records, or performing equality checks on quantized values, use canonical
quantization and packing helpers from the corresponding descriptor type rather
than open-coding ad-hoc scaling, truncation, or uniform float tolerances. When
comparing floating-point values with a tolerance to account for quantization
rounding error of step size Δ (such as an N-bit integer encoding), set the
tolerance to at most half the step size (0.5 * Δ) rather than the full step
size.

**Why.** Ad-hoc scaling and truncation in cache keys duplicate tricky packing
logic and risk mismatches against the descriptor records they represent.
Furthermore, uniform float tolerances ignore parameter bit depths, while a full
step size tolerance spans adjacent discrete buckets and matches distinct values;
rounding introduces an error of at most half the quantization step size (0.5 *
Δ).

## Examples

Bad:

```cpp
// Ad-hoc scaling and truncation when packing cache keys:
const uint32_t asKey() const {
    constexpr float kMultiplier = 32;
    const uint32_t contrast_part = SkScalarTruncToInt(contrast * kMultiplier) & 0xFF;
    const uint32_t device_gamma_part = (SkScalarTruncToInt(deviceGamma * kMultiplier) & 0xFF) << 16;
    return contrast_part | device_gamma_part;
}

// Full step size tolerance Δ matches distinct discrete values:
constexpr SkScalar tolerance = 1.0f / ((1 << 8) - 1);
if (SkScalarNearlyEqual(val, kTarget, tolerance)) { ... }
```

Good:

```cpp
// Use canonical quantization helpers from the descriptor type:
const uint32_t asKey() const {
    const uint32_t contrast_part = SkScalerContextRec::InternalContrastFromExternal(contrast);
    const uint32_t gamma_part = SkScalerContextRec::InternalGammaFromExternal(gamma) << 8;
    return contrast_part | gamma_part;
}

// Tolerance at most half the step size (0.5 * Δ) accounts only for rounding:
constexpr SkScalar tolerance = 0.5f / ((1 << 8) - 1);
if (SkScalarNearlyEqual(val, kTarget, tolerance)) { ... }
```
