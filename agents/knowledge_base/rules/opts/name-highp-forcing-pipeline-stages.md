---
name: skia/opts/name-highp-forcing-pipeline-stages
description: When defining specialized raster pipeline stages that omit low-precision implementations to force high-precision execution, suffix them with _force_highp rather than caller-specific conditions
rubric: readability
severity: should_fix
---

**Rule.** When introducing or naming specialized raster pipeline stages that
intentionally omit a low-precision (lowp) counterpart to prevent lowp execution,
use an explicit `_force_highp` suffix rather than caller-specific domain
conditions (such as `_large`). Name stages after their mechanical effect on
pipeline execution rather than specific caller geometry or coordinate
thresholds.

**Why.** SkRasterPipeline executes in lowp (16-bit fixed point) by default and
falls back to highp (32-bit float) only when a pipeline stage lacks a lowp
implementation. Naming a stage after a caller's triggering condition (such as
`_large`) obscures why the stage exists and leaks caller-specific heuristics
into the general pipeline stage table, whereas `_force_highp` clearly documents
its operational effect.

## Examples

Bad:

```cpp
HIGHP_STAGE(bilerp_clamp_8888_large, const SkRasterPipelineContexts::GatherCtx* ctx) {
    bilerp_clamp_large(ctx, &r, &g, &b, &a);
}
```

Good:

```cpp
HIGHP_STAGE(bilerp_clamp_8888_force_highp, const SkRasterPipelineContexts::GatherCtx* ctx) {
    bilerp_clamp_large(ctx, &r, &g, &b, &a);
}
```
