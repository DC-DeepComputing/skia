---
name: skia/codec/keep-conversionsupported-pure-for-reusability
description: When implementing SkCodec::conversionSupported or decode configuration, keep validation pure without mutating internal codec settings for partial configurations to preserve codec reusability
rubric: correctness
severity: should_fix
---

**Rule.** When implementing SkCodec::conversionSupported or configuring codec
decode state, do not mutate internal codec settings (such as setSrcXformFormat
or decoder pixel format options) for only a subset of supported configurations
without handling or resetting all cases. If color transformations or format
adaptations are handled downstream or via SkCodec's transformation pipeline,
treat conversionSupported strictly as a non-mutating validation check to ensure
the codec instance remains reusable across subsequent decodes with different
destination configurations.

**Why.** SkCodec instances are designed to be reused across multiple getPixels
calls with different destination SkImageInfo parameters. Mutating internal codec
state during validation for only specific color types leaves the codec in a
stale or corrupted state for subsequent decodes requesting other formats.

## Examples

Bad:

```cpp
bool SkAvifCodec::conversionSupported(const SkImageInfo& dstInfo,
                                      bool srcIsOpaque,
                                      bool needsColorXform) {
    switch (dstInfo.colorType()) {
        case kRGBA_1010102_SkColorType:
            // MUTATING codec state during validation breaks reusability for other formats
            this->setSrcXformFormat(skcms_PixelFormat_RGBA_1010102);
            break;
        default:
            break;
    }
    return true;
}
```

Good:

```cpp
bool SkCrabbyAvifCodec::conversionSupported(const SkImageInfo& dstInfo,
                                            bool srcIsOpaque,
                                            bool needsColorXform) {
    // Pure validation query without mutating internal codec state
    return dstInfo.colorType() == kRGBA_8888_SkColorType ||
           dstInfo.colorType() == kRGBA_1010102_SkColorType ||
           dstInfo.colorType() == kRGBA_F16_SkColorType;
}
```
