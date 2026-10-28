---
name: skia/gpu/omit-subpixel-coverage-varyings-for-quads
description: In GPU render steps for quads and per-edge AA geometry, fold constant coverage bias into vertex edge distances and omit subpixel scale varyings
rubric: safety
severity: should_fix
---

**Rule.** In GPU render steps and SkSL shaders for quads and per-edge AA
geometry, do not pass varying state to compute dynamic subpixel coverage scale
and bias. Assume a constant scale of 1.0, fold the 0.5 bias directly into edge
distances within the vertex shader, and omit coverage scale varyings and
fragment ALU adjustments.

**Why.** Unlike filled rectangles that expand and adjust coverage for subpixel
dimensions, arbitrary quads and per-edge AA geometry tile together where
subpixel expansion would exceed 0.5px outsets and disrupt seams or damage
tracking. Because coverage scale is invariant at 1.0 and bias is fixed at 0.5
for quads, passing coverage scale as an interpolated varying wastes interpolator
slots and fragment shader arithmetic.

## Examples

Bad:

```cpp
// Vertex shader
out float coverageScale;
coverageScale = abs(devPos.z);

// Fragment shader: passing varying and calculating dynamic bias
$pure half4 per_edge_aa_quad_coverage_fn(float4 coords, float4 edgeDistances, float coverageScale) {
    float2 outerDist = min(edgeDistances.xy, edgeDistances.zw);
    float c = min(outerDist.x, outerDist.y) * coords.w;
    float scale = coverageScale * coords.w;
    float bias = coverage_bias(scale);
    return half4(saturate(scale * (c + bias)));
}
```

Good:

```cpp
// Vertex shader: fold 0.5 bias directly into edgeDistances
edgeDistances += (1.5 - edgeAA) * abs(devPos.z);

// Fragment shader: no coverageScale varying or dynamic bias needed
$pure half4 per_edge_aa_quad_coverage_fn(float4 coords, float4 edgeDistances) {
    float2 outerDist = min(edgeDistances.xy, edgeDistances.zw);
    float c = min(outerDist.x, outerDist.y) * coords.w;
    return half4(saturate(c));
}
```
