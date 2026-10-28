---
name: skia/api/document-draw-type-flag-pipelines
description: When documenting Graphite draw type flags or RenderStep mappings, enumerate all rendering use cases and pipelines that depend on them
rubric: readability
severity: nit
---

**Rule.** When documenting Graphite draw type flags (such as in DrawTypeFlags or
InternalDrawTypeFlags) and their associated RenderSteps, enumerate all rendering
use cases and precompile pipelines that depend on them rather than describing
only a single pipeline or effect. Explicitly list each dependent feature—such as
blur filtering, raster path atlasing, and compute path atlasing for
CoverageMaskRenderStep.

**Why.** Incomplete doc comments for draw type flags misleadingly suggest that a
flag or RenderStep is dedicated to only one specific feature or paint effect.
Enumerating all dependent pipelines prevents developers and maintainers from
inadvertently breaking or overlooking other rendering paths (such as atlasing)
when modifying flag usage, precompilation logic, or render steps.

## Examples

Bad:

```cpp
// Corresponds to the CoverageMaskRenderStep. Used when creating Blur filter
// precompile pipelines.
kCoverageMask = DrawTypeFlags::kLast << 1,
```

Good:

```cpp
// Corresponds to the CoverageMaskRenderStep. Used when creating precompile Pipelines for
// blur-filtering and raster/compute path atlasing.
kCoverageMask = DrawTypeFlags::kLast << 1,
```
