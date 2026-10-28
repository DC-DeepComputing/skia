---
name: skia/decode-16bit-floats-with-skhalftofloat
description: When converting 16-bit color formats, decode float types with SkHalfToFloat/skvx::from_half and normalize unorm integer types by dividing by 65535.0f
rubric: correctness
severity: should_fix
---

**Rule.** When reading, unpacking, or converting 16-bit color types, distinguish
between floating-point and unsigned normalized integer formats. For 16-bit float
formats (e.g., kR16_float_SkColorType, kRGBA_F16_SkColorType), decode components
using SkHalfToFloat() or skvx::from_half (and SkFloatToHalf() / skvx::to_half
when packing) rather than dividing or multiplying by 65535.0f. Conversely, for
16-bit unorm integer formats (e.g., kR16_unorm_SkColorType,
kA16_unorm_SkColorType), normalize integer pixel values to [0.0, 1.0] by
dividing by 65535.0f instead of decoding them with SkHalfToFloat().

**Why.** 16-bit float formats store IEEE 754 half-precision values (SkHalf) with
sign, exponent, and mantissa bits, whereas 16-bit unorm formats store
fixed-point integers scaled to [0.0, 1.0]. Decoding unorm bits as half-floats
interprets integer bits as exponents and mantissas, while dividing half-float
bits by 65535.0f treats floating-point bit layouts as linear integers,
corrupting channel values in both directions.

## Examples

Bad:

```cpp
// Bad: dividing 16-bit float bits by 65535.0f
case kR16_float_SkColorType:
    return SkColor4f{*this->addr16(x, y) / 65535.0f, 0.0f, 0.0f, 1.0f};

// Bad: decoding 16-bit unorm integer as half-float
case kR16_unorm_SkColorType:
    return SkColor4f{SkHalfToFloat(*this->addr16(x, y)), 0.0f, 0.0f, 1.0f};
```

Good:

```cpp
// Good: decode 16-bit float with SkHalfToFloat
case kR16_float_SkColorType:
    return SkColor4f{SkHalfToFloat(*this->addr16(x, y)), 0.0f, 0.0f, 1.0f};

// Good: normalize 16-bit unorm by dividing by 65535.0f
case kR16_unorm_SkColorType:
    return SkColor4f{*this->addr16(x, y) / 65535.0f, 0.0f, 0.0f, 1.0f};
```
