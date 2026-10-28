---
name: skia/gpu/sksl-use-float-precision-for-coordinates-and-bounds
description: When defining SkSL parameters, local variables, or uniforms for coordinates, bounds, or sampling, use float types rather than half precision
rubric: safety
severity: should_fix
---

**Rule.** When defining parameters, local variables, or uniform descriptors for
device-space coordinates, bounding rectangles, clip bounds, or radii in SkSL
functions and GPU backends, use 32-bit float types (float, float2, float4, or
SkSLType::kFloat*) rather than half precision (half, half2, half4, or
SkSLType::kHalf*). In particular, do not downcast float coordinates to half or
half2 for intermediate arithmetic before texture sampling intrinsics (such as
sample()); perform coordinate arithmetic directly in float to avoid round-trip
conversion overhead and precision loss.

**Why.** Half precision (FP16 / mediump) has only 10 mantissa bits, losing
fractional subpixel precision for values above 1024 and coarsening to
multi-pixel steps beyond 2048, which degrades antialiasing on large render
targets. Furthermore, texture sampling intrinsics like sample() take float
coordinates; downcasting coordinates to half for intermediate calculations
introduces round-trip conversion overhead and precision loss without
computational gain.

## Examples

Bad:

```cpp
// Downcasting float coordinates to half for intermediate math before sample():
half2 lutCoords = half2(coords.x / kImgSize, coords.y / kImgSize);
half value = sample(lut, lutCoords).r - 0.5;

// In SkSL or Graphite shader snippets:
$pure half4 sk_circular_rrect_clip(half4 color,
                                   half4 rect,
                                   half2 radiusPlusHalf,
                                   half4 edgeSelect) { ... }

// In uniform dictionary definitions:
{"rect", SkSLType::kHalf4},
{"radius", SkSLType::kHalf2},
```

Good:

```cpp
// Performing coordinate math directly in float:
half value = sample(lut, coords / kImgSize).r - 0.5;

// In SkSL or Graphite shader snippets:
$pure half4 sk_circular_rrect_clip(half4 color,
                                   float4 rect,
                                   float2 radiusPlusHalf,
                                   half4 edgeSelect) { ... }

// In uniform dictionary definitions:
{"rect", SkSLType::kFloat4},
{"radius", SkSLType::kFloat2},
```
