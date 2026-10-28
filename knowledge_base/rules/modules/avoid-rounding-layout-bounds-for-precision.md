---
name: skia/modules/avoid-rounding-layout-bounds-for-precision
description: When reconciling floating-point discrepancies during text hit-testing or layout boundary comparisons, do not round clip bounds; check proximity within an epsilon instead
rubric: correctness
severity: should_fix
---

**Rule.** Do not round or mutate internal layout or clip rectangle boundaries
(such as calling littleRound on clip coordinates) to compensate for
floating-point inaccuracies or floored query coordinates. Accommodate precision
discrepancies by testing whether the query coordinate is within an epsilon (such
as SkScalarNearlyEqual) of the boundary.

**Why.** Unconditionally rounding layout or clip rectangle coordinates distorts
run geometry across all text lines and introduces subtle positioning errors.
Testing query coordinates against boundary edges using a small epsilon
accommodates accumulated floating-point drift without corrupting underlying clip
metrics.

## Examples

Bad:

```cpp
ClipContext context = context0;
// Unconditionally rounding clip coordinates distorts geometry across all runs
context.clip.fLeft = littleRound(context.clip.fLeft);
context.clip.fRight = littleRound(context.clip.fRight);
```

Good:

```cpp
ClipContext context = context0;
// Accommodate floating-point drift with an epsilon check rather than mutating bounds
if (SkScalarNearlyEqual(context.clip.fRight, dx, 0.01f)) {
    context.clip.fRight = dx;
}
```
