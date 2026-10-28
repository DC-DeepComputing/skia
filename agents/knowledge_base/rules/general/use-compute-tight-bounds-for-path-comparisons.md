---
name: skia/use-compute-tight-bounds-for-path-comparisons
description: When comparing geometric paths or validating bounds equality, use SkPath::computeTightBounds() instead of getBounds() to avoid false mismatches from off-curve control points
rubric: correctness
severity: should_fix
---

**Rule.** When comparing SkPath instances, verifying matching bounds, or
evaluating exact visual extents (such as in path comparison helpers or tests),
use SkPath::computeTightBounds() rather than SkPath::getBounds(). Do not rely on
getBounds() to test geometric containment or equality across paths with curve
segments.

**Why.** SkPath::getBounds() returns the bounding box of all points defining the
path, including off-curve control points. Geometrically equivalent or
overlapping paths (such as quadratics converted to cubics or curves with
differing control point coordinates) can have different control point bounds,
causing false mismatches in bounds comparisons. SkPath::computeTightBounds()
computes the true curve extrema.

## Examples

Bad:

```cpp
bool A8ComparePaths(const SkPath& a, const SkPath& b, A8CompareProc cmp) {
    auto ra = a.getBounds(),
         rb = b.getBounds();
    if (!ra.makeOutset(1, 1).contains(rb)) {
        return false;
    }
```

Good:

```cpp
bool A8ComparePaths(const SkPath& a, const SkPath& b, A8CompareProc cmp) {
    const auto ra = a.computeTightBounds(),
               rb = b.computeTightBounds();
    if (!ra.makeOutset(1, 1).contains(rb)) {
        return false;
    }
```
