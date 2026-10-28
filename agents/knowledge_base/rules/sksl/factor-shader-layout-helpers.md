---
name: skia/sksl/factor-shader-layout-helpers
description: When computing coordinates or layout across multi-variant SkSL shaders, factor coordinate mapping into dedicated layout helpers rather than duplicating math inline across shader entry points
rubric: design
severity: should_fix
---

**Rule.** In SkSL shader modules, retain and call dedicated layout helper
functions (such as $radial_grad_layout or $linear_grad_layout) to map
coordinates rather than duplicating coordinate-transformation math inline across
shader entry points. Preserve distinct layout, tiling, and colorization stages
so coordinate mapping remains consistent across stop and data variants.

**Why.** SkSL gradient and pattern shaders are structured around modular stages
(layout -> tiling -> colorization). Inlining coordinate mapping duplicates logic
across 4-stop, 8-stop, texture, and buffer shader entry points, increasing
boilerplate and risking divergence when layout math is updated.

## Examples

Bad:

```cpp
$pure half4 sk_radial_grad_4_shader(float2 coords, ...) {
    // Inlining layout math duplicates coordinate logic across all stop variants
    float2 t = float2(distance(coords, float2(0)), 1);
    t = $tile_grad(tileMode, t);
    return $colorize_grad_4(colorsParam, offsetsParam, t);
}
```

Good:

```cpp
$pure float2 $radial_grad_layout(float2 pos) {
    return float2(distance(pos, float2(0)), 1);
}

$pure half4 sk_radial_grad_4_shader(float2 coords, ...) {
    // Dedicated layout helper centralizes coordinate mapping across all variants
    float2 t = $radial_grad_layout(coords);
    t = $tile_grad(tileMode, t);
    return $colorize_grad_4(colorsParam, offsetsParam, t);
}
```
