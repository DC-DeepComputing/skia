---
name: skia/core/support-anisotropic-blur-in-separable-algorithms
description: When selecting or implementing separable SkBlurEngine algorithms, do not restrict algorithm dispatch to symmetric sigmas when independent 1D passes support anisotropic blurs
rubric: correctness
severity: should_fix
---

**Rule.** When implementing or selecting separable blur algorithms (such as in
SkBlurEngine::findAlgorithm), do not require symmetric blur radii (sigma.width()
== sigma.height()). Allow anisotropic blurs to use the fast separable passes
unless the underlying implementation specifically requires isotropic inputs.

**Why.** Gaussian and box blurs naturally separate into independent horizontal
and vertical 1D passes that handle unequal X and Y sigmas. Artificially
requiring equal sigmas forces anisotropic blurs to unnecessarily fall back to
slower general-purpose shader blurs or pipeline fallbacks.

## Examples

Bad:

```cpp
// Bad: Restricting separable algorithm selection to symmetric sigmas
const bool a8Blur = colorType == kAlpha_8_SkColorType;
const bool sigmasAreEqual = sigma.width() == sigma.height();

if (a8Blur && sigmasAreEqual) {
    return &fA8BlurAlgorithm;
}
return &fShaderBlurAlgorithm;
```

Good:

```cpp
// Good: Allow separable algorithms to handle anisotropic sigmas
const bool a8Blur = colorType == kAlpha_8_SkColorType;

if (a8Blur) {
    return &fA8BlurAlgorithm;
}
return &fShaderBlurAlgorithm;
```
