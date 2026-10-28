---
name: skia/sksl/hoist-channel-independent-shader-calculations
description: When computing multi-channel shader values using per-component helpers, hoist channel-independent coordinate math and texture fetches out of repeated invocations
rubric: safety
severity: should_fix
---

**Rule.** In GPU shader code (such as SkSL), hoist channel-independent
coordinate math, lattice calculations, and texture lookups out of per-component
helper functions. Execute invariant operations once and pass the precomputed
coordinates or samples into per-channel evaluation routines rather than
redundantly recomputing them across multiple channel invocations.

**Why.** GPU drivers and mobile shader compilers cannot be relied upon to
perform common subexpression elimination across texture sampling instructions or
function calls. Hoisting invariant coordinate math and texture fetches out of
per-channel loops or repeated component evaluations eliminates redundant texture
bandwidth and ALU cycles across millions of rasterized fragments.

## Examples

Bad:

```cpp
// Redundantly computing coordinates and sampling permutation textures per channel:
half4 tmp = half4(
    noise_function(kChanCoordR, noiseVec, stitchData, stitching, permutationSampler, noiseSampler),
    noise_function(kChanCoordG, noiseVec, stitchData, stitching, permutationSampler, noiseSampler),
    noise_function(kChanCoordB, noiseVec, stitchData, stitching, permutationSampler, noiseSampler),
    noise_function(kChanCoordA, noiseVec, stitchData, stitching, permutationSampler, noiseSampler));
```

Good:

```cpp
// Hoisting channel-independent coordinates and texture fetches outside per-channel calls:
half4 noiseXCoords = $noise_helper(noiseVec, stitchData, stitching, permutationSampler);
half4 tmp = $noise_function(noiseVec, noiseXCoords, noiseSampler);
```
