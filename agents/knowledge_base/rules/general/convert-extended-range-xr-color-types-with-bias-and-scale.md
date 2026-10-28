---
name: skia/convert-extended-range-xr-color-types-with-bias-and-scale
description: When converting or unpacking extended-range XR color types, apply linear bias and scale mapping instead of treating channels as standard unorm bit-shifts
rubric: correctness
severity: should_fix
---

**Rule.** When converting or unpacking pixel components from extended-range (XR)
color formats (such as kBGRA_10101010_XR_SkColorType or
kBGR_101010x_XR_SkColorType), do not group them with standard unorm formats or
use simple bit shifts. Extract components from their specific packed bit
positions (such as alpha at bit 54 in 64-bit words) and apply the extended-range
linear mapping: subtract the zero-point bias (384) and scale across the 510-step
nominal range (e.g., dividing by 510.0f for [0.0, 1.0] float output, or dividing
by 2 and clamping to [0, 255] for 8-bit integer destinations).

**Why.** Extended-range (XR) color formats represent values with an
offset-binary bias where 384 encodes 0.0 and 894 encodes 1.0 (values below 384
or above 894 represent extended/headroom color ranges). Grouping XR formats with
standard 10-bit or 16-bit unorm formats and using simple bit shifts treats raw
integer bits as linear unorm values, producing severely corrupted and
out-of-range color and alpha channels.

## Examples

Bad:

```cpp
case kRGBA_10x6_SkColorType:
case kBGRA_10101010_XR_SkColorType:
case kR16G16B16A16_unorm_SkColorType: {
    auto src64 = (const uint64_t*)src;
    for (int y = 0; y < srcInfo.height(); y++) {
        for (int x = 0; x < srcInfo.width(); x++) {
            // Bad: treating XR format as unorm and using simple bit shifts
            dst[x] = (src64[x] >> 48) >> 8;
        }
    }
    break;
}
```

Good:

```cpp
case kBGRA_10101010_XR_SkColorType: {
    auto src64 = (const uint64_t*)src;
    for (int y = 0; y < srcInfo.height(); y++) {
        for (int x = 0; x < srcInfo.width(); x++) {
            // Good: extract 10-bit alpha at bit 54, subtract 384 zero bias, and scale
            static constexpr int64_t kZero = 384;
            static constexpr int64_t kDivisor = 2;  // 510 / 255
            int64_t raw_alpha = src64[x] >> 54;
            dst[x] = static_cast<uint8_t>(SkTPin((raw_alpha - kZero) / kDivisor, 0, 255));
        }
    }
    break;
}
```
