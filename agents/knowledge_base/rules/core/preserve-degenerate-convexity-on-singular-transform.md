---
name: skia/core/preserve-degenerate-convexity-on-singular-transform
description: When transforming convex paths or geometry with singular matrices (determinant zero), preserve convexity as degenerate convex (kConvex_Degenerate) instead of resetting to unknown (kUnknown)
rubric: correctness
severity: should_fix
---

**Rule.** When transforming a path or geometric shape known to be convex using a
singular matrix (where the 2x2 determinant is zero), preserve its convexity
state as degenerate convex (SkPathConvexity::kConvex_Degenerate). Do not reset
convexity to uncomputed or unknown (SkPathConvexity::kUnknown).

**Why.** An affine transformation with a zero determinant collapses 2D geometry
into a point or line segment, which remains geometrically convex even though its
contour winding direction cannot be determined. Setting convexity to kUnknown
discards established convexity information and forces unnecessary downstream
recomputations, whereas kConvex_Degenerate preserves the convexity invariant.

## Examples

Bad:

```cpp
// Bad: resets known convexity to unknown on singular transform
if (det2x2 < 0) {
    convexity = SkPathConvexity_OppositeConvexDirection(convexity);
} else if (det2x2 > 0) {
    // direction preserved
} else /* det2x2 == 0 */ {
    convexity = SkPathConvexity::kUnknown;
}
```

Good:

```cpp
// Good: preserves convexity invariant as degenerate convex
if (det2x2 < 0) {
    convexity = SkPathConvexity_OppositeConvexDirection(convexity);
} else if (det2x2 > 0) {
    // direction preserved
} else /* det2x2 == 0 */ {
    convexity = SkPathConvexity::kConvex_Degenerate;
}
```
