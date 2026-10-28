---
name: skia/modules/omit-redundant-finite-checks-in-float-equality
description: When implementing exact floating-point equality that treats NaNs as equal, omit redundant SkScalarIsFinite checks and rely on operator== for finite values and infinities
rubric: readability
severity: should_fix
---

**Rule.** When implementing exact floating-point equality that considers NaNs
equal (such as for cache keys), do not write redundant SkScalarIsFinite or
SkIsFinite guards before the equality expression. Rely on operator== to match
both finite values and signed infinities (+Inf == +Inf, -Inf == -Inf), combining
it with NaN predicates like (SkIsNaN(x) && SkIsNaN(y)) without preceding
finiteness checks.

**Why.** In IEEE 754 arithmetic, operator== already evaluates to true for
matching finite values and matching infinities (+Inf and -Inf). Guarding an
equality check with SkScalarIsFinite or SkIsFinite introduces redundant
branching that duplicates the work of operator==.

## Examples

Bad:

```cpp
if (SkScalarIsFinite(x) && SkScalarIsFinite(y)) {
    return x == y;
}
return x == y || (SkIsNaN(x) && SkIsNaN(y));
```

Good:

```cpp
return x == y || (SkIsNaN(x) && SkIsNaN(y));
```
