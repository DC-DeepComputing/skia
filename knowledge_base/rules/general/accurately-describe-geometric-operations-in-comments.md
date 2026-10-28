---
name: skia/accurately-describe-geometric-operations-in-comments
description: When commenting mathematical, geometric, or floating-point comparison operations, accurately describe formulas, distance metrics, and IEEE 754 semantics rather than mathematically contradictory shorthand
rubric: readability
severity: should_fix
---

**Rule.** When writing or updating comments for mathematical, geometric, or
floating-point operations, bounds, or comparisons, accurately describe formulas,
distance metrics, geometric properties, and IEEE 754 semantics. Do not use
mathematically incorrect shorthand or contradictory descriptions, such as
calling an axis-aligned maximum coordinate offset 'Manhattan distance',
describing collinearity as 'cross product of the slopes', or claiming a fallback
like `x != x && y != y` matches infinities when `==` already equates infinities
and `!=` self-comparisons match NaNs.

**Why.** Inaccurate or colloquial comments describing mathematical formulas,
geometric metrics, or floating-point comparison semantics mislead maintainers
about established invariants and active code behavior. Precise descriptions
prevent confusion over whether edge cases like NaN equivalence or non-Euclidean
bounds are intentional.

## Examples

Bad:

```cpp
// 1. Geometric calculations and properties:
// This is the cross product of the slopes. If it is 0, then the slopes are equal.
const int64_t D0xU0toU1 = cross(D0, U0toU1);

// 2. Coordinate bounds and distance metrics:
// Coordinate will be up to 'sampleRadius' away (Manhattan distance) from input.

// 3. Floating-point comparison semantics:
// Inf == Inf, anything else is false
return x == y || (x != x && y != y);
```

Good:

```cpp
// 1. Geometric calculations and properties:
// If vector D0 and vector U0toU1 have an angle of 0, u1 is on the segment.
const int64_t D0xU0toU1 = cross(D0, U0toU1);

// 2. Coordinate bounds and distance metrics:
// Coordinate will be up to 'sampleRadius' away (maximum absolute offset in x or y).

// 3. Floating-point comparison semantics:
// Standard == equates finite numbers and infinities; fallback treats NaN == NaN as equal.
return x == y || (x != x && y != y);
```
