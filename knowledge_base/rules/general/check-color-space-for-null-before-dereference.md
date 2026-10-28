---
name: skia/check-color-space-for-null-before-dereference
description: When querying color spaces from SkImageInfo, SkColorInfo, or SkPixmap, check colorSpace() for nullptr before dereferencing methods like isSRGB() or gammaIsLinear()
rubric: correctness
severity: blocking
---

**Rule.** Never dereference pointers returned by SkImageInfo::colorSpace(),
SkColorInfo::colorSpace(), or SkPixmap::colorSpace() without verifying they are
non-null. Untagged images, surfaces, and pixmaps return nullptr for their color
space; guard member calls (such as ->isSRGB() or ->gammaIsLinear()) with a null
check or provide a default fallback.

**Why.** In Skia, untagged images and surfaces have no color space and return
nullptr from colorSpace(). Dereferencing an unverified color space pointer
causes undefined behavior and runtime crashes when processing untagged inputs.

## Examples

Bad:

```cpp
if ((layerPaint && (layerPaint->getImageFilter() || layerPaint->getColorFilter()))
    || !cinfo.fInfo.colorSpace()->isSRGB()) {
    return true;
}
```

Good:

```cpp
if ((layerPaint && (layerPaint->getImageFilter() || layerPaint->getColorFilter()))
    || (cinfo.fInfo.colorSpace() && !cinfo.fInfo.colorSpace()->isSRGB())) {
    return true;
}
```
