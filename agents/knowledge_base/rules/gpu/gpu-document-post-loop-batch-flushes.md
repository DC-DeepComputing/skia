---
name: skia/gpu/gpu-document-post-loop-batch-flushes
description: When finalizing batch-processing loops with a trailing flush call, retain or add explanatory comments clarifying that pending batched data must be emitted
rubric: readability
severity: nit
---

**Rule.** When finalizing batch-processing loops in Graphite (such as DrawWriter
iteration over draws or render steps), retain or add inline comments above
trailing flush calls (e.g., drawWriter.flush()). Clarify that the call is
necessary to emit any remaining pending data accumulated before iteration ended,
and do not remove existing comments during refactoring.

**Why.** In dynamic batching loops where flushes occur conditionally upon state
changes or buffer limits, trailing flush calls can appear redundant to
maintainers unfamiliar with batch lifecycles. Retaining explanatory comments
prevents post-loop flushes from being mistakenly deleted or reordered during
refactoring.

## Examples

Bad:

```cpp
    for (const auto& draw : fDraws) {
        ...
    }
    drawWriter.flush();
```

Good:

```cpp
    for (const auto& draw : fDraws) {
        ...
    }
    // Finish recording draw calls for any collected data still pending at end of the loop.
    drawWriter.flush();
```
