---
name: skia/omit-redundant-safemath-for-bitmap-allocation
description: When allocating an SkBitmap from an SkIRect, rely on SkIRect saturating math and tryAllocPixels instead of manual SkSafeMath checks
rubric: readability
severity: should_fix
---

**Rule.** When allocating an SkBitmap from an SkIRect or SkImageInfo, rely on
SkIRect's built-in saturating arithmetic (such as outset() or adjust()) and
SkBitmap::tryAllocPixels() to handle overflow validation. Do not manually
perform overflow calculations using SkSafeMath or add redundant SkTFitsIn<int>
or INT_MAX range checks on rectangle dimensions.

**Why.** SkIRect operations like outset() and adjust() internally use saturating
integer arithmetic (Sk32_sat_add and Sk32_sat_sub) to prevent overflow
wraparound, and SkBitmap::tryAllocPixels() validates image dimensions, row
bytes, and total allocation limits. Performing manual overflow math with
SkSafeMath and adding explicit integer bounds checks duplicates these built-in
safeguards and adds unnecessary code clutter.

## Examples

Bad:

```cpp
SkSafeMath safe;
size_t dstWToAlloc = safe.add(originalDstBounds.width(), safe.add(borderW, borderW));
size_t dstHToAlloc = safe.add(originalDstBounds.height(), safe.add(borderH, borderH));
size_t toAlloc = safe.mul(dstWToAlloc, dstHToAlloc);

SkBitmap dst;
SkIRect dstBounds = originalDstBounds;
dstBounds.outset(borderW, borderH);
SkImageInfo dstInfo;
if (!SkTFitsIn<int>(dstWToAlloc) || !SkTFitsIn<int>(dstHToAlloc) ||
        toAlloc > INT_MAX || !safe) {
    return nullptr;
} else {
    dstInfo = src.info().makeWH(dstWToAlloc, dstHToAlloc);
}
if (!dst.tryAllocPixels(dstInfo)) {
    return nullptr;
}
```

Good:

```cpp
SkIRect dstBounds = originalDstBounds;
dstBounds.outset(borderW, borderH);

SkBitmap dst;
SkImageInfo dstInfo = src.info().makeWH(dstBounds.width(), dstBounds.height());
if (!dst.tryAllocPixels(dstInfo)) {
    return nullptr;
}
```
