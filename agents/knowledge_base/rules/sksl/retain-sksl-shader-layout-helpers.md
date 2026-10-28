---
name: skia/sksl/retain-sksl-shader-layout-helpers
description: When simplifying coordinate math across SkSL shader variants, retain shared layout helper functions rather than duplicating inlined expressions into entry points
rubric: design
severity: should_fix
---

**Rule.** In modular SkSL shader pipelines (such as gradient shaders),
encapsulate coordinate layout and parameter mapping calculations in shared
layout helper functions (such as $linear_grad_layout or $radial_grad_layout)
across shader variants. When simplifying coordinate transformations or folding
parameters, update the shared layout helper's signature and implementation
rather than inlining duplicated coordinate arithmetic directly into individual
shader entry points.

**Why.** Inlining coordinate math across multiple shader variants (such as
4-stop, 8-stop, and texture shaders) duplicates layout calculations, risks
inconsistent coordinate handling, and leaves dead or stale helper declarations.
Retaining shared layout helpers preserves a consistent layout-tile-colorize
pipeline and centralizes coordinate transformations in a single place.

## Examples

Bad:

```cpp
// Inlining simplified layout math directly into individual shader variants
$pure half4 sk_linear_grad_4_shader(float2 coords, ...) {
    float2 t = float2(coords.x, 1);
    t = $tile_grad(tileMode, t);
    return $colorize_grad_4(...);
}

$pure half4 sk_linear_grad_8_shader(float2 coords, ...) {
    float2 t = float2(coords.x, 1);
    t = $tile_grad(tileMode, t);
    return $colorize_grad_8(...);
}
```

Good:

```cpp
// Encapsulating layout math in the shared helper function across variants
$pure float2 $linear_grad_layout(float2 pos) {
    return float2(pos.x, 1);
}

$pure half4 sk_linear_grad_4_shader(float2 coords, ...) {
    float2 t = $linear_grad_layout(coords);
    t = $tile_grad(tileMode, t);
    return $colorize_grad_4(...);
}
```
