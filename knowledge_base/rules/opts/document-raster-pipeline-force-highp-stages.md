---
name: skia/opts/document-raster-pipeline-force-highp-stages
description: When defining SkRasterPipeline stages that intentionally omit lowp counterparts to force high-precision execution, document this intent with an explanatory comment
rubric: readability
severity: should_fix
---

**Rule.** When defining an SkRasterPipeline stage that duplicates existing logic
but intentionally omits a lowp counterpart to force 32-bit float execution, add
an explanatory comment stating that the stage exists to force high precision.
Clarify the intent so future contributors do not inadvertently implement a lowp
variant or remove the stage as duplicate code.

**Why.** In SkRasterPipeline, a pipeline executes in highp if any of its
constituent stages lacks a lowp implementation. Omitting a lowp stage is a
deliberate mechanism to guarantee high-precision math for demanding shaders;
without an explicit comment, maintainers may see missing lowp stages or
duplicate highp stages as an oversight and add a lowp version, silently breaking
precision guarantees.

## Examples

Bad:

```cpp
// A specialized fused image shader for clamp-x, clamp-y, non-sRGB sampling.
HIGHP_STAGE(bilerp_clamp_8888_force_highp, const SkRasterPipelineContexts::GatherCtx* ctx) {
    bilerp_clamp_large(ctx, &r, &g, &b, &a);
}
```

Good:

```cpp
// This version exists to allow a shader to force high precision.
HIGHP_STAGE(bilerp_clamp_8888_force_highp, const SkRasterPipelineContexts::GatherCtx* ctx) {
    bilerp_clamp_large(ctx, &r, &g, &b, &a);
}
```
