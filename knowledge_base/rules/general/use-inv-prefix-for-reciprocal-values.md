---
name: skia/use-inv-prefix-for-reciprocal-values
description: When naming reciprocal or inverse values (such as 1/dx or 1/n), prefix identifiers with 'inv' rather than abbreviations like 'idx', 'idy', or 'oo'
rubric: readability
severity: should_fix
---

**Rule.** When naming reciprocal or inverse quantities (such as 1.0 / dx, 1.0 /
dy, 1.0 / det, or 1.0 / n), prefix the identifier with 'inv' (e.g., invDx,
invDy, invDet, invNumStops) rather than confusing or idiosyncratic abbreviations
like 'idx', 'idy', or 'oo' ('one over'). Reserve 'idx' strictly for container
subscripts or loop induction indices.

**Why.** In Skia and standard graphics conventions, 'inv' is the established
prefix for multiplicative inverses and reciprocals. Non-standard abbreviations
like 'idx' or 'idy' collide with loop or array indices, while informal
abbreviations like 'oo' ('one-over') are cryptic and non-obvious to readers.

## Examples

Bad:

```cpp
// Bad: 'idx'/'idy' confuse reciprocals with indices; 'oo' is cryptic shorthand
float idx = (std::abs(dx) <= kEpsilon) ? 0.0f : 1.0f / dx;
float idy = (std::abs(dy) <= kEpsilon) ? 0.0f : 1.0f / dy;
std::array<float, 4> derivs = {dx, dy, idx, idy};

float ooNumStops = 1.0 / high;
```

Good:

```cpp
// Good: 'inv' clearly signals a reciprocal / inverse quantity
float invDx = (std::abs(dx) <= kEpsilon) ? 0.0f : 1.0f / dx;
float invDy = (std::abs(dy) <= kEpsilon) ? 0.0f : 1.0f / dy;
std::array<float, 4> derivs = {dx, dy, invDx, invDy};

float invNumStops = 1.0 / high;
```
