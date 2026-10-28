---
name: skia/gpu/document-hardware-workarounds-in-buffer-allocations
description: When implementing GPU buffer padding, alignment, or over-allocation workarounds for hardware or driver bugs, document the rationale and cite the tracking bug
rubric: readability
severity: nit
---

**Rule.** When adding non-obvious buffer alignment, padding, over-allocation, or
zeroing logic to work around hardware or driver bugs (such as padding vertex
data to prevent speculative execution faults), add an explanatory comment citing
the specific hardware issue and tracking bug number or URL. Clarify the
constraint that necessitates the extra allocation or zeroing so maintainers
understand why the logic cannot be simplified or removed.

**Why.** Unusual buffer sizing, stride alignment, or padding and zeroing logic
appears redundant or arbitrary without an explanatory comment. Citing the
tracking bug and hardware constraint prevents future maintainers from
inadvertently removing the workaround or refactoring it away as dead code.

## Examples

Bad:

```cpp
VertexWriter StaticBufferManager::getVertexWriter(size_t count,
                                                  size_t stride,
                                                  BindBufferInfo* binding) {
    const size_t alignedCount = SkAlign4(count);
    const size_t alignedSize = validate_count_and_stride(alignedCount, stride);
    void* data = this->prepareStaticData(&fVertexBufferInfo, alignedSize, stride * 4, binding);
    if (alignedCount > count) {
        const uint32_t byteDiff = (alignedCount - count) * stride;
        void* zPtr = SkTAddOffset<void>(data, count * stride);
        memset(zPtr, 0, byteDiff);
    }
    return VertexWriter{data, count * stride};
}
```

Good:

```cpp
// Workaround for ARM speculative execution hardware bug (b/399631317): static vertex data
// must be padded to a 4-aligned count and zeroed out to prevent out-of-bounds reads.
VertexWriter StaticBufferManager::getVertexWriter(size_t count,
                                                  size_t stride,
                                                  BindBufferInfo* binding) {
    const size_t alignedCount = SkAlign4(count);
    const size_t alignedSize = validate_count_and_stride(alignedCount, stride);
    void* data = this->prepareStaticData(&fVertexBufferInfo, alignedSize, stride * 4, binding);
    if (alignedCount > count) {
        const uint32_t byteDiff = (alignedCount - count) * stride;
        void* zPtr = SkTAddOffset<void>(data, count * stride);
        memset(zPtr, 0, byteDiff);
    }
    return VertexWriter{data, count * stride};
}
```
