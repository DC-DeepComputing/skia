---
name: skia/api/name-saturating-arithmetic-helpers-with-sat
description: When defining saturating or clamping arithmetic helper functions, include an explicit 'Sat' or 'sat_' modifier in the identifier to distinguish clamped overflow from standard arithmetic
rubric: readability
severity: nit
---

**Rule.** When defining arithmetic helper functions that clamp or pin results to
minimum and maximum limits on overflow, include an explicit 'Sat' (in
UpperCamelCase, e.g., SkFixedSatAdd) or 'sat_' (in snake_case, e.g.,
Sk32_sat_add) in the function name. Do not use unqualified operation names (such
as SkFixedAdd) for operations that perform saturating arithmetic.

**Why.** Unqualified arithmetic names imply standard wrapping or unchecked
operations. Calling out saturation explicitly via 'Sat' or 'sat_' alerts callers
to the clamping behavior and adheres to existing Skia conventions like
Sk32_sat_add and Sk32_sat_sub.

## Examples

Bad:

```cpp
// BAD: Unqualified name implies standard addition without saturation
static inline SkFixed SkFixedAdd(SkFixed a, SkFixed b) {
    int64_t sum = (int64_t)a + b;
    return (SkFixed)(SkTPin<int64_t>(sum, SK_FixedMin, SK_FixedMax));
}
```

Good:

```cpp
// GOOD: Explicit 'Sat' or 'sat_' communicates clamping on overflow
static inline SkFixed SkFixedSatAdd(SkFixed a, SkFixed b) {
    int64_t sum = (int64_t)a + b;
    return (SkFixed)(SkTPin<int64_t>(sum, SK_FixedMin, SK_FixedMax));
}

static constexpr int32_t Sk32_sat_add(int32_t a, int32_t b) {
    return Sk64_pin_to_s32((int64_t)a + (int64_t)b);
}
```
