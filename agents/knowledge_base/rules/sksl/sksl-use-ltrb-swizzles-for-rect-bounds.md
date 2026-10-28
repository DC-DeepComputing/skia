---
name: skia/sksl/sksl-use-ltrb-swizzles-for-rect-bounds
description: When accessing vector coordinates representing rectangle bounds in SkSL shaders, use native LTRB swizzles (.LT, .RB, .LTRB) instead of generic .xyzw components
rubric: readability
severity: should_fix
---

**Rule.** In SkSL shader files and GPU shader generators, use native LTRB
swizzle components (.LT, .RB, .LTRB, .L, .T, .R, .B) rather than generic
Cartesian (.xy, .zw, .xyzw) or color components when operating on vectors
representing rectangle bounds (such as float4 or half4 rects storing left, top,
right, and bottom coordinates).

**Why.** SkSL natively supports L, T, R, and B swizzle components for vectors
storing rectangle bounds. Using .LT and .RB instead of .xy and .zw documents
geometric intent and distinguishes rectangular boundary coordinates from
arbitrary spatial vectors or texture coordinates.

## Examples

Bad:

```cpp
// Bad: Using generic Cartesian swizzles on rectangle bounds vectors
half2 pos = max(half2(rect.xy - coords), half2(coords - rect.zw));
float2 clampedCoord = clamp(texCoords, maskBounds.xy, maskBounds.zw);
```

Good:

```cpp
// Good: Using SkSL LTRB swizzles clarifies left/top and right/bottom bounds
half2 pos = max(half2(rect.LT - coords), half2(coords - rect.RB));
float2 clampedCoord = clamp(texCoords, maskBounds.LT, maskBounds.RB);
```
