---
name: skia/core/avoid-mutating-shared-paint-in-batch-loops
description: When iterating over a collection of draw items in batch drawing loops, instantiate a per-entry SkPaint copy rather than mutating a shared paint in place
rubric: correctness
severity: should_fix
---

**Rule.** When iterating over a collection of draw items (such as an image set,
atlas entries, or geometry batch), do not mutate a shared base SkPaint in place
across loop iterations using mutating helpers or paint setters. Instantiate a
per-entry local copy of the paint or explicitly reset modified attributes on
each iteration so that state modifications do not leak across items.

**Why.** Mutating helpers (such as image shader setup or alpha blending
adjustments) and direct setter calls modify the passed SkPaint object. Mutating
a single shared base paint across iterations causes modifications from earlier
entries to persist, corrupting paint parameters for subsequent items in the
batch.

## Examples

Bad:

```cpp
for (int i = 0; i < count; ++i) {
    // Mutating &basePaint in place corrupts paint state for subsequent iterations
    SkRect drawDst = SkModifyPaintAndDstForDrawImageRect(
            imageSet[i].fImage.get(), sampling,
            imageSet[i].fSrcRect, imageSet[i].fDstRect,
            constraint == kStrict_SrcRectConstraint, &basePaint);
    this->drawImageRect(imageSet[i].fImage.get(), imageSet[i].fSrcRect, drawDst,
                        sampling, &basePaint, constraint);
}
```

Good:

```cpp
for (int i = 0; i < count; ++i) {
    // Make a per-iteration copy so modifications do not leak across items
    SkPaint itemPaint = basePaint;
    SkRect drawDst = SkModifyPaintAndDstForDrawImageRect(
            imageSet[i].fImage.get(), sampling,
            imageSet[i].fSrcRect, imageSet[i].fDstRect,
            constraint == kStrict_SrcRectConstraint, &itemPaint);
    this->drawImageRect(imageSet[i].fImage.get(), imageSet[i].fSrcRect, drawDst,
                        sampling, &itemPaint, constraint);
}
```
