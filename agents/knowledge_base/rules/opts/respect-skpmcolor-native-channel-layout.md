---
name: skia/opts/respect-skpmcolor-native-channel-layout
description: When implementing SIMD blitter or pixel routines for SkPMColor, unpack and interleave components according to native byte order (BGRA on little-endian) rather than assuming RGBA
rubric: correctness
severity: should_fix
---

**Rule.** When implementing SIMD blitters, swizzlers, or pixel packing routines
for SkPMColor, unpack, process, and repack color components according to the
platform's native SkPMColor channel layout (BGRA memory order on little-endian
architectures). Do not assume device pixels are RGBA and attempt to compensate
with ad-hoc component swaps or inverted vector lane interleaving.

**Why.** On little-endian architectures, SkPMColor stores 32-bit premultiplied
pixels in BGRA byte order (SK_B32_SHIFT is 0, SK_G32_SHIFT is 8, SK_R32_SHIFT is
16, SK_A32_SHIFT is 24). Assuming RGBA causes red and blue channels to be
swapped in memory; compensating by swapping channels in premultiplied constants
masks the discrepancy and leads to confusing, error-prone vector interleaving.

## Examples

Bad:

```cpp
// Assuming device pixels are RGBA and swapping PMC channels to compensate:
vpmc_b = __lsx_vreplgr2vr_h(SkGetPackedR32(pmc));
vpmc_r = __lsx_vreplgr2vr_h(SkGetPackedB32(pmc));
// ...
lo = __lsx_vilvl_h(ba, rg); // rgbargba rgbargba
```

Good:

```cpp
// Correctly treating device pixels as native BGRA in memory:
vpmc_b = __lsx_vreplgr2vr_h(SkGetPackedB32(pmc));
vpmc_r = __lsx_vreplgr2vr_h(SkGetPackedR32(pmc));
// ...
lo = __lsx_vilvl_h(ra, bg); // bgra bgra bgra bgra
```
