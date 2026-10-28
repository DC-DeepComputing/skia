---
name: skia/gpu/name-trace-correlation-id-after-operation
description: When adding lifecycle or trace correlation identifiers for GPU resources, name them after the specific operation rather than generic object IDs
rubric: readability
severity: should_fix
---

**Rule.** Name event-tracking, stage-correlation, and trace identifiers after
the specific operation flow (such as `fCompilationID` or `compilationID`) rather
than generic object identifiers (such as `fID` or `pipelineID`). GPU `Resource`
objects already define intrinsic identifiers like `uniqueID()`, so disambiguate
operation-specific attempts and asynchronous phases by explicitly identifying
the operation being tracked.

**Why.** GPU resources already possess intrinsic identifiers (e.g.,
`Resource::uniqueID()`). Generic names like `fID` or `pipelineID` mislead
readers and trace consumers into thinking the value identifies the resource
object itself, rather than a specific attempt or phase (such as a compilation
attempt that may be retried or run concurrently across threads for the same
key).

## Examples

Bad:

```cpp
struct PipelineInfo {
    const uint32_t fHash = 0;
    const uint32_t fID = 0;
};
TRACE_EVENT_INSTANT2("skia.gpu", "CacheHit", TRACE_EVENT_SCOPE_THREAD,
                     "key", fHash, "pipelineID", fID);
```

Good:

```cpp
struct PipelineInfo {
    const uint32_t fUniqueKeyHash = 0;
    // The compilation ID distinguishes attempts/instantiations of the same key.
    const uint32_t fCompilationID = 0;
};
TRACE_EVENT_INSTANT2("skia.gpu", "CacheHit", TRACE_EVENT_SCOPE_THREAD,
                     "key", fUniqueKeyHash, "compilationID", fCompilationID);
```
