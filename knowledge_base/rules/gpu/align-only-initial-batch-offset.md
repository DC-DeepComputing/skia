---
name: skia/gpu/align-only-initial-batch-offset
description: When reserving buffer space for batched vertex or instance appends in GPU draw batchers, align only the initial batch offset rather than every reservation
rubric: correctness
severity: should_fix
---

**Rule.** When batching vertex or instance data into GPU buffers (such as in
DrawWriter), do not apply multi-stride alignment requirements to every
reservation or append call. Apply multi-stride alignment only to the initial
buffer offset when starting a new batch (e.g., when pendingCount == 0), keeping
consecutive appends contiguous within the draw.

**Why.** Aligning every reservation within an active batch inserts padding gaps
between consecutive append writes. Because batched draw calls render across
contiguous vertex ranges, gaps misalign vertex attribute offsets and cause
corrupted geometry to be drawn.

## Examples

Bad:

```cpp
// Bad: applying multi-stride alignment to every reservation creates gaps between batched appends
auto [writer, reservedChunk] = fDrawer.fManager->getVertexWriter(
        count, fStride, fAppendVerts ? fStride * 4 : fStride);
```

Good:

```cpp
// Good: only align to multi-stride at the start of a batch; internal appends stay contiguous
bool shouldAlign4 = fAppendVerts && !fDrawer.fPendingCount;
auto [writer, reservedChunk] = fDrawer.fManager->getVertexWriter(
        count, fStride, shouldAlign4 ? fStride * 4 : fStride);
```
