---
name: skia/use-standard-luminance-coefficients-for-grayscale
description: When converting RGB colors to grayscale or luminance, use standard BT.709 coefficients or SkComputeLuminance rather than an unweighted channel average
rubric: correctness
severity: should_fix
---

**Rule.** When converting RGB colors to grayscale or computing luminance, use
standard ITU-R BT.709 / sRGB luminance coefficients (0.2126 R + 0.7152 G +
0.0722 B, SK_LUM_COEFF_*, or SkComputeLuminance) rather than an unweighted
average of color channels. Weight float components by 0.2126f, 0.7152f, and
0.0722f, and use SkComputeLuminance for 8-bit integer channel calculations.

**Why.** Human visual perception is non-uniform across color channels, being
significantly more sensitive to green than red or blue. Computing grayscale as
an unweighted average ((r + g + b) / 3) distorts perceived brightness and causes
inconsistent rendering with Skia's shaders, blitters, and raster pipeline.

## Examples

Bad:

```cpp
static float compute_gray(SkColor4f c) {
    // WRONG: Flat unweighted average distorts perceived brightness.
    return (c.fR + c.fG + c.fB) * (1.f / 3.0f);
}

uint8_t to_gray(uint8_t r, uint8_t g, uint8_t b) {
    return (r + g + b) / 3;
}
```

Good:

```cpp
static float compute_gray(SkColor4f c) {
    // Correct: Standard ITU-R BT.709 / sRGB luminance weights.
    return c.fR * 0.2126f + c.fG * 0.7152f + c.fB * 0.0722f;
}

uint8_t to_gray(uint8_t r, uint8_t g, uint8_t b) {
    return SkComputeLuminance(r, g, b);
}
```
