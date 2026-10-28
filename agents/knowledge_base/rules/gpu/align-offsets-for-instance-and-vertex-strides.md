---
name: skia/gpu/align-offsets-for-instance-and-vertex-strides
description: When calculating buffer base offsets or alignment across pipeline transitions in draw batchers, select the active stride based on instanced drawing mode rather than assuming vertex stride
rubric: correctness
severity: should_fix
---

**Rule.** When computing buffer alignment or base offsets across pipeline state
changes in draw batchers (such as DrawWriter), account for both instance and
vertex strides based on whether instanced drawing is active. Do not assume
vertex stride alone or leave instance stride unhandled when preserving
contiguous buffer offsets across pipeline switches.

**Why.** Draw batchers multiplex both vertex and instance streams into shared
buffers. Assuming vertex stride causes instanced pipeline switches to drop
pending base offsets or compute misaligned offsets, leading to redundant buffer
rebinding or corrupted attribute rendering.

## Examples

Bad:

```cpp
if (!vertexStride || !fVertexStride) {
    fPendingBase = 0;
} else {
    const size_t currentBytes = fPendingBase * fVertexStride;
    fPendingBase = SkTo<uint32_t>((currentBytes + vertexStride - 1) / vertexStride);
}
```

Good:

```cpp
const size_t prevStride = fInstanceStride ? fInstanceStride : fVertexStride;
const size_t currStride = instanceStride ? instanceStride : vertexStride;
if (prevStride && currStride) {
    const size_t prevBytes = fPendingBase * prevStride;
    fPendingBase = SkTo<uint32_t>((prevBytes + currStride - 1) / currStride);
} else {
    fPendingBase = 0;
}
```
