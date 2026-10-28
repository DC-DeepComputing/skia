---
name: skia/omit-redundant-isfinite-in-float-equality
description: When comparing floating-point numbers for equality or equivalence, omit redundant isfinite checks because operator== already compares finite values and infinities identically
rubric: readability
severity: should_fix
---

**Rule.** Do not add redundant isfinite checks (such as SkScalarIsFinite or
std::isfinite) when testing floating-point values for equality or equivalence.
In IEEE 754 arithmetic, operator== evaluates to true for matching finite values
as well as matching infinities (+Inf == +Inf and -Inf == -Inf); only NaN values
require explicit predicates (such as SkScalarIsNaN or std::isnan) if an
equivalence relation requires NaNs to compare equal.

**Why.** Branching on isfinite before evaluating == adds unnecessary control
flow based on the misconception that infinities cannot be compared with
operator==. Relying directly on operator== handles both finite values and signed
infinities uniformly without redundant guards.

## Examples

Bad:

```cpp
bool exactlyEqual(SkScalar x, SkScalar y) {
    if (SkScalarIsFinite(x) && SkScalarIsFinite(y)) {
        return x == y;
    }
    // Inf == Inf, anything else is false
    return x == y || (SkScalarIsNaN(x) && SkScalarIsNaN(y));
}
```

Good:

```cpp
bool exactlyEqual(SkScalar x, SkScalar y) {
    // operator== handles finite numbers and infinities; handle NaN if required.
    return x == y || (SkScalarIsNaN(x) && SkScalarIsNaN(y));
}
```
