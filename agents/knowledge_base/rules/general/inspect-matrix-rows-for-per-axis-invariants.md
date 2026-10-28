---
name: skia/inspect-matrix-rows-for-per-axis-invariants
description: When analyzing or validating transformation matrices per coordinate axis, inspect matrix entries row-by-row (rc(row, col)) rather than swapping off-diagonal shear components across columns
rubric: correctness
severity: should_fix
---

**Rule.** When analyzing or decomposing affine transformation matrices per
coordinate axis (such as testing whether an axis has identity scale, zero shear,
or integer translation), inspect matrix entries row-by-row (rc(row, col)): row 0
governs x' output and row 1 governs y' output. Do not swap off-diagonal shear
components (rc(0,1) vs rc(1,0)) or test column elements when validating an axis.

**Why.** In standard affine matrix math, the transformed coordinate x' is
computed from the dot product of row 0 with the input vector, and y' is computed
from row 1. Swapping off-diagonal entries (e.g. testing rc(1,0) for the X-axis
instead of rc(0,1)) mistakenly evaluates whether X influences Y rather than
whether Y influences X, corrupting per-axis alignment or transform
classification.

## Examples

Bad:

```cpp
// Testing X-axis alignment mistakenly inspects rc(1,0) (shear affecting Y)
bool xAxis = SkScalarNearlyEqual(1.f, m.rc(0,0)*invW, kRoundEpsilon) &&
             SkScalarNearlyEqual(0.f, m.rc(1,0)*invW, kRoundEpsilon) && // Bug: row 1 affects y'
             SkScalarNearlyEqual(tx,  m.rc(0,2)*invW, kRoundEpsilon);
bool yAxis = SkScalarNearlyEqual(0.f, m.rc(0,1)*invW, kRoundEpsilon) && // Bug: row 0 affects x'
             SkScalarNearlyEqual(1.f, m.rc(1,1)*invW, kRoundEpsilon) &&
             SkScalarNearlyEqual(ty,  m.rc(1,2)*invW, kRoundEpsilon);
```

Good:

```cpp
// Row 0 determines x' (scale m00, shear from y m01, translation tx)
bool xAxis = SkScalarNearlyEqual(1.f, m.rc(0,0)*invW, kRoundEpsilon) &&
             SkScalarNearlyEqual(0.f, m.rc(0,1)*invW, kRoundEpsilon) &&
             SkScalarNearlyEqual(tx,  m.rc(0,2)*invW, kRoundEpsilon);
// Row 1 determines y' (shear from x m10, scale m11, translation ty)
bool yAxis = SkScalarNearlyEqual(0.f, m.rc(1,0)*invW, kRoundEpsilon) &&
             SkScalarNearlyEqual(1.f, m.rc(1,1)*invW, kRoundEpsilon) &&
             SkScalarNearlyEqual(ty,  m.rc(1,2)*invW, kRoundEpsilon);
```
