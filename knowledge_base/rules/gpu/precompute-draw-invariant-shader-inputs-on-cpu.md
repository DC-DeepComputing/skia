---
name: skia/gpu/precompute-draw-invariant-shader-inputs-on-cpu
description: When parameterizing GPU shader functions or uniforms, precompute draw-invariant expressions on the CPU rather than computing derived values inside shaders
rubric: safety
severity: should_fix
---

**Rule.** Precompute draw-invariant shader inputs on the CPU host rather than
passing multiple parameters or uniforms to compute a derived value inside a
shader function. If shader inputs are only used to evaluate a derived constant
or geometry threshold, compute the derived value on the host and pass it
directly to the shader.

**Why.** Evaluating draw-invariant arithmetic expressions in fragment shaders
forces the GPU to execute redundant ALU operations on every fragment. Deriving
values on the CPU conserves uniform buffer storage and parameter registers while
eliminating per-pixel shading overhead.

## Examples

Bad:

```cpp
// In SkSL:
$pure half4 $rrect_blur_coverage_fn(float2 coords,
                                    float4 proxyRect,
                                    half cornerRadius,
                                    half blurRadius,
                                    sampler2D ninePatch) {
    ...
    half edgeSize = 2.0 * blurRadius + cornerRadius + 0.5;
```

Good:

```cpp
// On CPU (uniform / parameter setup):
const float edgeSize = 2.0f * blurRadius + SkRRectPriv::GetSimpleRadii(devRRect).fX + 0.5f;

// In SkSL:
$pure half4 $rrect_blur_coverage_fn(float2 coords,
                                    float4 proxyRect,
                                    half edgeSize,
                                    sampler2D ninePatch) {
```
