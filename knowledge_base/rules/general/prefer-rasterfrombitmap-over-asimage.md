---
name: skia/prefer-rasterfrombitmap-over-asimage
description: When converting an SkBitmap to an SkImage, call SkImages::RasterFromBitmap(bitmap) directly rather than invoking the legacy member wrapper bitmap.asImage()
rubric: readability
severity: nit
---

**Rule.** When converting an SkBitmap into an SkImage, call
SkImages::RasterFromBitmap(bitmap) directly rather than invoking
bitmap.asImage(). Prefer the modern plural-namespace factory function over the
legacy SkBitmap member wrapper.

**Why.** SkBitmap::asImage() is a legacy forwarding wrapper that merely calls
SkImages::RasterFromBitmap(*this). Directly invoking SkImages::RasterFromBitmap
aligns with Skia's plural-namespace factory architecture and eliminates
redundant wrapper indirection.

## Examples

Bad:

```cpp
bm.setImmutable();
fGainCurvesXYM = bm.asImage();
```

Good:

```cpp
bm.setImmutable();
fGainCurvesXYM = SkImages::RasterFromBitmap(bm);
```
