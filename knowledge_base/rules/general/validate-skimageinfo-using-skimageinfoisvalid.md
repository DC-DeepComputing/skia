---
name: skia/validate-skimageinfo-using-skimageinfoisvalid
description: When validating SkImageInfo parameters, use SkImageInfoIsValid() instead of manual width and height checks to catch dimension overflow and invalid color info
rubric: correctness
severity: should_fix
---

**Rule.** When validating an SkImageInfo (or SkColorInfo) before allocating
memory, creating images or surfaces, or reading/writing pixels, use
SkImageInfoIsValid(info) (or SkColorInfoIsValid) instead of manual non-positive
dimension checks like width() <= 0 || height() <= 0. Verify that dimensions do
not exceed SK_MaxS32 >> 2 to prevent arithmetic overflow in row byte and buffer
calculations, and verify that colorType and alphaType are not kUnknown_.

**Why.** Manual dimension checks like width() <= 0 || height() <= 0 miss invalid
color and alpha types (kUnknown_SkColorType, kUnknown_SkAlphaType) and do not
guard against dimensions exceeding SK_MaxS32 >> 2. Excessive dimensions can
cause 32-bit arithmetic overflow during row byte calculations or memory
allocation, leading to buffer under-allocation or memory corruption.
SkImageInfoIsValid comprehensively checks positive bounds, the overflow ceiling,
and valid color info.

## Examples

Bad:

```cpp
if (newInfo.width() <= 0 || newInfo.height() <= 0) {
    return nullptr;
}
```

Good:

```cpp
if (!SkImageInfoIsValid(newInfo)) {
    return nullptr;
}
```
