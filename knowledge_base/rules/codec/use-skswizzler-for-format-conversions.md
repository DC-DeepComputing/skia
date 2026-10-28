---
name: skia/codec/use-skswizzler-for-format-conversions
description: When converting decoded scanlines to destination pixel formats in SkCodec implementations, use SkSwizzler instead of writing ad-hoc conversion loops or falling back to memcpy
rubric: correctness
severity: blocking
---

**Rule.** In SkCodec implementations, use SkSwizzler to convert decoded
scanlines to requested destination pixel formats (such as via
initializeSwizzler() and fSwizzler->swizzle()). Do not implement ad-hoc color
type conversion branching or fall back to memcpy for seemingly matching formats.

**Why.** Codec callers can request destination formats not anticipated by manual
conversion branches (such as kGray_8_SkColorType), and fallback memcpy assumes
identical pixel layouts and row strides, resulting in corrupted pixel data.
SkSwizzler handles channel swapping, format conversion, and alpha
premultiplication correctly and efficiently.

## Examples

Bad:

```cpp
if (dstInfo.colorType() == kRGBA_8888_SkColorType) {
    // ... manual color conversion ...
} else if (dstInfo.colorType() == kRGB_565_SkColorType) {
    // ... manual 565 conversion ...
} else {
    // Corrupts pixel layouts such as kGray_8_SkColorType or mismatched strides
    memcpy(dstRow, srcRow, std::min(srcRowBytes, dstRowBytes));
}
```

Good:

```cpp
// Initialize SkSwizzler to handle destination color and alpha conversions
fSwizzler = SkSwizzler::Make(this->getEncodedInfo(), nullptr, swizzlerInfo, swizzlerOptions);
if (!fSwizzler) {
    return kInvalidConversion;
}
// ...
// Swizzle scanline to destination format
fSwizzler->swizzle(dstRow, srcRow.data());
```
