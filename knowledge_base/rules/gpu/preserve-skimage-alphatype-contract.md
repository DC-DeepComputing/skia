---
name: skia/gpu/preserve-skimage-alphatype-contract
description: When implementing backend SkImage representations, preserve the public alphaType contract (e.g. kPremul for YUVA images) rather than altering it to match intermediate shader outputs
rubric: correctness
severity: should_fix
---

**Rule.** When defining or constructing backend SkImage instances (such as
planar or YUVA images), do not alter the public logical alphaType() (e.g.,
exposing kUnpremul_SkAlphaType instead of kPremul_SkAlphaType) to match
intermediate shader stage outputs. Maintain the standard public SkImageInfo
contract across backends, and reconcile internal intermediate shader
transformations locally within backend pipeline and key generation.

**Why.** Altering an SkImage's logical alphaType changes its public SkImageInfo
contract, breaking client expectations and deviating from Ganesh backend
behavior. It also subtly changes SkImage::makeRawShader() semantics, where
kUnpremul images are not automatically premultiplied. Intermediate shader stage
representations must be reconciled during pipeline key generation rather than
leaking into public image metadata.

## Examples

Bad:

```cpp
static SkAlphaType yuva_alpha_type(const SkYUVAInfo& yuvaInfo) {
    // If an alpha channel is present we always use kUnpremul. This is because the final
    // interleaved RGBA sample produced in the shader is unpremul (and similar if flattened).
    return yuvaInfo.hasAlpha() ? kUnpremul_SkAlphaType : kOpaque_SkAlphaType;
}
```

Good:

```cpp
static SkAlphaType yuva_alpha_type(const SkYUVAInfo& yuvaInfo) {
    // Retain kPremul even though intermediate shader sampling produces unpremul RGBA.
    return yuvaInfo.hasAlpha() ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
}

// In pipeline key generation, reconcile internal shader stage format locally:
SkAlphaType srcAT = imageToDraw->alphaType() == kPremul_SkAlphaType
                            ? kUnpremul_SkAlphaType
                            : imageToDraw->alphaType();
```
