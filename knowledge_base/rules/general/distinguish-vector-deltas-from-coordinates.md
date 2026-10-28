---
name: skia/distinguish-vector-deltas-from-coordinates
description: When declaring variables representing coordinate differences, edge vectors, or displacements, use delta or vector indicators like dx or vx rather than absolute coordinate names
rubric: readability
severity: nit
---

**Rule.** Name variables, parameters, and fields representing coordinate
differences, edge vectors, or relative displacements using delta or vector
indicators (such as 'dx'/'dy', 'vx'/'vy', or 'lastVx') rather than bare
coordinate names (such as 'x'/'y' or 'lastX'). Reserve bare 'x' and 'y'
identifiers strictly for absolute points or positions.

**Why.** Coordinate names like 'x', 'y', or 'lastX' imply absolute spatial
positions (such as vertex coordinates). Using vector or delta indicators
('vx'/'vy' or 'dx'/'dy') explicitly communicates that the variable represents a
displacement or direction vector, preventing misinterpretation in geometric
math.

## Examples

Bad:

```cpp
// Bad: 'lastX' and 'lastY' imply absolute point positions, but hold vector components
SkVector v0 = polygonVerts[currIndex] - polygonVerts[prevIndex];
SkScalar lastX = v0.fX;
SkScalar lastY = v0.fY;
if (lastX * v1.fX < 0) { ... }
```

Good:

```cpp
// Good: 'lastVx' and 'lastVy' clearly indicate vector components / displacements
SkVector v0 = polygonVerts[currIndex] - polygonVerts[prevIndex];
SkScalar lastVx = v0.fX;
SkScalar lastVy = v0.fY;
if (lastVx * v1.fX < 0) { ... }
```
