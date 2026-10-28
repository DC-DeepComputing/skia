---
name: skia/guard-make-color-space-against-null
description: When applying an optional color space override to SkColorInfo or SkImageInfo, guard makeColorSpace() with a null check to avoid clobbering the existing color space
rubric: correctness
severity: should_fix
---

**Rule.** Do not pass an optional or nullable color space pointer directly to
SkColorInfo::makeColorSpace() or SkImageInfo::makeColorSpace() without checking
for null. Because makeColorSpace(nullptr) unconditionally replaces the existing
color space with null rather than leaving it unchanged, only invoke
makeColorSpace() when the override pointer is non-null.

**Why.** SkColorInfo::makeColorSpace() and SkImageInfo::makeColorSpace()
construct a new instance with the provided color space. When an optional color
space override (such as SaveLayerRec::fColorSpace) is null, passing nullptr
overwrites the existing valid color space with null (reverting to
unmanaged/untagged fallback) instead of preserving the prior device or image's
color space.

## Examples

Bad:

```cpp
// rec.fColorSpace is optional; if null, makeColorSpace(nullptr) overwrites priorDevice's color space with null.
this->internalDrawDeviceWithFilter(
        /*src=*/nullptr, priorDevice, filters, restorePaint,
        DeviceCompatibleWithFilter::kUnknown,
        priorDevice->imageInfo().colorInfo().makeColorSpace(sk_ref_sp(rec.fColorSpace)));
```

Good:

```cpp
SkColorInfo filterColorInfo = priorDevice->imageInfo().colorInfo();
if (rec.fColorSpace) {
    filterColorInfo = filterColorInfo.makeColorSpace(sk_ref_sp(rec.fColorSpace));
}
this->internalDrawDeviceWithFilter(
        /*src=*/nullptr, priorDevice, filters, restorePaint,
        DeviceCompatibleWithFilter::kUnknown, filterColorInfo);
```
