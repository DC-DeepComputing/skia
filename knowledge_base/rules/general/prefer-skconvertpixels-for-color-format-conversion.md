---
name: skia/prefer-skconvertpixels-for-color-format-conversion
description: When converting colors or pixel buffers to destination color types, use SkConvertPixels instead of writing ad-hoc SkColorType switch cases or manual packing routines
rubric: design
severity: should_fix
---

**Rule.** When converting colors or pixel buffers to destination pixel formats
(such as in color fill, erase, or packing routines), use existing pixel
conversion helpers like SkConvertPixels rather than implementing ad-hoc switches
on SkColorType or manual channel packing routines.

**Why.** Skia supports numerous SkColorType formats with complex format-specific
invariants, including forcing opaque alpha for '...x' types, color space
conversions, premultiplication, and sRGB encodings. Writing custom per-format
switches is error-prone and incomplete, whereas SkConvertPixels delegates to
SkRasterPipeline and correctly handles all supported color types and color
spaces.

## Examples

Bad:

```cpp
static uint64_t convert_to_pixel(const SkColor4f& c, SkColorType ct) {
    switch (ct) {
        case kRGB_565_SkColorType:
            return pack_565(c);
        case kRGBA_8888_SkColorType:
            return pack_8888(c);
        case kRGB_101010x_SkColorType:
            return pack_101010x(c);
        // Prone to omitting format rules, color space transforms, or alpha packing...
    }
    return 0;
}
```

Good:

```cpp
const auto dst = SkImageInfo::Make(1, 1, this->colorType(), this->alphaType(),
                                   sk_ref_sp(this->colorSpace()));
const auto src = SkImageInfo::Make(1, 1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType,
                                   sk_ref_sp(srcCS));
uint64_t dstPixel[2] = {};
if (!SkConvertPixels(dst, dstPixel, sizeof(dstPixel), src, &color, sizeof(color))) {
    return false;
}
```
