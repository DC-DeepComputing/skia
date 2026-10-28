---
name: skia/gpu/document-shared-post-processing-after-divergent-paths
description: When a shared GPU post-processing step immediately follows divergent capability-dependent execution paths, add an inline comment clarifying that the operation covers all preceding branches
rubric: readability
severity: nit
---

**Rule.** When a shared GPU post-processing or finalization step (such as
resolving query sets, copying query buffers, or resolving passes) immediately
follows divergent capability-dependent execution branches, add an inline comment
clarifying that the step handles outputs from all preceding code paths. Do not
leave the converging call directly beneath a conditional branch without
clarifying that it applies to the entire sequence rather than just the adjacent
branch.

**Why.** When divergent execution branches converge into a shared finalization
call without an explanatory comment, readers can easily misinterpret the shared
step as belonging solely to the immediately preceding conditional branch.
Clarifying comment annotations prevent incorrect assumptions about control flow
and ensure maintainers understand that prior alternative paths are finalized
there as well.

## Examples

Bad:

```cpp
    if (fSharedContext->dawnCaps()->supportsCommandBufferTimestamps()) {
        fCommandEncoder.WriteTimestamp(fTimestampQuerySet, 1);
    } else if (!fWroteFirstPassTimestamps) {
        // If we didn't have any passes then we can't report anything.
        fTimestampQueryBuffer = nullptr;
        fTimestampQueryXferBuffer = nullptr;
        return;
    }
    fCommandEncoder.ResolveQuerySet(
            fTimestampQuerySet, 0, 2, fTimestampQueryBuffer->dawnBuffer(), 0);
```

Good:

```cpp
    if (fSharedContext->dawnCaps()->supportsCommandBufferTimestamps()) {
        fCommandEncoder.WriteTimestamp(fTimestampQuerySet, 1);
    } else if (!fWroteFirstPassTimestamps) {
        // If we didn't have any passes then we can't report anything.
        fTimestampQueryBuffer = nullptr;
        fTimestampQueryXferBuffer = nullptr;
        return;
    }
    // This resolve covers both timestamp code paths (command encoder and render/compute pass).
    fCommandEncoder.ResolveQuerySet(
            fTimestampQuerySet, 0, 2, fTimestampQueryBuffer->dawnBuffer(), 0);
```
