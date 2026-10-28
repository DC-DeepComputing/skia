---
name: skia/codec/support-unsigned-numerators-in-rational-helpers
description: When implementing rational or fraction-to-float conversion helpers in image codecs, do not restrict numerators to int32_t; support uint32_t via templates or overloads
rubric: correctness
severity: should_fix
---

**Rule.** When implementing helpers to convert rational numbers
(numerator/denominator pairs) to floating-point in image codecs or metadata
parsers, do not restrict the numerator parameter to `int32_t` if any metadata
fields use `uint32_t`. Use a template parameter or dedicated overloads so both
signed and unsigned numerators are handled without implicit narrowing.

**Why.** In image metadata specifications like ISO 21496-1 and AVIF gain maps,
some rational fields use signed numerators while others use unsigned numerators.
Restricting the numerator to `int32_t` causes implicit narrowing of `uint32_t`
values, leading to signed integer overflow and sign inversion for values
exceeding INT32_MAX.

## Examples

Bad:

```cpp
float FractionToFloat(int32_t numerator, uint32_t denominator) {
    if (denominator == 0) {
        return 0.f;
    }
    return static_cast<double>(numerator) / denominator;
}
```

Good:

```cpp
template <typename NumeratorType>
float FractionToFloat(NumeratorType numerator, uint32_t denominator) {
    if (denominator == 0) {
        return 0.f;
    }
    return static_cast<double>(numerator) / denominator;
}
```
