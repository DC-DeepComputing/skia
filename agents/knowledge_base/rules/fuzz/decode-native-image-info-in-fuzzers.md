---
name: skia/fuzz/decode-native-image-info-in-fuzzers
description: In image decoder fuzzers, decode into the native codec->getInfo() format rather than overriding the destination color type via makeColorType
rubric: testing
severity: nit
---

**Rule.** In image decoder fuzzers, decode directly into the native SkImageInfo
returned by codec->getInfo() instead of overriding the destination color type
(such as with makeColorType(kN32_SkColorType)). Pass the native info directly to
bitmap allocation and codec decode operations (like tryAllocPixels and
getPixels).

**Why.** Overriding the destination color type forces color conversion and
swizzling inside the codec, bypassing native pixel decoding paths and preventing
the fuzzer from exercising format-specific rasterization logic for the image's
native color type.

## Examples

Bad:

```cpp
SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
SkBitmap bitmap;
if (!bitmap.tryAllocPixels(info)) {
    return false;
}
(void)codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes());
```

Good:

```cpp
SkImageInfo info = codec->getInfo();
SkBitmap bitmap;
if (!bitmap.tryAllocPixels(info)) {
    return false;
}
(void)codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes());
```
