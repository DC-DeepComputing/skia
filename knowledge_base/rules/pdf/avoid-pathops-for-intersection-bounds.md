---
name: skia/pdf/avoid-pathops-for-intersection-bounds
description: When computing the bounding box of intersecting geometry or clips, intersect conservative bounds directly instead of invoking topological PathOps boolean operations
rubric: safety
severity: should_fix
---

**Rule.** Do not use topological boolean path operations (such as Op() or
kIntersect_SkPathOp) solely to compute the bounding box of intersecting
geometry. When exact resulting path contours are not needed, compute and
intersect conservative bounding rectangles directly (such as via
SkRect::intersect()) before mapping coordinates.

**Why.** Boolean path operations perform expensive curve intersection, segment
subdivision, and topological sorting to construct an exact output path. When
only the bounding box is required (such as for annotations, culling, or link
targets), running a full topological boolean op introduces significant
computational overhead compared to intersecting axis-aligned bounding
rectangles.

## Examples

Bad:

```cpp
SkPath path = SkPath::Rect(rect).makeTransform(this->localToDevice());
SkPath clip = SkClipStack_AsPath(this->cs());
if (auto result = Op(clip, path, kIntersect_SkPathOp)) {
    path = *result;
}
SkRect transformedRect = pageXform.mapRect(path.getBounds());
```

Good:

```cpp
SkRect clipBounds;
this->cs().getBounds(&clipBounds);
SkRect pathBounds = path.getBounds();
if (!pathBounds.intersect(clipBounds)) {
    return;
}
SkRect transformedRect = pageXform.mapRect(pathBounds);
```
