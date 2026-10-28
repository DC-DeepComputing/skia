---
name: skia/api/keep-internal-draw-type-flags-private
description: When adding precompilation support for internal RenderStep variants or paint-effect strategies in Graphite, define them in InternalDrawTypeFlags rather than public DrawTypeFlags
rubric: design
severity: should_fix
---

**Rule.** Do not expose internal RenderStep variants or paint-effect execution
strategies as public DrawTypeFlags in Graphite; keep them in
InternalDrawTypeFlags and drive their precompilation through PaintOptions.
Reserve public DrawTypeFlags strictly for high-level geometry and user-facing
draw types invoked directly by callers.

**Why.** Public DrawTypeFlags define client-facing draw commands, whereas how
those commands are executed (such as rendering blurs analytically versus through
coverage masks) is an internal backend pipeline detail. Exposing internal
execution strategies in public enums leaks private RenderStep implementations
and prevents the backend from evolving its pipeline selection logic without
breaking public API compatibility.

## Examples

Bad:

```cpp
// include/gpu/graphite/GraphiteTypes.h
enum DrawTypeFlags : uint16_t {
    kNone         = 0,
    kSimpleShape  = 1 << 0,
    // Bad: Exposing internal RenderStep strategy in public DrawTypeFlags
    kAnalyticBlur = 1 << 6,
    kLast         = kAnalyticBlur,
};
```

Good:

```cpp
// src/gpu/graphite/InternalDrawTypeFlags.h
enum InternalDrawTypeFlags : uint16_t {
    // Good: Internal RenderStep variant isolated in InternalDrawTypeFlags;
    // precompilation is driven via PaintOptions mask filter choke points.
    kAnalyticBlur = DrawTypeFlags::kLast << 1,
    kCoverageMask = DrawTypeFlags::kLast << 2,
    kLastInternal = kCoverageMask,
};
```
