---
name: skia/gpu/scope-ssbo-stride-alignment-assertions
description: In dual-mode GPU paths handling storage and uniform buffers, do not assert stride divisibility on common offsets; scope alignment assertions to the storage-buffer branch
rubric: correctness
severity: blocking
---

**Rule.** In dual-mode GPU buffer paths supporting both storage buffers (SSBOs)
and uniform buffers, do not assert that buffer offsets are multiples of uniform
struct size in shared or common code paths. Scope element stride divisibility
assertions (such as SkASSERT(offset % dataSize == 0)) and offset-to-index
conversions strictly inside the storage buffer conditional branch.

**Why.** Uniform buffer allocations align to hardware-mandated binding
requirements (such as 256 bytes) rather than struct size, so uniform buffer
offsets are rarely exact multiples of struct size. In contrast, storage buffers
are stride-aligned to allow indexing by array element; asserting divisibility
outside the storage buffer branch triggers false-positive assertion failures
when uniform buffers are active.

## Examples

Bad:

```cpp
SkASSERT(uniformData.fGpuData.fOffset % uniformDataSize == 0);
writer.write(uniformData.fCpuData.data(), uniformDataSize);

if (useStorageBuffers) {
    uniformData.fGpuData.fOffset /= uniformDataSize;
    uniformData.fGpuData.fSize = uniformData.fGpuData.fBuffer->size();
}
```

Good:

```cpp
writer.write(uniformData.fCpuData.data(), uniformDataSize);

if (useStorageBuffers) {
    SkASSERT(uniformData.fGpuData.fOffset % uniformDataSize == 0);
    uniformData.fGpuData.fOffset /= uniformDataSize;
    uniformData.fGpuData.fSize = uniformData.fGpuData.fBuffer->size();
}
```
