---
name: skia/gpu/avoid-command-buffer-tracking-for-queue-write-buffer
description: When updating GPU buffers via WebGPU Queue::WriteBuffer, do not force new buffer allocations across command buffer boundaries
rubric: safety
severity: should_fix
---

**Rule.** Do not track command buffer boundaries or force new GPU buffer
allocations before calling wgpu::Queue::WriteBuffer(). In WebGPU, queue writes
are serialized in queue order and handle resource hazards internally, requiring
only that the destination buffer is unmapped and active.

**Why.** WebGPU and Dawn manage queue synchronization and execution hazards
automatically for Queue::WriteBuffer() updates on unmapped buffers. Allocating a
new GPU buffer solely because the active command buffer changed creates
unnecessary memory allocations and fragments buffer capacity.

## Examples

Bad:

```cpp
// Anti-pattern: forcing buffer reallocation when the active command buffer changes
if (!fCurrentBuffer || fCurrentBuffer->slotsUsed() == kNumSlots ||
    cb != fCurrentCommandBuffer) {
    fCurrentBuffer = createNewBuffer();
}
queue.WriteBuffer(fCurrentBuffer->dawnBuffer(), offset, data, size);
```

Good:

```cpp
// Idiomatic: rely on WebGPU queue order to safely update unmapped buffers across command buffers
if (!fCurrentBuffer || fCurrentBuffer->slotsUsed() == kNumSlots) {
    fCurrentBuffer = createNewBuffer();
}
queue.WriteBuffer(fCurrentBuffer->dawnBuffer(), offset, data, size);
```
