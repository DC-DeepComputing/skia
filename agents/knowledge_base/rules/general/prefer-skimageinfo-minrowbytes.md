---
name: skia/prefer-skimageinfo-minrowbytes
description: When computing row bytes or row size from an SkImageInfo, use info.minRowBytes() or minRowBytes64() instead of manual width and bytesPerPixel multiplication
rubric: readability
severity: should_fix
---

**Rule.** When computing minimum row bytes, row size, or default stride for an
image represented by an SkImageInfo, call info.minRowBytes() (or minRowBytes64()
for 64-bit width or overflow avoidance) instead of manual multiplication like
bytesPerPixel() * width() or hardcoded constants like width * 4. Note that
minRowBytes() returns 0 if the row byte size overflows 31 bits. When custom or
non-minimum strides are required, derive the stride using info.bytesPerPixel()
or SkColorTypeBytesPerPixel() rather than raw numeric literals.

**Why.** Manual multiplication duplicates arithmetic and risks integer overflow
or layout assumptions when formats change. SkImageInfo::minRowBytes()
encapsulates color-type bytes-per-pixel lookups and includes overflow detection,
returning 0 on 31-bit overflow (or returning uint64_t via minRowBytes64()).

## Examples

Bad:

```cpp
// Hardcoded bytes-per-pixel
constexpr size_t rowBytes = W * 4;
SkReadPixelsRec rec(info, pixels, rowBytes, 0, 0);

// Manual arithmetic with bytesPerPixel()
fExpectedDstRowSize =
        static_cast<size_t>(dstInfo.bytesPerPixel()) * static_cast<size_t>(dstInfo.width());
```

Good:

```cpp
// Use minRowBytes() helper
const size_t rowBytes = info.minRowBytes();
SkReadPixelsRec rec(info, pixels, rowBytes, 0, 0);

// Handles overflow checking automatically
fDstMinRowBytes = dstInfo.minRowBytes();
```
