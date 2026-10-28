---
name: skia/check-rectstaysrect-before-maprect-draw
description: When transforming destination rectangles with SkMatrix::mapRect for rectangular draw calls, verify matrix.rectStaysRect() or preserve geometry using matrix-aware drawing
rubric: correctness
severity: should_fix
---

**Rule.** Do not use SkMatrix::mapRect() to transform destination rectangles for
rectangular drawing operations (such as drawImageRect() or drawRect()) unless
the matrix is verified to satisfy rectStaysRect() or preservesAxisAlignment().
When the transformation may rotate, shear, or contain perspective, preserve the
geometry using matrix-aware canvas transformations (such as canvas->concat()) or
quad/path draw calls.

**Why.** SkMatrix::mapRect() computes an axis-aligned bounding box enclosing
transformed corner vertices. Unless the matrix preserves axis alignment, mapping
a rectangle through non-orthogonal rotations or shears expands the bounds into
an axis-aligned box, corrupting geometry when drawn with rectangular APIs.

## Examples

Bad:

```cpp
// Distorts rotated or sheared images into an axis-aligned bounding box
if (imageSet[i].fMatrixIndex >= 0) {
    preViewMatrices[imageSet[i].fMatrixIndex].mapRect(&drawDst);
}
if (!imageSet[i].fHasClip) {
    this->drawImageRect(imageSet[i].fImage.get(), imageSet[i].fSrcRect, drawDst,
                        sampling, &paint, constraint);
}
```

Good:

```cpp
// Delegate directly to drawImageRect only when no pre-transform is applied
if (!imageSet[i].fHasClip && imageSet[i].fMatrixIndex < 0) {
    this->drawImageRect(imageSet[i].fImage.get(), imageSet[i].fSrcRect, imageSet[i].fDstRect,
                        sampling, &paint, constraint);
    continue;
}
// For entries with pre-view matrices, preserve quad geometry or verify rectStaysRect()
```
