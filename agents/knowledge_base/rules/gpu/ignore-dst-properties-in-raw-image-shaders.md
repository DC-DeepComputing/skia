---
name: skia/gpu/ignore-dst-properties-in-raw-image-shaders
description: When sampling raw image shaders (isRaw()), do not condition alpha or color transformations on destination properties; convert only to the image's own alphaType
rubric: correctness
severity: should_fix
---

**Rule.** When generating shader pipeline keys or sampling logic for raw image
shaders (such as via isRaw()), never condition color or alpha transformations on
destination properties like dstColorInfo.alphaType(). Raw shaders must ignore
destination color management; any internal alpha reconciliation must target the
image's own declared alphaType, never the destination surface.

**Why.** Raw image shaders (created via SkImage::makeRawShader) are intended for
non-color data (such as normals or material properties) and guarantee that no
color space or destination premultiplication transformation is applied.
Factoring destination surface properties into raw shader pipelines violates raw
shader semantics and corrupts raw data channels.

## Examples

Bad:

```cpp
if (origShader->isRaw()) {
    if (imageToDraw->alphaType() == kUnpremul_SkAlphaType &&
        keyContext.dstColorInfo().alphaType() != kUnpremul_SkAlphaType) {
        steps = SkColorSpaceXformSteps(imageToDraw->colorSpace(),
                                       imageToDraw->alphaType(),
                                       imageToDraw->colorSpace(),
                                       kPremul_SkAlphaType);
    }
}
```

Good:

```cpp
SkAlphaType srcAT = imageToDraw->alphaType() == kPremul_SkAlphaType
                            ? kUnpremul_SkAlphaType
                            : imageToDraw->alphaType();
if (origShader->isRaw()) {
    steps = SkColorSpaceXformSteps(imageToDraw->colorSpace(),
                                   srcAT,
                                   imageToDraw->colorSpace(),
                                   imageToDraw->alphaType());
}
```
