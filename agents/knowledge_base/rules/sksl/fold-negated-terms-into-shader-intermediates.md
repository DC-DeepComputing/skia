---
name: skia/sksl/fold-negated-terms-into-shader-intermediates
description: When intermediate variables in SkSL shaders only appear negated or squared, fold the unary negation into the variable definition to save ALU operations
rubric: safety
severity: nit
---

**Rule.** In SkSL shaders and mathematical routines, fold unary negations into
intermediate variable definitions (e.g. naming the variable negB instead of b)
when the variable is only consumed with a negative sign or squared. Eliminating
the unary negation at the definition site saves redundant negation ALU
operations across downstream expressions.

**Why.** When an intermediate variable in an algebraic formula (such as the
quadratic formula) is only used in negated forms or squared (where sign cancels
out), absorbing the negation into the variable definition eliminates redundant
unary negation instructions without altering numerical results.

## Examples

Bad:

```cpp
// Bad: 'b' is only used negated (-b, -c/b) or squared (b*b), wasting negation ALU ops.
float b = -2 * (dRadius * radius0 + pos.x);
float t = -c / b;
float d = b*b - 4*a*c;
t = invA * (-b + quadSign * sqrt(d));
```

Good:

```cpp
// Good: Fold the negation into 'negB' to eliminate redundant unary negation operations.
float negB = 2 * (dRadius * radius0 + pos.x);
float t = c / negB;
float d = negB*negB - 4*a*c;
t = invA * (negB + quadSign * sqrt(d));
```
