---
name: skia/gpu/avoid-preemptive-buffer-reservation-zeroing
description: When reserving GPU buffer memory for immediately appended vertex or instance data, do not preemptively zero reservations; defer padding to flushes or state transitions
rubric: safety
severity: should_fix
---

**Rule.** Do not preemptively zero or clear memory during buffer reservation
calls when data will be appended immediately afterwards. Defer tail padding and
alignment zeroing until buffer exhaustion, pipeline state changes, or buffer
flushes.

**Why.** Preemptively zeroing buffer reservations forces memory to be written
twice when subsequent operations append vertex or instance attributes, degrading
CPU memory bandwidth. Deferring tail padding ensures unwritten alignment bytes
are cleared lazily only when a buffer is finalized or flushed.

## Examples

Bad:

```cpp
if (fReservedCount >= alignedCount) {
    if (fAppendVerts && countDiff && fNextWriter) {
        fNextWriter.zeroBytes(countDiff * fStride);
    }
    return;
}
```

Good:

```cpp
if (fReservedCount >= alignedCount) {
    return;
}
// Tail padding and alignment bytes are zeroed lazily upon flush or pipeline state switch:
// zWriter.zeroBytes(countDiff * fAppendStride);
```
