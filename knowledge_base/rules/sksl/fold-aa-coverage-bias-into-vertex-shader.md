---
name: skia/sksl/fold-aa-coverage-bias-into-vertex-shader
description: When computing analytic anti-aliasing edge distances with constant coverage ramps in shaders, fold coverage bias into vertex edge distances to eliminate varyings
rubric: safety
severity: should_fix
---

**Rule.** When evaluating analytic anti-aliasing edge distance coverage with
constant coverage ramps (scale = 1.0, bias = 0.5) in GPU shaders, fold the 0.5
coverage bias into the vertex shader edge distances (scaled by abs(devPos.z) for
perspective) rather than passing a coverage scale varying and calculating the
bias per fragment. In the fragment shader, evaluate saturate(c) directly on the
interpolated edge distances.

**Why.** In geometry with constant coverage slopes, pre-biasing edge distances
in the vertex stage accounts for perspective interpolation and eliminates an
inter-stage varying interpolator. This saves hardware varying bandwidth and
avoids redundant per-pixel scale, bias, and ALU operations in the fragment
shader.

## Examples

Bad:

```cpp
// Vertex shader:
edgeDistances += (1.0 - edgeAA) * abs(devPos.z);
coverageScale = abs(devPos.z);

// Fragment shader:
float c = min(outerDist.x, outerDist.y) * coords.w;
float scale = coverageScale * coords.w;
float bias = coverage_bias(scale);
return half4(saturate(scale * (c + bias)));
```

Good:

```cpp
// Vertex shader:
// Pre-bias by 0.5 (0.5 for AA, 1.5 for non-AA) scaled by perspective W:
edgeDistances += (1.5 - edgeAA) * abs(devPos.z);

// Fragment shader:
float c = min(outerDist.x, outerDist.y) * coords.w;
return half4(saturate(c));
```
