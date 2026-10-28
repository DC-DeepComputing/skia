---
name: skia/prefer-mappoint-and-mapvector-over-single-element-batch-call
description: When transforming a single SkPoint or SkVector with SkMatrix, call mapPoint or mapVector returning by value instead of single-element mapPoints or mapVectors
rubric: readability
severity: nit
---

**Rule.** When transforming an individual point or vector using SkMatrix, call
mapPoint() or mapVector() returning the transformed instance by value instead of
calling the batch methods mapPoints() or mapVectors() with a single-element span
(such as {&v, 1}).

**Why.** mapPoint() and mapVector() return transformed instances directly by
value, enabling clean initialization of const variables and inline expressions.
Calling batch mapPoints() or mapVectors() with a single-element span adds
unnecessary boilerplate and forces declaring a mutable temporary variable.

## Examples

Bad:

```cpp
SkVector v = SkVector::Make(SkIntToScalar(geom.fX), SkIntToScalar(geom.fY));
matrix.mapVectors({&v, 1});
return IVector(SkScalarRoundToInt(v.fX), SkScalarRoundToInt(v.fY));
```

Good:

```cpp
const SkVector v = matrix.mapVector({SkIntToScalar(geom.fX), SkIntToScalar(geom.fY)});
return IVector(SkScalarRoundToInt(v.fX), SkScalarRoundToInt(v.fY));
```
