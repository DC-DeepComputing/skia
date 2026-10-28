---
name: skia/prefer-skimageinfo-modifier-helpers
description: When deriving a new SkImageInfo with modified dimensions, color, or alpha properties, use modifier methods like makeWH or makeDimensions instead of SkImageInfo::Make
rubric: readability
severity: nit
---

**Rule.** When creating a modified SkImageInfo derived from an existing
instance, use modifier helper methods (such as makeWH(), makeDimensions(),
makeColorType(), makeAlphaType(), or makeColorSpace()) instead of calling
SkImageInfo::Make() with individual getters. Retain the unmodified properties
through the instance method rather than passing colorType(), alphaType(), or
refColorSpace() manually.

**Why.** SkImageInfo modifier methods preserve unchanged dimensions, color type,
alpha type, and color space automatically. Calling SkImageInfo::Make() with
manual getter calls is redundant and verbose, and risks accidentally passing
mismatched or incomplete color properties when updating size or format.

## Examples

Bad:

```cpp
sk_sp<Surface> surface =
        Surface::MakeScratch(recorder,
                             SkImageInfo::Make(dstBounds.width(),
                                               dstBounds.height(),
                                               fImageInfo.colorType(),
                                               fImageInfo.alphaType(),
                                               fImageInfo.refColorSpace()),
                             "DstCopyFallback");
```

Good:

```cpp
sk_sp<Surface> surface =
        Surface::MakeScratch(recorder,
                             fImageInfo.makeWH(dstBounds.width(),
                                               dstBounds.height()),
                             "DstCopyFallback");
```
