---
name: skia/gpu/precompute-matrix-inverses-on-cpu
description: When preparing GPU shader uniforms or instance data, precompute invariant operations like matrix inversions, signs, offsets, or reciprocal dimensions on the CPU
rubric: safety
severity: should_fix
---

**Rule.** When passing parameters to GPU shaders (such as vertex or fragment
SkSL), precompute coordinate-independent or uniform-only expressions—such as
matrix inversions (e.g., device-to-local transforms), signs, constant offsets,
scale factors, or reciprocals (e.g., inverse dimensions like invImgSize)—on the
CPU during uniform or instance setup. Upload normalized or precomputed values
directly so shaders use fast vector multiplication rather than evaluating
invariant arithmetic, math intrinsics like sign(), offsets, inverse() calls, or
per-pixel divisions repeatedly per vertex or fragment.

**Why.** Evaluating invariant arithmetic, math intrinsics, matrix inversions, or
per-pixel divisions inside vertex or fragment shaders executes redundantly
across thousands of vertices or millions of rasterized pixels. Computing
invariant expressions and reciprocals once on the CPU before uploading instance
data or uniforms saves GPU ALU cycles, replaces expensive fragment divisions
with multiplications, and reduces register pressure.

## Examples

Bad:

```cpp
// In vertex shaders, evaluating matrix inversion per vertex:
float3 localCoords = inverse(localToDevice) * drawCoords;

// In fragment shaders, computing invariant uniform math or signs per pixel:
float scaleSign = sign(dRadius);
float dist = length(vec) + (0.5 - solidRadius) * invTextureRadius;

// Normalizing coordinates by dividing by raw dimensions per pixel:
half4 color = sample(s, pos / imgSize);
```

Good:

```cpp
// In CPU RenderStep, precompute matrix inverse and upload directly:
SkMatrix deviceToLocal;
(void)params.transform().matrix().asM33().invert(&deviceToLocal);
instances.append(1) << deviceToLocal;
float3 localCoords = deviceToLocal * drawCoords;

// In CPU uniform setup, precalculate invariant signs, offsets, and reciprocals:
// CPU: dRadius = gradData.fRadii[0] > 0 ? 1 : -1;
//      circle.w = (solidRadius - 0.5f) * invTextureRadius;
//      invImgSize = {1.0f / imgWidth, 1.0f / imgHeight};
float t = length(pos) * dRadius - radius0;
float dist = length(vec) - normSolidRadius;
half4 color = sample(s, pos * invImgSize);
```
