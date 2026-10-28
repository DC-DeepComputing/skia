---
name: skia/gpu/document-raster-subset-pixelref-cache-workaround
description: When subsetting raster images via SkBitmap::extractSubset and wrapping them in an SkImage to preserve caching, document that SkImage::makeSubset breaks SkPixelRef sharing
rubric: readability
severity: nit
---

**Rule.** When subsetting raster images by extracting an SkBitmap subset and
wrapping it with SkCopyPixelsMode::kNever instead of calling
SkImage::makeSubset(), add an explanatory comment. Clarify that
SkImage::makeSubset() deep-copies raster pixel data and allocates a new
SkPixelRef, destroying texture cache sharing.

**Why.** SkImage::makeSubset() always creates an independent copy of raster
pixel data and assigns a new SkPixelRef, which prevents GPU texture caches from
recognizing that the subset belongs to the original cached texture. Documenting
this workaround prevents future maintainers from inadvertently replacing the
multi-step bitmap extraction with makeSubset() during cleanup.

## Examples

Bad:

```cpp
auto imageProc = [&](SkIRect iTileR) {
    if (SkBitmap subsetBmp; bm.extractSubset(&subsetBmp, iTileR)) {
        return SkImage_Raster::MakeFromBitmap(subsetBmp, SkCopyPixelsMode::kNever);
    }
    return sk_sp<SkImage>(nullptr);
};
```

Good:

```cpp
auto imageProc = [&](SkIRect iTileR) {
    // We must subset as a bitmap and then turn it into an SkImage if we want
    // caching to work. Image subsets always make a copy of the pixels and lose
    // the association with the original's SkPixelRef.
    if (SkBitmap subsetBmp; bm.extractSubset(&subsetBmp, iTileR)) {
        return SkImage_Raster::MakeFromBitmap(subsetBmp, SkCopyPixelsMode::kNever);
    }
    return sk_sp<SkImage>(nullptr);
};
```
