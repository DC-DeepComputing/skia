---
name: skia/xps/adjust-orthogonal-bounds-independently
description: When adjusting bounding boxes or extents for independent orthogonal conditions, initialize base dimensions and adjust each axis independently rather than branching combinatorially
rubric: design
severity: should_fix
---

**Rule.** When adjusting bounding boxes or extents based on independent
orthogonal conditions (such as independent X and Y tile modes, clamp flags, or
margins), initialize the rectangle to base dimensions upfront and adjust each
axis in independent conditional blocks. Do not branch combinatorially over all
permutations of axis conditions to set multi-axis bounds.

**Why.** Branching combinatorially across orthogonal axes duplicates adjustment
math across branches, multiplies code paths exponentially, and risks missing
edge cases. Initializing base bounds upfront and modifying each axis
independently keeps control flow linear and co-locates dimension adjustments
with axis-specific operations.

## Examples

Bad:

```cpp
// Bad: Branching over combinations of orthogonal X and Y conditions duplicates bounds math.
XPS_RECT bound = {};
if (SkTileMode::kClamp == xy[0] && SkTileMode::kClamp == xy[1]) {
    bound.x = -BIG_F;
    bound.y = -BIG_F;
    bound.width = bWidth + 2 * BIG_F;
    bound.height = bHeight + 2 * BIG_F;
} else if (SkTileMode::kClamp == xy[0]) {
    bound.x = -BIG_F;
    bound.y = 0.0f;
    bound.width = bWidth + 2 * BIG_F;
    bound.height = bHeight;
} else if (SkTileMode::kClamp == xy[1]) {
    bound.x = 0.0f;
    bound.y = -BIG_F;
    bound.width = bWidth;
    bound.height = bHeight + 2 * BIG_F;
}
```

Good:

```cpp
// Good: Initialize to base bounds and adjust each axis in independent conditional blocks.
XPS_RECT bound = {0, 0, bWidth, bHeight};
if (SkTileMode::kClamp == xy[0]) {
    bound.x -= BIG_F;
    bound.width += 2 * BIG_F;
}
if (SkTileMode::kClamp == xy[1]) {
    bound.y -= BIG_F;
    bound.height += 2 * BIG_F;
}
```
