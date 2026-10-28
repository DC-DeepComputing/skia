---
name: skia/prefer-isnan-over-self-comparison
description: When checking floating-point values for NaN, use dedicated predicates like SkIsNaN or std::isnan instead of self-equality comparisons like x == x or x != x
rubric: readability
severity: should_fix
---

**Rule.** When checking whether a floating-point value is NaN (or verifying
non-NaN), use an explicit predicate function such as SkIsNaN(x) or
std::isnan(x). Do not test for NaN using self-comparison expressions like x != x
or x == x.

**Why.** Comparing a floating-point value against itself (x != x or x == x)
relies on IEEE 754 NaN comparison semantics that obscure intent and confuse
readers. Explicit predicates like SkIsNaN or std::isnan make the check
self-documenting, avoid compiler diagnostic quirks or optimization subtleties,
and maintain consistency across the codebase.

## Examples

Bad:

```cpp
if (fStartT != fStartT || fEndT != fEndT) {
    return false;
}
```

Good:

```cpp
if (SkIsNaN(fStartT) || SkIsNaN(fEndT)) {
    return false;
}
```
