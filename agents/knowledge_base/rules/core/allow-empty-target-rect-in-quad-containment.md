---
name: skia/core/allow-empty-target-rect-in-quad-containment
description: When testing whether a convex polygon or quad contains a target rectangle via corner vertex half-planes, do not reject the target with isEmpty()
rubric: correctness
severity: should_fix
---

**Rule.** When testing whether a convex polygon, quad, or transformed rectangle
contains a target rectangle by evaluating its corner vertices against boundary
half-plane equations, do not reject the target rectangle using isEmpty(). Verify
that only the container geometry is non-empty and sorted to form valid
inward-facing edge equations; the target rectangle's corner coordinates remain
valid test points even if the target is empty, degenerate, or unsorted.

**Why.** In Skia, SkRect::isEmpty() evaluates to true for zero-width,
zero-height, or unsorted rectangles. Rejecting the target rectangle upfront with
isEmpty() causes containment tests to incorrectly return false for degenerate
shapes (such as points or flat line segments) and unsorted rectangles whose
corner coordinates all lie safely inside the container quad.

## Examples

Bad:

```cpp
skvx::int4 SkRectPriv::QuadContainsRectMask(const SkM44& m, const SkRect& a, const SkRect& b) {
    // Bad: Rejecting 'b' with isEmpty() falsely fails for points or unsorted rects inside 'a'
    if (a.isEmpty() || b.isEmpty()) {
        return skvx::int4(0);
    }
    // ... test b's 4 corners against transformed edge equations of a ...
}
```

Good:

```cpp
skvx::int4 SkRectPriv::QuadContainsRectMask(const SkM44& m, const SkRect& a, const SkRect& b) {
    // Only 'a' needs to be non-empty and sorted to form valid inward-facing edge equations.
    if (a.isEmpty()) {
        return skvx::int4(0);
    }
    // 'b' is only used to define its 4 corners to check against the edges, which is
    // valid regardless of b's emptiness or sortedness.
    // ... test b's 4 corners against transformed edge equations of a ...
}
```
