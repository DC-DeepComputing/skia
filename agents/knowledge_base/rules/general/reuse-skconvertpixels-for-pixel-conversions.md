---
name: skia/reuse-skconvertpixels-for-pixel-conversions
description: When converting pixels or packing colors across SkColorType formats, reuse SkConvertPixels instead of hand-rolling format switch statements or treating kSRGBA_8888 as linear unorm
rubric: correctness
severity: should_fix
---

**Rule.** When transforming or packing pixels between arbitrary color
formats—such as converting an SkColor4f into destination pixel formats or
copying pixel buffers—use SkConvertPixels rather than authoring custom
format-conversion switch statements or manual color math. Never treat
kSRGBA_8888_SkColorType as linear unorm8 (such as falling through to
kRGBA_8888_SkColorType); if converting colors manually, sRGB transfer function
encoding is required, but constructing 1x1 SkImageInfo descriptors with
SkConvertPixels handles all format and color space conversions automatically.

**Why.** Hand-rolling pixel packing and color-space conversions is error-prone,
duplicates logic across formats, and introduces discrepancies in luminance math,
transfer functions, or channel order. In particular, kSRGBA_8888_SkColorType
requires non-linear sRGB transfer function encoding, so quantizing linear float
channels directly produces corrupted colors. SkConvertPixels already handles all
supported SkColorType, SkAlphaType, and SkColorSpace combinations via
SkRasterPipeline.

## Examples

Bad:

```cpp
static uint64_t convert_to_pixel(SkColor4f c, SkColorType ct) {
    switch (ct) {
        case kAlpha_8_SkColorType: return rnd(c.fA * 255);
        case kGray_8_SkColorType:  return rnd(compute_gray(c) * 255);
        case kRGBA_8888_SkColorType: [[fallthrough]];
        case kRGB_888x_SkColorType:  [[fallthrough]];
        case kSRGBA_8888_SkColorType:
            return rnd(c.fR * 255) | (rnd(c.fG * 255) << 8) | (rnd(c.fB * 255) << 16) | (rnd(c.fA * 255) << 24);
    }
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
