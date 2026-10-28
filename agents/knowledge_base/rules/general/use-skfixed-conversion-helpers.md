---
name: skia/use-skfixed-conversion-helpers
description: When implementing fixed-point math, use SkFixed typedefs and SkFixed.h conversion helpers; document invariants if bypassing standard helpers like SkFloatToFixed for performance
rubric: readability
severity: should_fix
---

**Rule.** When implementing fixed-point math, declare variables, parameters,
return types, and constants using semantic fixed-point typedefs (such as SkFixed
for 16.16 and SkFixed3232 for 32.32) rather than raw primitive integer types
(int32_t, int64_t), and use SkFixed.h conversion helpers (such as SkIntToFixed,
SkIntToFixed3232, SkFixed3232ToFixed, and SkFixedFloorToInt) instead of manual
bit shifts (<< 16, << 32). If deliberately bypassing standard conversion helpers
like SkFloatToFixed in performance-critical code to avoid saturation checks
(sk_float_saturate2int), add an inline comment explaining why the helper is
omitted and the mathematical invariant guaranteeing values will not overflow or
underflow.

**Why.** Primitive integer types and raw bit shifts obscure fixed-point scale
and precision, risking confusion between fixed-point formats and integer counts,
as well as undefined behavior or sign truncation. Standard helpers like
SkFloatToFixed centralize scale factors and saturation checks
(sk_float_saturate2int); when omitted in inner loops for performance,
documenting why and the non-overflow invariant prevents reviewer confusion and
ensures the unchecked cast is safe.

## Examples

Bad:

```cpp
// Bad: Raw integer types, manual shifts, and undocumented helper bypass
static constexpr int64_t max_fx = ((int64_t)INT16_MAX) << 32;
static constexpr int32_t sk_saturate2fixed(int64_t x) {
    return (int32_t)(x >> 16);
}

// Undocumented float-to-int cast bypassing SkFloatToFixed saturation
constexpr float kFixedMult = 64.0f * 65536.0f;
int32_t stepXFixed = static_cast<int32_t>(stepX * kFixedMult);
```

Good:

```cpp
// Good: Semantic fixed-point typedefs and SkFixed.h conversion helpers
static constexpr SkFixed3232 max_fx = SkIntToFixed3232(INT16_MAX);
static constexpr SkFixed sk_saturate2fixed(SkFixed3232 x) {
    return SkFixed3232ToFixed(x);
}

// Documented bypass of SkFloatToFixed saturation for guaranteed ranges
// Note: Direct static_cast avoids SkFloatToFixed saturation checks;
// stepX is mathematically guaranteed not to overflow or underflow.
constexpr float kFixedMult = 64.0f * 65536.0f;
int32_t stepXFixed = static_cast<int32_t>(stepX * kFixedMult);
```
