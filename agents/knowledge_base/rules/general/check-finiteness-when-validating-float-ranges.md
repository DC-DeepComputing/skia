---
name: skia/check-finiteness-when-validating-float-ranges
description: When validating floating-point coordinates against numeric thresholds for low-precision or fast paths, check finiteness to prevent NaN from falsely passing range checks
rubric: correctness
severity: should_fix
---

**Rule.** When testing floating-point coordinates, scalars, or bounds against
numeric range thresholds (such as [INT16_MIN, INT16_MAX] or fixed-point limits
for low-precision fast paths), explicitly verify finiteness using
`!std::isfinite(val)` (or `.isFinite()`). Do not rely solely on relational
inequality checks (such as `val > MAX || val < MIN`), as relational comparisons
against NaN always evaluate to false and allow non-finite values to bypass
out-of-range guards.

**Why.** Under IEEE 754 rules, relational comparisons (<, >, <=, >=) against NaN
evaluate to false. Out-of-bounds guards using only relational operators fail to
detect NaN, erroneously routing non-finite coordinates to low-precision or
fixed-point routines that cause undefined behavior, integer overflow, or
rendering artifacts.

## Examples

Bad:

```cpp
for (float val : SkSpan<const float>(deviceCoords.asScalars(), 4)) {
    if (val > INT16_MAX || val < INT16_MIN) {
        shouldUseHighPrecision = true;
        break;
    }
}
```

Good:

```cpp
for (float val : SkSpan<const float>(deviceCoords.asScalars(), 4)) {
    if (val > INT16_MAX || val < INT16_MIN || !std::isfinite(val)) {
        shouldUseHighPrecision = true;
        break;
    }
}
```
