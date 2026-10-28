---
name: skia/gpu/prefer-extractsubset-over-makesubset-for-raster-tiles
description: When slicing raster images into tiles for texture caching, extract subsets via SkBitmap::extractSubset and SkCopyPixelsMode::kNever rather than SkImage::makeSubset
rubric: safety
severity: should_fix
---

**Rule.** Do not use SkImage::makeSubset() to generate sub-rect tile views of
raster-backed images when texture caching or zero-copy performance is required.
Instead, obtain the underlying SkBitmap via as_IB(image)->getROPixels(), slice
tiles using SkBitmap::extractSubset(), and wrap them into an SkImage using
SkImage_Raster::MakeFromBitmap(..., SkCopyPixelsMode::kNever).

**Why.** SkImage::makeSubset() on raster images performs a deep copy of the
pixel buffer and allocates a new SkPixelRef, losing the original pixel ref's
generation identity. This incurs redundant memory allocations and causes GPU
texture cache misses across frames, whereas SkBitmap::extractSubset() with
SkCopyPixelsMode::kNever shares the existing SkPixelRef zero-copy.

## Examples

Bad:

```cpp
// Slicing raster image tiles with makeSubset copies pixels and creates a new SkPixelRef
auto imageProc = [&](SkIRect iTileR) {
    return image->makeSubset(recorder, iTileR, {});
};
```

Good:

```cpp
// Share the underlying SkPixelRef zero-copy to preserve texture cache identity
if (SkBitmap bm; as_IB(image)->getROPixels(nullptr, &bm)) {
    auto imageProc = [&](SkIRect iTileR) -> sk_sp<SkImage> {
        if (SkBitmap subsetBmp; bm.extractSubset(&subsetBmp, iTileR)) {
            return SkImage_Raster::MakeFromBitmap(subsetBmp, SkCopyPixelsMode::kNever);
        }
        return nullptr;
    };
    ...
}
```
