---
name: skia/sksl/validate-float-constant-fold-range
description: When constant-folding floating-point expressions or reciprocals in SkSL, verify the evaluated double fits within target float bounds rather than checking input non-zero
rubric: correctness
severity: should_fix
---

**Rule.** When constant-folding floating-point expressions, matrix
multiplications, or reciprocals using double precision in SkSL, verify that the
computed double fits within the target float representation's valid range (e.g.
*value >= -FLT_MAX && *value <= FLT_MAX or minimumValue() / maximumValue(), plus
*value != 0.0 for division reciprocals). Do not rely solely on checking that
input operands were non-zero or finite before evaluation.

**Why.** A non-zero floating-point divisor (such as a denormal or very small
value) can cause reciprocal division to overflow to infinity in double or
produce an in-range double that exceeds the finite range of a 32-bit float
(FLT_MAX). Verifying that the evaluated result satisfies *value >= -FLT_MAX &&
*value <= FLT_MAX && *value != 0.0 simultaneously guards against overflow to
infinity, underflow to zero, exceeding single-precision float bounds, and NaNs
(since relational comparisons with NaN evaluate to false).

## Examples

Bad:

```cpp
if (!value || !std::isfinite(*value) || *value == 0.0f) {
    return nullptr;
}
exprs.push_back(Literal::Make(pos, 1.0 / *value, &type));
```

Good:

```cpp
*value = sk_ieee_double_divide(1.0, *value);
if (*value >= -FLT_MAX && *value <= FLT_MAX && *value != 0.0) {
    exprs.push_back(Literal::Make(pos, *value, &type));
} else {
    return nullptr;
}
```
