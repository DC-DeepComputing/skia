---
name: skia/document-geometric-solver-root-selection
description: When implementing numerical or geometric solvers with multiple roots or sign choices, document the selection logic and underlying geometric invariants
rubric: readability
severity: nit
---

**Rule.** In shader, geometric, or numerical solvers where an equation yields
multiple mathematical solutions or branch choices, document the non-obvious sign
or root selection logic with an inline comment. Explain how the chosen sign or
formula distinguishes geometric invariants (such as choosing the larger
parameter t, maximizing radius, or handling enclosing vs. conical geometries)
rather than leaving bare algebraic sign choices unexplained.

**Why.** Equations with multiple roots or branch choices (such as quadratic
formulas in conical gradients or curve intersection finders) often rely on
subtle geometric constraints to pick a valid solution. Without clarifying
comments, maintainers cannot tell whether sign choices or root filters are
intentional geometric constraints or arbitrary implementations, risking
regressions during optimization or cleanup.

## Examples

Bad:

```cpp
// Quadratic case
float d = b*b - 4*a*c;
if (d < 0) {
    return float2(0, -1);
}

float sign = sign(1 - dRadius);
t = invA * (-b + sign * sqrt(d));
```

Good:

```cpp
// Quadratic case
float d = b*b - 4*a*c;
if (d < 0) {
    return float2(0, -1);
}

// T should be as large as possible, so when one circle fully encloses the other,
// the sign will be positive or negative depending on the sign of dRadius.
// When this isn't the case and they form a cone, the sign will always be positive.
float sign = sign(1 - dRadius);
t = invA * (-b + sign * sqrt(d));
```
