---
name: skia/codec/convert-bits-per-pixel-to-bytes
description: When computing row byte sizes, strides, or buffer allocations from pixel counts and bitsPerPixel, convert bits to bytes before multiplying by width
rubric: correctness
severity: should_fix
---

**Rule.** When computing row byte sizes, strides, or buffer allocations from
pixel counts and bitsPerPixel (such as SkEncodedInfo::bitsPerPixel()), convert
bits to bytes (e.g. dividing by 8) before multiplying by pixel width. Do not
multiply pixel width directly by bitsPerPixel when byte quantities are required,
and assert byte divisibility when byte-aligned formats are expected.

**Why.** Multiplying pixel count or image width directly by bits per pixel
computes bits per row rather than bytes per row. Neglecting to divide bits by 8
calculates an 8x oversized buffer allocation or causes incorrect byte offsets
and strides when indexing image memory.

## Examples

Bad:

```cpp
size_t bitsPerPixel = static_cast<size_t>(dstInfo.bitsPerPixel());
SkSafeMath safe;
fBytesPerEncodedRow = safe.mul(safe.castTo<size_t>(dstInfo.width()), bitsPerPixel);
```

Good:

```cpp
size_t bitsPerPixel = static_cast<size_t>(dstInfo.bitsPerPixel());
SkASSERT((bitsPerPixel % 8) == 0);
size_t bytesPerPixel = bitsPerPixel / 8;
SkSafeMath safe;
fBytesPerEncodedRow = safe.mul(safe.castTo<size_t>(dstInfo.width()), bytesPerPixel);
```
