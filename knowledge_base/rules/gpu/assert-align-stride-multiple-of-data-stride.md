---
name: skia/gpu/assert-align-stride-multiple-of-data-stride
description: When accepting separate data and alignment strides for GPU buffer suballocations, assert that alignment stride is at least and an exact multiple of data stride
rubric: correctness
severity: should_fix
---

**Rule.** When accepting a separate data stride and alignment stride for buffer
suballocations or vertex/instance writers, assert that the alignment stride is
at least the data stride and an exact multiple of the data stride
(SkASSERT(alignStride >= dataStride && (alignStride % dataStride == 0))). Do not
permit an alignment stride smaller than the element stride or one that does not
partition into whole elements.

**Why.** If the alignment stride is smaller than or not an exact multiple of the
data stride, aligning an offset to the alignment stride can position
suballocations inside an element boundary or disrupt contiguity for subsequent
vertex appends. Asserting this invariant catches caller miscalculations at debug
time before they produce corrupted GPU draw calls.

## Examples

Bad:

```cpp
std::pair<VertexWriter, BindBufferInfo> DrawBufferManager::getVertexWriter(size_t count,
                                                                           size_t dataStride,
                                                                           size_t alignStride) {
    uint32_t requiredBytes = validate_count_and_stride(count, dataStride);
    if (!requiredBytes) {
        return {};
    }
    // ...
}
```

Good:

```cpp
std::pair<VertexWriter, BindBufferInfo> DrawBufferManager::getVertexWriter(size_t count,
                                                                           size_t dataStride,
                                                                           size_t alignStride) {
    SkASSERT(alignStride >= dataStride && (alignStride % dataStride == 0));
    uint32_t requiredBytes = validate_count_and_stride(count, dataStride);
    if (!requiredBytes) {
        return {};
    }
    // ...
}
```
