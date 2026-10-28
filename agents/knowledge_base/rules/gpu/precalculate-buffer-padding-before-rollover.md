---
name: skia/gpu/precalculate-buffer-padding-before-rollover
description: When dynamic buffer allocations require alignment padding across rollovers, check for overflow and finalize the active buffer before requesting a new writer
rubric: correctness
severity: should_fix
---

**Rule.** When dynamic buffer allocations can overflow and transition to a new
underlying buffer, check whether the allocation will overflow and pad or
finalize the active buffer before requesting a writer to the new buffer. Never
attempt to pad or zero bytes using a newly returned writer or query the
allocator for padding after rollover, as the writer points into the new buffer
and leaves the previous buffer's trailing alignment unpadded.

**Why.** When a buffer manager dynamically allocates a new underlying GPU buffer
upon rollover, any newly returned writer points into the beginning of the new
buffer. Calculating padding or zeroing bytes after obtaining the new writer
writes into the new buffer at an offset instead of padding the old buffer,
leaving trailing data in the prior buffer unfinalized and corrupting alignments
or offsets in the new buffer.

## Examples

Bad:

```cpp
// Querying for padding or using the new writer after allocating a new chunk:
auto [writer, reservedChunk] = fDrawer.fManager->getVertexWriter(alignedCount, fStride);
if (writer && fAppendVerts && countDiff) {
    // BUG: writer points into the new buffer, leaving the old buffer unpadded!
    writer.zeroBytes(countDiff * fStride, count * fStride);
}
```

Good:

```cpp
// Check if the allocation will overflow and pad/finalize the current buffer first:
if (fAppendVerts && fDrawer.fManager->willVertexOverflow(alignedCount, fStride)) {
    fDrawer.padAndZero();
    alignedCount = SkAlign4(count);
}
auto [writer, reservedChunk] = fDrawer.fManager->getVertexWriter(alignedCount, fStride);
```
