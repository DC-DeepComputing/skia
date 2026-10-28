---
name: skia/prefer-skmaskbuilder-over-skmask-const-cast
description: When defining or calling functions that populate or mutate mask pixel buffers or bounds, accept SkMaskBuilder rather than casting away const on SkMask
rubric: readability
severity: should_fix
---

**Rule.** When defining or calling functions that populate or mutate mask pixel
buffers, dimensions, or formats, accept SkMaskBuilder& (or SkMaskBuilder*)
rather than const SkMask& or SkMask&. Access mutable buffers and bounds through
SkMaskBuilder methods (image(), bounds(), getAddr8()) instead of casting away
const on SkMask::fImage.

**Why.** SkMask enforces an immutable view with 'const uint8_t* const fImage',
ensuring safe sharing across raster and text pipelines. Using const_cast on
SkMask circumvents const-correctness and obscures write intent, whereas
SkMaskBuilder is the explicit derived builder type designed for allocating and
modifying mask buffers.

## Examples

Bad:

```cpp
static void applyLUTToA8Mask(const SkMask& mask, const uint8_t* lut) {
    uint8_t* SK_RESTRICT dst = const_cast<uint8_t*>(mask.fImage);
    // ...
}
```

Good:

```cpp
static void applyLUTToA8Mask(SkMaskBuilder& mask, const uint8_t* lut) {
    uint8_t* SK_RESTRICT dst = mask.image();
    // ...
}
```
