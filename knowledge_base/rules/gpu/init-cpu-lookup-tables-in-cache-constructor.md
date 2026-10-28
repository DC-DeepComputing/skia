---
name: skia/gpu/init-cpu-lookup-tables-in-cache-constructor
description: When initializing pure CPU lookup tables or immutable caches in Graphite, construct them directly in the owning cache constructor rather than Context::finishInitialization()
rubric: design
severity: should_fix
---

**Rule.** Initialize static or immutable CPU lookup tables directly in the
owning cache's constructor (such as GlobalCache) rather than deferring setup to
two-phase lifecycle methods like Context::finishInitialization(). Reserve
finishInitialization() exclusively for GPU resources that require a
ResourceProvider, Caps, or command submission.

**Why.** Pure CPU data structures have no dependency on GPU devices or command
queues. Initializing them directly at construction eliminates unnecessary
coupling between Context lifecycle and cache internals, avoids multi-phase
initialization, and guarantees thread-safe immutability without requiring
runtime locks or checks.

## Examples

Bad:

```cpp
// In Context::finishInitialization():
#if defined(GPU_TEST_UTILS) && defined(SK_ENABLE_SPARSE_STRIPS)
    auto pathRendererStrategy = fSharedContext->caps()->requestedPathRendererStrategy();
    if (pathRendererStrategy.has_value() &&
        pathRendererStrategy.value() == PathRendererStrategy::kCPUSparseStripsMSAA8) {
        if (!fSharedContext->globalCache()->initializeMSAA8MaskLUT()) {
            return false;
        }
    }
#endif
```

Good:

```cpp
// In GlobalCache.cpp constructor:
GlobalCache::GlobalCache()
        : fGraphicsPipelineCache(kGlobalGraphicsPipelineCacheSizeLimit, &fStats)
        , fComputePipelineCache(kGlobalComputePipelineCacheSizeLimit)
        , fDynamicSamplers({})
#if defined(SK_ENABLE_SPARSE_STRIPS)
        , fMSAAMaskLUT(GenerateMSAALUT<uint8_t>())
#endif
        {}
```
