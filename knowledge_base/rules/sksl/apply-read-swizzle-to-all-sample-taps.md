---
name: skia/sksl/apply-read-swizzle-to-all-sample-taps
description: When implementing multi-tap texture sampling or manual shader filtering, apply required format transforms or read swizzles to every sample tap before blending
rubric: correctness
severity: should_fix
---

**Rule.** When implementing multi-tap texture sampling, boundary fallbacks, or
manual shader filtering (such as bilinear filtering across clamp boundaries or
bicubic interpolation), ensure every sample tap undergoes required format
transforms or read swizzles before the samples are blended or interpolated. Do
not interpolate raw texture samples with transformed samples or omit swizzling
on diagonal and corner taps.

**Why.** Omitting a format swizzle or channel transformation on any tap in a
multi-tap sampling sequence causes incompatible channel formats to be mixed
together, corrupting color or alpha channels in interpolated boundary texels.

## Examples

Bad:

```cpp
if (sampleExtraX && sampleExtraY) {
    half4 extraColorXY = sample(s, float2(extraCoordX, extraCoordY) * invImgSize);
    color = mix(mix(color, extraColorX, absError.x),
                mix(extraColorY, extraColorXY, absError.x),
                absError.y);
}
```

Good:

```cpp
if (sampleExtraX && sampleExtraY) {
    half4 extraColorXY = sample(s, float2(extraCoordX, extraCoordY) * invImgSize);
    extraColorXY = $apply_swizzle(readSwizzle, extraColorXY);
    color = mix(mix(color, extraColorX, absError.x),
                mix(extraColorY, extraColorXY, absError.x),
                absError.y);
}
```
