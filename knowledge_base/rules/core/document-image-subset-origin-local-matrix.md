---
name: skia/core/document-image-subset-origin-local-matrix
description: When adjusting shader local matrices to account for image subset origins, document that logical (0, 0) maps to the subset's top-left backing storage offset
rubric: readability
severity: nit
---

**Rule.** When adjusting shader local matrices to account for image subset
origins (such as translating by -this->subset().topLeft()), document that the
image's logical (0, 0) corresponds to the subset's top-left corner in backing
storage. Ensure overridden or duplicated shader creation methods retain this
explanatory comment so the translation is not perceived as an unexplained magic
offset.

**Why.** When sampling a sub-rectangle of an image or bitmap directly from
backing storage, logical coordinates start at (0, 0) whereas backing pixel data
resides at the subset's top-left offset. An unexplained translation in the local
matrix looks like arbitrary magic arithmetic to maintainers unless the origin
mapping invariant is clearly documented.

## Examples

Bad:

```cpp
// Coordinate translation without documenting why the origin adjustment is needed:
SkMatrix subsetOrigin = SkMatrix::Translate(-this->subset().topLeft());
subsetOrigin.postConcat(lm);
return fBitmap.makeShader(tileMode, tileMode, sampling, subsetOrigin);
```

Good:

```cpp
// The special image's logical (0,0) is at its subset's topLeft() so we need to
// account for that in the local matrix used when sampling.
SkMatrix subsetOrigin = SkMatrix::Translate(-this->subset().topLeft());
subsetOrigin.postConcat(lm);
return fBitmap.makeShader(tileMode, tileMode, sampling, subsetOrigin);
```
