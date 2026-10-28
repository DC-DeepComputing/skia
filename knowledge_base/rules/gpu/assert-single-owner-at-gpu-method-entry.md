---
name: skia/gpu/assert-single-owner-at-gpu-method-entry
description: When defining or adding public methods to single-threaded GPU classes guarded by SingleOwner, invoke ASSERT_SINGLE_OWNER at method entry to catch data races
rubric: safety
severity: should_fix
---

**Rule.** In single-threaded GPU classes that enforce thread confinement with
SingleOwner (such as Graphite's Context, Recorder, PrecompileContext, and
Ganesh's GrDirectContext or SurfaceContext), invoke ASSERT_SINGLE_OWNER (or
ASSERT_SINGLE_OWNER_PRIV) at the entry of public and client-facing methods. Do
not execute context operations or delegate to shared helpers without first
validating single-thread ownership.

**Why.** GPU context and recording classes are not thread-safe and rely on
single-threaded execution. Invoking ASSERT_SINGLE_OWNER upon method entry
ensures concurrent calls, data races, and re-entrant usage from multiple threads
fail fast in debug builds rather than causing silent state corruption.

## Examples

Bad:

```cpp
void Context::syncPipelineData(size_t maxSize) {
    if (fPersistentPipelineStorage) {
        fSharedContext->syncPipelineData(fPersistentPipelineStorage, maxSize);
    }
}
```

Good:

```cpp
void Context::syncPipelineData(size_t maxSize) {
    ASSERT_SINGLE_OWNER

    if (fPersistentPipelineStorage) {
        fSharedContext->syncPipelineData(fPersistentPipelineStorage, maxSize);
    }
}
```
