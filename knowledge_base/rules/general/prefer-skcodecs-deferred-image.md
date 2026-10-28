---
name: skia/prefer-skcodecs-deferred-image
description: When creating an SkImage from an SkCodec or implementing image deserialization procs, use SkCodecs::DeferredImage rather than eager codec->getImage
rubric: readability
severity: should_fix
---

**Rule.** When creating an SkImage from an SkCodec or implementing image
deserialization callbacks in SkDeserialProcs (fImageDataProc or fImageProc), use
SkCodecs::DeferredImage(std::move(codec), alphaType) instead of eagerly decoding
via codec->getImage(...). Pass the optional SkAlphaType directly to
DeferredImage rather than manually modifying SkImageInfo to handle alpha-type
fallback.

**Why.** Eagerly decoding via codec->getImage() forces immediate raster pixel
allocation during deserialization, bypassing Skia's lazy picture drawing
pipeline and deferred decoding caches. SkCodecs::DeferredImage preserves lazy
deferred decoding semantics and automatically handles format-appropriate
alpha-type defaults (defaulting transparent images to kPremul_SkAlphaType),
eliminating repetitive SkImageInfo manipulation.

## Examples

Bad:

```cpp
SkImageInfo info = codec->getInfo();
if (alphaType.has_value()) {
    info = info.makeAlphaType(*alphaType);
} else if (kUnpremul_SkAlphaType == info.alphaType()) {
    info = info.makeAlphaType(kPremul_SkAlphaType);
}
return std::get<0>(codec->getImage(info));
```

Good:

```cpp
return SkCodecs::DeferredImage(std::move(codec), alphaType);
```
