---
name: skia/sksl/use-matching-plane-sampler-in-multiplane-shaders
description: In multi-plane image shaders with per-plane samplers or dimensions, verify each sampling branch passes dedicated plane variables rather than copy-pasted parameters
rubric: correctness
severity: blocking
---

**Rule.** In multi-plane image shaders accepting separate per-plane parameters
(such as samplers sY/sU/sV/sA, image sizes imgSizeY/imgSizeU/invImgSizeUV, or
channel selectors in YUVA shaders), verify that each plane's sampling call and
fallback branch consistently passes variables corresponding to the target plane.
Audit symmetric or copy-pasted sampling branches across planes to ensure no
parameters inadvertently retain identifiers from another plane like Y.

**Why.** In multi-plane shaders, sampling calls across planes share nearly
identical signatures and control flow (such as branching between cubic and
standard sampling). Copy-pasting sampling expressions easily leaves mismatched
samplers, dimensions, or channel selectors (such as passing sY or imgSizeY when
sampling chroma or alpha), resulting in corrupt reconstructed color channels or
sampling out-of-bounds plane data.

## Examples

Bad:

```cpp
// Retaining sY sampler when sampling alpha plane
half4 sampleColorA = $cubic_filter_image(coords, invImgSizeY, subset, tileModeX, tileModeY,
                                         cubicCoeffs, sY);

// Retaining imgSizeY dimension in fallback branch when sampling plane U
half4 sampleColorU = (useCubic != 0)
    ? $cubic_filter_image(coords, imgSizeU, subset, tileModeX, tileModeY, cubicCoeffs, swiz, sU)
    : $sample_image(coords, imgSizeY, subset, tileModeX, tileModeY, filterMode, swiz, sU);
```

Good:

```cpp
// Pass dedicated alpha plane sampler sA
half4 sampleColorA = $cubic_filter_image(coords, invImgSizeY, subset, tileModeX, tileModeY,
                                         cubicCoeffs, sA);

// Consistently pass plane U dimension imgSizeU across all branches
half4 sampleColorU = (useCubic != 0)
    ? $cubic_filter_image(coords, imgSizeU, subset, tileModeX, tileModeY, cubicCoeffs, swiz, sU)
    : $sample_image(coords, imgSizeU, subset, tileModeX, tileModeY, filterMode, swiz, sU);
```
