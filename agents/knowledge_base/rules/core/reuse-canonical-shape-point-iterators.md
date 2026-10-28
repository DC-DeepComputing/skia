---
name: skia/core/reuse-canonical-shape-point-iterators
description: When generating vertex sequences for standard geometric shapes like rects, ovals, or round rects, reuse SkPathMakers point iterators instead of hand-rolling coordinate arithmetic
rubric: design
severity: should_fix
---

**Rule.** When generating point sequences for standard shapes (rectangles,
ovals, or rounded rectangles) parameterized by direction and start index, reuse
canonical point iterators from SkPathMakers.h (such as SkPath_RectPointIterator,
SkPath_OvalPointIterator, and SkPath_RRectPointIterator). Do not open-code
manual vertex coordinate calculations, corner arrays, or midpoint averaging
arithmetic.

**Why.** Hand-rolling coordinate arithmetic, corner arrays, and midpoint
averaging duplicates geometric logic and easily introduces calculation typos
(such as adding 0.5f instead of multiplying). Reusing existing shape point
iterators ensures correct winding direction, start index offset handling, and
consistent vertex ordering across path representations.

## Examples

Bad:

```cpp
static SkPoint ave(SkPoint a, SkPoint b) {
    return {(a.fX + b.fX) * 0.5f, (a.fY + b.fY) + 0.5f};
}
...
pts[0] = pts[8] = ave(tl, tr);
if (dir == SkPathDirection::kCW) {
    pts[1] = tr; pts[2] = ave(tr, br);
}
```

Good:

```cpp
SkPath_OvalPointIterator ovalIter(r, dir, index);
SkPath_RectPointIterator rectIter(r, dir, index + (dir == SkPathDirection::kCW ? 0 : 1));

storage[0] = ovalIter.current();
for (unsigned i = 0; i < 4; ++i) {
    storage[i * 2 + 1] = rectIter.next();
    storage[i * 2 + 2] = ovalIter.next();
}
```
