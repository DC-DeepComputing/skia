---
name: skia/force-opaque-alpha-and-omit-unpremul-for-rgbx
description: When reading, converting, or packing '...x' pixel formats, force alpha to fully opaque, omit unpremultiplication, and do not write source alpha
rubric: correctness
severity: should_fix
---

**Rule.** When reading, unpacking, or converting pixel colors from RGBx or BGRx
formats (such as kRGB_888x_SkColorType, kRGB_101010x_SkColorType, or
kRGB_F16F16F16x_SkColorType), do not perform unpremultiplication, and explicitly
set the alpha channel to opaque (1.0f or 0xFF). When packing or converting
pixels into '...x' formats, force the unused alpha channel bits to fully opaque
(e.g., 0x3 << 30 for 101010x or 0xFF << 24 for 888x) rather than writing source
alpha. Never preserve or divide by uninitialized padding bits in memory.

**Why.** In '...x' formats, colors are semantically always opaque, so
unpremultiplying color channels is unnecessary and dividing by uninitialized
padding bits risks division-by-zero or NaN corruption. Conversely, packing
source alpha into '...x' formats violates the opaque invariant and can cause
rendering defects when downstream hardware or consumers expect opaque values.

## Examples

Bad:

```cpp
// Bad: Reading from RGBx formats unpremultiplies by uninitialized padding bits:
if (p4[3] && needsUnpremul) {
    p4 = p4 * skvx::float4(1.0f / p4[3], 1.0f / p4[3], 1.0f / p4[3], 1.0f);
}

// Bad: Packing into RGBx formats writes source alpha rather than forcing opaque:
case kRGB_101010x_SkColorType:
    return r | (g << 10) | (b << 20) | (a << 30);
```

Good:

```cpp
// Good: When reading, omit unpremultiplication and force alpha to 1.0f / 0xFF:
p4[3] = 1.0f;  // Explicitly force alpha opaque; ignore undefined padding bits.

// Good: When packing, force unused alpha channel bits to fully opaque:
case kRGB_101010x_SkColorType:
    return r | (g << 10) | (b << 20) | (0x3 << 30);
```
