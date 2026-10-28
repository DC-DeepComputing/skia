---
name: skia/factor-repeated-expressions-into-lambdas
description: When evaluating repeated non-trivial boolean expressions, coordinate math, or pairwise predicates across multiple elements within a function, extract the operation into a descriptive named local lambda
rubric: design
severity: should_fix
---

**Rule.** When a non-trivial boolean expression, coordinate check, or arithmetic
calculation is evaluated repeatedly across multiple operands or element pairs
within a function, extract the logic into a descriptive named local lambda or
helper function. Invoke the named helper with each operand pair rather than
duplicating identical bitwise, relational, or mathematical subexpressions
inline.

**Why.** Repeating non-trivial bitwise, relational, or mathematical expressions
across multiple variable pairs clutters the enclosing algorithm, increases
cognitive load, and invites copy-paste errors. Encapsulating the repeated
calculation in a named local lambda documents intent clearly while compilers
inline the call without runtime overhead.

## Examples

Bad:

```cpp
if (!(
    // Non-zero, axis-aligned vectors.
    ((v0.fX == 0) ^ (v0.fY == 0)) &
    ((v1.fX == 0) ^ (v1.fY == 0)) &
    ((v2.fX == 0) ^ (v2.fY == 0)) &
    ((v3.fX == 0) ^ (v3.fY == 0)) &

    // Check orthogonal adjacent vectors in alternating dimensions.
    ((v0.fX == 0) ^ (v1.fX == 0)) & ((v0.fY == 0) ^ (v1.fY == 0)) &
    ((v1.fX == 0) ^ (v2.fX == 0)) & ((v1.fY == 0) ^ (v2.fY == 0)) &
    ((v2.fX == 0) ^ (v3.fX == 0)) & ((v2.fY == 0) ^ (v3.fY == 0))
)) {
    return std::nullopt;
}
```

Good:

```cpp
const auto axis_aligned_orthogonal = [](const SkVector& a, const SkVector& b) {
    return ((a.fX == 0) ^ (b.fX == 0)) &
           ((a.fY == 0) ^ (b.fY == 0));
};

if (!(
    // Non-zero, axis-aligned vectors.
    ((v0.fX == 0) ^ (v0.fY == 0)) &
    ((v1.fX == 0) ^ (v1.fY == 0)) &
    ((v2.fX == 0) ^ (v2.fY == 0)) &
    ((v3.fX == 0) ^ (v3.fY == 0)) &

    axis_aligned_orthogonal(v0, v1) &
    axis_aligned_orthogonal(v1, v2) &
    axis_aligned_orthogonal(v2, v3)
)) {
    return std::nullopt;
}
```
