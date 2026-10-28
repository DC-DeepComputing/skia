---
name: skia/gpu/document-aligned-count-recalculation-on-overflow
description: When an allocation overflows a dynamic buffer and triggers a flush, document why aligned count calculations are reset to omit pending elements
rubric: readability
severity: nit
---

**Rule.** When an allocation request in dynamic buffer management or draw
batchers overflows the active buffer and triggers a flush, document why aligned
count calculations are reset. Clarify in an inline comment that prior alignment
accounted for pending elements in the old buffer that are about to be flushed,
which no longer apply when allocating from the new buffer.

**Why.** In batched draw buffers, initial alignment padding depends on
uncommitted pending elements in the current buffer chunk. If the reservation
triggers a buffer rollover, pending draws are flushed, making the original
alignment calculation invalid; precise comments prevent maintainers from
misunderstanding why the reservation count is recalculated.

## Examples

Bad:

```cpp
if (fDrawer.fManager->willVertexOverflow(alignedCount, fStride, ...)) {
    fDrawer.padAndZero();
    // When we overflow the buffer, the previous alignedCount may be insufficient.
    alignedCount = SkAlign4(count);
}
```

Good:

```cpp
if (fDrawer.fManager->willVertexOverflow(alignedCount, fStride, ...)) {
    fDrawer.padAndZero();
    // When we overflow the buffer, the previous alignedCount will be incorrect, since
    // it accounted for the about-to-be-flushed fPendingCount, which will not apply to
    // writing to the new buffer.
    alignedCount = SkAlign4(count);
}
```
