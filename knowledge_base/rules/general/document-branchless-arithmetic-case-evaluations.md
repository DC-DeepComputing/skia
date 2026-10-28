---
name: skia/document-branchless-arithmetic-case-evaluations
description: When using branchless arithmetic or encoded parameter formulas to handle multiple modes without branching, document how discrete inputs evaluate to identity or active operations
rubric: readability
severity: nit
---

**Rule.** When using branchless arithmetic expressions or encoded parameter
values (such as `max`, `min`, `mix`, `clamp`, `saturate`, or additive flag
offsets) to conditionally apply operations, blend modes, or data formats without
branching, document the case breakdown in an inline comment. Explicitly explain
how each discrete mode, flag, or format maps to the parameters and evaluates
mathematically to either an identity/no-op or the active transformation.

**Why.** Branchless arithmetic tricks in shaders and raster pipelines simulate
control flow without explicit `if`/`else` branches, making edge cases, format
variations, and operational modes difficult to deduce at a glance. Documenting
the mathematical evaluation for each discrete state ensures maintainers can
verify correctness, understand identity fallbacks, and avoid introducing
regressions when modifying parameters.

## Examples

Bad:

```cpp
// Variant 1: Conditional opacity or premultiplication via max()
color.a = max(color.a, opaque);
color.rgb = color.rgb * max(color.a, 1.0 - doPremul);

// Variant 2: Folding format modes (YUVA vs YUV) via offset and saturate()
half alpha = saturate($sample_image(coords, invImgSizeY, sA).r + yuvToRGBXlateAlphaParam.w);
```

Good:

```cpp
// Variant 1: Document discrete input evaluations to identity or active mode
// Opaque cases:
//  1. No-op (opaque = 0) -> max(...) = color.a -> identity function
//  2. Make opaque (opaque = 1) -> max(...) = 1.0 -> make opaque
color.a = max(color.a, opaque);
// Premul cases:
//  1. No-op (doPremul = 0) -> max(...) = 1.0 -> identity function
//  2. Premultiply (doPremul = 1) -> max(...) = color.a -> premul
color.rgb = color.rgb * max(color.a, 1.0 - doPremul);

// Variant 2: Document parameter encodings for each data format
// When it's a Y_U_V_A texture, yuvToRGBXlateAlphaParam.w is 0 and we have a real A sampler so
// alpha is just that saturated value (should be a no-op). For Y_U_V, we set
// yuvToRGBXlateAlphaParam.w to 1 and sample from Y, which after the saturate is always 1.
half alpha = saturate($sample_image(coords, invImgSizeY, sA).r + yuvToRGBXlateAlphaParam.w);
```
