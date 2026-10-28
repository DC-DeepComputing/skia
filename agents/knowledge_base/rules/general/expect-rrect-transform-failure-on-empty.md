---
name: skia/expect-rrect-transform-failure-on-empty
description: When transforming an SkRRect with a matrix scaling dimensions to zero, expect SkRRect::transform to fail rather than returning an empty round rect
rubric: correctness
severity: should_fix
---

**Rule.** When transforming an SkRRect via SkRRect::transform(), expect the
operation to fail (returning std::nullopt or false) if the transformation matrix
scales any dimension to zero, collapses the rectangle to empty, or yields
non-finite bounds. In callers and test assertions, check for failure
(!result.has_value() or false) rather than expecting a valid empty-typed SkRRect
(kEmpty_Type).

**Why.** SkRRect::transform() validates that transformed bounds are both finite
and non-empty (!newRect.isFinite() || newRect.isEmpty()) before computing
transformed corner radii. A zero scale collapses width or height to zero,
violating the non-empty invariant and causing transform() to bail out rather
than producing an empty SkRRect.

## Examples

Bad:

```cpp
SkMatrix matrix;
matrix.setScale(0, 1);
auto other = rr.transform(matrix);
REPORTER_ASSERT(reporter, other.has_value() && other->getType() == SkRRect::kEmpty_Type);
```

Good:

```cpp
SkMatrix matrix;
matrix.setScale(0, 1);
auto other = rr.transform(matrix);
REPORTER_ASSERT(reporter, !other.has_value());
```
