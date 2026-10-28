---
name: skia/gpu/encapsulate-resource-caching-in-provider
description: When providing cached GPU resources via ResourceProvider, encapsulate creation and caching in a unified findOrCreate method instead of separate find and cache APIs
rubric: design
severity: should_fix
---

**Rule.** Do not split resource caching into separate lookup and cache/insert
methods on ResourceProvider implementations. Encapsulate cache key computation,
resource compilation, and cache insertion within a unified findOrCreate* method
that accepts high-level descriptors.

**Why.** Exposing separate find and cache operations leaks internal cache keys,
key hashing, and compilation lifecycles to callers, forcing clients to
orchestrate multi-step caching logic. Consolidating creation within findOrCreate
keeps caching and resource lifecycle encapsulated within the provider and
simplifies call sites.

## Examples

Bad:

```cpp
// DawnResourceProvider.h
wgpu::RenderPipeline findBlitWithDrawPipeline(uint32_t key);
void cacheBlitWithDrawPipeline(uint32_t key, wgpu::RenderPipeline pipeline);

// In caller:
auto pipeline = resourceProvider->findBlitWithDrawPipeline(key);
if (!pipeline) {
    pipeline = create_blit_render_pipeline(...);
    resourceProvider->cacheBlitWithDrawPipeline(key, pipeline);
}
```

Good:

```cpp
// DawnResourceProvider.h
BlitWithDrawEncoder findOrCreateBlitWithDrawEncoder(const RenderPassDesc& renderPassDesc,
                                                    SampleCount srcSampleCount);

// In caller:
DawnResourceProvider::BlitWithDrawEncoder blit =
        fResourceProvider->findOrCreateBlitWithDrawEncoder(renderPassDesc, srcSampleCount);
```
