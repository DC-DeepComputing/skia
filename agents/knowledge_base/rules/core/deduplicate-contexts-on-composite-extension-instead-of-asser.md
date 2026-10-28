---
name: skia/core/deduplicate-contexts-on-composite-extension-instead-of-asser
description: When implementing extension or concatenation methods for composite structures or pipelines, deduplicate or merge incoming resource contexts instead of asserting their absence
rubric: correctness
severity: should_fix
---

**Rule.** When implementing concatenation or extension methods (such as extend()
or append()) for composite structures, pipelines, or command builders, do not
assert that incoming resource contexts or buffers are absent from the
destination container. Account for valid edge cases such as self-extension or
shared resource buffers by delegating to registration helpers that merge access
states and deduplicate entries.

**Why.** Asserting exclusivity or container absence fails during valid
operations such as extending a pipeline with itself or chaining operations that
access the same buffer. Delegating to idempotent registration helpers that merge
usage flags and deduplicate references preserves correct container state without
triggering false-positive debug assertions.

## Examples

Bad:

```cpp
for (const SkRasterPipeline_MemoryCtxPatch& patch : src.fMemoryCtxPatches) {
    // Asserts exclusivity, which fails on self-extension or shared contexts
    SkASSERT(std::none_of(fMemoryCtxPatches.begin(), fMemoryCtxPatches.end(),
                          [=](const SkRasterPipeline_MemoryCtxPatch& p) {
                              return p.context == patch.context;
                          }));
    fMemoryCtxPatches.push_back(patch);
}
```

Good:

```cpp
for (const SkRasterPipeline_MemoryCtxPatch& patch : src.fMemoryCtxPatches) {
    // Delegates to helper that merges access flags and deduplicates contexts
    this->addMemoryContext(patch.context, patch.bytesPerPixel, patch.load, patch.store);
}
```
