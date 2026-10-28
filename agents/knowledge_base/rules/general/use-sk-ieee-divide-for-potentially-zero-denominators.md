---
name: skia/use-sk-ieee-divide-for-potentially-zero-denominators
description: When performing floating-point division where denominators may be zero or unvalidated, use sk_ieee_float_divide or sk_ieee_double_divide instead of operator/
rubric: readability
severity: should_fix
---

**Rule.** When performing floating-point division where the denominator may be
zero or where division is executed prior to checking finiteness, use
sk_ieee_float_divide or sk_ieee_double_divide from SkFloatingPoint.h instead of
the raw '/' operator. Do not perform blind division using raw '/' and only
validate finiteness or validity on the result afterwards.

**Why.** Under IEEE 754, floating-point division by zero yields +/-infinity or
NaN. However, C and C++ compilers (such as MSVC warning C4723) and sanitizers
(Clang UBSan float-divide-by-zero) flag float division by zero as an error.
Skia's sk_ieee_float_divide and sk_ieee_double_divide suppress these sanitizer
diagnostics with SK_NO_SANITIZE, allowing code to divide blindly and inspect the
result downstream without tripping sanitizer aborts.

## Examples

Bad:

```cpp
// Blind division with raw '/' can trigger UBSan or compiler divide-by-zero warnings
double recip = 1.0 / *value;
if (std::isfinite(recip) && recip != 0.0) {
    values[index] = recip;
}
```

Good:

```cpp
// Use sk_ieee_double_divide to safely divide and inspect finiteness downstream
double recip = sk_ieee_double_divide(1.0, *value);
if (std::isfinite(recip) && recip != 0.0) {
    values[index] = recip;
}
```
