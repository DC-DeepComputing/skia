---
name: skia/gpu/query-path-renderer-strategy-from-provider
description: When checking the active PathRendererStrategy in Graphite, query RendererProvider::pathRendererStrategy() or internal fStrategy rather than Caps
rubric: correctness
severity: blocking
---

**Rule.** When querying or branching on the active PathRendererStrategy in
Graphite, access it through RendererProvider::pathRendererStrategy() (or
fStrategy within RendererProvider). Do not attempt to query the active strategy
from Caps, which only exposes the optional requestedPathRendererStrategy()
override.

**Why.** In Graphite, the active PathRendererStrategy is determined, validated
against hardware capabilities, and owned by RendererProvider. Caps only holds an
optional user-requested override (requestedPathRendererStrategy()) and lacks an
active strategy accessor.

## Examples

Bad:

```cpp
#ifdef SK_ENABLE_VELLO_SHADERS
    // Don't initialize Vello if the strategy wouldn't use it.
    const PathRendererStrategy strategy = caps->pathRendererStrategy();
    if (strategy == PathRendererStrategy::kComputeAnalyticAA ||
        strategy == PathRendererStrategy::kComputeMSAA16 ||
        strategy == PathRendererStrategy::kComputeMSAA8) {
        fVelloRenderer = std::make_unique<VelloRenderer>(caps);
    }
#endif
```

Good:

```cpp
#ifdef SK_ENABLE_VELLO_SHADERS
    // Don't initialize Vello if the strategy wouldn't use it.
    if (fStrategy == PathRendererStrategy::kComputeAnalyticAA ||
        fStrategy == PathRendererStrategy::kComputeMSAA16 ||
        fStrategy == PathRendererStrategy::kComputeMSAA8) {
        fVelloRenderer = std::make_unique<VelloRenderer>(caps);
    }
#endif
```
