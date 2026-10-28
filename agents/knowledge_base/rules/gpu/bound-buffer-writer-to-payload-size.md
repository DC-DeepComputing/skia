---
name: skia/gpu/bound-buffer-writer-to-payload-size
description: When returning a bounded VertexWriter or BufferWriter for allocations with trailing padding, initialize the writer with the requested payload size rather than padded allocation size
rubric: safety
severity: should_fix
---

**Rule.** When returning a bounded buffer writer (such as VertexWriter or
BufferWriter) for an allocation that includes trailing alignment or sentinel
padding, initialize the writer with the requested logical payload size (e.g.,
count * stride) rather than the aligned buffer size. Keep zeroed padding or
alignment slack outside the caller's writable bounds.

**Why.** Allocating extra buffer space for hardware workarounds (such as zeroed
trailing bytes to prevent speculative vertex execution crashes) or alignment
must not expand the caller's writable range. Initializing the writer with the
requested payload size prevents callers from inadvertently overwriting zeroed
padding bytes and ensures debug bounds checks catch buffer overruns.

## Examples

Bad:

```cpp
const size_t alignedCount = SkAlign4(count);
const size_t alignedSize = validate_count_and_stride(alignedCount, stride);
void* data = this->prepareStaticData(&fVertexBufferInfo, alignedSize, stride * 4, binding);
if (alignedCount > count) {
    const uint32_t byteDiff = (alignedCount - count) * stride;
    void* zPtr = SkTAddOffset<void>(data, count * stride);
    memset(zPtr, 0, byteDiff);
}
return VertexWriter{data, alignedSize};
```

Good:

```cpp
const size_t size = count * stride;
const size_t alignedCount = SkAlign4(count);
const size_t alignedSize = validate_count_and_stride(alignedCount, stride);
void* data = this->prepareStaticData(&fVertexBufferInfo, alignedSize, stride * 4, binding);
if (alignedCount > count) {
    const uint32_t byteDiff = (alignedCount - count) * stride;
    void* zPtr = SkTAddOffset<void>(data, count * stride);
    memset(zPtr, 0, byteDiff);
}
return VertexWriter{data, size};
```
