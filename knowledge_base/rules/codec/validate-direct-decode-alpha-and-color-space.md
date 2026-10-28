---
name: skia/codec/validate-direct-decode-alpha-and-color-space
description: When implementing image codec fast paths that decode directly into destination buffers, verify destination alpha type and color space match source metadata
rubric: correctness
severity: blocking
---

**Rule.** When implementing image codec fast paths that decode rows directly
into destination buffers to bypass swizzling or color transformation, verify
that destination alpha type and color space strictly match source metadata.
Specifically, require unpremultiplied destination alpha (kUnpremul_SkAlphaType)
if the source format is unpremultiplied (such as PNG), and ensure destination
color space matches the source ICC profile (e.g. using
skcms_ApproximatelyEqualProfiles) before skipping conversion.

**Why.** Bypassing SkSwizzler or applyColorXform skips required alpha
premultiplication and color space conversion when the destination requests
kPremul_SkAlphaType or a differing color space, corrupting pixel values.
Furthermore, because kN32_SkColorType resolves to RGBA on Android and BGRA on
desktop, checking only fixed color types can cause fast paths to execute on one
platform while falling back on another, masking bugs during desktop testing.

## Examples

Bad:

```cpp
bool SkPngRustCodec::canReadRow() {
    rust_png::ColorType color_type = fReader->output_color_type();
    uint8_t bits_per_component = fReader->output_bits_per_component();
    // Flawed fast path: ignores destination alpha type and color space
    switch (this->dstInfo().colorType()) {
        case kRGBA_8888_SkColorType:
            return color_type == rust_png::ColorType::Rgba && bits_per_component == 8;
        default:
            return false;
    }
}
```

Good:

```cpp
bool SkPngRustCodec::canReadRow() {
    // PNG data is unpremultiplied; direct decode is invalid for premultiplied dst.
    if (this->dstInfo().alphaType() != kUnpremul_SkAlphaType) {
        return false;
    }
    // Destination color space must match the encoded profile.
    if (!!this->getEncodedInfo().profile() != !!this->dstInfo().colorSpace()) {
        return false;
    }
    if (this->getEncodedInfo().profile() && this->dstInfo().colorSpace()) {
        skcms_ICCProfile dstProfile;
        this->dstInfo().colorSpace()->toProfile(&dstProfile);
        if (!skcms_ApproximatelyEqualProfiles(this->getEncodedInfo().profile(), &dstProfile)) {
            return false;
        }
    }
    return true;
}
```
