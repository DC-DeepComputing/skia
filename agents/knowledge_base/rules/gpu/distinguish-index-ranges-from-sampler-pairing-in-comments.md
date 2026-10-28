---
name: skia/gpu/distinguish-index-ranges-from-sampler-pairing-in-comments
description: When documenting GPU resource binding requirements, do not conflate independent texture and buffer index ranges with texture-sampler binding pairing
rubric: readability
severity: nit
---

**Rule.** When documenting backend resource binding requirements (such as
fComputeUsesDistinctIdxRangesForTextures in ResourceBindingRequirements), do not
explain separate texture and buffer index ranges in terms of texture-to-sampler
binding pairing. Document backend binding requirements using accurate shading
language semantics rather than conflating independent resource indexing
concepts.

**Why.** In shading languages such as MSL, resource index spaces are partitioned
per resource type (textures and buffers have independent index ranges starting
at zero), which is completely orthogonal to whether textures and samplers share
binding indices. Explaining index range separation as texture-sampler pairing is
factually incorrect and misleads developers about backend binding mechanics.

## Examples

Bad:

```cpp
// SkSL Metal codegen always assigns the same binding index to a texture and its sampler, so
// specify that compute shaders should act accordingly.
fResourceBindingReqs.fComputeUsesDistinctIdxRangesForTextures = true;
```

Good:

```cpp
// Metal uses distinct index ranges for textures vs buffers (e.g. texture(0) vs buffer(0)).
fResourceBindingReqs.fComputeUsesDistinctIdxRangesForTextures = true;
```
