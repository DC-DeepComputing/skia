---
name: skia/avoid-copy-initialization-before-out-transform
description: When calling mapping or transformation functions accepting separate input and output arguments, pass distinct variables directly instead of copy-initializing for in-place transformation
rubric: readability
severity: nit
---

**Rule.** When calling mapping, transformation, or conversion functions that
accept separate input and output parameters (such as mapRect, inverseMapRect, or
mapPoints), pass the source input and destination output variables directly. Do
not copy-initialize the output variable from the input variable beforehand to
perform an in-place transformation.

**Why.** Passing distinct input and output variables directly clarifies data
flow and avoids redundant copy-initialization. In coordinate-space
transformations, copy-initializing the output from the input also creates
confusing semantic mismatch where a variable named for one coordinate space
temporarily holds coordinates from another.

## Examples

Bad:

```cpp
PixelSpace<SkRect> srcSampleBounds = dstSampleBounds;
SkAssertResult(srcToDst.inverseMapRect(srcSampleBounds, &srcSampleBounds));
```

Good:

```cpp
PixelSpace<SkRect> srcSampleBounds;
SkAssertResult(srcToDst.inverseMapRect(dstSampleBounds, &srcSampleBounds));
```
